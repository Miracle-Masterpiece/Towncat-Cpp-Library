//#################################################### ./src/allocator.cpp ####################################################

#include <allocators/allocator.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <utility>

namespace tca
{    
    allocator::allocator() : base_allocator() {

    }

    allocator::allocator(base_allocator* parent) : base_allocator(parent) {

    }

    allocator::allocator(allocator&& a) : base_allocator(std::move(a)) {
        
    }

    allocator& allocator::operator= (allocator&& a) {
        if (&a != this) {
            base_allocator::operator=(std::move(a));
        }
        return *this;
    }

    void allocator::deallocate(void* p, std::size_t) {
        deallocate(p);
    }

    allocator::~allocator() {

    }
}


namespace tca
{
    
namespace internal 
{
    thread_local allocator* scoped_allocator;
}
    
    static malloc_free_allocator malloc_allocator;
    
    allocator* default_alloc = &malloc_allocator;

    allocator* get_default_allocator() {
        return default_alloc;
    }

    void set_default_allocator(allocator* allocator) {
        default_alloc = 
                        (allocator != nullptr) ? allocator : &malloc_allocator;
    }

    allocator* get_scoped_or_default() {
        return internal::scoped_allocator ? internal::scoped_allocator : get_default_allocator();
    }

    allocator* get_exception_allocator() {
        return get_default_allocator();
    }
}

namespace tca
{
    scope_allocator::scope_allocator(allocator* allocator) : m_prev(internal::scoped_allocator) {
        internal::scoped_allocator = allocator;
    }
    
    scope_allocator::~scope_allocator() {
        if (internal::scoped_allocator != nullptr) {
            internal::scoped_allocator = m_prev;
        }
    }
    
    allocator* scope_allocator::get_prev() const {
        return m_prev;
    }
}

//#################################################### ./src/arena_free_list_allocator.cpp ####################################################

#include <allocators/arena_free_list_allocator.hpp>
#include <allocators/helpers.hpp>
#include <cpp/lang/system.hpp>
#include <utility>


namespace tca
{

    /**
    * 
    */
    struct arena_free_list_allocator::memblock {
        memblock*   left;
        memblock*   right;
        union {
            std::size_t key;
            std::size_t m_size;
        };
        signed char height;
        bool m_free;
    };

    arena_free_list_allocator::arena_free_list_allocator() : 
        allocator(nullptr),
        m_alignas(0),
        m_header_size(0),
        m_min_block_size(0),
        m_data(nullptr),
        m_length(0),
        m_start(nullptr),
        m_free_size(0),
        m_tree() {

    }

    void arena_free_list_allocator::init(void* data, std::size_t length, std::size_t align) {
        m_length            = length;
        m_header_size       = align_up(sizeof(memblock), align);
        m_min_block_size    = m_header_size + m_alignas;
        m_alignas           = align;
        m_data              = data;
        {//выравнивание адреса
            std::uintptr_t padding = calc_padding_for((uintptr_t) m_data, (uintptr_t) m_alignas);
            m_start = static_cast<void*>(static_cast<unsigned char*>(m_data) + padding);
            TC_ALIGN_ASSERT(m_start, m_alignas);
            m_free_size = length - padding;
        }
        memblock* block = reinterpret_cast<memblock*>(m_start);
        block->m_size   = m_free_size - m_header_size;
        link(block);
    }

    arena_free_list_allocator::arena_free_list_allocator(std::size_t length, std::size_t align, tca::base_allocator* allocator) : 
        arena_free_list_allocator() {
        void* data = allocator->allocate_align(length, align);
        if (!data) 
            return;
        m_parent = allocator;
        init(data, length, align);
    }

    arena_free_list_allocator::arena_free_list_allocator(void* data, std::size_t length, std::size_t align) :
        allocator(nullptr),
        m_alignas(align),
        m_header_size(align_up(sizeof(memblock), m_alignas)),
        m_min_block_size(m_header_size + m_alignas),
        m_data(data),
        m_length(length),
        m_start(nullptr),
        m_free_size(0),
        m_tree() {
        init(data, length, align);
    }

    arena_free_list_allocator::arena_free_list_allocator(arena_free_list_allocator&& alloc) : 
        allocator(std::move(alloc)),
        m_alignas(alloc.m_alignas),
        m_header_size(alloc.m_header_size),
        m_min_block_size(alloc.m_min_block_size),
        m_data(alloc.m_data),
        m_length(alloc.m_length),
        m_start(alloc.m_start),
        m_free_size(alloc.m_free_size),
        m_tree(std::move(alloc.m_tree)) {
        alloc.m_alignas           = 0;
        alloc.m_header_size       = 0;
        alloc.m_min_block_size    = 0;
        alloc.m_data              = nullptr;
        alloc.m_length            = 0;
        alloc.m_start             = nullptr;
        alloc.m_free_size         = 0;
    }

    void arena_free_list_allocator::cleanup() {
        if (m_parent != nullptr && m_data != nullptr) {
            m_parent->deallocate(m_data, m_length);
            m_data   = nullptr;
            m_parent = nullptr;
        }
    }

    arena_free_list_allocator& arena_free_list_allocator::operator= (arena_free_list_allocator&& alloc) {
        if (&alloc != this) {
            cleanup();
            allocator::operator=(std::move(alloc));
            m_alignas           = alloc.m_alignas;
            m_header_size       = alloc.m_header_size;
            m_min_block_size    = alloc.m_min_block_size;
            m_data              = alloc.m_data;
            m_length            = alloc.m_length;
            m_start             = alloc.m_start;
            m_free_size         = alloc.m_free_size;
            m_tree              = std::move(alloc.m_tree);
            alloc.m_alignas           = 0;
            alloc.m_header_size       = 0;
            alloc.m_min_block_size    = 0;
            alloc.m_data              = nullptr;
            alloc.m_length            = 0;
            alloc.m_start             = nullptr;
            alloc.m_free_size         = 0;
        }
        return *this;
    }

    void arena_free_list_allocator::link(memblock* block) {
        assert(block != nullptr);
        block->left     = nullptr;
        block->right    = nullptr;
        block->height   = 0;
        block->m_free   = true;
        m_tree.insert_entry(block);
    }
    
    void arena_free_list_allocator::unlink(memblock* block) {
        assert(block != nullptr);
        block->m_free = false;
        memblock* removed = m_tree.remove_entry(block);
        assert(removed == block);
    }

    arena_free_list_allocator::~arena_free_list_allocator() {
        cleanup();
    }

    void* arena_free_list_allocator::allocate(std::size_t sz) {
        void* const p = allocate_align(sz, m_alignas);
        assert(((std::uintptr_t) p % m_alignas) == 0);
        return p;
    }
        
    void* arena_free_list_allocator::allocate_align(std::size_t sz, std::size_t) {
        if (!m_data)
            return nullptr;
        sz = align_up(sz, m_alignas);
        
        memblock* block = m_tree.ceil_entry(sz);
        if (!block) {
            merge_all();
            block = m_tree.ceil_entry(sz);
            if (!block)
                return nullptr;
        }

        if (can_split(block, sz))
            split(block, sz);

        unlink(block);

        return reinterpret_cast<void*>(reinterpret_cast<unsigned char*>(block) + m_header_size);
    }

    bool arena_free_list_allocator::can_split(const memblock* block, std::size_t piece) const {
        assert(block != nullptr);
        return (block->m_size > piece) && (block->m_size - piece >= m_min_block_size);
    }

    void arena_free_list_allocator::split(memblock* block, std::size_t piece) {
        assert(block != nullptr);
        assert(can_split(block, piece));
        assert((piece % m_alignas) == 0);
        
        memblock* next  = reinterpret_cast<memblock*>(reinterpret_cast<unsigned char*>(block) + (m_header_size + piece));
        next->m_size    = block->m_size - piece - m_header_size;
        block->m_size   = piece;
        
        link(next);
    }

    void arena_free_list_allocator::merge_all() {
        typedef unsigned char uchar;
        typedef memblock mb;
        for (std::size_t off = 0; off < m_free_size; ) {
            mb* block = reinterpret_cast<mb*>(reinterpret_cast<uchar*>(m_start) + off);
            if (block->m_free) {                
                const std::size_t offset_to_next_block = off + m_header_size + block->m_size;
                if (offset_to_next_block < m_free_size) {
                    mb* next = reinterpret_cast<mb*>(reinterpret_cast<uchar*>(m_start) + offset_to_next_block);
                    if (next->m_free) {
                        memblock* deleted = m_tree.remove_entry(next);
                        assert(deleted != nullptr);
                        assert(deleted == next);
                        block->m_size += m_header_size + next->m_size;
                        continue;
                    }
                }
            }
            off += (m_header_size + block->m_size);
        }
        print_log();
    }

    void arena_free_list_allocator::deallocate(void* p) {
        if (p == nullptr) 
            return;
        link(reinterpret_cast<memblock*>(reinterpret_cast<unsigned char*>(p) - m_header_size));
    }

    void arena_free_list_allocator::print_log() const {
        typedef unsigned char uchar;
        typedef memblock mb;
        for (std::size_t off = 0; off < m_free_size; ) {
            mb* block = reinterpret_cast<mb*>(reinterpret_cast<uchar*>(m_start) + off);
            tc::system::tsprintf("[p: %p, is_free: %s, sz: %zu]\n", reinterpret_cast<void*>(block), (block->m_free ? "true" : "false"), block->m_size);
            off += (m_header_size + block->m_size);
        }
    }
}

//#################################################### ./src/basebuf.cpp ####################################################

#include <cpp/lang/io/basebuf.hpp>

namespace tc
{

    string basebuf::to_string(tca::allocator* alloc) const {
        string result(alloc);

        result
        .append("[position=").append(tc::to_string(m_position, alloc))
        .append(", limit=").append(tc::to_string(m_limit, alloc))
        .append(", capacity=").append(tc::to_string(m_capacity, alloc))
        .append(']');

        return result;
    }

} //namespace tc

//#################################################### ./src/base_allocator.cpp ####################################################

#include <allocators/malloc_free_allocator.hpp>
#include <cstdlib>
#include <cstdio>
#include <cassert>

namespace tca{

    base_allocator::base_allocator() : base_allocator(nullptr) {

    }

    base_allocator::base_allocator(base_allocator* parent) : m_parent(parent) {
        
    }

    base_allocator::base_allocator(base_allocator&& base) : m_parent(base.m_parent) {
        base.m_parent = nullptr;
    }

    base_allocator& base_allocator::operator= (base_allocator&& base) {
        if (&base != this)
        {
            m_parent      = base.m_parent;
            base.m_parent = nullptr;
        }
        return *this;
    }

    base_allocator::~base_allocator() {

    }

    void* base_allocator::allocate(std::size_t sz) {
        assert(m_parent != nullptr);
        return m_parent->allocate(sz);
    }

    void* base_allocator::allocate_align(std::size_t sz, std::size_t align) {
        assert(m_parent != nullptr);
        return m_parent->allocate_align(sz, align);
    }

    void base_allocator::deallocate(void* p, std::size_t sz) {
        assert(m_parent != nullptr);
        m_parent->deallocate(p, sz);
    }
}

//#################################################### ./src/bsd_socket.cpp ####################################################

#include <internal/bsd_socket.hpp>
#include <iostream>
#include <cpp/lang/types.hpp>
#include <cpp/lang/utils/utils.hpp>

#if defined(__WIN32)
#define _____WIN_CODE____(___code) ___code 
#define SHUT_RD SD_RECEIVE 
#define SHUT_WR SD_SEND
#else
#define _____WIN_CODE____(___code)
#endif
    
#if defined(__linux__) || defined(__APPLE__)
#define _____UNIX_CODE____(___code) ___code 
#else
#define _____UNIX_CODE____(___code) 
#endif

namespace tc 
{

namespace bsd_socket 
{

    void ms_to_timeval(struct timeval* tv, timepoint millisec) {
        // Меня настолько взбесили предупреждения, 
        // что я решил сделать функцию, которая сама будет присваивать и делать static_cast
        utils::assign_static_cast( tv->tv_sec, millisec / 1000 );
        utils::assign_static_cast( tv->tv_usec, (millisec % 1000) * 1000 );
    }   

    timepoint timeval_to_ms(struct timeval* tv) {
        
        timepoint s_sec;
        utils::assign_static_cast(s_sec, tv->tv_sec * 1000);

        timepoint s_usec;
        utils::assign_static_cast(s_usec, tv->tv_usec / 1000);

        return s_sec + s_usec;
    }

    const char* socket_error_string() {
        _____WIN_CODE____(
            int err = WSAGetLastError();
           switch(err) {
            case WSA_INVALID_HANDLE:
                return "Specified event object handle is invalid";
            case WSA_NOT_ENOUGH_MEMORY:
                return "Insufficient memory available";
            case WSA_INVALID_PARAMETER:
                return "One or more parameters are invalid";
            case WSA_OPERATION_ABORTED:
                return "Overlapped operation aborted";
            case WSA_IO_INCOMPLETE:
                return "Overlapped I/O event object not in signaled state";
            case WSA_IO_PENDING:
                return "Overlapped operations will complete later";
            case WSAEINTR:
                return "Interrupted function call";
            case WSAEBADF:
                return "File handle is not valid";
            case WSAEACCES:
                return "Permission denied";
            case WSAEFAULT:
                return "Bad address";
            case WSAEINVAL:
                return "Invalid argument";
            case WSAEMFILE:
                return "Too many open files";
            case WSAEWOULDBLOCK:
                return "Resource temporarily unavailable";
            case WSAEINPROGRESS:
                return "Operation now in progress";
            case WSAEALREADY:
                return "Operation already in progress";
            case WSAENOTSOCK:
                return "Socket operation on nonsocket";
            case WSAEDESTADDRREQ:
                return "Destination address required";
            case WSAEMSGSIZE:
                return "Message too long";
            case WSAEPROTOTYPE:
                return "Protocol wrong type for socket";
            case WSAENOPROTOOPT:
                return "Bad protocol option";
            case WSAEPROTONOSUPPORT:
                return "Protocol not supported";
            case WSAESOCKTNOSUPPORT:
                return "Socket type not supported";
            case WSAEOPNOTSUPP:
                return "Operation not supported";
            case WSAEPFNOSUPPORT:
                return "Protocol family not supported";
            case WSAEAFNOSUPPORT:
                return "Address family not supported by protocol family";
            case WSAEADDRINUSE:
                return "Address already in use";
            case WSAEADDRNOTAVAIL:
                return "Cannot assign requested address";
            case WSAENETDOWN:
                return "Network is down";
            case WSAENETUNREACH:
                return "Network is unreachable";
            case WSAENETRESET:
                return "Network dropped connection on reset";
            case WSAECONNABORTED:
                return "Software caused connection abort";
            case WSAECONNRESET:
                return "Connection reset by peer";
            case WSAENOBUFS:
                return "No buffer space available";
            case WSAEISCONN:
                return "Socket is already connected";
            case WSAENOTCONN:
                return "Socket is not connected";
            case WSAESHUTDOWN:
                return "Cannot send after socket shutdown";
            case WSAETOOMANYREFS:
                return "Too many references";
            case WSAETIMEDOUT:
                return "Connection timed out";
            case WSAECONNREFUSED:
                return "Connection refused";
            case WSAELOOP:
                return "Cannot translate name";
            case WSAENAMETOOLONG:
                return "Name too long";
            case WSAEHOSTDOWN:
                return "Host is down";
            case WSAEHOSTUNREACH:
                return "No route to host";
            case WSAENOTEMPTY:
                return "Directory not empty";
            case WSAEPROCLIM:
                return "Too many processes";
            case WSAEUSERS:
                return "User quota exceeded";
            case WSAEDQUOT:
                return "Disk quota exceeded";
            case WSAESTALE:
                return "Stale file handle reference";
            case WSAEREMOTE:
                return "Item is remote";
            case WSASYSNOTREADY:
                return "Network subsystem is unavailable";
            case WSAVERNOTSUPPORTED:
                return "Winsock.dll version out of range";
            case WSANOTINITIALISED:
                return "Successful WSAStartup not yet performed";
            case WSAEDISCON:
                return "Graceful shutdown in progress";
            case WSAENOMORE:
                return "No more results";
            case WSAECANCELLED:
                return "Call has been canceled";
            case WSAEINVALIDPROCTABLE:
                return "Procedure call table is invalid";
            case WSAEINVALIDPROVIDER:
                return "Service provider is invalid";
            case WSAEPROVIDERFAILEDINIT:
                return "Service provider failed to initialize";
            case WSASYSCALLFAILURE:
                return "System call failure";
            case WSASERVICE_NOT_FOUND:
                return "Service not found";
            case WSATYPE_NOT_FOUND:
                return "Class type not found";
            case WSA_E_NO_MORE:
                return "No more results";
            case WSA_E_CANCELLED:
                return "Call was canceled";
            case WSAEREFUSED:
                return "Database query was refused";
            case WSAHOST_NOT_FOUND:
                return "Host not found";
            case WSATRY_AGAIN:
                return "Nonauthoritative host not found";
            case WSANO_RECOVERY:
                return "This is a nonrecoverable error";
            case WSANO_DATA:
                return "Valid name, no data record of requested type";
            case WSA_QOS_RECEIVERS:
                return "QoS receivers";
            case WSA_QOS_SENDERS:
                return "QoS senders";
            case WSA_QOS_NO_SENDERS:
                return "No QoS senders";
            case WSA_QOS_NO_RECEIVERS:
                return "QoS no receivers";
            case WSA_QOS_REQUEST_CONFIRMED:
                return "QoS request confirmed";
            case WSA_QOS_ADMISSION_FAILURE:
                return "QoS admission error";
            case WSA_QOS_POLICY_FAILURE:
                return "QoS policy failure";
            case WSA_QOS_BAD_STYLE:
                return "QoS bad style";
            case WSA_QOS_BAD_OBJECT:
                return "QoS bad object";
            case WSA_QOS_TRAFFIC_CTRL_ERROR:
                return "QoS traffic control error";
            case WSA_QOS_GENERIC_ERROR:
                return "QoS generic error";
            case WSA_QOS_ESERVICETYPE:
                return "QoS service type error";
            case WSA_QOS_EFLOWSPEC:
                return "QoS flowspec error";
            case WSA_QOS_EPROVSPECBUF:
                return "Invalid QoS provider buffer";
            case WSA_QOS_EFILTERSTYLE:
                return "Invalid QoS filter style";
            case WSA_QOS_EFILTERTYPE:
                return "Invalid QoS filter type";
            case WSA_QOS_EFILTERCOUNT:
                return "Incorrect QoS filter count";
            case WSA_QOS_EOBJLENGTH:
                return "Invalid QoS object length";
            case WSA_QOS_EFLOWCOUNT:
                return "Incorrect QoS flow count";
            case WSA_QOS_EUNKOWNPSOBJ:
                return "Unrecognized QoS object";
            case WSA_QOS_EPOLICYOBJ:
                return "Invalid QoS policy object";
            case WSA_QOS_EFLOWDESC:
                return "Invalid QoS flow descriptor";
            case WSA_QOS_EPSFLOWSPEC:
                return "Invalid QoS provider-specific flowspec";
            case WSA_QOS_EPSFILTERSPEC:
                return "Invalid QoS provider-specific filterspec";
            case WSA_QOS_ESDMODEOBJ:
                return "Invalid QoS shape discard mode object";
            case WSA_QOS_ESHAPERATEOBJ:
                return "Invalid QoS shaping rate object";
            case WSA_QOS_RESERVED_PETYPE:
                return "Reserved policy QoS element type";
            default:
                return "No error";
           }
        );
        _____UNIX_CODE____(
            return strerror(errno)
        );
    }

    SOCK_TYPE open_tcp(inet_family family) {
        SOCK_TYPE sock = socket(family, SOCK_STREAM, 0);
        if (sock == NULL_SOCKET)
            throw_except<socket_exception>("Socket open error: %s", socket_error_string());
        return sock;
    }

    void close(SOCK_TYPE sock) {
        _____WIN_CODE____(
            if (closesocket(sock) != 0)
                throw_except<socket_exception>("Socket close error: %s", socket_error_string());
        );
        _____UNIX_CODE____(
            if (::close(sock) != 0)
                throw_except<socket_exception>("Socket close error: %s", socket_error_string());
        );
    }

    void bind(SOCK_TYPE sock, const inet_address& address, unsigned int port) {
        if (address.get_family() == inet_family::IPV4) {
            sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = address.get_family();
            addr.sin_port   = htons((unsigned short) port);
            address.get_in_addr(&addr.sin_addr);
            if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0)
                throw_except<bind_exception>("Socket bind error: %s", socket_error_string());
        } 
        
        else if (address.get_family() == inet_family::IPV6){
            sockaddr_in6 addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin6_family = address.get_family();
            addr.sin6_port   = htons((unsigned short) port);
            address.get_in6_addr(&addr.sin6_addr);
            if (::bind(sock, (sockaddr*) &addr, sizeof(addr)) != 0)
                throw_except<bind_exception>("Socket bind error: %s", socket_error_string());
        } else {
            throw_except<illegal_argument_exception>("Bind: Wrong address family");
        }
    }

    void backlog(SOCK_TYPE sock, int maxq) {
        if (::listen(sock, maxq) != 0)
            throw_except<socket_exception>("Socket backlog error: %s", socket_error_string());
    }

    SOCK_TYPE accept(SOCK_TYPE serv_sock, socket_address* client_addr) {
        sockaddr_storage addr_storage;
        socklen_t size = sizeof(addr_storage);
        
        SOCK_TYPE client_socket = ::accept(serv_sock, reinterpret_cast<sockaddr*>(&addr_storage), &size);
        
        _____WIN_CODE____(
            if (client_socket == NULL_SOCKET) {
                if (WSAGetLastError() == WSAEWOULDBLOCK)
                    return NULL_SOCKET;
                else
                    throw_except<socket_exception>("Socket accept error: %s", socket_error_string());
            }
        );
    
        _____UNIX_CODE____(
            if (client_socket == NULL_SOCKET) {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                    return NULL_SOCKET;
                else
                    throw_except<socket_exception>("Socket accept error: %s", socket_error_string());
            }
        );

        {
            if (size == sizeof(sockaddr_in)) {
                sockaddr_in* a = reinterpret_cast<sockaddr_in*>(&addr_storage);
                *client_addr = socket_address(inet_address::as_in_addr(&a->sin_addr), ntohs(a->sin_port));
            } 
            
            else if (size == sizeof(sockaddr_in6)) {
                sockaddr_in6* a = reinterpret_cast<sockaddr_in6*>(&addr_storage);
                *client_addr = socket_address(inet_address::as_in6_addr(&a->sin6_addr), ntohs(a->sin6_port));
            }

            else
                throw_except<socket_exception>("Unsupported address family type");
        }

        return client_socket;
    }
    
    void set_blocking(SOCK_TYPE sock, bool block_mode) {
        _____WIN_CODE____(
            u_long mode = block_mode ? 0 : 1;
            if (ioctlsocket(sock, FIONBIO, &mode) != 0)
                throw_except<socket_exception>("Socket set_blocking error: %s", socket_error_string());
        );
        _____UNIX_CODE____(
            int flags = fcntl(sock, F_GETFL, 0);
            if (flags == -1) 
                throw_except<socket_exception>("Socket set_blocking: (F_GETFL) error: %s", socket_error_string());
            
            flags = block_mode ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
            
            if(fcntl(sock, F_SETFL, flags) != 0)
                throw_except<socket_exception>("Socket set_blocking: (F_SETFL) error: %s", socket_error_string());
        );
    }

    void connect(SOCK_TYPE client_sock, const inet_address& address, unsigned int port) {
        if (address.get_family() == inet_family::IPV4)
        {
            sockaddr_in addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin_family = address.get_family();
            addr.sin_port   = htons((unsigned short) port);
            address.get_in_addr(&addr.sin_addr);
            
            if (::connect(client_sock, (sockaddr*) &addr, sizeof(addr)) != 0)
            {
                throw_except<connect_exception>("Connect fail: %s", socket_error_string());
            }

        } 
        else if (address.get_family() == inet_family::IPV6)
        {
            sockaddr_in6 addr;
            std::memset(&addr, 0, sizeof(addr));
            addr.sin6_family    = address.get_family();
            addr.sin6_port      = htons((unsigned short) port);
            address.get_in6_addr(&addr.sin6_addr);
            
            if (::connect(client_sock, (sockaddr*)&addr, sizeof(addr)) != 0)
            {
                throw_except<connect_exception>("Connect fail: %s", socket_error_string());
            }
        } 
        else
        {
            throw_except<illegal_argument_exception>("Connect: Wrong address family");
        }
    }
 
    std::size_t send(SOCK_TYPE sock, const char* data, std::size_t length, bool is_blocking) {
        _____WIN_CODE____(
            int sended = ::send(sock, data, (int) length, 0);
            if (sended == SOCKET_ERROR) {
                
                int err = WSAGetLastError();
                
                if (err == WSAEWOULDBLOCK)
                {
                    return 0;
                }
                else if (err == WSAETIMEDOUT && is_blocking)
                {
                    throw_except<socket_timeout_exception>(socket_error_string());
                }
                else if (err == WSAECONNRESET)
                {
                    throw_except<connect_exception>("Connection is closed by remote side");
                }
                
                throw_except<io_exception>(socket_error_string());
            } 

            return (std::size_t) sended;
        );
        
        _____UNIX_CODE____
        (
            errno = 0;
            ssize_t sended = ::send(sock, data, length, MSG_NOSIGNAL);
            if (sended == -1) {
                
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    if (is_blocking) 
                        throw_except<socket_timeout_exception>(socket_error_string());
                    return 0;
                } 
                else if (errno == EPIPE) 
                {
                    throw_except<connect_exception>("Connection is closed by remote side");
                }

                throw_except<io_exception>(socket_error_string());
            } 
            
            return (std::size_t) sended;
        );
    }

    std::size_t recv(SOCK_TYPE sock, char* data, std::size_t length, bool is_blocking) {
        _____WIN_CODE____
        (
            int received = ::recv(sock, data, (int) length, 0);
            if (received == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                    return 0;
                else if (err == WSAETIMEDOUT && is_blocking)
                    throw_except<socket_timeout_exception>(socket_error_string());
                else if (err == WSAECONNRESET)
                    throw_except<connect_exception>("Connection is closed by remote side");
            }
            else if (received == 0)
            {
                throw_except<connect_exception>("Connection is closed by remote side");
            }

            return (std::size_t) received;
        );

        _____UNIX_CODE____
        (
            errno = 0;
            ssize_t received = ::recv(sock, data, length, 0);
            if (received == -1)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    if (is_blocking)
                    {
                        throw_except<socket_timeout_exception>(socket_error_string());    
                    }
                    return 0;
                }
                throw_except<io_exception>(socket_error_string());
            }
            else if (received == 0) 
            {
                throw_except<connect_exception>("Connection is closed by remote side");
            }

            return (std::size_t) received;
        );
    }

    void get_sock_name(SOCK_TYPE sock, socket_address& address) {
        sockaddr_storage addr_storage = {};
        
        socklen_t len = sizeof(addr_storage);
        
        if (getsockname(sock, reinterpret_cast<sockaddr*>(&addr_storage), &len) != 0)
            throw_except<socket_exception>(socket_error_string());

        if (len == sizeof(sockaddr_in))
        {
            sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(&addr_storage);
            address = socket_address(inet_address::as_in_addr(&addr->sin_addr), ntohs(addr->sin_port));
        } 
        else if (len == sizeof(sockaddr_in6))
        {
            sockaddr_in6* addr = reinterpret_cast<sockaddr_in6*>(&addr_storage);
            address = socket_address(inet_address::as_in6_addr(&addr->sin6_addr), ntohs(addr->sin6_port));
        } 
        else
        {
            throw_except<unsupported_operation_exception>("Unsupported address family type");
        }
    }

    void shutdown_in(SOCK_TYPE sock) {
        if (::shutdown(sock, SHUT_RD) != 0)
            throw_except<socket_exception>("Shutdown read: %s", socket_error_string());
    }
    
    void shutdown_out(SOCK_TYPE sock) {
        if (::shutdown(sock, SHUT_WR) != 0)
            throw_except<socket_exception>("Shutdown write: %s", socket_error_string());
    }

    template<int LEVEL, int OPTNAME>
    void set_int_sock_opt(SOCK_TYPE sock, const socket_option& value) {
        int val = value.int_value;
        if (setsockopt(sock, LEVEL, OPTNAME, (const char*) &val, sizeof(val)) != 0)
            throw_except<socket_exception>("%s fail: %s", __func__ , socket_error_string());
    }

    template<int LEVEL, int OPTNAME>
    void set_time_sock_opt(SOCK_TYPE sock, const socket_option& value) {
        _____WIN_CODE____
        (
            DWORD ms = (DWORD) value.timeout.millis;
            if (setsockopt(sock, LEVEL, OPTNAME, (const char*) &ms, sizeof(ms)) != 0)
                throw_except<socket_exception>("%s fail: %s", __func__ , socket_error_string());
        );
        
        _____UNIX_CODE____
        (
            struct timeval tv;
            ms_to_timeval(&tv, value.timeout.millis);
            if (setsockopt(sock, LEVEL, OPTNAME, (const char*) &tv, sizeof(tv)) != 0)
                throw_except<socket_exception>("%s fail: %s", __func__ , socket_error_string());
        ); 
    }
    
    template<int LEVEL, int OPTNAME>
    void get_int_sock_opt(SOCK_TYPE sock, socket_option& value) {
        int val         = 0;
        socklen_t len   = sizeof(val);
        if (getsockopt(sock, LEVEL, OPTNAME, (char*) &val, &len) != 0)
            throw_except<socket_exception>("%s fail: %s", __func__ , socket_error_string());
        value.int_value = val;  
    }

    template<int LEVEL, int OPTNAME>
    void get_time_sock_opt(SOCK_TYPE sock, socket_option& value) {
        _____WIN_CODE____(
            DWORD ms = 0;
            socklen_t len = sizeof(ms);
            if (getsockopt(sock, LEVEL, OPTNAME, (char*) &ms, &len) != 0)
                throw_except<socket_exception>("%s fail: %s", __func__ , socket_error_string());
            value.timeout.millis = (timepoint) ms;
        );

        _____UNIX_CODE____(
            struct timeval tv;
            std::memset(&tv, 0, sizeof(tv));
            socklen_t len = sizeof(tv);
            if (getsockopt(sock, LEVEL, OPTNAME, (char*) &tv, &len) != 0)
                throw_except<socket_exception>("%s fail: %s", __func__ , socket_error_string());
            value.timeout.millis = timeval_to_ms(&tv);
        ); 
    }
 
    void set_linger(SOCK_TYPE sock, const socket_option& value) {
        struct linger l;
        
        utils::assign_static_cast( l.l_linger, value.linger.sec_time );
        utils::assign_static_cast( l.l_onoff,  value.linger.on_off );
        
        if (setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*) &l, sizeof(l)) != 0)
            throw_except<socket_exception>("set_linger fail: %s", socket_error_string());
    }

    void get_linger(SOCK_TYPE sock, socket_option& value) {
        struct linger l = {};
        socklen_t len = sizeof(l);
        if (getsockopt(sock, SOL_SOCKET, SO_LINGER, (char*) &l, &len) != 0)
            throw_except<socket_exception>("set_linger fail: %s", socket_error_string());
        utils::assign_static_cast(value.linger.on_off,     l.l_onoff);
        utils::assign_static_cast(value.linger.sec_time,   l.l_linger);
    }

    void set_keep_alive(SOCK_TYPE sock, const socket_option& value) {
        
        socket_option opt;
        if (value.keepalive.on_off)
        {
            opt.int_value = true;
        }
        else
        {
            opt.int_value = false;
        }

        set_int_sock_opt<SOL_SOCKET, SO_KEEPALIVE>(sock, opt);

        if (!value.keepalive.on_off)
        {
            return;
        }

        _____UNIX_CODE____
        (
            
            int idle;       utils::assign_static_cast(idle,     value.keepalive.time_to_first);
            int interval;   utils::assign_static_cast(interval, value.keepalive.time_interval);
            int count;      utils::assign_static_cast(count,    value.keepalive.try_count);
            
            if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle)) != 0)
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", socket_error_string());
            
            if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval)) != 0) 
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", socket_error_string());
            
            if (setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count)) != 0) 
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", socket_error_string());
        );

        _____WIN_CODE____
        (
            struct tcp_keepalive ka;
            ka.onoff                = 1; 
            ka.keepalivetime        = value.keepalive.time_to_first;
            ka.keepaliveinterval    = value.keepalive.time_interval;
        
            DWORD bytesReturned;
            if (WSAIoctl(sock, SIO_KEEPALIVE_VALS, &ka, sizeof(ka), NULL, 0, &bytesReturned, NULL, NULL) != 0)
                throw_except<socket_exception>("set_keep_alive_ex fail: %s", socket_error_string());
        );
    }

    void set_sock_opt(SOCK_TYPE sock, int opt, const socket_option& value) {
        switch(opt) {
            case socket_option::bsd::TCP_NODELAY_:
                set_int_sock_opt<IPPROTO_TCP, TCP_NODELAY>(sock, value);
                return;
            case socket_option::bsd::SO_REUSEADDR_:
                set_int_sock_opt<SOL_SOCKET, SO_REUSEADDR>(sock, value);
                return;
            case socket_option::bsd::SO_BROADCAST_:
                set_int_sock_opt<SOL_SOCKET, SO_BROADCAST>(sock, value);
                return;
            case socket_option::bsd::IP_TOS_:
                set_int_sock_opt<IPPROTO_TCP, IP_TOS>(sock, value);
                return;
            case socket_option::bsd::SO_RCVBUF_:
                set_int_sock_opt<SOL_SOCKET, SO_RCVBUF>(sock, value);
                return;
            case socket_option::bsd::SO_SNDBUF_:
                set_int_sock_opt<SOL_SOCKET, SO_SNDBUF>(sock, value);
                return;
            case socket_option::bsd::SO_OOBINLINE_:
                set_int_sock_opt<SOL_SOCKET, SO_OOBINLINE>(sock, value);
                return;
            case socket_option::bsd::SO_LINGER_:
                set_linger(sock, value);
                return;
            case socket_option::bsd::SO_RCVTIMEO_:
                set_time_sock_opt<SOL_SOCKET, SO_RCVTIMEO>(sock, value);
                return;
            case socket_option::bsd::SO_SNDTIMEO_:
                set_time_sock_opt<SOL_SOCKET, SO_SNDTIMEO>(sock, value);
                return;
            case socket_option::bsd::SO_KEEPALIVE_:
                set_keep_alive(sock, value);
                return;
            default:
                throw_except<illegal_argument_exception>("Option %i is invalid", (int) opt);
        }
    }

    void get_sock_opt(SOCK_TYPE sock, int opt, socket_option& value) {
        switch(opt) {
            case socket_option::bsd::TCP_NODELAY_:
                get_int_sock_opt<IPPROTO_TCP, TCP_NODELAY>(sock, value);
                return;
            case socket_option::bsd::SO_REUSEADDR_:
                get_int_sock_opt<SOL_SOCKET, SO_REUSEADDR>(sock, value);
                return;
            case socket_option::bsd::SO_BROADCAST_:
                get_int_sock_opt<SOL_SOCKET, SO_BROADCAST>(sock, value);
                return;
            case socket_option::bsd::IP_TOS_:
                get_int_sock_opt<IPPROTO_TCP, IP_TOS>(sock, value);
                return;
            case socket_option::bsd::SO_LINGER_:
                get_linger(sock, value);
                return;
            case socket_option::bsd::SO_RCVTIMEO_:
                get_time_sock_opt<SOL_SOCKET, SO_RCVTIMEO>(sock, value);
                return;
            case socket_option::bsd::SO_SNDTIMEO_:
                get_time_sock_opt<SOL_SOCKET, SO_SNDTIMEO>(sock, value);
                return;
            case socket_option::bsd::SO_RCVBUF_:
                get_int_sock_opt<SOL_SOCKET, SO_RCVBUF>(sock, value);
                return;
            case socket_option::bsd::SO_SNDBUF_:
                get_int_sock_opt<SOL_SOCKET, SO_SNDBUF>(sock, value);
                return;
            case socket_option::bsd::SO_OOBINLINE_:
                get_int_sock_opt<SOL_SOCKET, SO_OOBINLINE>(sock, value);
                return;
            case socket_option::bsd::SO_KEEPALIVE_:
                get_int_sock_opt<SOL_SOCKET, SO_KEEPALIVE>(sock, value);    
                return;
            default:
                throw_except<illegal_argument_exception>("Option %i is invalid!", (int) opt);
        }
    }

}//bsd_socket

}//jstd


#if 0
#include <internal/bsd_socket_class.hpp>
#include <cpp/lang/net/inet.hpp>

// clang++ -g -std=c++11 -O0 -Wextra -pedantic -Wall -Wsign-promo -Wfloat-equal -Wconversion -Wno-nested-anon-types -Wno-unused-parameter -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable -Wnon-virtual-dtor -Wno-unused-function  -o a.exe -I"include" -I"./tmp/include/" ./src/system.cpp ./src/bsd_socket.cpp ./src/inet.cpp ./src/exceptions.cpp ./src/stacktrace.cpp ./src/inetaddr.cpp ./src/ip_parser.cpp ./src/mutex.cpp -lws2_32 -L./tmp/static/ -lliblz4_static
int main(int argc, const char** args) {
    printf("sock\n");
    using namespace jstd::bsd_socket;

    const unsigned int PORT = 25565;
    const tc::inet_address ADDRESS = tc::inet_address::localhost(tc::inet_family::IPV4);

    try {

    tc::inet_context inet_ctx;

    if (argc > 1)
    {
        socket_impl sock;
        
        if (strcmp(args[1], "-s") == 0)
        {
            sock.create(tc::inet_family::IPV4);
            sock.bind(tc::inet_address(), PORT);
            sock.listen(64);

            socket_impl client;
            sock.accept(&client);

            client.set_blocking(false);
            std::printf("client connected\n");
            while (1)
            {
                char buf[4];
                std::size_t readed = client.read(buf, sizeof(buf));
                if (readed > 0)
                {
                    for (std::size_t i = 0; i < readed; ++i)
                    {
                        std::printf("%c", buf[i]);
                    }
                    std::printf("\n");
                }
                
                std::printf("i\n");

            }

        }
        else if (strcmp(args[1], "-c") == 0)
        {
            sock.create(tc::inet_family::IPV4);
            sock.connect(ADDRESS, PORT);
            char buf[1024];
            while (1)
            {
                std::scanf("%s", buf);
                std::printf("send: '%s'", buf);
                sock.write(buf, std::strlen(buf));
            }
        }
    }
    
    } catch(const tc::throwable& t) {
        std::printf("%s\n", t.cause());
    }

}

#endif

//#################################################### ./src/bsd_socket_class.cpp ####################################################

#include <internal/bsd_socket_class.hpp>
#include <internal/bsd_socket.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <iostream>

namespace tc 
{
namespace bsd_socket 
{

    typedef std::intptr_t SOCKET_HANDLE_TYPE;

    sock_handle::sock_handle() : handle((SOCKET_HANDLE_TYPE) NULL_SOCKET) {

    }
    
    sock_handle::sock_handle(std::intptr_t handle) : handle(handle) {

    }
    
    sock_handle::sock_handle(sock_handle&& id) {
        std::swap(handle, id.handle);
    }
    
    sock_handle& sock_handle::operator= (sock_handle&& id) {
        if (&id != this)
        {
            std::swap(handle, id.handle);
        }
        return *this;
    }

    sock_handle::~sock_handle() {

    }

    void sock_handle::close() {
        if ((SOCK_TYPE) handle == NULL_SOCKET)
            return;
        try {
            bsd_socket::close((SOCK_TYPE) handle);
            utils::assign_static_cast(handle, NULL_SOCKET);
        } catch (...) {
            utils::assign_static_cast(handle, NULL_SOCKET);
            throw;
        }
    }

/**
 * =====================================================================================================================================================================
 * =====================================================================================================================================================================
 */

    socket_impl::socket_impl() : base_socket<sock_handle>(), _blocking(true) {
        
    }
    
    socket_impl::socket_impl(socket_impl&& sock) : base_socket<sock_handle>(std::move(sock)), _blocking(sock._blocking) {

    }

    socket_impl& socket_impl::operator= (socket_impl&& sock) {
        if (&sock != this)
        {
            base_socket<sock_handle>::operator=(std::move(sock));
            _blocking = sock._blocking;
        }
        return *this;
    }

    void socket_impl::set_socket_option(int opt_id, const socket_option& value) {
        bsd_socket::set_sock_opt((SOCK_TYPE) sock_id.handle, opt_id, value);
    }
    
    socket_option socket_impl::get_socket_option(int opt_id) const {
        socket_option ret;
        bsd_socket::get_sock_opt((SOCK_TYPE) sock_id.handle, opt_id, ret);
        return ret;
    }
    
    void socket_impl::connect(const char* host, unsigned int port) {
        connect(inet_address(host), port);
    }

    void socket_impl::connect(const inet_address& host, unsigned int port) {
        bsd_socket::connect((SOCK_TYPE) sock_id.handle, host, port);
        socket_address addr;
        bsd_socket::get_sock_name((SOCK_TYPE) sock_id.handle, addr);
        set_port(port);
        set_address(host);
        set_localport(addr.get_port());
    }
    
    void socket_impl::bind(const inet_address& address, unsigned int port) {
        bsd_socket::bind((SOCK_TYPE) sock_id.handle, address, port);
    }

    void socket_impl::listen(int backlog) {
        bsd_socket::listen((SOCK_TYPE) sock_id.handle, backlog);
    }

    bool socket_impl::accept(base_socket<sock_handle>* client) {
        JSTD_DEBUG_CODE(
            if (client == nullptr)
                throw_except<null_pointer_exception>("client is nullptr!");
        )
        
        socket_address client_address;
        SOCK_TYPE client_sock_id = bsd_socket::accept((SOCK_TYPE) sock_id.handle, &client_address);
        
        if (client_sock_id != NULL_SOCKET)
        {
            client->set_address(client_address.get_address());
            client->set_localport(port);
            client->set_port(client_address.get_port());
            client->set_descriptor(sock_handle((SOCKET_HANDLE_TYPE) client_sock_id));
            return true;
        }
            
        return false;
    }
    
    void socket_impl::create(inet_family family) {
        sock_handle sockID((SOCKET_HANDLE_TYPE) bsd_socket::open_tcp(family));
        sock_id = std::move(sockID); 
    }
    
    bool socket_impl::is_created() const {
        return (SOCK_TYPE) sock_id.handle != NULL_SOCKET;
    }

    void socket_impl::close() {
        sock_id = sock_handle((SOCKET_HANDLE_TYPE) NULL_SOCKET);
    }
    
    void socket_impl::shutdown_in() {
        bsd_socket::shutdown_in((SOCK_TYPE) sock_id.handle);
    }
    
    void socket_impl::shutdown_out() {
        bsd_socket::shutdown_out((SOCK_TYPE) sock_id.handle);
    }
    
    std::size_t socket_impl::read(char buf[], std::size_t len) {
        return bsd_socket::recv((SOCK_TYPE) sock_id.handle, buf, len, _blocking);
    }
    
    std::size_t socket_impl::write(const char* data, std::size_t len) {
        return bsd_socket::send((SOCK_TYPE) sock_id.handle, data, len, _blocking);
    }

    void socket_impl::set_blocking(bool is_block) {
        bsd_socket::set_blocking((SOCK_TYPE) sock_id.handle, is_block);
        _blocking = is_block;
    }
    
    bool socket_impl::is_blocking() const {
        return _blocking;
    }

    socket_impl::~socket_impl() {
        
    }

}

}

//#################################################### ./src/bytebuf.cpp ####################################################

#include <cpp/lang/io/bytebuf.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cassert>

namespace tc
{

    
    bytebuf::bytebuf(tca::allocator* alloc) : bytebuf(0, alloc) {
        
    }
    
    bytebuf::bytebuf(std::size_t capacity, tca::allocator* alloc) : basebuf(), m_alloc(alloc) {
        if (capacity > 0)
        {
            char* data = (char*) m_alloc->allocate_align(capacity, alignof(char));
            if(!data)
                throw_except<out_of_memory_error>("Out of memory");
            m_data      = data;
            m_capacity  = capacity;
            m_limit     = capacity;
        }
    }

    bytebuf& bytebuf::operator= (const bytebuf& buf) {
        assert(m_alloc != nullptr);
        
        char* newdata       = nullptr;
        std::size_t newcap  = buf.m_capacity;
        if (newcap > 0)
        {
            newdata = (char*) m_alloc->allocate_align(newcap, alignof(char));
            if (!newdata)
                throw_except<out_of_memory_error>("Out of memory");
            std::memcpy(newdata, buf.m_data, newcap);
        }

        if (m_data)
            m_alloc->deallocate(m_data);
        
        m_data      = newdata;
        m_capacity  = newcap;
        
        m_limit     = buf.m_limit;
        m_mark      = buf.m_mark;
        m_position  = buf.m_position;
        m_order     = buf.m_order;
        m_readonly  = buf.m_readonly;

        return *this;
    }

    bytebuf::bytebuf(const bytebuf& buf) : basebuf(), m_alloc(buf.m_alloc) {
        char* newdata       = nullptr;
        std::size_t newcap  = buf.m_capacity;
        if (newcap > 0)
        {
            newdata = (char*) m_alloc->allocate_align(newcap, alignof(char));
            if (!newdata)
                throw_except<out_of_memory_error>("Out of memory");
            std::memcpy(newdata, buf.m_data, newcap);
        }

        if (m_data)
            m_alloc->deallocate(m_data);
        
        m_data      = newdata;
        m_capacity  = newcap;
        
        m_limit     = buf.m_limit;
        m_mark      = buf.m_mark;
        m_position  = buf.m_position;
        m_order     = buf.m_order;
        m_readonly  = buf.m_readonly;
    }

    bytebuf::bytebuf(bytebuf&& buf) : basebuf(), m_alloc(buf.m_alloc) {
        m_data      = buf.m_data;
        m_capacity  = buf.m_capacity;
        m_position  = buf.m_position;
        m_limit     = buf.m_limit;
        m_mark      = buf.m_mark;
        m_order     = buf.m_order;
        m_readonly  = buf.m_readonly;
        
        m_data      = nullptr;
        m_capacity  = 0;
    }

    bytebuf& bytebuf::operator= (bytebuf&& buf) {
        if (&buf == this)
            return *this;

        if (get_allocator() == buf.get_allocator())
        {
            std::swap(m_data,       buf.m_data);
            std::swap(m_capacity,   buf.m_capacity);
            std::swap(m_position,   buf.m_position);
            std::swap(m_limit,      buf.m_limit);
            std::swap(m_mark,       buf.m_mark);
            std::swap(m_order,      buf.m_order);
            std::swap(m_readonly,   buf.m_readonly);
        }
        else
        {
            *this = buf;
        }

        return *this;
    }

    bytebuf::~bytebuf() {
        if(m_data)
            m_alloc->deallocate(m_data);
    }
}

//#################################################### ./src/channel.cpp ####################################################

#include <cpp/lang/io/channel.hpp>

namespace tc
{

    channel::channel() {

    }

    channel::channel(channel&&) {

    }

    channel& channel::operator= (channel&&) {
        return *this;
    }
    
    channel::~channel() {

    }

}

//#################################################### ./src/compressor.cpp ####################################################

#include <cpp/lang/compress/compressor.hpp>

    const char* compressor::get_input() const {
        return input;
    }

    std::size_t compressor::get_size() const {
        return input_size;
    }

    void compressor::finish() {
        finished = true;
    }

    compressor::compressor() : input(nullptr), input_size(0) {

    }
    
    compressor::compressor(compressor&& other) {

    }
    
    compressor& compressor::operator= (compressor&& other) {
        if (&other != this)
        {
            input       = other.input;
            input_size  = other.input_size; 
        }
        return *this;
    }
    
    void compressor::set_input(const char* data, std::size_t len) {
        input       = data;
        input_size  = len;
    }
    
    bool compressor::is_finished() const {
        return finished;
    }

    compressor::~compressor() {

    }

//#################################################### ./src/control_block.cpp ####################################################

#include <internal/smart_ptrs/control_block.hpp>

namespace tc
{
namespace internal
{
    control_block::~control_block() {
        
    }

    void control_block::destroy_control_block() {
        tca::allocator* alloc = m_allocator;
        
        control_block* self = this;
        self->~control_block();
        
        alloc->deallocate(static_cast<void*>(this));
    }
} //namespace internal
} //namespace tc

//#################################################### ./src/cstr.cpp ####################################################

#include <cpp/lang/cstr.hpp>

namespace tc
{

    c_str::c_str(const char* s) : m_cstr(s), m_length(~(std::size_t) 0) {
        
    }

    c_str::operator const char* () const {
        return m_cstr;
    }
    
    std::size_t c_str::length() const {
        if (m_length == ~(std::size_t) 0)
        {
            if (m_cstr != nullptr)
            {
                m_length = std::strlen(m_cstr);
            }
            else
            {
                m_length = 0;
            }
        }
        return m_length;
    }

    const char& c_str::operator[] (std::size_t idx) const {
#ifndef NDEBUG
        check_index(idx, length());
#endif//NDEBUG
        return m_cstr[idx];
    }

    bool c_str::equals(const c_str& s) const {
        std::size_t len1 = length();
        std::size_t len2 = s.length();
        if (len1 != len2)
            return false;
        for (std::size_t i = 0, len = len1; i < len; ++i)
            if ((*this)[i] != s[i])
                return false;
        return true;
    }

    bool c_str::operator==(const c_str& s) const {
        return equals(s);
    }
    
    bool c_str::operator!=(const c_str& s) const {
        return !equals(s);
    }

    std::size_t c_str::hashcode() const {
        return objects::hashcode(m_cstr,  m_cstr + m_length, hash_for<char>());
    }

    bool c_str::is_empty() const {
        return length() == 0;
    }

}

//#################################################### ./src/date.cpp ####################################################

#include <cpp/lang/utils/date.hpp>
#include <cstring>
#include <utility>
#include <cpp/lang/exceptions.hpp>
#include <cerrno>

namespace tc
{

    bool get_local_time(const std::time_t* time, std::tm* localtime) {
        JSTD_WIN_CODE
        (
            return localtime_s(localtime, time) == 0;
        )
        JSTD_UNIX_CODE
        (
            return localtime_r(time, localtime) != nullptr;
        )
    }

    date::date(int day, int month, int year, int second, int minute, int hour) {
        std::memset(&_localTime, 0, sizeof(_localTime));
         
        day     = (day     == 0) ? 1       : day;
        month   = (month   == 0) ? 1       : month;
        year    = (year    == 0) ? 1900    : year;
        _localTime.tm_year    = year - 1900;
        _localTime.tm_mon     = month - 1;
        _localTime.tm_mday    = day;
        _localTime.tm_sec     = second;
        _localTime.tm_min     = minute;
        _localTime.tm_hour    = hour;
        _localTime.tm_isdst   = -1;

        _time = mktime(&_localTime);
    }

    date::date(std::time_t date){
        set_time(date);
    }

    tc::string date::to_string(tca::allocator* alloc) const {
        char buf[128];    
        strftime(buf, sizeof(buf), "%d.%m.%Y %H:%M:%S", &_localTime);
        return tc::string(buf, alloc);
    }

    /*static*/ date date::now() {
        std::time_t current;
        std::time(&current);
        return date(current);
    }

    /*static*/ date date::of_seconds(timepoint sec) {
        return date(static_cast<std::time_t>(sec));
    }

    /*static*/ date date::of_milliseconds(timepoint ms) {
        return of_seconds(ms / 1000);
    }

    bool date::equals(const date& date) const {
        return _time == date._time;
    }

    std::size_t date::hashcode() const {
        return (std::size_t) _time;
    }
    
    int date::compare_to(const date& date) const {
        if (_time < date._time) return -1;
        if (_time > date._time) return  1;
        return 0;
    }

    bool date::operator== (const date& date) const {
        return equals(date);
    }

    bool date::operator!= (const date& date) const {
        return !equals(date);
    }
    
    bool date::operator> (const date& date) const {
        return compare_to(date) > 0;
    }
    
    bool date::operator< (const date& date) const {
        return compare_to(date) < 0;
    }

    bool date::operator>= (const date& date) const {
        return compare_to(date) >= 0;
    }
    
    bool date::operator<= (const date& date) const {
        return compare_to(date) <= 0;
    }

    void date::set_time(std::time_t date) {
        _time            = date;
        if (!get_local_time(&_time, &_localTime))
            throw_except<runtime_exception>(std::strerror(errno));
    }

    std::time_t date::get_time() const {
        return _time;
    }

    int date::get_day() const {
        return _localTime.tm_mday;
    }

    int date::get_month() const {
        return _localTime.tm_mon + 1;
    }

    int date::get_year() const {
        return _localTime.tm_year + 1900;
    }
    
    int date::get_hour() const {
        return _localTime.tm_hour;
    }
    
    int date::get_minute() const {
        return _localTime.tm_min;
    }
    
    int date::get_second() const {
        return _localTime.tm_sec;
    }

    int date::get_week() const {
        return _localTime.tm_wday;
    }

}// namespace jstd

//#################################################### ./src/decompressor.cpp ####################################################

#include <cpp/lang/compress/decompressor.hpp>


namespace tc
{

    decompressor::decompressor() : input(nullptr), input_size(0) {

    }

    decompressor::decompressor(decompressor&& other) : input(other.input), input_size(other.input_size) {
        other.input         = nullptr;
        other.input_size    = 0;
    }
        
    decompressor& decompressor::operator=(decompressor&& other) {
        if (&other != this)
        {
            input       = other.input;
            input_size  = other.input_size;
            other.input         = nullptr;
            other.input_size    = 0;
        }
        return *this;
    }
        
    void decompressor::set_input(const char* in, std::size_t length) {
        input       = in;
        input_size  = length;
    }
    
    decompressor::~decompressor() {

    }
}


//#################################################### ./src/errors.cpp ####################################################

#include <cpp/lang/errors.hpp>
#include <cstring>
#include <cpp/lang/utils/cond_compile.hpp>

#if defined(_WIN32)
# include <windows.h>
#endif

#include <cpp/lang/string.hpp>
#include <iostream>

namespace tc
{
    bool error_category::equivalent(int err, const class error_condition& cond) const {
        return default_error_condition(err) == cond;
    }
    
    bool error_category::equivalent(const class error_code& err, int cond) const {
        return (*this == err.get_category()) && (err.get_value() == cond);
    }
    
    error_condition error_category::default_error_condition(int err) const {
        return error_condition(err, *this);
    }
}

namespace tc
{

namespace internal
{
    /**
     * Error category for POSIX errno error codes.
     * 
     * Implements error_category for errno values. Provides the name "system"
     * and error messages via strerror().
     * 
     * @note
     *      On POSIX systems, this is equivalent to system_category.
     *      On Windows, system_category handles GetLastError() separately.
     * 
     * @warning
     *      strerror() may not be thread-safe on all platforms. Consider using
     *      strerror_r() or strerror_s() in thread-safe implementations.
     * 
     * @example
     *      int fd = open("/path", O_RDONLY);
     *      if (fd == -1) {
     *          error_code ec(errno, generic_category());
     *          string msg = ec.get_message();  // "No such file or directory"
     *      }
     * 
     * @see
     *      error_category, system_category(), error_code
     */
    class errno_category : public error_category {
        errno_category(const errno_category&) = delete;
        errno_category(errno_category&&) = delete;
        errno_category& operator= (const errno_category&) = delete;
        errno_category& operator= (errno_category&&) = delete;
    public:    
        
        errno_category() = default;
        
        /**
         * @return The category name "errno".
         */
        const char* get_name() const override {
            return "errno";
        }
        
        /**
         * Converts errno to a human-readable message.
         * 
         * @param err
         *      The errno value.
         * 
         * @param alloc
         *      Allocator for the returned string.
         * 
         * @return Error message string.
         */
        string get_message(int err, tca::allocator* alloc) const override {
            return string(std::strerror(err), alloc);
        }
    };

    #if defined(JSTD_OS_WINDOWS)
    /**
     * Error category for Windows system error codes.
     * 
     * Implements error_category for Windows GetLastError() codes.
     * Provides the name "system" and error messages via FormatMessageA().
     * 
     * @note
     *      Uses English (US) locale for consistent error messages across
     *      different system locales.
     * 
     * @warning
     *      The error message buffer is fixed at 128 characters. Very long
     *      error messages may be truncated.
     * 
     * @example
     *      HANDLE h = CreateFileA("nonexistent.txt", ...);
     *      if (h == INVALID_HANDLE_VALUE) {
     *          error_code ec(GetLastError(), windows_category());
     *          string msg = ec.get_message();  // "The system cannot find the file specified."
     *      }
     * 
     * @see
     *      error_category, system_category(), error_code
     */
    class windows_category : public error_category {
        windows_category(const windows_category&) = delete;
        windows_category(windows_category&&) = delete;
        windows_category& operator= (const windows_category&) = delete;
        windows_category& operator= (windows_category&&) = delete;
    public:    
        
        windows_category() = default;
    
        /**
         * @return The category name "system".
         */
        const char* get_name() const override {
            return "system";
        }
        
        /**
         * Converts Windows error code to a human-readable message.
         * 
         * @param err
         *      The Windows error code (from GetLastError()).
         * 
         * @param alloc
         *      Allocator for the returned string.
         * 
         * @return
         *      Error message string.
         * 
         * @note
         *      Uses FORMAT_MESSAGE_FROM_SYSTEM to retrieve system error strings.
         *      Messages are returned in English (US).
         */
        string get_message(int err, tca::allocator* alloc) const override {
            const std::size_t MSG_BUF_SIZE = 128;
            char msg_buffer[MSG_BUF_SIZE];
            msg_buffer[0] = '\0';

            DWORD result = FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
                NULL, 
                static_cast<DWORD>(err),
                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
                msg_buffer, 
                MSG_BUF_SIZE, 
                NULL
            );

            if (result == 0)
                return "N/A";

            return string(msg_buffer, alloc);
        }

        // 
        error_condition default_error_condition(int err) const override;

    };
    #endif

} //namespace internal
} //namespace tc

#if defined(_WIN32)
#  include <windows.h>
#endif
#ifndef EROFS
#  define EROFS EINVAL
#endif
#ifndef ENAMETOOLONG
#  define ENAMETOOLONG EINVAL
#endif
#ifndef EMLINK
#  define EMLINK EINVAL
#endif
#ifndef ENOTEMPTY
#  define ENOTEMPTY EINVAL
#endif

#if defined(JSTD_OS_WINDOWS)
namespace tc
{
namespace internal
{

inline int win32_to_posix_error(int err) noexcept {
#if defined(_WIN32)
    switch (err) {
        case 0: return 0;
        // --- Доступ и права ---
        case ERROR_ACCESS_DENIED:       return EACCES;
        case ERROR_WRITE_PROTECT:        return EROFS;
        case ERROR_PRIVILEGE_NOT_HELD:
        case ERROR_CANNOT_MAKE:         return EPERM;
        case ERROR_NOACCESS:            return EFAULT;

        // --- Файлы, каталоги и пути ---
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_BAD_NETPATH:
        case ERROR_BAD_NET_NAME:
        case ERROR_BAD_PATHNAME:
        case ERROR_DEV_NOT_EXIST:
        case ERROR_MOD_NOT_FOUND:
        case ERROR_NETNAME_DELETED:
        case ERROR_INVALID_NAME:        return ENOENT;

        case ERROR_ALREADY_EXISTS:
        case ERROR_FILE_EXISTS:
        case ERROR_FILE_CORRUPT:        return EEXIST;

        case ERROR_DIRECTORY:           return ENOTDIR;
        case ERROR_DIR_NOT_EMPTY:       return ENOTEMPTY;
        case ERROR_FILENAME_EXCED_RANGE: return ENAMETOOLONG;
        case ERROR_NOT_SAME_DEVICE:     return EXDEV;
        case ERROR_TOO_MANY_LINKS:      return EMLINK;

        // --- Дескрипторы и память ---
        case ERROR_INVALID_HANDLE:      return EBADF;
        case ERROR_NOT_ENOUGH_MEMORY:
        case ERROR_OUTOFMEMORY:         return ENOMEM;
        case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
        case ERROR_NO_MORE_SEARCH_HANDLES: return ENFILE;

        // --- Аргументы и параметры ---
        case ERROR_INVALID_PARAMETER:
        case ERROR_INVALID_DATA:
        case ERROR_INVALID_ADDRESS:
        case ERROR_BAD_PIPE:
        case ERROR_BAD_USERNAME:
        case ERROR_INVALID_SIGNAL_NUMBER:
        case ERROR_META_EXPANSION_TOO_LONG:
        case ERROR_NEGATIVE_SEEK:
        case ERROR_NONE_MAPPED:
        case ERROR_NO_TOKEN:
        case ERROR_SECTOR_NOT_FOUND:
        case ERROR_SEEK:
        case ERROR_THREAD_1_INACTIVE:   return EINVAL;

        // --- Блокировки и пайпы ---
        case ERROR_BUSY:
        case ERROR_DEVICE_IN_USE:
        case ERROR_LOCK_VIOLATION:
        case ERROR_SHARING_VIOLATION:
        case ERROR_PIPE_BUSY:
        case ERROR_PIPE_CONNECTED:
        case ERROR_CHILD_NOT_COMPLETE:
        case ERROR_SERVICE_REQUEST_TIMEOUT:
        case ERROR_SIGNAL_PENDING:
        case ERROR_TIMEOUT:             return EBUSY;

        case ERROR_BROKEN_PIPE:
        case ERROR_NO_DATA:             return EPIPE;

        // --- Ввод-Вывод (I/O) и диски ---
        case ERROR_IO_DEVICE:
        case ERROR_CRC:
        case ERROR_DISK_CORRUPT:
        case ERROR_OPEN_FAILED:
        case ERROR_BEGINNING_OF_MEDIA:
        case ERROR_BUS_RESET:
        case ERROR_DEVICE_DOOR_OPEN:
        case ERROR_DEVICE_REQUIRES_CLEANING:
        case ERROR_DS_GENERIC_ERROR:
        case ERROR_EOM_OVERFLOW:
        case ERROR_FILEMARK_DETECTED:
        case ERROR_INVALID_BLOCK_LENGTH:
        case ERROR_NO_DATA_DETECTED:
        case ERROR_NO_SIGNAL_SENT:
        case ERROR_SETMARK_DETECTED:
        case ERROR_SIGNAL_REFUSED:
        case ERROR_UNEXP_NET_ERR:       return EIO;

        // --- Место на диске и квоты ---
#ifdef ENOSPC
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:
        case ERROR_END_OF_MEDIA:        return ENOSPC;
#else
        case ERROR_DISK_FULL:
        case ERROR_HANDLE_DISK_FULL:
        case ERROR_END_OF_MEDIA:        return EINVAL;
#endif
        case ERROR_NO_SYSTEM_RESOURCES: return EFBIG;

        // --- Исполняемые файлы (EXE) ---
        case ERROR_BAD_EXE_FORMAT:
        case ERROR_EXE_MACHINE_TYPE_MISMATCH:
        case ERROR_EXE_MARKED_INVALID:
        case ERROR_INVALID_EXE_SIGNATURE:
        case ERROR_IOPL_NOT_ENABLED:   return ENOEXEC;

        // --- Не поддерживается ---
        case ERROR_BAD_NET_RESP:
        case ERROR_CALL_NOT_IMPLEMENTED:
        case ERROR_NOT_SUPPORTED:       return ENOSYS;

        // --- Ошибки оборудования ---
        case ERROR_BAD_DEVICE:
        case ERROR_BAD_UNIT:
        case ERROR_INVALID_DRIVE:       return ENODEV;

        case ERROR_FILE_INVALID:        return ENXIO;

        // --- Асинхронность и ресурсы ---
        case ERROR_ACTIVE_CONNECTIONS:
        case ERROR_COMMITMENT_LIMIT:
        case ERROR_IO_INCOMPLETE:
        case ERROR_IO_PENDING:
        case ERROR_MAX_THRDS_REACHED:
        case ERROR_NONPAGED_SYSTEM_RESOURCES:
        case ERROR_NO_PROC_SLOTS:
        case ERROR_OPEN_FILES:
        case ERROR_PAGED_SYSTEM_RESOURCES:
        case ERROR_PAGEFILE_QUOTA:
        case ERROR_WORKING_SET_QUOTA:   return EAGAIN;

        // --- Разное ---
        case ERROR_CANCELLED:
        case ERROR_INVALID_AT_INTERRUPT_TIME: return EINTR;

        case ERROR_MORE_DATA:           return EMSGSIZE;
        case ERROR_POSSIBLE_DEADLOCK:   return EDEADLK;
        case ERROR_PROCESS_ABORTED:     return EFAULT;
        case ERROR_PROC_NOT_FOUND:      return ESRCH;
        case ERROR_SHARING_BUFFER_EXCEEDED: return ENOLCK;

        default: return 0;
    }
#else
    return 0;
#endif
}

    error_condition windows_category::default_error_condition(int err) const {
        if (err == 0) {
            return error_condition(0, generic_category());
        }

    #if defined(_WIN32) && !defined(__CYGWIN__)
        int posix_val = win32_to_posix_error(err);
        if (posix_val != 0) {
            return error_condition(posix_val, generic_category());
        }
    #else
        // Список базовых кодов POSIX с защитными #ifdef
        switch (err) {
    #ifdef E2BIG
            case E2BIG:
    #endif
    #ifdef EACCES
            case EACCES:
    #endif
    #ifdef EADDRINUSE
            case EADDRINUSE:
    #endif
    #ifdef EADDRNOTAVAIL
            case EADDRNOTAVAIL:
    #endif
    #ifdef EAFNOSUPPORT
            case EAFNOSUPPORT:
    #endif
    #ifdef EAGAIN
            case EAGAIN:
    #endif
    #ifdef EALREADY
            case EALREADY:
    #endif
    #ifdef EBADF
            case EBADF:
    #endif
    #ifdef EBADMSG
            case EBADMSG:
    #endif
    #ifdef EBUSY
            case EBUSY:
    #endif
    #ifdef ECANCELED
            case ECANCELED:
    #endif
    #ifdef ECHILD
            case ECHILD:
    #endif
    #ifdef ECONNABORTED
            case ECONNABORTED:
    #endif
    #ifdef ECONNREFUSED
            case ECONNREFUSED:
    #endif
    #ifdef ECONNRESET
            case ECONNRESET:
    #endif
    #ifdef EDEADLK
            case EDEADLK:
    #endif
    #ifdef EDESTADDRREQ
            case EDESTADDRREQ:
    #endif
            case EDOM:
    #ifdef EEXIST
            case EEXIST:
    #endif
    #ifdef EFAULT
            case EFAULT:
    #endif
    #ifdef EFBIG
            case EFBIG:
    #endif
    #ifdef EHOSTUNREACH
            case EHOSTUNREACH:
    #endif
    #ifdef EIDRM
            case EIDRM:
    #endif
            case EILSEQ:
    #ifdef EINPROGRESS
            case EINPROGRESS:
    #endif
    #ifdef EINTR
            case EINTR:
    #endif
    #ifdef EINVAL
            case EINVAL:
    #endif
    #ifdef EIO
            case EIO:
    #endif
    #ifdef EISCONN
            case EISCONN:
    #endif
    #ifdef EISDIR
            case EISDIR:
    #endif
    #ifdef ELOOP
            case ELOOP:
    #endif
    #ifdef EMFILE
            case EMFILE:
    #endif
    #ifdef EMLINK
            case EMLINK:
    #endif
    #ifdef EMSGSIZE
            case EMSGSIZE:
    #endif
    #ifdef ENAMETOOLONG
            case ENAMETOOLONG:
    #endif
    #ifdef ENETDOWN
            case ENETDOWN:
    #endif
    #ifdef ENETRESET
            case ENETRESET:
    #endif
    #ifdef ENETUNREACH
            case ENETUNREACH:
    #endif
    #ifdef ENFILE
            case ENFILE:
    #endif
    #ifdef ENOBUFS
            case ENOBUFS:
    #endif
    #ifdef ENODATA
            case ENODATA:
    #endif
    #ifdef ENODEV
            case ENODEV:
    #endif
    #ifdef ENOENT
            case ENOENT:
    #endif
    #ifdef ENOEXEC
            case ENOEXEC:
    #endif
    #ifdef ENOLCK
            case ENOLCK:
    #endif
    #ifdef ENOLINK
            case ENOLINK:
    #endif
    #ifdef ENOMEM
            case ENOMEM:
    #endif
    #ifdef ENOMSG
            case ENOMSG:
    #endif
    #ifdef ENOPROTOOPT
            case ENOPROTOOPT:
    #endif
    #ifdef ENOSPC
            case ENOSPC:
    #endif
    #ifdef ENOSR
            case ENOSR:
    #endif
    #ifdef ENOSTR
            case ENOSTR:
    #endif
    #ifdef ENOSYS
            case ENOSYS:
    #endif
    #ifdef ENOTCONN
            case ENOTCONN:
    #endif
    #ifdef ENOTDIR
            case ENOTDIR:
    #endif
    #if defined ENOTEMPTY && (!defined EEXIST || ENOTEMPTY != EEXIST)
            case ENOTEMPTY:
    #endif
    #ifdef ENOTRECOVERABLE
            case ENOTRECOVERABLE:
    #endif
    #ifdef ENOTSOCK
            case ENOTSOCK:
    #endif
    #if defined ENOTSUP && (!defined ENOSYS || ENOTSUP != ENOSYS)
            case ENOTSUP:
    #endif
    #ifdef ENOTTY
            case ENOTTY:
    #endif
    #ifdef ENXIO
            case ENXIO:
    #endif
    #if defined EOPNOTSUPP && (!defined ENOTSUP || EOPNOTSUPP != ENOTSUP)
            case EOPNOTSUPP:
    #endif
    #ifdef EOVERFLOW
            case EOVERFLOW:
    #endif
    #ifdef EOWNERDEAD
            case EOWNERDEAD:
    #endif
    #ifdef EPERM
            case EPERM:
    #endif
    #ifdef EPIPE
            case EPIPE:
    #endif
    #ifdef EPROTO
            case EPROTO:
    #endif
    #ifdef EPROTONOSUPPORT
            case EPROTONOSUPPORT:
    #endif
    #ifdef EPROTOTYPE
            case EPROTOTYPE:
    #endif
            case ERANGE:
    #ifdef EROFS
            case EROFS:
    #endif
    #ifdef ESPIPE
            case ESPIPE:
    #endif
    #ifdef ESRCH
            case ESRCH:
    #endif
    #ifdef ETIME
            case ETIME:
    #endif
    #ifdef ETIMEDOUT
            case ETIMEDOUT:
    #endif
    #ifdef ETXTBSY
            case ETXTBSY:
    #endif
    #if defined EWOULDBLOCK && (!defined EAGAIN || EWOULDBLOCK != EAGAIN)
            case EWOULDBLOCK:
    #endif
    #ifdef EXDEV
            case EXDEV:
    #endif
            return error_condition(err, generic_category());
        default:
            break;
        }
    #endif

        // Если прямого соответствия с POSIX нет
        return error_condition(err, *this);
    }

} //namespace internal
} //namespace tc
#endif

namespace tc
{
    const error_category& generic_category() {
        static const internal::errno_category errno_cat;
        return errno_cat;
    }

    const error_category& system_category() {
        JSTD_WIN_CODE (
            static const internal::windows_category system_cat;
            return system_cat;
        )
        return generic_category();   
    }
}

namespace tc
{
    error_code::error_code() : val(0), category(&system_category()) {

    }
    
    error_code::error_code(int val, const error_category& ecat) : val(val), category(&ecat) {

    }

    error_category::~error_category() {

    }
} //namespace tc

namespace tc
{
    error_condition::error_condition() : error_condition(0, generic_category()) {

    }
    
    error_condition::error_condition(int err, const error_category& ecat) : val(err), category(&ecat) {

    }

} //namespace tc

//#################################################### ./src/exceptions.cpp ####################################################

#include <cpp/lang/exceptions.hpp>
#include <utility>

namespace tc 
{

    throwable::throwable() noexcept : m_calltrace() {
        _cause[0] = 0;
    }

    throwable::throwable(const char* cause) noexcept : m_calltrace() {
        std::size_t len         = std::strlen(cause);
        std::size_t max_size    = sizeof(_cause);
        std::size_t max         = len >= max_size ? max_size - 1 : len;
        std::memcpy(_cause, cause, max);
        _cause[max] = 0; 
    }
    
    throwable::throwable(const throwable& t) noexcept : m_calltrace(t.m_calltrace) {
        if (&t != this)
            std::memcpy(_cause, t._cause, sizeof(_cause));
    }
    
    throwable::throwable(throwable&& t) noexcept : m_calltrace(std::move(t.m_calltrace)) {
        if (&t != this)
            std::memcpy(_cause, t._cause, sizeof(_cause));
    }
    
    throwable& throwable::operator= (const throwable& t) noexcept {
        if (&t != this)
        {
            m_calltrace = t.m_calltrace;
            std::memcpy(_cause, t._cause, sizeof(_cause));
        }
        return *this;
    }
    
    throwable& throwable::operator= (throwable&& t) noexcept {
        if (&t != this)
        {
            m_calltrace = std::move(t.m_calltrace);
            std::memcpy(_cause, t._cause, sizeof(_cause));
        }
        return *this;
    }
    
    throwable::~throwable() noexcept {

    }
    
    const char* throwable::cause() const noexcept {
        return _cause;
    }

    void throwable::print_stack_trace() const {
        m_calltrace.print();
    }
    
    const calltrace& throwable::get_calltrace() const {
        return m_calltrace;
    }
    
    void throwable::set_calltrace(calltrace&& calltrace) {
        m_calltrace = std::move(calltrace);
    }

#define TEMPLATE__EXCEPT_CLASS_IMPL(clazz_name, super_clazz)\
    clazz_name::clazz_name() noexcept : super_clazz() {}\
    clazz_name::clazz_name(const char* cause) noexcept : super_clazz(cause) {}\
                   
TEMPLATE__EXCEPT_CLASS_IMPL(error,                               throwable)
TEMPLATE__EXCEPT_CLASS_IMPL(out_of_memory_error,                 error)
TEMPLATE__EXCEPT_CLASS_IMPL(exception,                           throwable)
TEMPLATE__EXCEPT_CLASS_IMPL(runtime_exception,                   exception)
TEMPLATE__EXCEPT_CLASS_IMPL(null_pointer_exception,              runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(security_exception,                  runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(interrupted_exception,               exception)

/**
 * #################################################################
 *                  R A N G E  E X C E P T I O N S
 * #################################################################
 */
TEMPLATE__EXCEPT_CLASS_IMPL(index_out_of_bound_exception,        runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(overflow_exception,                  runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(underflow_exception,                 runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(no_such_element_exception,           runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(stack_overflow_error,                error)

/**
 * #################################################################
 *                  S T A T E  E X C E P T I O N S
 * #################################################################
 */
TEMPLATE__EXCEPT_CLASS_IMPL(illegal_argument_exception,          runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(illegal_state_exception,             runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(invalid_mark_exception,              illegal_state_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(unsupported_operation_exception,             exception)
TEMPLATE__EXCEPT_CLASS_IMPL(utf_format_exception,                runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(invalid_data_format_exception,       runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(number_format_exception,             runtime_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(class_cast_exception,                runtime_exception)

/**
 * #################################################################
 *                  I O E X C E P T I O N S
 * #################################################################
 */
TEMPLATE__EXCEPT_CLASS_IMPL(io_exception,                        exception)
TEMPLATE__EXCEPT_CLASS_IMPL(file_not_found_exception,            io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(eof_exception,                       io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(closed_exception,                    io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(readonly_exception,                  io_exception)

//socket exceptions
TEMPLATE__EXCEPT_CLASS_IMPL(socket_exception,                    io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(unknow_host_exception,               io_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(connect_exception,                   socket_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(bind_exception,                      socket_exception)
TEMPLATE__EXCEPT_CLASS_IMPL(socket_timeout_exception,            socket_exception)


    

}

//#################################################### ./src/file.cpp ####################################################

#include <cpp/lang/io/file.hpp>
#include <cpp/lang/io/file_filter.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/math.hpp>
#include <cstring>
#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <utility>
#include <cassert>

namespace tc
{

    file::file() {
        _path[0] = '\0';
    }

    file::file(const char* path, std::size_t path_length) {
        path_length = path_length == npos() ? std::strlen(path) : path_length;
        
        const std::size_t BUF_SIZE  = sizeof(_path) - 1;
        std::size_t result_size     = math::min(BUF_SIZE, path_length);
        
        std::memcpy(_path, path, result_size);
        _path[result_size] = '\0';
        
        filesystem::normalize_path(_path, result_size);
    }
    
    file::file(const file& f) {
        std::memcpy(_path, f._path, sizeof(_path));
    }
    
    file::file(file&& f) {
        std::memcpy(_path, f._path, sizeof(_path));
      	f._path[0] = '\0';
    }
    
    file& file::operator= (const file& f) {
        if (&f != this)
            std::memcpy(_path, f._path, sizeof(_path));
        return *this;
    }
    
    file& file::operator= (file&& f) {
        if (&f != this)
	{
            std::memcpy(_path, f._path, sizeof(_path));
	    f._path[0] = '\0';
	}
        return *this;
    }
    
    file::~file() {
        
    }
    
    bool file::is_file() const {
        if (_path[0] == '\0')
            return false;
        return filesystem::is_file(_path);
    }

    bool file::exists() const {
        if (_path[0] == '\0')
            return filesystem::exists(".");
        return filesystem::exists(_path);
    }

    bool file::is_dir() const {
        if (_path[0] == '\0')
            return filesystem::is_dir(".");
        return filesystem::is_dir(_path);
    }

    bool file::mkdir() {
        if (_path[0] == '\0')
            return true;
        return filesystem::mkdir(_path);
    }

    bool file::mkdirs() {
        if (_path[0] == '\0')
            return true;
        return filesystem::mkdirs(_path);
    }

    std::uintmax_t file::length() const {
        return filesystem::length(_path);
    }

    timepoint file::last_modified() const {
        return filesystem::last_modified(_path);
    }

    bool file::set_last_modified(uint64_t ms) {
        return filesystem::set_last_modified(_path, ms);   
    }

    bool file::is_absolute_path() const {
        return filesystem::is_absolute(_path);
    }

    bool file::create_new_file() {
        return filesystem::create_new_file(_path);
    }

    bool file::remove() {
        return filesystem::remove(_path);
    }

    bool file::rename_to(const char* new_name) {
        return filesystem::rename_to(_path, new_name);
    }

    bool file::can_execute() const {
        return filesystem::can_execute(_path);
    }

    bool file::can_read() const {
        return filesystem::can_read(_path);
    }

    bool file::can_write() const {
        return filesystem::can_write(_path);
    }

    bool file::set_executable(bool on_off) {
        return filesystem::set_executable(_path, on_off);
    }

    bool file::set_readable(bool on_off) {
        return filesystem::set_readable(_path, on_off);
    }

    bool file::set_writable(bool on_off) {
        return filesystem::set_writable(_path, on_off);
    }

    const char* file::c_str() const {
        return _path;
    }

    tc::string file::get_name(tca::allocator* alloc) const {
        tc::string str_path(alloc);
     
        // Временный буфер для нормализованного пути.
        char path[sizeof(_path)];
        std::memcpy(path, _path, sizeof(path));
        filesystem::normalize_path(path);

        // Поиск индекса, откуда начинается название файла
        const std::size_t len = std::strlen(path);
        std::size_t idx = 0;
        for (std::size_t i = len; i > 0; ) {
            --i;
            if (filesystem::is_separator(path[i]))
            {
                idx = i + 1;
                break;    
            }
        }

        assert(len >= idx);
        str_path.append(path + idx, len - idx);

        return tc::string(std::move(str_path));
    }

    array<file> file::list_files(const file_filter& filter, tca::allocator* allocator) const {

        const char* path = _path[0] != '\0' ? _path : ".";

        std::size_t count_files = filesystem::count_files_in_directory(path, filter);
        
        array<file> files(count_files, allocator);

        directory_iterator begin(path);
        directory_iterator end;

        std::size_t i = 0;
        char path_buf[filesystem::MAX_LENGTH_PATH];
        while (begin != end) {
            
            directory_entry entry   = *begin;
            std::size_t path_length = std::strlen(entry.get_name());
            
            if (filter.apply(entry.get_name(), path_length))
            {
                if (_path[0] != '\0')
                    std::snprintf(path_buf, sizeof(path_buf), "%s/%s", path, entry.get_name());
                else
                    std::snprintf(path_buf, sizeof(path_buf), "%s", entry.get_name());
                files[i++]  = file(path_buf);
            }
            
            ++begin;
        }

        return array<file>(std::move(files));
    }

    /*static*/ file file::runtime_path() {
        char buf[filesystem::MAX_LENGTH_PATH];
        std::size_t sz = filesystem::runtime_path(buf, sizeof(buf));
        return file(buf, sz);
    }

    /*static*/ file file::make(const char* format, ...) {
        va_list args;
        char buf[filesystem::MAX_LENGTH_PATH];
        
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        
        return file(buf);
    }

    /*static*/ file file::make_absolute(const char* format, ...) {
        char rt_path[filesystem::MAX_LENGTH_PATH];
        std::size_t len = filesystem::runtime_path(rt_path, sizeof(rt_path));
        
        if (len + 1 > (int) sizeof(rt_path))
            throw_except<illegal_state_exception>("buffer is small!");
        
        rt_path[len++] = filesystem::FILE_SEPARATOR;
        
        assert(sizeof(rt_path) >= len);
        std::size_t free_space = sizeof(rt_path) - len;
        
        {
            va_list args;
            va_start(args, format);
            std::vsnprintf(rt_path + len, free_space, format, args);
            va_end(args);
        }
        
        return file(rt_path);
    }

    file file::get_parent() const {
        char parent_path_buf[filesystem::MAX_LENGTH_PATH];
        filesystem::get_parent(_path, parent_path_buf, sizeof(parent_path_buf));
        return file(parent_path_buf);
    }

    std::size_t file::hashcode() const {
        return objects::hashcode(_path, _path + std::strlen(_path), hash_for<char>());
    }

    file file::plus(const file& f) const {
        char buf[io::constants::MAX_LENGTH_PATH];
        std::snprintf(buf, sizeof(buf), "%s/%s", _path, f._path);
        return file(buf);
    }

    bool file::equals(const file& f) const {
        const char* p1 = _path;
        const char* p2 = f._path;
        while (*p1 && *p2)
            if (*(p1++) != *(p2++))
                return false;
        return true;
    }
}

//#################################################### ./src/filesystem.cpp ####################################################

#include <allocators/inline_linear_allocator.hpp>
#include <internal/io/io_helpers.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/utils/coder.hpp>
#include <cpp/lang/string.hpp>
#include <cpp/lang/math.hpp>
#include <cerrno>
#include <cassert>

namespace tc
{
    using fs = filesystem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      C   R   O   S   S   P   L   A   T   F   O   R   M    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace tc
{
    /**
     * 
     */
    static std::size_t normlen(const char* path, std::size_t len) {
        return len != fs::npos() ? len : std::strlen(path);
    }

    /**
     * 
     */
    bool fs::is_separator(char ch) {
        return ch == '\\' || ch == '/';
    }
    
    /**
     * Заменяет двойной разделитель, на одинарный
     * /test///aa//bb//ccc" преобразутеся в /test/aa/bb/cc
     * 
     * Но если путь сетевой
     * //test///aa//bb//ccc" преобразутеся в //test/aa/bb/cc
     * 
     */
    static std::size_t remove_double_separators(char path[], std::size_t path_length) {
        std::size_t wp = 0;
        std::size_t rp = 0;
        
        if (path_length > 2)
        {
            //проверка на то, что это сетевой путь. То есть, путь начинающийся на "\\" - "\\bla\bla\bla"
            if (fs::is_separator(path[rp]) && fs::is_separator(path[rp + 1]))
            {
                rp += 2, wp += 2;
            }
        }
        
        while (rp < path_length)
        {    
            if (fs::is_separator(path[rp]) && (rp + 1) >= path_length)
            {
                ++rp;
                continue;
            }

            if (rp + 1 < path_length)
            {
                if (fs::is_separator(path[rp]) && fs::is_separator(path[rp + 1]))
                {
                    ++rp;
                    continue;
                }
            }

            path[wp++] = path[rp++];
        }

        return wp;
    }
    
    /**
     * 
     */
    std::size_t fs::normalize_path(char path[], std::size_t path_length) {
        path_length = remove_double_separators(path, normlen(path, path_length));
        std::size_t wp = 0;
        std::size_t rp = 0;

        const std::size_t plen = path_length;

        while (rp < plen)
        {
            if (rp != 0 && (rp + 1 < plen) && path[rp] == '.' && is_separator(path[rp + 1]))
            {
                ++rp;
                if (wp > 0 && is_separator(path[wp - 1]))
                    --wp;
                continue;
            }
            //  C:/user/folder/../test
            else if ((rp + 2) < plen)
            {
                if (path[rp] == '.' && path[rp + 1] == '.' && is_separator(path[rp + 2]))
                {
                    rp += 3;
                    if (wp > 0)
                    {
                        --wp;
                        while (wp > 0 && !is_separator(path[wp - 1]))
                            --wp;
                    }
                    continue;
                }
            }
         
            path[wp++] = path[rp++];
        }
        path[wp] = '\0';
        return wp;
    }
    
    #if 0
    /**
     * Заменяет все разделители на константу зависящую от системы.
     * @see filesystem::FILE_SEPARATOR
     */
    static void replace_separators_to_system_separators(char path[], std::size_t path_length) {
        for (std::size_t i = 0; i < path_length; ++i) {
            if (fs::is_separator(path[i]))
                path[i] = filesystem::FILE_SEPARATOR;
        }
    }
    #endif

    /**
     * 
     */
    std::size_t get_parent_from_absolute(char path[], std::size_t path_length = filesystem::npos()) {
        path_length = normlen(path, path_length);
        for (std::size_t i = path_length; i > 0; )
        {
            if (fs::is_separator(path[--i]))
            {
                path[i] = '\0';
                return i;
            }
        }
        return 0;
    }
    
    /**
     * 
     */
    std::size_t fs::get_parent(const char* path, char out_path[], std::size_t buf_size) {
        if (buf_size == 0) return 0;
        
        std::size_t len = std::strlen(path);
        
        //Вычисляем максимальную длину выходной строки, с учётом 0-терминатора.
        std::size_t max_path_size    = math::min(buf_size - 1, len);
        
        if (max_path_size == 0)
        {
            out_path[0] = '\0';
            return 0;
        }

        //Путь в буфер
        std::memcpy(out_path, path, max_path_size);
        
        //Добавляем 0-териминатор
        out_path[max_path_size] = '\0';
        
        //Нормализация пути, для удаления мусора (/////, ./././)
        len = fs::normalize_path(out_path);
        
        return get_parent_from_absolute(out_path, len);
    }

    bool fs::mkdirs(const char* path)
    {
        std::size_t len = std::strlen(path);

        if (len == 0)
            return true;

        std::size_t start = 0;
        if (is_absolute(path))
        {
            if (is_separator(path[0]))
            {
                start = 1;
            }
            else if (len >= 3 && path[1] == ':' && is_separator(path[2]))
            {
                start = 3;
            }
        }


        char part_buffer[io::constants::MAX_LENGTH_PATH];
        for (std::size_t i = start; i < len; ++i)
        {
            if (is_separator(path[i]) || i + 1 == len)
            {
                std::size_t part_length = i + 1;

                if (part_length >= io::constants::MAX_LENGTH_PATH)
                    throw_except<io_exception>("Path very large '%s'", path);
                
                std::memcpy(part_buffer, path, part_length);
                part_buffer[part_length] = '\0';

                if (!exists(part_buffer))
                {
                    if (!mkdir(part_buffer))
                        return false;
                }
            }
        }

        return true;
    }

    std::size_t filesystem::count_files_in_directory(const char* path, const file_filter& filter) {
        directory_iterator begin(path);
        directory_iterator end;
        std::size_t count_files = 0;
        while (begin != end) {
            
            directory_entry entry = *begin;
            
            const char* name    = entry.get_name();
            std::size_t len     = std::strlen(name);
            
            if (filter.apply(name, len))
                ++count_files;            

            ++begin;
        }
        return count_files;
    }

    bool fs::can_execute(const char* path) {
        return get_access(path, CAN_EXECUTE);
    }
    
    bool filesystem::can_read(const char* path) {
        return get_access(path, CAN_READ);
    }
    
    bool filesystem::can_write(const char* path) {
        return get_access(path, CAN_WRITE);
    }

    bool filesystem::set_executable(const char* path, bool on_off) {
        return set_access(path, CAN_EXECUTE, on_off);
    }

    bool filesystem::set_readable(const char* path, bool on_off) {
        return set_access(path, CAN_READ, on_off);
    }
    
    bool filesystem::set_writable(const char* path, bool on_off) {
        return set_access(path, CAN_WRITE, on_off);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          D   I   R   E   N   T   R   Y   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace tc
{
    directory_entry::directory_entry(const char* path, std::size_t path_length) {
        path_length      = normlen(path, path_length);
        std::size_t size = math::min(sizeof(_name) - 1, path_length);
        std::memcpy(_name, path, size);
        _name[size] = 0;
    }
    
    directory_entry::directory_entry(const directory_entry& ent) {
        std::memcpy(_name, ent._name, sizeof(_name));
    }
    
    directory_entry::directory_entry(directory_entry&& ent) {
        std::memcpy(_name, ent._name, sizeof(_name));
    }
    
    directory_entry& directory_entry::operator= (const directory_entry& ent) {
        if (&ent != this)
            std::memcpy(_name, ent._name, sizeof(_name));
        return *this;
    }
    
    directory_entry& directory_entry::operator= (directory_entry&& ent) {
        if (&ent != this)
            std::memcpy(_name, ent._name, sizeof(_name));
        return *this;
    }
    
    directory_entry::~directory_entry() {

    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  P   O   S   I   X
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(JSTD_OS_MAC) || defined(JSTD_OS_LINUX) || defined(JSTD_OS_UNIX)
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include <utime.h>
#include <cerrno>

#define READ_OWNER 		        S_IRUSR 	//Чтение для владельца
#define WRITE_OWNER 		    S_IWUSR 	//Запись для владельца
#define EXECUTABLE_OWNER 	    S_IXUSR	    //Выполнение для владельца

#define READ_GROUP 		        S_IRGRP 	//Чтение для группы
#define WRITE_GROUP 		    S_IWGRP 	//Запись для группы
#define EXECUTABLE_GROUP 	    S_IXGRP	    //Выполнение для группы

#define READ_OTHER 		        S_IROTH 	//Чтение для остальных
#define WRITE_OTHER 		    S_IWOTH 	//Запись для остальных
#define EXECUTABLE_OTHER 	    S_IXOTH	    //Выполнение для остальных

#define NIX_READ              (READ_OWNER | READ_GROUP | READ_OTHER)
#define NIX_WRITE             (WRITE_OWNER | WRITE_GROUP | WRITE_OTHER)
#define NIX_EXECUTE           (EXECUTABLE_OWNER | EXECUTABLE_GROUP | EXECUTABLE_OTHER)
#define PERMISSION_ALL        (NIX_READ | NIX_WRITE | NIX_EXECUTE)

namespace tc
{

    static void throw_error() {
        assert(errno != 0);
        internal::io::throw_error_code(error_code(errno, system_category()));
    }

    static void init_stat(const char* path, struct stat* filestat) {
        if (stat(path, filestat) != 0)
            throw_error();
    }
    
    std::size_t fs::runtime_path(char rbuf[], std::size_t bufsize) {
        ssize_t len = readlink("/proc/self/exe", rbuf, bufsize);
        if (len == -1) throw_error();
        for (ssize_t i = len; i > 0; ) {
            --i;
            if (rbuf[i] == '/') {
                rbuf[i] = '0';
                return (std::size_t) i;
            }
        }
        return (std::size_t) len;  
    }

    bool fs::is_absolute(const char* path) {
        std::size_t len = std::strlen(path);
        if (len < 1)
            return false;
        if (path[0] == '/' || path[0] == '\\')
            return true;
        return false;
    }

    bool fs::exists(const char* path){
        struct stat filestat;
		return stat(path, &filestat) == 0;
	}

    bool fs::is_file(const char* path){
        if (!exists(path))
            return false;
        
        struct stat filestat;
        init_stat(path, &filestat);
        
        return S_ISREG(filestat.st_mode);
    }

    bool fs::is_dir(const char* path){
        if (!exists(path))
            return false;
        
        struct stat filestat;
        init_stat(path, &filestat);
        
        return S_ISDIR(filestat.st_mode);
    }

    std::uintmax_t fs::length(const char* path) {    
        struct stat fstat;
        if (stat(path, &fstat) == 0)
            return static_cast<std::uintmax_t>(fstat.st_size);
        return 0;
    }

    timepoint fs::last_modified(const char* path) {        
        if (!fs::exists(path))
            return 0;

        struct stat filestat;
        init_stat(path, &filestat);
		
        struct timespec& sTime = filestat.st_mtim;
        
        return (timepoint) ( (sTime.tv_nsec / 1000000) + (sTime.tv_sec * 1000) );
    }

    bool fs::set_last_modified(const char* path, timepoint ms_time) {
        if (!fs::exists(path))
            return 0;
        
        struct stat fstat;
        init_stat(path, &fstat);
        
        utimbuf timeBuf;
        timeBuf.actime  = fstat.st_atim.tv_sec;
        timeBuf.modtime = ms_time / 1000;  //ms to sec
        
        if (utime(path, &timeBuf) != 0)
            throw_error();
        
        return true;
    }

    bool fs::create_new_file(const char* path) {
        //       \/ это чёртово двойное двоеточие нужно, т.к иначе компилятор будет визжать, 
        //          из-за попытки вызвать filesystem::open(const char*, const char*, int, int).
        int fd = ::open(path, O_CREAT, PERMISSION_ALL);
        if (fd == -1)
        {
            if (errno == EEXIST)
                return false;
            throw_error();
        }
        
        return true;
    }

    bool fs::mkdir(const char* path) {
        if (::mkdir(path, PERMISSION_ALL) != 0)
            throw_error();
        return true;
    }

    bool fs::remove(const char* path) {
		if (!fs::exists(path))
            return false;

        if (::remove(path) != 0)
			throw_error();

		return true;
    }

    bool fs::rename_to(const char* old_path, const char* new_path) {
		
        if (!fs::exists(old_path))
            return false;
        
        if (rename(old_path, new_path) != 0)
            throw_error();
		
        return true;
    }

    bool fs::get_access(const char* path, unsigned int mode){

        int os_mode = 0;
        if (mode & fs::CAN_READ)    os_mode |= R_OK;
        if (mode & fs::CAN_WRITE)   os_mode |= W_OK;
        if (mode & fs::CAN_EXECUTE) os_mode |= X_OK;

        if (access(path, os_mode) != 0)
            return false;

		return true;
	}

    bool fs::set_access(const char* path, unsigned int mode, bool on_off) {
        if (!fs::exists(path))
            return false;
        
        struct stat filestat;
        init_stat(path, &filestat);

        if (mode == fs::CAN_READ)    mode = S_IRUSR;
        if (mode == fs::CAN_WRITE)   mode = S_IWUSR;
        if (mode == fs::CAN_EXECUTE) mode = S_IXUSR;

        unsigned int new_mode = on_off ?
                                        (filestat.st_mode | mode) : (~mode & filestat.st_mode);
        
        if (chmod(path, new_mode) != 0)
            throw_error();

        return true;
    }

    expected<FILE*, error_code> fs::open(const char* path, const char* mark) {
        FILE* handle = fopen(path, mark);   
        if (!handle)
        {
            return unexpected(error_code(errno, generic_category()));
        }
        return handle;
    }

    int fs::open_fd(const char* path, int oflags, int pmode) {
        
        int fd = ::open(path, oflags, pmode);
        if (fd == -1)
        {
            if (errno == EACCES)
                throw_except<security_exception>(std::strerror(errno));
            throw_except<io_exception>(std::strerror(errno));
        }
    
        return fd;
    }

    void fs::close_fd(int fd) {
        if (::close(fd) != 0)
        {
            throw_except<io_exception>(std::strerror(errno));
        }
    }
    
}


namespace tc
{
    directory_iterator::directory_iterator(const char* path) : _dir(nullptr), _entry(nullptr) {
        if (path != nullptr)
        {
            _dir = opendir(path);
            if (_dir == nullptr) {
                if (errno == EACCES) 
                    throw_error();
            }
            else
            {
                this->operator++();
            }
        }
    }
    
    directory_iterator::directory_iterator(directory_iterator&& it) : _dir(it._dir), _entry(it._entry) {
        it._dir     = nullptr;
        it._entry   = nullptr;
    }
    
    directory_iterator& directory_iterator::operator= (directory_iterator& it) {
        if (&it != this)
        {
            close();
            _dir        = it._dir;
            _entry      = it._entry;
            it._dir     = nullptr;
            it._entry   = nullptr;
        }
        return *this;
    }
    
    void directory_iterator::close() {
        if (_dir != nullptr)
        {
            closedir(_dir);
            _dir    = nullptr;
            _entry  = nullptr;
        }
    }

    directory_iterator::~directory_iterator() {
        close();
    }

    bool directory_iterator::operator!= (const directory_iterator& it) const {
        return _entry != it._entry;
    }

    static bool is_current_or_prev_folder(const char* path, std::size_t len) {
        return (len == 1 && path[0] == '.') || (len == 2 && path[0] == '.' && path[1] == '.');
    }
    
    directory_iterator& directory_iterator::operator++() {
        while (true) {
            _entry = readdir(_dir);
            if (_entry == nullptr)
                break;
            std::size_t len = std::strlen(_entry->d_name);
            if (is_current_or_prev_folder(_entry->d_name, len))
                continue;
            break;
        }
        return *this;
    }
    
    directory_entry directory_iterator::operator*() const {
        return directory_entry(_entry->d_name);
    }
}


#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              W   I   N   D   O   W   S
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(JSTD_OS_WINDOWS)
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#define WCHAR_BYTE_ORDER byte_order::LE
namespace tc
{
    
    static void throw_error() {
        DWORD lastErr = GetLastError();
        assert(lastErr != ERROR_SUCCESS);
        internal::io::throw_error_code(error_code(lastErr, system_category()));
    }
    
    static std::size_t wchar_to_char(const wchar_t* wstr, std::size_t wstrlen, char str[], std::size_t strlen) {
        return utf::u16_to_u8(wstr, wstrlen, str, strlen, WCHAR_BYTE_ORDER);
    }
    
    static std::size_t char_to_wchar(const char* str, std::size_t strlen, wchar_t wstr[], std::size_t wstrlen) {
        return utf::u8_to_u16(str, strlen, wstr, wstrlen, WCHAR_BYTE_ORDER);
    }

    static DWORD get_file_attrib(const char* path) {
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
        
        return GetFileAttributesW(wbuf);
    }

    std::size_t fs::runtime_path(char buf[], std::size_t bufsize) {
        
        // GetModuleFileNameW
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        std::size_t len = (std::size_t) GetModuleFileNameW(NULL, wbuf, (DWORD) io::constants::MAX_LENGTH_PATH);
        
        //Удаление имени исполняемого файла
        for (std::size_t i = len; i > 0; )
        {
            --i;
            if (wbuf[i] == '\\' || wbuf[i] == '/')
            {
                len = i;
                break;
            }
        }
        
        return wchar_to_char(wbuf, len, buf, bufsize);
    }
    
    bool fs::is_absolute(const char* path) {
        std::size_t len = std::strlen(path);
        
        if (len < 3)
            return false;
        
        char tom_name           = path[0];
        char double_dots        = path[1];
        char slash              = path[2];
        
        if ((tom_name < 'a' || tom_name > 'z')) return false;
        if ((tom_name < 'A' || tom_name > 'Z')) return false;
        if (double_dots != ':')                 return false;
        if (!fs::is_separator(slash))           return false;
        
        return true;
    }

    bool fs::exists(const char* path) {
        DWORD attrib = get_file_attrib(path);
        if (attrib != INVALID_FILE_ATTRIBUTES)
            return true;
		return false;
    }

    bool fs::is_file(const char* path) {
        if (!exists(path))
            return false;
        return !is_dir(path);
    }
    
    bool fs::is_dir(const char* path) {
		if (!exists(path))
            return false;
        DWORD attrib = get_file_attrib(path);
		if (attrib == INVALID_FILE_ATTRIBUTES)
			throw_error();
		return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    std::uintmax_t fs::length(const char* path) {
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        
        std::size_t len = std::strlen(path);
        char_to_wchar(path, len, wbuf, io::constants::MAX_LENGTH_PATH);
        
        struct _stat fstat;
        if (_wstat(wbuf, &fstat) == 0)
            return static_cast<std::uintmax_t>(fstat.st_size);

        return 0;
    }
    
    timepoint fs::last_modified(const char* path) {
        if (!fs::exists(path))
            return 0;
        
        wchar_t wpath[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wpath, io::constants::MAX_LENGTH_PATH);

		HANDLE hFile = CreateFileW(
                                    wpath,
                                    FILE_WRITE_ATTRIBUTES,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    nullptr,
                                    OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS,
                                    nullptr
                                );               		

		if (hFile == INVALID_HANDLE_VALUE)
        {
		    if (GetLastError() != ERROR_FILE_NOT_FOUND)
                throw_error();
            return 0;
		}

		FILETIME lastModified;
		if (!GetFileTime(hFile, NULL, NULL, &lastModified)) {
		    CloseHandle(hFile);
            throw_error();
            return 0;
		}
		CloseHandle(hFile);

		ULARGE_INTEGER time;
		time.LowPart    = lastModified.dwLowDateTime;
		time.HighPart   = lastModified.dwHighDateTime;

		return (timepoint) ((time.QuadPart / 10000) - 11644473600000U);
    }

    bool fs::set_last_modified(const char* path, timepoint ms_time) {
        
        wchar_t wpath[io::constants::MAX_LENGTH_PATH];        
        char_to_wchar(path, std::strlen(path), wpath, io::constants::MAX_LENGTH_PATH);

        HANDLE hFile = CreateFileW(
            wpath,
            FILE_WRITE_ATTRIBUTES,              // Доступ только для изменения атрибутов
            FILE_SHARE_READ | FILE_SHARE_WRITE, // Общий доступ
            NULL,                               // Защита по умолчанию
            OPEN_EXISTING,                      // Открыть только существующий файл
            FILE_FLAG_BACKUP_SEMANTICS,         // Обычный файл
            NULL                                // Без шаблона
        );

        if (hFile == INVALID_HANDLE_VALUE)
        {
             if (GetLastError() != ERROR_FILE_NOT_FOUND)
                throw_error();
            return false;
        }

        ULONGLONG fileTime  = (ms_time * 10000ULL) + 116444736000000000ULL; 
        
        FILETIME ft;
        ft.dwLowDateTime    = (DWORD) (fileTime);
        ft.dwHighDateTime   = (DWORD) (fileTime >> 32);
        
        BOOL result = SetFileTime(
            hFile,
            NULL,          // Время создания
            NULL,          // Время последнего доступа
            &ft            // Время последней модификации
        );
        
        CloseHandle(hFile); 
        
        return result; 
    }

    bool fs::create_new_file(const char* path) {
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);

        HANDLE file = CreateFileW(
            wbuf,  
            GENERIC_WRITE,                                                      // Режим доступа
            0,                                                                  // Совместный доступ
            NULL,                                                               // Атрибуты безопасности
            CREATE_NEW,                                                         // Создание нового файла, если он не существует
            FILE_ATTRIBUTE_NORMAL,                                              // Атрибуты файла
            NULL                                                                // Шаблон файла
        );

        if (file == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_EXISTS)
                return false;
            throw_error();
        }
        
        CloseHandle(file);
        return true;
    }

    bool fs::mkdir(const char* path) {

        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);

        bool created        = CreateDirectoryW(wbuf, NULL) != 0;
        bool alreadyExists  = GetLastError() == ERROR_ALREADY_EXISTS;
        
        if (created || alreadyExists) return true;
        else                          throw_error();

        //Чтобы компилятор не истерил
        return false;
    }

    bool fs::remove(const char* path) {
        if (!fs::exists(path))
            return false;

        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);

        if (fs::is_dir(path))
        {
            if (!RemoveDirectoryW(wbuf))
                throw_error();
        }
        else
        {
            if (!DeleteFileW(wbuf))
                throw_error();
        }

		return true;
    }

    bool fs::rename_to(const char* old_path, const char* new_path) {
        
        if (!fs::exists(old_path))
        {
            return false;
        }
        
        wchar_t wbuf_old[io::constants::MAX_LENGTH_PATH];
        wchar_t wbuf_new[io::constants::MAX_LENGTH_PATH];
        
        char_to_wchar(old_path, std::strlen(old_path), wbuf_old, io::constants::MAX_LENGTH_PATH);
        char_to_wchar(new_path, std::strlen(new_path), wbuf_new, io::constants::MAX_LENGTH_PATH);

        if (_wrename(wbuf_old, wbuf_new) != 0)
            throw_error();

        return true;
    }
    
    bool fs::get_access(const char* path, unsigned int mode){
        if (!fs::exists(path))
            return false;
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        
        {
            char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
        }

        struct _stat filestat;
        if (_wstat(wbuf, &filestat) != 0)
            throw_error();
        
        if (mode == fs::CAN_READ)    mode = _S_IREAD;
        if (mode == fs::CAN_WRITE)   mode = _S_IWRITE;
        if (mode == fs::CAN_EXECUTE) mode = _S_IEXEC;
    
        return filestat.st_mode & mode;
	}
    
    bool fs::set_access(const char* path, unsigned int mode, bool on_off) {
        if (!fs::exists(path))
            return false;
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
        
        struct _stat filestat;
        if (_wstat(wbuf, &filestat) != 0)
            throw_error();

        if (mode == fs::CAN_READ)    mode = _S_IREAD;
        if (mode == fs::CAN_WRITE)   mode = _S_IWRITE;
        if (mode == fs::CAN_EXECUTE) mode = _S_IEXEC;

        unsigned int new_mode = on_off ? 
                                        (filestat.st_mode | mode) : (~mode & filestat.st_mode);
        
        if (_wchmod(wbuf, (int) new_mode) != 0)
            throw_error();
        
        return true;
    }

    expected<FILE*, error_code> fs::open(const char* path, const char* mark) {
        const std::size_t MARK_BUF_SIZE = 8;

        wchar_t wbuf_path[io::constants::MAX_LENGTH_PATH];
        wchar_t wbuf_mark[MARK_BUF_SIZE];
        
        char_to_wchar(path, std::strlen(path), wbuf_path, io::constants::MAX_LENGTH_PATH);
        char_to_wchar(mark, std::strlen(mark), wbuf_mark, MARK_BUF_SIZE);

        FILE* handle = _wfopen(wbuf_path, wbuf_mark);
        
        if (!handle)
        {
            return unexpected(error_code(errno, generic_category()));
        }
        
        return handle;
    }

    error_code fs::close(FILE* handle) {
        if (fclose(handle) != 0)
            return error_code(errno, generic_category());
        return error_code();
    }

    error_code fs::flush(FILE* handle) {
        if (fflush(handle) != 0)
            return error_code(errno, generic_category());
        return error_code();
    }

    int fs::open_fd(const char* path, int oflags, int pmode) {
    
        wchar_t wpath[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wpath, io::constants::MAX_LENGTH_PATH);

        int fd = ::_wopen(wpath, oflags, pmode);
        if (fd == -1)
        {
            if (errno == EACCES)
                throw_except<security_exception>(std::strerror(errno));
            throw_except<io_exception>(std::strerror(errno));
        }
    
        return fd;
    }

    void fs::close_fd(int fd) {
        if (::_close(fd) != 0)
        {
            throw_except<io_exception>(std::strerror(errno));
        }
    }
}

namespace tc
{

    static bool is_current_or_prev_folder(const wchar_t* path) {
        std::size_t len = wstring::str_len(path);
        return (len == 1 && path[0] == '.') || (len == 2 && path[0] == '.' && path[1] == '.');
    }

    directory_iterator::directory_iterator(const char* path) : _dir(), _entry(), _end(path == nullptr ? true : false) {
        if (path != nullptr) {

            wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
            std::size_t len = char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
            
            if (len >= io::constants::MAX_LENGTH_PATH - 3)
                throw_except<io_exception>("Path very long");

            //TODO: этот кусок говна нужен только потому, что винде нужен шаблон. 
            //Если просто передать "C:\\" -- то эта чёртова система не поймёт, что нужны все папки в указанной директории.
            wbuf[len++] = (wchar_t) filesystem::FILE_SEPARATOR;
            wbuf[len++] = L'*';
            wbuf[len]   = 0;

            _dir = FindFirstFileW(wbuf, &_entry);
            if (_dir != INVALID_HANDLE_VALUE)
            {
                if (is_current_or_prev_folder(_entry.cFileName)) 
                    ++(*this);
            }
            else
            {
                _end = true;
            }
        }
    }   

    directory_iterator::directory_iterator(directory_iterator&& it) : _dir(std::move(it._dir)), _entry(std::move(it._entry)) {
        it._dir     = {0};
        //it._entry   = {0};
    }
    
    directory_iterator& directory_iterator::operator= (directory_iterator& it) {
        if (&it != this)
        {
            std::swap(_dir, it._dir);
            std::swap(_entry, it._entry);
        } 
        return *this;
    }
    
    void directory_iterator::close() {
        if (_dir != INVALID_HANDLE_VALUE)
        {
            FindClose(_dir);
            _dir    = INVALID_HANDLE_VALUE;
        }
    }

    directory_iterator::~directory_iterator() {
        close();
    }
    
    bool directory_iterator::operator!= (const directory_iterator& it) const {
        return _end != it._end;
    }
    
    directory_iterator& directory_iterator::operator++ () {   
        START:
        if (_end) 
            throw_except<illegal_state_exception>("End of directory stream!");
        
        if (FindNextFileW(_dir, &_entry) != 0)
        {
            if (is_current_or_prev_folder(_entry.cFileName))
                goto START;
        }
        else
        {
            _end = true;
        }
        
        return *this;
    }
    
    directory_entry directory_iterator::operator* () const {
        tca::inline_linear_allocator<filesystem::MAX_LENGTH_PATH> allocator;
        
        //Строка в кодировке виндоус (UTF-16)
        const wchar_t* entry_path   =_entry.cFileName;
        std::size_t path_length     = wstring::str_len(entry_path);

        //Перекодировка в UTF-8
        char buf[io::constants::MAX_LENGTH_PATH];
        std::size_t converted_length = wchar_to_char(entry_path, path_length, buf, io::constants::MAX_LENGTH_PATH);

        return directory_entry(buf, converted_length);
    }
}

#endif

//#################################################### ./src/freelist_allocator.cpp ####################################################

#include <allocators/freelist_allocator.hpp>
#include <cpp/lang/math.hpp>
#include <internal/memory.hpp>
#include <cstddef>
#include <cstdint>

#include <iostream>
#include <cstdio>


namespace tca
{
namespace internal
{

    /**
     * Flags for memblock key field.
     * 
     * The key field stores both the block size and flags:
     * USE_BIT:     set if block is currently allocated
     * LARGE_BIT:   set if block is a large allocation from parent
     * 
     * Block size is stored in the lower bits with flags masked out.
     */

    // Bit indicating block is allocated.
    const memsize_t USE_BIT    = 0x01u;
    
    // Bit indicating large allocation.
    const memsize_t LARGE_BIT  = 0x02u;
    
    // Mask to clear flag bits.
    const memsize_t CLEAR_BITS = ~0x03u;

    /**
     * Sets or clears a flag in the block key.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param flag
     *      The flag bit to modify.
     * 
     * @param use
     *      true to set the flag, false to clear it.
     */
    inline static void memblock_set_flag(memblock* b, memsize_t flag, bool use) {
        memsize_t bit = use ? flag : 0;
        b->key &= ~flag;
        b->key |= bit;
    }
    
    /**
     * Checks if a flag is set in the block key.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param flag
     *      The flag bit to check.
     * 
     * @return
     *      true if the flag is set, false otherwise.
     */
    inline static bool memblock_get_flag(const memblock* b, memsize_t flag) {
        return b->key & flag;
    }

    /**
     * Sets or clears the LARGE_BIT flag.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param use
     *      true for large allocation, false for normal block.
     */
    inline static void memblock_setlarge(memblock* b, bool use) {
        memblock_set_flag(b, LARGE_BIT, use);
    }

    /**
     * Checks if the block is a large allocation.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @return
     *      true if the block is a large allocation from parent.
     */
    inline static bool memblock_is_large(const memblock* b) {
        return memblock_get_flag(b, LARGE_BIT);
    }

    /**
     * Retrieves the block size (without flags).
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @return
     *      The block size in bytes.
     */
    inline static memsize_t memblock_get_size(const memblock* b) {
        return b->key & CLEAR_BITS;
    }

    /**
     * Sets the block size (clears flags).
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param sz
     *      The block size in bytes.
     */
    inline static void memblock_set_size(memblock* b, memsize_t sz) {
        b->key = sz;
    }
    
    /**
     * Retrieves the size of a large allocation.
     * 
     * @param b
     *      Pointer to the block (must be marked as large).
     * 
     * @return
     *      The large allocation size in bytes.
     */
    inline static std::size_t memblock_get_largesize(const large_memblock* b) {
        assert(memblock_is_large(&b->base));
        return b->large_size;
    }

    /**
     * Marks a block as a large allocation and stores its size.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @param sz
     *      The large allocation size in bytes.
     */
    inline static void memblock_set_largesize(large_memblock* b, std::size_t sz) {
        memblock_set_size(&b->base, 0);
        memblock_setlarge(&b->base, true);
        b->large_size = sz;
    }

    inline static void memblock_setuse(memblock* b, bool use) {
        memblock_set_flag(b, USE_BIT, use);
    }

    /**
     * Checks if the block is currently allocated.
     * 
     * @param b
     *      Pointer to the block.
     * 
     * @return
     *      true if allocated, false if free.
     */
    inline static bool memblock_is_use(const memblock* b) {
        return memblock_get_flag(b, USE_BIT);
    }

} //namespace internal
} //namespace tca

namespace tca
{
    using fla = free_list_allocator;
    
    fla:: free_list_allocator() : allocator(nullptr), page_list(nullptr), tree(), page_size(0) {

    }

    fla:: free_list_allocator(allocator* parent, std::size_t page_size) :
    allocator(parent),
    page_list(nullptr),
    tree(),
    page_size(static_cast<internal::memsize_t>(page_size)) {

    }

    fla:: free_list_allocator(free_list_allocator&& alloc) :
    allocator(std::move(alloc)),
    page_list(alloc.page_list),
    tree(std::move(alloc.tree)),
    page_size(alloc.page_size) {
        alloc.page_list = nullptr;
        alloc.page_size = 0;
    }
    
    free_list_allocator& fla:: operator= (free_list_allocator&& alloc) {
        if (&alloc == this)
            return *this;
        std::swap(page_list, alloc.page_list);
        std::swap(tree,      alloc.tree);
        std::swap(page_size, alloc.page_size);
        return *this;
    }

    
    void* fla:: allocate(std::size_t sz) {
        return allocate_align(sz, alignof(std::max_align_t));
    }
    
    void fla:: link(internal::memblock* h) {
        h->left     = nullptr;
        h->right    = nullptr;
        h->height   = 0;
        tree.insert_entry(h);
    }
    
    void fla:: unlink(internal::memblock* h) {
        internal::memblock* unlinked = tree.remove_entry(h);
        assert(h == unlinked);
    }

    void fla:: allocate_page() {
        internal::page_header* newpage = internal::page_new(page_list, page_size, MIN_ALIGN, m_parent);
        if (!newpage)
            return;
        page_list = newpage;
        
        internal::memblock* header = static_cast<internal::memblock*>(newpage->dat);
        internal::memblock_set_size(header, page_size);

        link(header);
    }

    void* fla:: alloc_from_parent(std::size_t sz, std::size_t align) {
        using internal::memsize_t;
        using internal::memblock;
        using internal::large_memblock;
        using internal::memblock_setuse;
        using internal::memblock_set_largesize;
        using internal::fallback_t;
        
        sz = LARGE_BLOCK_HEADER_SIZE + (align - 1) + sz;
        void* memory = m_parent->allocate_align(sz, alignof(large_memblock));
        
        uintptr_t  hdr      = reinterpret_cast<uintptr_t>(memory);
        uintptr_t  user_ptr = align_up(hdr + LARGE_BLOCK_HEADER_SIZE, align);
        
        fallback_t* fallback_ptr = reinterpret_cast<fallback_t*>(user_ptr - sizeof(fallback_t));
        
        TC_ALIGN_ASSERT(fallback_ptr, alignof(fallback_t));
        *fallback_ptr = static_cast<fallback_t>(user_ptr - hdr);

        large_memblock* block = reinterpret_cast<large_memblock*>(hdr);
        TC_ALIGN_ASSERT(block, alignof(large_memblock));
        
        memblock_set_largesize(block, sz);

        TC_ALIGN_ASSERT(user_ptr, align);
        return reinterpret_cast<void*>(user_ptr);
    }
    
    void  fla:: free_from_parent(void* p) {
        using internal::large_memblock;
        using internal::memblock_get_largesize;
        using internal::fallback_t;
        
        uintptr_t user_ptr      = reinterpret_cast<uintptr_t>(p);
        fallback_t fallback     = *reinterpret_cast<fallback_t*>(user_ptr - sizeof(fallback));
        large_memblock* block   = reinterpret_cast<large_memblock*>(user_ptr - fallback);

        m_parent->deallocate(block, memblock_get_largesize(block));
    }

    void* fla:: allocate_align(std::size_t sz, std::size_t align) {
        assert(m_parent != nullptr);
        
        using internal::memsize_t;
        using internal::memblock;
        using internal::memblock_set_size;
        using internal::memblock_get_size;
        using internal::memblock_setuse;
        using internal::fallback_t;

        align   = tc::math::max( align, static_cast<std::size_t>(MIN_ALIGN) );
        sz      = tc::math::max(sz, static_cast<std::size_t>(HDR_DIFF));
        sz      = align_up( sz, static_cast<std::size_t>(MIN_ALIGN) );
        memsize_t min_need_size;
        
        if (align > MIN_ALIGN)
        {
            min_need_size = static_cast<memsize_t>(HDR_SIZE + sz + align - 1);
        }
        else
        {
            min_need_size = static_cast<memsize_t>(HDR_SIZE + sz);
        }

        
        if (min_need_size >= mmap_threshold())
        {
            return alloc_from_parent(sz, align);
        }

        for (std::size_t i = 0; i < 3; ++i)
        {
            memblock* block = tree.ceil_entry(min_need_size);
            if (!block)
            {
                if (i == 0)
                {
                    compact();
                } else if (i == 1)
                {
                    allocate_page();
                }
                continue;
            }
            unlink(block);

            // Указатель на начало блока (первый байт заголовка)
            uintptr_t  start_block  = reinterpret_cast<uintptr_t>(block);
            
            // Указатель на пользовательский заголовок
            uintptr_t  user_ptr     = align_up<uintptr_t>(start_block + HDR_SIZE, align);
            
            // Размер заголовка (вместе с padding)
            memsize_t     hdr_with_padd  = static_cast<memsize_t>(user_ptr - start_block);
            
            // Размер padding
            memsize_t     padding        = hdr_with_padd - HDR_SIZE;

            // Общий размер блока (Включая заголовок)
            memsize_t block_size = memblock_get_size(block);

            /*
                [[HDR]..............................................]
                ^    ^    ^          ^
                0    8    24         64
                
                [[HDR]....]......[HDR]....]...................................]
                ^    ^    ^          ^
                0    8    24         128
            */

            // Разделение, если паддинг слишком большой, и туда можно поместить блок памяти
            if (padding >= MIN_BLOCK_SIZE)
            {
                memblock* front  = reinterpret_cast<memblock*>(start_block);
                memblock_set_size(front, padding);
                block_size      -= padding;
                start_block     += padding;
                link(front);
            }
            
            memsize_t used = static_cast<memsize_t>(user_ptr + sz - start_block);

            assert(block_size >= used);

            // Максимум между реальным задействованным объемом и пессимистичным запросом
            memsize_t raw_target = tc::math::max(used, min_need_size);

            // Округление вверх до выравнивания заголовка блока
            memsize_t target_size = align_up<memsize_t>(raw_target, alignof(internal::memblock));

            // Сколько реально остаётся под следующий блок
            memsize_t rem = (block_size > target_size) ? (block_size - target_size) : 0;

            // Разделение, если остаток достаточно велик для нового блока
            if (rem >= MIN_BLOCK_SIZE)
            {
                memblock* next = reinterpret_cast<memblock*>(start_block + target_size);
                TC_ALIGN_ASSERT(next, alignof(internal::memblock));
                
                memblock_set_size(next, rem);
                block_size -= rem;
                link(next);
            }

            {
                memsize_t fallback = static_cast<memsize_t>(user_ptr - start_block);
                
                /*
                    Записываем в user_ptr - sizeof(fallback_t) значение (user_ptr - start_block), 
                    сколько нужно сдвинуться назад, чтобы получить фактический указатель на заголовок
                */
                
                *reinterpret_cast<fallback_t*>(user_ptr - sizeof(fallback_t)) = static_cast<fallback_t>(fallback);

                memblock* current = reinterpret_cast<memblock*>(start_block);
                TC_ALIGN_ASSERT(current, alignof(memblock));

                memblock_set_size(current,  block_size);
                memblock_setuse(current,    true);

                // link(current);
            }
            
        
            TC_ALIGN_ASSERT(user_ptr, align);
            return reinterpret_cast<void*>(user_ptr);

            // return nullptr;
        }

        return nullptr;
    }
    
    void fla:: deallocate(void* p) {
        if (!p)
            return;

        using internal::memblock;
        using internal::fallback_t;
        using internal::memblock_setuse;
        using internal::memblock_is_large;
        
        uintptr_t  user_ptr     = reinterpret_cast<uintptr_t>(p);
        uintptr_t  fallback_ptr = user_ptr - sizeof(fallback_t);
        fallback_t      fallback     = *reinterpret_cast<fallback_t*>(fallback_ptr);

        memblock* block = reinterpret_cast<memblock*>(user_ptr - fallback);

        if (memblock_is_large(block))
        {
            free_from_parent(p);
            return;
        }

        memblock_setuse(block, false);
        link(block);
    }

    void fla:: compact() {
        
        using internal::memblock;
        using internal::memblock_is_use;
        using internal::memblock_get_size;
        using internal::memblock_set_size;
        using internal::memsize_t;
        
        const internal::page_header* page = page_list;
        
        while (page)
        {

            uintptr_t start = reinterpret_cast<uintptr_t>(page->dat);
            std::size_t i        = 0;
            while (i < page->user_size)
            {
                memblock* block      = reinterpret_cast<memblock*>(start + i);
                memsize_t block_size = internal::memblock_get_size(block);
                
                if (i + block_size >= page->user_size)
                {
                    break;
                }

                if (!memblock_is_use(block))
                {
                    memblock* next  = reinterpret_cast<memblock*>(start + i + block_size);
                    if (!memblock_is_use(next))
                    {
                        unlink(block);
                        unlink(next);
                        block_size += memblock_get_size(next);
                        memblock_set_size(block, block_size);
                        link(block);
                        continue;
                    }
                }
                
                i += block_size;
            }

            page = page->next;
        }
    }

    void fla:: print_log() const {
        const internal::page_header* page = page_list;
        
        std::size_t cnt_pages = 0;
        
        while (page)
        {
            std::size_t total_size = 0;
            using internal::memblock;
            using internal::memblock_is_use;
            std::printf("############### PAGE %zu ###############\n", cnt_pages++);

            uintptr_t start = reinterpret_cast<uintptr_t>(page->dat);
            std::size_t i        = 0;
            while (i < page->user_size)
            {
                const memblock* block = reinterpret_cast<memblock*>(start + i);
                std::size_t block_size = internal::memblock_get_size(block);
                total_size += block_size;
                // assert(block_size >= HDR_SIZE);
                printf("[t: %zu, u: %zu, f: %s]\n", block_size, block_size - HDR_SIZE, memblock_is_use(block) ? "false" : "true");
                i += internal::memblock_get_size(block);
            }

            assert(total_size == page->user_size);
            
            page = page->next;
        }

    }

    fla:: ~free_list_allocator() {
        using internal::page_header;
        using internal::page_delete;
        for (page_header* page = page_list; page != nullptr; )
        {
            page_header* next = page->next;
            page_delete(page, m_parent);
            page = next;
        }
    }
}

//#################################################### ./src/ibstream.cpp ####################################################

#include <cpp/lang/io/ibstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc
{

    ibstream::ibstream() : m_allocator(nullptr), m_buffer(nullptr), m_capacity(0), m_offset(0), m_limit(0), m_in(nullptr) {

    }

    ibstream::ibstream(istream* stream, std::size_t buf_size, tca::allocator* allocator) : ibstream() {
        char* data = (char*) allocator->allocate_align(buf_size, alignof(char));
        m_allocator  = allocator;
        m_buffer     = data;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_limit      = 0;
        m_in         = stream;
    }
    
    ibstream::ibstream(istream* stream, char* buf, std::size_t buf_size) : ibstream() {
        m_buffer     = buf;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_limit      = 0;
        m_in         = stream;
    }
    
    ibstream::ibstream(ibstream&& stream) : 
    m_allocator(stream.m_allocator), 
    m_buffer(stream.m_buffer),
    m_capacity(stream.m_capacity),
    m_offset(stream.m_offset),
    m_limit(stream.m_limit),
    m_in(stream.m_in) {
        stream.m_allocator   = nullptr;
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
        stream.m_limit       = 0;
        stream.m_in          = nullptr;
    }
    
    ibstream& ibstream::operator= (ibstream&& stream) {
        if (&stream != this) {
            if (m_in != nullptr)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_allocator   = stream.m_allocator;
            m_buffer      = stream.m_buffer;
            m_capacity    = stream.m_capacity;
            m_offset      = stream.m_offset;
            m_limit       = stream.m_limit;
            m_in          = stream.m_in;
            
            stream.m_allocator   = nullptr;
            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
            stream.m_limit       = 0;
            stream.m_in          = nullptr;
        }
        return *this;
    }
    
    void ibstream::fill_buffer() {
        std::size_t readed = m_in->read(m_buffer, m_capacity);
        m_limit  = (readed == istream::eof_value()) ? 0 : readed;
        m_offset = 0;
    }

    std::size_t ibstream::read(char* buf, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_in)
                throw_except<io_exception>("stream is null");
        );
        
        std::size_t total_readed = 0;
        
        while (total_readed < sz)
        {
            if (m_offset >= m_limit)
            {
                fill_buffer();
                if (m_limit == 0)
                    break; // EOF
            }
            
            std::size_t available = m_limit - m_offset;
            std::size_t to_read = std::min(available, sz - total_readed);
            
            if (to_read > 0)
            {
                memcpy(buf + total_readed, m_buffer + m_offset, to_read);
                m_offset += to_read;
                total_readed += to_read;
            }
        }
        
        return total_readed > 0 ? 
                                    total_readed : istream::eof_value();
    }
    
    void ibstream::free() {
        if (m_allocator)
        {
            m_allocator->deallocate(m_buffer, m_capacity);
            m_allocator = nullptr;
        }
    }

    ibstream::~ibstream() {
		error_code dontcare;
        close(dontcare);
    }

    void ibstream::close(error_code& err) {
        if (m_in == nullptr)
            return;
        m_in->close(err);
        m_in = nullptr;
        free();
    }
}

//#################################################### ./src/idstream.cpp ####################################################

#include <cpp/lang/io/idstream.hpp>
#include <iostream>

namespace tc {

    idstream::idstream() : m_in(nullptr) {
        
    }

    idstream::idstream(istream* in) : m_in(in) {
        if (in == nullptr)
            throw_except<null_pointer_exception>("in is null");
    }
    
    idstream::idstream(idstream&& stream) : m_in(stream.m_in) {
        stream.m_in = nullptr;
    }

    idstream& idstream::operator= (idstream&& stream) {
        if (&stream != this)
        {
            if (m_in)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_in         = stream.m_in;
            stream.m_in  = nullptr;
        }
        return *this;
    }

    idstream::~idstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    std::size_t idstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (!m_in)
                throw_except<io_exception>("stream is null");
        )
        return m_in->read(buf, sz);
    }

    void idstream::close(error_code& err) {
        if (m_in == nullptr)
            return;
        m_in->close(err);
        m_in = nullptr;
    }
}

//#################################################### ./src/ifstream.cpp ####################################################

#include <cpp/lang/io/ifstream.hpp>
#include <internal/io/io_helpers.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>

namespace tc
{

    ifstream::ifstream() : istream(), m_handle(nullptr) {

    }

    ifstream::ifstream(const char* path) : ifstream(file(path)) {

    }
    
    ifstream::ifstream(const string& path) : ifstream(path.c_str()) {
    
    }

    ifstream::ifstream(const file& file) : m_handle(nullptr) {
        expected<FILE*, error_code> fhandle = filesystem::open(file.c_str(), "rb");

        if (!fhandle)
        {
            internal::io::throw_error_code(fhandle.error());
        }

        m_handle = fhandle.value();
    }
    
    ifstream::ifstream(ifstream&& stream) : istream(), m_handle(stream.m_handle) {
        stream.m_handle      = nullptr;
    }
    
    ifstream& ifstream::operator= (ifstream&& stream) {
        if (&stream != this)
        {
            if (m_handle)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_handle        = stream.m_handle;
            stream.m_handle = nullptr;
        }
        return *this;
    }
    
    void ifstream::close(error_code& err) {
        if (m_handle == nullptr)
            return;
        err = filesystem::close(m_handle);
        m_handle     = nullptr;
    }
    
    ifstream::~ifstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    std::size_t ifstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE(
            if (m_handle == nullptr)
                throw_except<io_exception>("file stream not open")
        );
        std::size_t readed = fread(buf, 1, sz, m_handle);   
        if (readed == 0)
        {
            if (feof(m_handle))
            {
                return istream::eof_value();
            }
            if (ferror(m_handle))
            {
                throw_except<io_exception>(strerror(errno));
            }
        }
        return readed;
    }
}

//#################################################### ./src/image.cpp ####################################################

#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/exceptions.hpp>
#include <utility>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <internal/img_utils.hpp>

namespace tc {

    image::image(tca::allocator* alloc) :
    m_allocator(alloc),
    m_pixels(nullptr),
    m_width(0),
    m_height(0),
    m_channels(0) {

    }
    
    image::image(unsigned char* data, tca::allocator* allocator, int w, int h, int channels) :
    m_allocator(allocator),
    m_pixels(data), 
    m_width(w), 
    m_height(h),
    m_channels(channels) {

    }

    image::image(int width, int height, int channels, tca::allocator* allocator) : image(allocator) {
        unsigned char* pixels = reinterpret_cast<unsigned char*>(allocator->allocate_align((width * height * channels), alignof(unsigned char)));
        if (!pixels)
            throw_except<out_of_memory_error>("out of memory");
        std::memset(pixels, 0, (width * height * channels));
        m_pixels    = pixels;
        m_width     = width;
        m_height    = height;
        m_channels  = channels;
    }
    
    image::~image() {
        if (m_pixels)
        {
            m_allocator->deallocate(m_pixels);
        }
    }

    image::image(const image& img) : image(img.m_allocator) {
        if (img.m_pixels)
        {
            std::size_t len = static_cast<std::size_t>(img.m_width * img.m_height * img.m_channels);
            
            unsigned char* pixels = (unsigned char*) m_allocator->allocate_align(len, alignof(char));
            if (!pixels)
                throw_except<out_of_memory_error>("out of memory");
            std::memcpy(pixels, img.m_pixels, len);
            
            m_pixels    = pixels;
            m_width     = img.m_width;
            m_height    = img.m_height;
            m_channels  = img.m_channels;
        }
    }

    image& image::operator= (const image& img) {
        if (&img != this) {
            unsigned char* pixels = nullptr;
            
            if (img.m_pixels)
            {
                std::size_t len = static_cast<std::size_t>(img.m_width * img.m_height * img.m_channels);
                pixels = (unsigned char*) m_allocator->allocate_align(len, alignof(char));
                if (!pixels)
                    throw_except<out_of_memory_error>("out of memory");
                std::memcpy(pixels, img.m_pixels, len);
            }

            if (m_pixels)
                m_allocator->deallocate(m_pixels);

            m_pixels    = pixels;
            m_width     = img.m_width;
            m_height    = img.m_height;
            m_channels  = img.m_channels;
        }
        return *this;
    }

    image::image(image&& img) : 
    m_allocator(img.m_allocator), 
    m_pixels(img.m_pixels), 
    m_width(img.m_width), 
    m_height(img.m_height), 
    m_channels(img.m_channels) {
        img.m_pixels    = nullptr;
        img.m_width     = 0;
        img.m_height    = 0;
        img.m_channels  = 0;
    }

    image& image::operator= (image&& img) {
        if (&img != this) {
            if (get_allocator() == img.get_allocator())
            {
                std::swap(m_pixels,     img.m_pixels);
                std::swap(m_width,      img.m_width);
                std::swap(m_height,     img.m_height);
                std::swap(m_channels,   img.m_channels);
            }
            else
            {
                *this = img;
            }
        }
        return *this;
    }
    
    int image::get_width() const {
        return m_width;
    }

    int image::get_height() const {
        return m_height;
    }

    unsigned char* image::pixels() {
        return m_pixels;
    }
    
    const unsigned char* image::pixels() const {
        return m_pixels;
    }

    image::pixel image::get_pixel(int x, int y) const {
        JSTD_DEBUG_CODE(
            if (x < 0 || x >= m_width) throw_except<index_out_of_bound_exception>("x %i >= width %i", x, m_width);
            if (y < 0 || y >= m_height) throw_except<index_out_of_bound_exception>("y %i >= width %i", y, m_height);
        );
        
        int idx = (x + y * m_width) * m_channels;

        if (m_channels == 4)
        {
            unsigned char r = m_pixels[idx + 0];
            unsigned char g = m_pixels[idx + 1];
            unsigned char b = m_pixels[idx + 2];
            unsigned char a = m_pixels[idx + 3];
            return {r, g, b, a};
        }
        else if (m_channels == 3)
        {
            unsigned char r = m_pixels[idx + 0];
            unsigned char g = m_pixels[idx + 1];
            unsigned char b = m_pixels[idx + 2];
            return {r, g, b, 255};
        }
        else if (m_channels == 1)
        {
            unsigned char g = m_pixels[idx];
            return {g, g, g, 255};
        }
        else
        {
            throw make_except<illegal_state_exception>("%i channels not support", m_channels);
        }
    }
    
    /**
     * 
     */
    void image::set_pixel(int x, int y, const pixel& p) {
        JSTD_DEBUG_CODE(
            if (x < 0 || x >= m_width) throw_except<index_out_of_bound_exception>("x %i >= width %i", x, m_width);
            if (y < 0 || y >= m_height) throw_except<index_out_of_bound_exception>("y %i >= width %i", y, m_height);
        );
        int idx = (x + y * m_width) * m_channels;

        if (m_channels == 4)
        {
            m_pixels[idx + 0] = p.red;
            m_pixels[idx + 1] = p.green;
            m_pixels[idx + 2] = p.blue;
            m_pixels[idx + 3] = p.alpha;
        }
        else if (m_channels == 3)
        {
            m_pixels[idx + 0] = p.red;
            m_pixels[idx + 1] = p.green;
            m_pixels[idx + 2] = p.blue;
        }
        else if (m_channels == 1)
        {
            
            float red   = static_cast<float>(p.red);
            float green = static_cast<float>(p.green);
            float blue  = static_cast<float>(p.blue);
            
            // 0.299 * R + 0.587 * G + 0.114 * B
            float grayscale = 0.299f * red + 0.587f * green + 0.114f * blue;

            m_pixels[idx] = static_cast<unsigned char>(grayscale);
        }
        else
        {
            throw make_except<illegal_state_exception>("%i channels not support", m_channels);
        }

    }

    int image::get_channels() const {
        return m_channels;
    }

    image image::resize(int neww, int newh, tca::allocator* alloc) const {
        if (neww <= 0 || newh <= 0)
            throw_except<illegal_argument_exception>("invalid width or height");

        if (!alloc)
            alloc = m_allocator;

        image resized_image(neww, newh, m_channels, alloc);
        
        int error;
        if (m_channels == 4)
        {
            error = internal::resize_image_alpha(
                m_pixels, m_width, m_height,
                resized_image.pixels(), neww, newh, 
                m_channels,
                3, /*alpha index*/
                m_allocator
            );
        }
        else
        {
            error = internal::resize_image(
                m_pixels, m_width, m_height,
                resized_image.pixels(), neww, newh, 
                m_channels, 
                m_allocator
            );
        }

        if (error != 0)
            throw_except<illegal_state_exception>("resize error: %i", error);
            
        return resized_image;
    }

    string image::to_string(tca::allocator* alloc) const {
        string result(alloc);
        
        result
        .append("[w:").append(tc::to_string(m_width, alloc)).append(',')
        .append("h:").append(tc::to_string(m_height, alloc)).append(',')
        .append("c:").append(tc::to_string(m_channels, alloc)).append(']');

        return result;
    }

    /**static */ image image::lock(unsigned char* data, tca::allocator* allocator, int width, int height, int channels) {
        JSTD_DEBUG_CODE(
            check_non_null(data);
            check_non_null(allocator);
        );
        return image(data, allocator, width, height, channels);
    }
}



//#################################################### ./src/imageio.cpp ####################################################

#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/io/iostream.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

/**
 * Macro overriding STB image's malloc with custom allocator.
 */
#define STBI_MALLOC(size) tc_malloc(size)

/**
 * Macro overriding STB image's realloc with custom allocator.
 */
#define STBI_REALLOC(pointer, new_size) tc_realloc(pointer, new_size)

/**
 * Macro overriding STB image's free with custom allocator.
 */
#define STBI_FREE(pointer) tc_free(pointer)

/**
 * Enables detailed error messages from STB image write library.
 */
#define STBIW_FAILURE_USERMSG

/**
 * Instructs STB image write library to include implementation.
 */
#define STB_IMAGE_WRITE_IMPLEMENTATION

/**
 * Macro overriding STB image write's malloc with custom allocator.
 */
#define STBIW_MALLOC(size) tc_malloc(size)

/**
 * Macro overriding STB image write's realloc with custom allocator.
 */
#define STBIW_REALLOC(pointer, new_size) tc_realloc(pointer, new_size)

/**
 * Macro overriding STB image write's free with custom allocator.
 */
#define STBIW_FREE(pointer) tc_free(pointer)

static void* tc_malloc(std::size_t sz);
static void* tc_realloc(void* p, std::size_t sz);
static void  tc_free(void* ptr);

#include <cpp/lang/utils/images/stb_image.h>
#include <cpp/lang/utils/images/stb_image_write.h>

/**
 * Returns the singleton allocator instance used by STB libraries.
 * 
 * @return
 *      Pointer to the static allocator.
 */
static tca::malloc_free_allocator* get_allocator_for_stb() {
    static tca::malloc_free_allocator alloc;
    return &alloc;
}

/**
 * Allocates memory using the STB allocator.
 * 
 * @param sz
 *      Size in bytes to allocate.
 * 
 * @return void*
 *      Pointer to allocated memory.
 */
static void* tc_malloc(std::size_t sz) {
    return get_allocator_for_stb()->allocate(sz);
}

/**
 * Reallocates memory using the STB allocator.
 * 
 * @param p
 *      Pointer to previously allocated memory.
 * 
 * @param sz
 *      New size in bytes.
 * 
 * @return
 *      void* pointer to reallocated memory.
 */
static void* tc_realloc(void* p, std::size_t sz) {
    return get_allocator_for_stb()->reallocate(p, sz);
}

/**
 * Deallocates memory using the STB allocator.
 * 
 * @param ptr
 *      Pointer to memory to free.
 */
static void tc_free(void* ptr) {
    return get_allocator_for_stb()->deallocate(ptr);
}

namespace tc 
{

namespace imageio 
{

namespace read_callbacks
{

/**
 * Wrapper structure for input stream with EOF tracking.
 */
struct stream_wrapper {
    istream* m_in;  //input stream for load image
    bool     m_eof; //end of file flag  
};

    /**
     * Reads data from the stream.
     * 
     * STB image callback that reads a specified number of bytes from the stream.
     * 
     * @param user
     *      Pointer to stream_wrapper instance.
     * 
     * @param data
     *      Output buffer for read data.
     * 
     * @param size
     *      Number of bytes to read.
     * 
     * @return
     *      Number of bytes actually read.
     */
    int read_callback(void* user, char* data, int size) {
        stream_wrapper* in = static_cast<stream_wrapper*>(user);
        int readed = static_cast<int>(
                                        in->m_in->read(data, static_cast<std::size_t>(size) 
                                    )
        );
        if (readed < size)
        {
            in->m_eof = true;
        }    
        return readed;
    }

    /**
     * Skips data in the stream.
     * 
     * STB image callback that skips a specified number of bytes in the stream.
     * 
     * @param user
     *      Pointer to stream_wrapper instance.
     * 
     * @param n
     *      Number of bytes to skip.
     */
    void skip_callback(void *user, int n) {
        stream_wrapper* in = static_cast<stream_wrapper*>(user);
        int skipped = static_cast<int>(in->m_in->skip(static_cast<std::size_t>(n)));
        if (skipped < n)
        {
            in->m_eof = true;
        }
    }

    /**
     * Checks if the stream has reached end-of-file.
     * 
     * STB image callback that returns EOF status.
     * 
     * @param user
     *      Pointer to stream_wrapper instance.
     * 
     * @return
     *      1 if EOF reached, 0 otherwise.
     */
    int eof_callback(void *user) {
        stream_wrapper* in = static_cast<stream_wrapper*>(user);
        return in->m_eof ? 1 : 0;
    }
}

    image load_image(istream* in, tca::allocator* allocator) {        
        
        int width;
        int heigth;
        int channels;
        
        read_callbacks::stream_wrapper wrapper = {in, false};

        stbi_io_callbacks callbacks = {
            read_callbacks::read_callback,
            read_callbacks::skip_callback,
            read_callbacks::eof_callback,
        };

        stbi_uc* pixels = stbi_load_from_callbacks(&callbacks, static_cast<void*>(&wrapper), &width, &heigth, &channels, 0);
        if (!pixels)
        {
            throw_except<illegal_state_exception>("%s", stbi_failure_reason());
        }
            
        return image::lock(pixels, get_allocator_for_stb(), width, heigth, channels);

    }

    image load_image(const file& file, tca::allocator* allocator) {
        ifstream in(file);
        image img;
        try {
            img = load_image(&in, allocator);
        } catch (...) {
            throw;
        }
        return image(std::move(img));
    }
    
    /**
     * Write callback for STB image library.
     * 
     * Callback function that writes image data to an output stream.
     * 
     * @param context
     *      Pointer to the output stream (cast from void*).
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param size
     *      Number of bytes to write.
     */
    static void save_context(void* context, void* data, int size){
		assert(size >= 0);
        ostream* out = reinterpret_cast<ostream*>(context);
        out->write(reinterpret_cast<const char*>(data), (std::size_t) size);
	}

    void write_image(const file& file, const image& img, const char* ext) {
        ofstream out(file);
        try {
            write_image(&out, img, ext);
        } catch (...) {
            throw;
        }
    }

    void write_image(ostream* out, const image& img, const char* ext) {
        int width    = img.get_width();
        int height   = img.get_height();
        int channels = img.get_channels();
        
        int error = 0;

        if (std::strcmp(ext, "png") == 0)
        {
            error = stbi_write_png_to_func(save_context, out, width, height, channels, img.pixels(), 0);
        } 
        else if (std::strcmp(ext, "jpeg") == 0 || std::strcmp(ext, "jpg") == 0)
        {
            error = stbi_write_jpg_to_func(save_context, out, width, height, channels, img.pixels(), 100);
        } 
        else if (std::strcmp(ext, "tga") == 0)
        {
            error = stbi_write_tga_to_func(save_context, out, width, height, channels, img.pixels());
        }
        else if (std::strcmp(ext, "bmp") == 0)
        {
            error = stbi_write_bmp_to_func(save_context, out, width, height, channels, img.pixels());
        } 

        if (error == 0)
        {
            throw_except<illegal_state_exception>("stbi_write error: %i", error);
        }
    }

}

}

//#################################################### ./src/image_packer.cpp ####################################################

#include <allocators/linear_allocator.hpp>
#include <cpp/lang/utils/images/image_packer.hpp>
#include <cpp/lang/utils/images/image_tree.hpp>
#include <cpp/lang/utils/images/image.hpp>
#include <algorithm>

namespace tc {
    using namespace texturing;

    image_packer::image_packer() : 
    m_allocator(nullptr), 
    m_root(), 
    m_images(),
    m_width(0),
    m_height(0) {

    }

    image_packer::image_packer(const image* img_array, std::size_t count_images, int w, int h, tca::allocator* allocator) :
    m_allocator(allocator),
    m_root(),
    m_images(img_array),
    m_cnt_imgs(count_images),
    m_width(w),
    m_height(h) {
        
    }

    image_packer::image_packer(image_packer&& packer) : 
    m_allocator(packer.m_allocator),
    m_root(std::move(packer.m_root)),
    m_images(packer.m_images),
    m_width(packer.m_width),
    m_height(packer.m_height) {
        packer.m_images         = nullptr;
        packer.m_cnt_imgs       = 0;
        packer.m_width          = 0;
        packer.m_height         = 0;
    }
    
    image_packer& image_packer::operator=(image_packer&& packer) {
        if (&packer != this) 
        {
            m_allocator         = packer.m_allocator;
            m_root              = std::move(packer.m_root);
            m_images            = packer.m_images;
            m_width             = packer.m_width;
            m_height            = packer.m_height;

            packer.m_images = nullptr;
            packer.m_width  = 0;
            packer.m_height = 0;
        }
        return *this;
    }

    image_packer::~image_packer() {

    }

    std::size_t get_max_mem_size_for_max_image(const image* images, std::size_t len) {
        std::size_t mem_size    = 0;
        for (std::size_t i = 0; i < len; ++i)
        {
            const image& img = images[i];
            mem_size = std::max<std::size_t>(mem_size, (std::size_t) (img.get_width() * img.get_height() * img.get_channels()));
        }
        return mem_size;
    }

    void image_packer::create_tree() {
        if (!m_root)   
        
        m_root = polymorph::allocate_unique<node>(m_allocator, node(m_width, m_height, m_allocator));

        BEGIN: 
        for (std::size_t i = 0; i < m_cnt_imgs; ++i)
        {
            const image& img = m_images[i];
            node* n = m_root->put_image(img.get_width(), img.get_height(), i);
            if (!n)
            {
                /**
                 * Если не получилось вставить изображение, начинаем заного, 
                 * но увеличиваем конечное изображение в два раза.
                 */
                m_width  <<= 1;
                m_height <<= 1;
                *m_root = node(m_width, m_height, m_allocator);
                goto BEGIN;
            }
        }
    }

    image image_packer::pack(int scale_factor, int out_image_channels) {
        JSTD_DEBUG_CODE (
            if (scale_factor <= 0)           throw_except<illegal_argument_exception>("scale_factor %li is illegal", (long int) scale_factor);
            if (m_allocator == nullptr)      throw_except<illegal_state_exception>("m_allocator must be != null");
            if (out_image_channels <= 0 || out_image_channels > 4)
                throw_except<illegal_argument_exception>("out_image_channels %li is illegal", (long int) scale_factor);
        );
            
        if (!m_root) create_tree();
        image result(m_root->get_rect().w / scale_factor, m_root->get_rect().h / scale_factor, out_image_channels, m_allocator);

        /**
         * Размер временного буфера, куда будет сохранено масштабированное изображение.
         * Буфер делится на rescale, так как если изначальное изображение весит килобайт, то масштабированный на 2, размер будет в два раза меньше и так далее.
         */
        std::size_t buffer_size_for_resized_image = get_max_mem_size_for_max_image(m_images, m_cnt_imgs) / static_cast<std::size_t>(scale_factor);

        /**
         * Линейный аллокатор, который нужен, чтобы выделять память под хранения временного масштабированного изображения
         */
        tca::linear_allocator allocator_for_rescaled_image; 
        
        //Если делитель больше 1, то масштабирование изображения требуется, а значит требуется и временный буфер.
        if (scale_factor > 1)
            allocator_for_rescaled_image = tca::linear_allocator(buffer_size_for_resized_image, m_allocator);

        struct callback_image {
            /**
             * Массив оригинальных изображений.
             */
            const image* m_array_of_image;
            
            /**
             * Указатель на результирующее изображение (атлас).
             * В который будет записано изображение.
             */
            image* m_atlas;
            
            /**
             * Делитель для уменьшения изображения.
             * Для корректных результатов должен быть кратен степени двойки. 2 4 8 16 32....1024
             */
            int m_rescale;
            
            /**
             * Аллокатор для выделения памяти под уменьшенное изображение.
             * Поскольку void operator() (node*) вызывается каждый раз для каждого изображения, то внутри него предвыделить буфер невозможно.
             * 
             */
            tca::linear_allocator*  m_allocator_for_tmp_resized_image;

            //Я даже описывать не хочу, какого чёрта тут происходит. Stupid Fuck.
            void operator()(const node* n) {
                std::size_t image_index = n->get_id();

                const image& img_original = m_array_of_image[image_index];
                int w_image = img_original.get_width();
                int h_image = img_original.get_height();

                image rescaled_image(m_allocator_for_tmp_resized_image); 
                if (m_rescale > 1 && m_allocator_for_tmp_resized_image != nullptr)
                    rescaled_image = img_original.resize(w_image / m_rescale, h_image / m_rescale, m_allocator_for_tmp_resized_image);

                const image& img = (m_rescale > 1) ? rescaled_image : img_original;

                /**
                 * Нода имеет координату, ширину и величину изображения.
                 * Причём ширина и высота всегда идентична изображению!
                 */
                const int x = n->get_rect().x     / m_rescale;
                const int y = n->get_rect().y     / m_rescale;
                const int w = n->get_rect().w     / m_rescale;
                const int h = n->get_rect().h     / m_rescale;
                
                // xo & yo - это позиции resized изображении.
                for (int xo = 0; xo < w; ++xo)
                {
                    for (int yo = 0; yo < h; ++yo)
                    {
                        // xx & yy - это позиция в атласе
                        int xx = x + xo;
                        int yy = y + yo;
                        image::pixel from_resized = img.get_pixel(xo, yo);
                        m_atlas->set_pixel(xx, yy, from_resized);
                    }
                }

                if (m_allocator_for_tmp_resized_image != nullptr)
                    m_allocator_for_tmp_resized_image->reset();
            }
        };

        callback_image callback;
        callback.m_array_of_image   = m_images;
        callback.m_atlas            = &result;
        callback.m_rescale          = scale_factor;
        callback.m_allocator_for_tmp_resized_image = scale_factor > 1 ? &allocator_for_rescaled_image : nullptr;

        m_root->depth_search(callback);

        return image(std::move(result));
    }

    array<image_packer::uv> image_packer::get_uv() const {
        array<image_packer::uv> uvs(m_cnt_imgs, m_allocator);
        
        struct node_visitor {
            array<image_packer::uv>* m_array;
            void operator() (const node* n) {
                const rect& pos = n->get_rect();
                int u0 = pos.x;
                int v0 = pos.y;
                int u1 = pos.x + pos.w;
                int v1 = pos.y + pos.h;
                image_packer::uv texcoord;
                texcoord.u0 = u0;
                texcoord.v0 = v0;
                texcoord.u1 = u1;
                texcoord.v1 = v1;
                (*m_array)[n->get_id()] = texcoord;
            }
        };

        node_visitor visitor;
        visitor.m_array = &uvs;
        
        m_root->depth_search(visitor);

        return array<image_packer::uv>(std::move(uvs));
    }
}

#if 0

#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/file.hpp>
#include <cpp/lang/string.hpp>
#include <cpp/lang/io/utility.hpp>
#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/utils/utils.hpp>

class png_filter : public tc::file_filter {
public:
    bool apply(const char* path, std::size_t path_length) const override {
        return tc::string(path).ends_with(".png");   
    }
};


tc::array<tc::image> read_all_images(const tc::file& path) {
    tc::array<tc::file> files;
    {
        png_filter filter;
        files = path.list_files(filter);
    }

    tc::array<tc::image> imgs(files.length);
    for (std::size_t i = 0; i < imgs.length; ++i)
    {
        imgs[i] = tc::imageio::load_image(files[i]);
    }

    return imgs;
}

namespace jstd
{
    template<>
    struct compare_to<image> {
        int operator() (const image& a, const image& b) const {
            return (b.get_width() * b.get_height()) - (a.get_width() * a.get_height());
        }
    };
}

int main(int argc, const char** args) {
    try {
        tc::file root = "./images/";    
        tc::array<tc::image> images = read_all_images(root);
        printf("len: %zu\n", images.length);
        {
            tc::utils::quick_sort(images.data(), images.length);
        }
        {
            tc::image_packer packer(images.data(), images.length, 1024, 1024);
            tc::image atlas = packer.pack(argc > 1 ? (*args[1]) - '0' : 1);
            tc::imageio::write_image(tc::file("./img.png"), &atlas, "png");
        }
    } catch( const tc::throwable& t) {
        std::printf("except: %s\n", t.cause());
    }
    
}

#endif

//#################################################### ./src/image_tree.cpp ####################################################

#include <cpp/lang/utils/images/image_tree.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <new>

namespace tc
{

namespace texturing
{

    node::node() : 
    m_allocator(nullptr), 
    m_left(), 
    m_right(), 
    m_rect(), 
    m_ID(NULL_ID) {

    }

    node::node(int w, int h, tca::allocator* allocator) : node({0, 0, w, h}, allocator) {

    }

    node::node(const rect& r, tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_left(), 
    m_right(), 
    m_rect(r), 
    m_ID(NULL_ID) {

    }

    node::node(node&& n) :
    m_allocator(n.m_allocator),
    m_left(std::move(n.m_left)),
    m_right(std::move(n.m_right)),
    m_rect(n.m_rect),
    m_ID(n.m_ID) {
        n.m_ID          = NULL_ID;
    }
    
    node& node::operator= (node&& n) {
        if (&n != this) {
            m_left      = std::move(n.m_left);
            m_right     = std::move(n.m_right);
            m_rect      = n.m_rect;
            m_ID        = n.m_ID;
            n.m_ID      = NULL_ID;
        }
        return *this;
    }
    
    node::~node() {
        
    }

    bool node::is_leaf() const {
        return !m_left && !m_right;
    }

    node* node::put_image(int w, int h, std::size_t imageID) {
        
        if (m_allocator == nullptr)
            throw_except<illegal_state_exception>("allocator must be != null");
        
        if (w < 0 || h < 0)
            throw_except<illegal_argument_exception>("Width or height must be >= 0");

        if (!is_leaf()){
			node* n = m_left->put_image(w, h, imageID);
			if (n != nullptr)
				return n;
			return m_right->put_image(w, h, imageID);
		}

        if (m_ID != NULL_ID || m_rect.w < w || m_rect.h < h)
			return nullptr;

        
		if (m_rect.w == w && m_rect.h == h){
			m_ID = imageID;
			return this;
		}
		
        polymorph::unique_ptr<node> left, right;
        
        if (m_rect.w - w > m_rect.h - h) //Если высота больше ширины, то разбиваем по x
        {
            left    = polymorph::allocate_unique<node>(m_allocator, node({m_rect.x, m_rect.y, w, m_rect.h}, m_allocator));
            right   = polymorph::allocate_unique<node>(m_allocator, node({m_rect.x + w, m_rect.y, m_rect.w - w, m_rect.h}, m_allocator));
		}
        else
        {								//Иначе по y
            left    = polymorph::allocate_unique<node>(m_allocator, node({m_rect.x, m_rect.y,m_rect.w, h}, m_allocator));
            right   = polymorph::allocate_unique<node>(m_allocator, node({m_rect.x, m_rect.y + h, m_rect.w, m_rect.h - h}, m_allocator));
		}
        
        m_left  = std::move(left);
        m_right = std::move(right);
        
		return m_left->put_image(w, h, imageID);
    }

    const rect& node::get_rect() const {
        return m_rect;
    }

    std::size_t node::get_id() const {
        return m_ID;
    }

}//namespace jstd
}//namespace texturing

//#################################################### ./src/img_utils.cpp ####################################################

#include <internal/img_utils.hpp>
#include <cpp/lang/utils/array_view.hpp>
#include <cpp/lang/array.hpp>
#include <cpp/lang/math.hpp>
#include <cassert>
#include <cerrno>

/**
 * Image resizing implementation using Mitchell-Netravali cubic filter.
 * 
 * This module provides high-quality image resizing using a separable filter approach
 * with the Mitchell-Netravali cubic kernel (B = 1/3, C = 1/3).
 * 
 * The resizing is performed in two passes: first horizontally, then vertically.
 * Each pass uses the Mitchell-Netravali filter with a radius of 2 for smooth,
 * artifact-free scaling.
 * 
 * The algorithm works by:
 * 1. Precomputing filter contributions for each output pixel in each dimension
 * 2. Applying horizontal convolution to a temporary buffer
 * 3. Applying vertical convolution to produce the final output
 * 
 * @example
 *      // Resize RGB image to 800x600
 *      int result = tc::internal::resize(input_data, 1920, 1080, 
 *                                         output_data, 800, 600, 3);
 *      if (result == 0) {
 *          // Success
 *      }
 */
namespace tc
{
namespace internal
{

/**
 * Per-pixel filter contribution data.
 * 
 * For each output pixel, this structure stores the range of input pixels
 * that contribute to it, along with their corresponding weights.
 * 
 * @note
 *      The weights are normalized so that they sum to 1.0, preserving
 *      overall image brightness during resizing.
 */
struct contribution {
    // Starting index of contributing input pixels.
    int start;
    
    // Ending index of contributing input pixels (inclusive)
    int end;
    
    // Normalized weights for each contributing pixel.
    tc::array<float> weights;
};

/**
 * Mitchell-Netravali cubic filter kernel.
 * 
 * A cubic spline filter with B = 1/3 and C = 1/3. 
 * 
 * @param x
 *      The distance from the center (absolute value).
 * 
 * @return
 *      The filter weight at distance x.
 * 
 * @note
 *      This filter provides a good balance between sharpness and ringing.
 *      It is widely used in image processing applications.
 * 
 * @see
 *      https://en.wikipedia.org/wiki/Mitchell%E2%80%93Netravali_filters
 */
static float mitchel(float x) {
    x = tc::math::abs(x);
    
    const float B = 1.0f / 3.0f;
    const float C = 1.0f / 3.0f;
    
    if (x < 1)
    {
        return ((12 - 9*B - 6*C) * (x*x*x) + (-18 + 12*B + 6*C) * (x*x) + (6 - 2*B)) * (1.0f / 6.0f);
    }
    else if (x < 2)
    {
        return ((-B - 6*C) * (x*x*x) + (6*B + 30*C) * (x*x) + (-12*B - 48*C) * x + (8*B + 24*C)) * (1.0f / 6.0f);
    }
    else
    {
        return 0;
    }
}

#if 0
static float linear(float x) {
    x = tc::math::abs(x);
    
    if (x < 1.0f)
    {
        return 1.0f - x;
    }
    
    return 0.0f;
}

static float sinc(float x) {
    return math::sin(x * math::PI) / (x * math::PI);
}

static float lanczos(float x) {
    if (x == 0) return 1;
    const float RAD = 2.0f;
    
    x = math::abs(x);
    if (x < RAD)
    {
        return sinc(x) * sinc(x / RAD);
    }

    return 0;
}
#endif

/**
 * Calculates filter weights for a single output pixel.
 * 
 * For a given output pixel position, this function computes the weights
 * for all input pixels that contribute to it based on the Mitchell-Netravali filter.
 * 
 * The algorithm:
 * 1. For each input pixel in the range [start, start+count):
 *    - Calculate the distance from the input pixel center to the output position
 *    - Apply scaling factor if downscaling (scale_factor < 1.0)
 *    - Compute filter weight using Mitchell kernel
 * 2. Normalize weights so they sum to 1.0
 * 
 * @param weights
 *      Output array to store calculated weights.
 * 
 * @param center
 *      The floating-point center position of the output pixel.
 * 
 * @param start
 *      The starting input pixel index.
 * 
 * @param count
 *      The number of contributing input pixels.
 * 
 * @param scale_factor
 *      The scaling factor for this dimension (w_out / w_in or h_out / h_in).
 * 
 * @note
 *      When downscaling (scale_factor < 1.0), the distances are multiplied
 *      by the scale factor to effectively broaden the filter's reach and
 *      prevent aliasing (anti-aliasing).
 * 
 * @note
 *      Weights are normalized by dividing by their sum to preserve
 *      overall brightness in the output image.
 */
static void calc_weights(tc::array<float>& weights, float center, int start, int count, float scale_factor) {
    float sum_weights = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        int x     = start + i;
        float dst = math::abs(static_cast<float>(x) + 0.5f - center);
        
        if (scale_factor < 1.0f)
        {
            dst *= scale_factor;
        }
        
        float weight = mitchel(dst);

        weights[i]   = weight;
        sum_weights += weight;
    }

    if (sum_weights > 0.0f)
    {
        for (int i = 0; i < count; ++i)
        {
        weights[i] /= sum_weights;
        }
    }
}

/**
 * Initializes filter contributions for one dimension.
 * 
 * Precomputes the contribution structure for each output pixel in a dimension.
 * For each output pixel, this function determines which input pixels
 * contribute to it and calculates their weights using the Mitchell-Netravali filter.
 * 
 * @param contrib_param
 *      Pointer to contribution array to fill.
 * 
 * @param in_size
 *      The input dimension size (w_in or h_in).
 * 
 * @param out_size
 *      The output dimension size (w_out or h_out).
 * 
 * @param alloc
 *      Allocator for allocate weight buffers
 * 
 * @note
 *      The contribution array must be pre-allocated with out_size elements.
 * 
 * @note
 *      The sampling radius depends on the scaling direction:
 *      - Upscaling (out_size > in_size): fixed radius of 2 pixels
 *      - Downscaling (out_size < in_size): radius of 2/scale to prevent aliasing
 * 
 * @note
 *      Mitchell-Netravali uses radius 2 (support of 4 pixels on each side).
 *      This provides good quality with reasonable performance.
 */
static void init_contribution(array_view<contribution>& contrib_param, int w_in, int w_out, tca::allocator* alloc) {

    // The ratio of the new to the old one
    float wscale    = static_cast<float>(w_out) / static_cast<float>(w_in);

    // Sampling radius
    // When downscaling, we need a larger input radius
    const float RAD = 2.0f;
    
    // const float RAD = 1.0f;

    {
        float xrad = wscale < 1.0f ? (RAD / wscale) : RAD;
        for (int x = 0; x < w_out; ++x)
        {
            // Center of the output pixel in input pixel coordinates
            // 
            // The +0.5f aligns pixel centers correctly (pixel center convention)
            float center = (static_cast<float>(x) + 0.5f) / wscale;
            
            // Determine the range of input pixels that contribute

            int start    = math::clamp( 
                static_cast<int>( math::floor(center - xrad) ), 0, w_in - 1 
            );
            int end      = math::clamp(
                static_cast<int>( math::ceil (center + xrad) ), 0, w_in - 1
            );
            
            contribution& contrib = contrib_param[x];

            int count         = end - start + 1;
            contrib.start    = start;
            contrib.end      = end;
            contrib.weights  = array<float>(count, alloc);

            // Calculate actual weights for all contributing input pixels
            calc_weights(contrib.weights, center, start, count, wscale);
        }
    }
}

/**
 * Resizes an image using Mitchell-Netravali separable filter.
 * 
 * This function resizes an image to new dimensions using high-quality
 * Mitchell-Netravali cubic resampling. The process is performed in two
 * separable passes:
 * 1. Horizontal resizing (width) to a temporary buffer
 * 2. Vertical resizing (height) to the final output buffer
 * 
 * The separable approach is significantly faster than a full 2D convolution
 * while producing nearly identical results.
 * 
 * The Mitchell-Netravali filter (B = 1/3, C = 1/3) provides:
 * - Excellent image quality with minimal ringing
 * - Smooth transitions without overshoot
 * - Good sharpness retention
 * - Ideal for photographic images
 * 
 * @param in
 *      Input image data in row-major order.
 * 
 * @param w_in
 *      Input image width.
 * 
 * @param h_in
 *      Input image height.
 * 
 * @param out
 *      Output image data buffer (must be pre-allocated).
 * 
 * @param w_out
 *      Desired output width.
 * 
 * @param h_out
 *      Desired output height.
 * 
 * @param channels
 *      Number of color channels (1 for grayscale, 3 for RGB, 4 for RGBA).
 * 
 * @param alloc
 *      Allocator for temporary buffers.
 * 
 * @return 0 on success, non-zero on error.
 * 
 * @note
 *      The algorithm uses a two-pass separable convolution:
 *      - First pass: Resize width using horizontal filter
 *      - Second pass: Resize height using vertical filter
 * 
 * @note
 *      Temporary buffer size: w_out * h_in * channels * sizeof(float)
 *      This can be memory-intensive for large images.
 * 
 * @warning
 *      The output buffer must be at least w_out * h_out * channels bytes.
 *      The input and output buffers must not overlap.
 * 
 * @example
 *      // Resize a 1920x1080 RGB image to 800x600
 *      unsigned char* input = load_image("input.jpg");
 *      unsigned char* output = new unsigned char[800 * 600 * 3];
 *      int result = tc::internal::resize(input, 1920, 1080, 
 *                                         output, 800, 600, 3);
 * 
 * @see
 *      init_contribution
 *      mitchel
 *      calc_weights
 */
int resize_image(const unsigned char* in, int w_in, int h_in, unsigned char* out, int w_out, int h_out, int channels, tca::allocator* alloc) {
    tc::array_view<const unsigned char>  input (in, static_cast<std::size_t> (w_in * h_in * channels));
    tc::array_view<unsigned char>        output(out, static_cast<std::size_t>(w_out * h_out * channels));
    
    tc::array<float> tmp (w_out * h_in * channels, alloc);

    // An attempt to reduce the number of selections. One large array is allocated that stores data for rows and columns.
    tc::array<contribution> contributes(w_out + h_out, alloc);
    
    tc::array_view<contribution> contrib_x (contributes.data(),  w_out);    
    tc::array_view<contribution> contrib_y (contributes.data() + w_out, h_out);

    init_contribution(contrib_x, w_in, w_out, alloc);
    init_contribution(contrib_y, h_in, h_out, alloc);

    {// width
        for (int y = 0; y < h_in; ++y)
        {
            for (int x = 0; x < w_out; ++x)
            {
                int idx = (x + y * w_out) * channels;
                contribution& contrib = contrib_x[x];
                for (int c = 0; c < channels; ++c)
                {
                    float result = 0;
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_x   = contrib.start + i;
                        int from_idx = (from_x + y * w_in) * channels;
                        result += static_cast<float>(input[from_idx + c]) * contrib.weights[i];
                    }
                    tmp[idx + c] = result;
                }
            }
        }
    }

    {// height
        for (int x = 0; x < w_out; ++x)
        {
            for (int y = 0; y < h_out; ++y)
            {
                int idx = (x + y * w_out) * channels;
                const contribution& contrib = contrib_y[y];
                for (int c = 0; c < channels; ++c)
                {
                    float result = 0;
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_y   = contrib.start + i;
                        int from_idx = (x + from_y * w_out) * channels;
                        result += tmp[from_idx + c] * contrib.weights[i];
                    }
                    output[idx + c] = static_cast<unsigned char>(math::round(math::clamp(result, 0.0f, 255.0f)));
                }
            }
        }
    }
    return 0;
}

/**
 * Resizes an image with alpha channel support using Mitchell-Netravali filter.
 * 
 * This function resizes an image with special handling for alpha channel.
 * Unlike standard resize, it properly handles premultiplied alpha by:
 * 1. During horizontal pass, non-alpha channels are multiplied by their alpha value
 * 2. During vertical pass, the alpha channel is processed separately
 * 3. After convolution, non-alpha channels are divided by alpha to restore
 *    the original color values (un-premultiplication)
 * 
 * This approach ensures that colors at the edges of transparent regions
 * don't bleed incorrectly, which is a common problem when resizing images
 * with alpha channel using naive filtering.
 * 
 * The algorithm:
 * 1. Horizontal pass: multiply each pixel's RGB by its alpha (premultiply)
 * 2. Vertical pass: process RGB and alpha separately
 * 3. Final: divide RGB by alpha to restore original colors (un-premultiply)
 * 
 * @param in
 *      Input image data in row-major order.
 * 
 * @param w_in
 *      Input image width.
 * 
 * @param h_in
 *      Input image height.
 * 
 * @param out
 *      Output image data buffer (must be pre-allocated).
 * 
 * @param w_out
 *      Desired output width.
 * 
 * @param h_out
 *      Desired output height.
 * 
 * @param channels
 *      Number of color channels (3 for RGB, 4 for RGBA).
 * 
 * @param alpha_index
 *      Index of the alpha channel (-1 if no alpha).
 * 
 * @param alloc
 *      Allocator for temporary buffers.
 * 
 * @return 
 *      0 on success, non-zero on error.
 * 
 * @note
 *      This function is essential for resizing images with transparency.
 *      Without this handling, transparent edges would show dark or light
 *      fringes (color bleeding from adjacent pixels).
 * 
 * @note
 *      The function uses the same Mitchell-Netravali filter as the standard
 *      resize, but with additional alpha-aware processing.
 * 
 * @warning
 *      The alpha_index must be valid (0..channels-1) if alpha is present.
 *      For RGB images (no alpha), pass alpha_index = -1.
 * 
 * 
 * This ensures that transparent pixels don't incorrectly contribute color
 * to the final result.
 */
int resize_image_alpha(const unsigned char* in, int w_in, int h_in, unsigned char* out, int w_out, int h_out, int channels, int alpha_index, tca::allocator* alloc) {
    const std::size_t MAX_CHANNELS = 4;

    if (alpha_index >= channels)
        return EINVAL;
        
    tc::array_view<const unsigned char>  input (in, static_cast<std::size_t> (w_in * h_in * channels));
    tc::array_view<unsigned char>        output(out, static_cast<std::size_t>(w_out * h_out * channels));
    
    tc::array<float> tmp (w_out * h_in * channels, alloc);

    // An attempt to reduce the number of selections. One large array is allocated that stores data for rows and columns.
    tc::array<contribution> contributes(w_out + h_out, alloc);
    
    tc::array_view<contribution> contrib_x (contributes.data(),         w_out);    
    tc::array_view<contribution> contrib_y (contributes.data() + w_out, h_out);

    init_contribution(contrib_x, w_in, w_out, alloc);
    init_contribution(contrib_y, h_in, h_out, alloc);

    {// width
        for (int y = 0; y < h_in; ++y)
        {
            for (int x = 0; x < w_out; ++x)
            {
                int idx = (x + y * w_out) * channels;
                contribution& contrib = contrib_x[x];
                for (int c = 0; c < channels; ++c)
                {
                    float result = 0;
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_x   = contrib.start + i;
                        int from_idx = (from_x + y * w_in) * channels;
                        float pixel = static_cast<float>(input[from_idx + c]);
                        
                        if (alpha_index >= 0 && c != alpha_index)
                        {
                            float a = static_cast<float>(input[from_idx + alpha_index]) / 255.0f;
                            pixel *= a;
                        }

                        result += pixel * contrib.weights[i];

                    }
                    tmp[idx + c] = result;
                }
            }
        }
    }

    {// height
        for (int x = 0; x < w_out; ++x)
        {
            for (int y = 0; y < h_out; ++y)
            {
                int idx = (x + y * w_out) * channels;
                const contribution& contrib = contrib_y[y];
                
                float pixels[MAX_CHANNELS] = {0.0f};
                for (int c = 0; c < channels; ++c)
                {
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_y   = contrib.start + i;
                        int from_idx = (x + from_y * w_out) * channels;
                        pixels[c]   += tmp[from_idx + c] * contrib.weights[i];
                    }
                }
                
                float alpha = 1.0f;
                if (alpha_index >= 0)
                {
                    alpha = pixels[alpha_index] / 255.0f;
                }

                for (int c = 0; c < channels; ++c)
                {
                    float pixel = pixels[c];
                    if (alpha_index >= 0 && c != alpha_index)
                    {
                        if (alpha >= 0.001f)
                        {
                            pixel /= alpha;
                        }
                        else
                        {
                            pixel = 0.0f;
                        }
                    }
                    output[idx + c] = static_cast<unsigned char>(math::round(math::clamp(pixel, 0.0f, 255.0f)));
                }
                
            }
        }
    }
    return 0;
}

} //namespace internal
} //namespace tc

//#################################################### ./src/imstream.cpp ####################################################

#include <cpp/lang/io/imstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>

namespace tc
{


    imstream::imstream() : m_buffer(nullptr), m_capacity(0), m_offset(0) {

    }

    imstream::imstream(const char* buffer, std::size_t capacity) : m_buffer(buffer), m_capacity(capacity), m_offset(0) {
        if (buffer == nullptr)
            throw_except<null_pointer_exception>("buffer is null");
    }
    
    imstream::imstream(imstream&& stream) : m_buffer(stream.m_buffer), m_capacity(stream.m_capacity), m_offset(stream.m_offset) {
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
    }
    
    imstream& imstream::operator= (imstream&& stream) {
        if (&stream != this) {
            if (m_buffer != nullptr)
                close();
            m_buffer     = stream.m_buffer;
            m_capacity   = stream.m_capacity;
            m_offset     = stream.m_offset;

            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
        }
        return *this;
    }
    
    imstream::~imstream() {

    }
    
    std::size_t imstream::read(char buf[], std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (m_buffer == nullptr)
                throw_except<io_exception>("Stream is null");
        );
        sz = std::min(m_capacity - m_offset, sz);
        if (sz == 0)
            return istream::eof_value();

        std::memcpy(buf, m_buffer + m_offset, sz);
        m_offset += sz;
        
        return sz;
    }

    void imstream::close(error_code& err) {
        m_buffer = nullptr;
    }
    
    void imstream::reset() {
        m_offset = 0;
    }
}

//#################################################### ./src/inet.cpp ####################################################

#include <cpp/lang/net/inet.hpp>
#include <cpp/lang/exceptions.hpp>

#if defined(_WIN32)
#include <winsock2.h>
#include <internal/bsd_socket_errors.hpp>
#define __VERSION22 MAKEWORD(2, 2)
#define __VERSION20 MAKEWORD(2, 0)
#endif//_WIN32


namespace tc {

    /*static*/ bool inet_context::already_init = false;
    inet_context::inet_context() {
        init_inet();
    }

    inet_context::~inet_context() {
        close_inet();
    }

    void init_inet() {
#if defined(_WIN32)
        WSAData data;
        int err = WSAStartup(__VERSION22, &data);
        if (err != 0)
            throw_except<socket_exception>("Windows socket initialize error: %s", bsd_socket::socket_error_string());
#endif
        if (inet_context::already_init)
            throw_except<illegal_state_exception>("inet_contex already initialized!");
        inet_context::already_init = true;
    }
    
    void close_inet() {
#if defined(_WIN32)
        WSACleanup();
#endif
        if (!inet_context::already_init)
            throw_except<illegal_state_exception>("inet_contex already deinitialized!");
        inet_context::already_init = false;
    }
}

//#################################################### ./src/inetaddr.cpp ####################################################

#include <cpp/lang/net/inetaddr.hpp>

#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno>
#elif defined(JSTD_OS_WINDOWS)
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#else 
#error Unsupported platform!
#endif

#include <cstring>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/string.hpp>
#include <allocators/inline_linear_allocator.hpp>
#include <cpp/lang/array.hpp>
#include <cpp/lang/numbers.hpp>
#include <internal/inet/ip_parser.h>

#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
#define UNIX_CODE(code) code
#define WIN_CODE(code)
#elif defined(JSTD_OS_WINDOWS)
#define UNIX_CODE(code)
#define WIN_CODE(code) code
#endif

namespace tc
{

  /*static */ const char* getStringError() {
    
    #if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
            return strerror(errno);
    #elif _WIN32
            int error = WSAGetLastError();
            switch(error){
                case 0:
                    return "No error!";
                case WSAEAFNOSUPPORT: 
                    return "Inet family type is not suppoted!";
                case ERROR_INVALID_PARAMETER : 
                    return "Invalid parameter! Buffer overflow or buffer length equal zero!";
            }
            return "InetAddress unknow error!";
    #endif
    }
    
    //У меня нет ни малейшего понятия, что значит Gai в названии функции.
    //Two tausen years later: Gai = это GetAddrInfo
    /*static*/ const char* getGaiStringError(int errorCode) {
#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
        return gai_strerror(errorCode);
#elif defined(JSTD_OS_WINDOWS)
        switch (errorCode) {
            case 0: 
                return "No error!";
            case WSATRY_AGAIN:  // EAI_AGAIN
                return "A temporary failure in name resolution occurred.";
            case WSAEINVAL:  // EAI_BADFLAGS
                return "An invalid value was provided for the ai_flags member of the pHints parameter.";
            case WSANO_RECOVERY:  // EAI_FAIL
                return "A nonrecoverable failure in name resolution occurred.";
            case WSAEAFNOSUPPORT:  // EAI_FAMILY
                return "The ai_family member of the pHints parameter is not supported.";
            case WSA_NOT_ENOUGH_MEMORY:  // EAI_MEMORY
                return "A memory allocation failure occurred.";
            case WSAHOST_NOT_FOUND:  // EAI_NONAME
                return "The name does not resolve for the supplied parameters or the pNodeName and pServiceName parameters were not provided.";
            case WSATYPE_NOT_FOUND:  // EAI_SERVICE
                return "The pServiceName parameter is not supported for the specified ai_socktype member of the pHints parameter.";
            case WSAESOCKTNOSUPPORT:  // EAI_SOCKTYPE
                return "The ai_socktype member of the pHints parameter is not supported.";
            case WSANOTINITIALISED :
                return "Windows sockets is not initialized.";
        }
        return "InetAddress unknow error!";
#endif
    }


    /*static*/ const inet_family inet_family::NONE = {0};
    /*static*/ const inet_family inet_family::IPV4(AF_INET);
    /*static*/ const inet_family inet_family::IPV6(AF_INET6);

    inet_address::inet_address() : IPv4(), family(inet_family::IPV4) {
        
        std::memset(m_data_buffer, 0, sizeof(m_data_buffer));
        m_host_name[0] = '\0';
    }

    inet_address::inet_address(inet_family family) : inet_address() {
        this->family = family;
    }

    inet_address::inet_address(const inet_address& addr) : IPv4(), family(addr.family) {
        
        std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
        std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
    }
    
    inet_address::inet_address(inet_address&& addr) : IPv4(), family(addr.family) {
        
        std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
        std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
    }

    inet_address& inet_address::operator= (const inet_address& addr) {
        
        if (&addr != this){
            std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
            std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
            family = addr.family;
        }
        return *this;
    }
    
    inet_address& inet_address::operator= (inet_address&& addr) {
        
        if (&addr != this)
        {
            std::memcpy(m_host_name, addr.m_host_name, sizeof(addr.m_host_name));
            std::memcpy(m_data_buffer, addr.m_data_buffer, sizeof(addr.m_data_buffer));
            family = addr.family;
        }

        return *this;
    }

    inet_address::~inet_address() {
        
    }

    bool contains(const char* str, char c) {
        for (int i = 0; ;++i)
        {
            if (str[i] == '\0')
                break;
            if (str[i] == c)
                return true;
        }
        return false;
    }

    inet_address::inet_address(const char* ip) {
        std::memset(m_host_name, 0, sizeof(m_host_name));
        
        if (contains(ip, '.'))
        {
            family = inet_family::IPV4;
            (*this) = parse_IPv4(ip);
        }
        else if(contains(ip, ':'))
        {
            family = inet_family::IPV6;
            (*this) = parse_IPv6(ip);
        }
        else
        {
            throw_except<illegal_state_exception>("Illegal address: %s", ip);
        }
    }

    /*static*/ inet_address inet_address::as_ip4(const unsigned char buf[], std::size_t bufsize) {
        if (bufsize < 4)
            throw_except<illegal_argument_exception>("bufsize must be >= 4");
        
        inet_address ip4;
        ip4.family = inet_family::IPV4;
        for (std::size_t i = 0; i < 4; ++i)
            ip4.IPv4.m_byte_view[i] = (uint8_t) buf[i];
        
        return ip4;
    }

    /*static*/ inet_address inet_address::as_ip6(const uint16_t buf[], std::size_t bufsize) {
        if (bufsize < 8)
            throw_except<illegal_argument_exception>("bufsize must be >= 8");
        
        inet_address ip6;
        ip6.family = inet_family::IPV6;
        for (std::size_t i = 0; i < 8; ++i)
            ip6.IPv6.m_short_view[i] = (uint16_t) htons(buf[i]);

        return ip6;
    }

    inet_address inet_address::parse_IPv4(const char* ip_string) {
        JSTD_DEBUG_CODE(
            if (ip_string == nullptr)
                throw_except<illegal_argument_exception>("ip_string == null");
        );
        using namespace net::internal;
        return ip_parser(ip_string).parse(inet_family::IPV4);
    }

    inet_address inet_address::parse_IPv6(const char* ip_string) {
        JSTD_DEBUG_CODE(
            if (ip_string == nullptr)
                throw_except<illegal_argument_exception>("ip_string == null");
        );
        using namespace net::internal;
        return ip_parser(ip_string).parse(inet_family::IPV6);
    }

    std::size_t inet_address::hashcode() const {
        if (family == inet_family::IPV4)
        {
            const uint8_t* b = IPv4.m_byte_view;
            const uint8_t* e = IPv4.m_byte_view + sizeof(IPv4.m_byte_view) / sizeof(*IPv4.m_byte_view);
            return objects::hashcode(b, e, hash_for<uint8_t>());
        } 
        else if (family == inet_family::IPV6)
        {
            const uint8_t* b = IPv4.m_byte_view;
            const uint8_t* e = IPv4.m_byte_view + sizeof(IPv4.m_byte_view) / sizeof(*IPv4.m_byte_view);
            return objects::hashcode(b, e, hash_for<uint8_t>());
        }
        return 0;
    }
    
    bool inet_address::equals(const inet_address& addr) const {
        if (&addr == this)
            return true;
        
        if (family == inet_family::IPV4 && addr.family == inet_family::IPV4)
        {
            
            const uint8_t* b0 = IPv4.m_byte_view;
            const uint8_t* e0 = IPv4.m_byte_view + sizeof(IPv4.m_byte_view) / sizeof(*IPv4.m_byte_view);
            const uint8_t* b1 = addr.IPv4.m_byte_view;
            const uint8_t* e1 = addr.IPv4.m_byte_view + sizeof(addr.IPv4.m_byte_view) / sizeof(*addr.IPv4.m_byte_view);

            return  objects::equals(b0, e0, b1, e1) && family == addr.family;
        } 
        
        else if (family == inet_family::IPV6 && addr.family == inet_family::IPV6)
        {
            
            const uint8_t* b0 = IPv6.m_byte_view;
            const uint8_t* e0 = IPv6.m_byte_view + sizeof(IPv6.m_byte_view) / sizeof(*IPv6.m_byte_view);
            const uint8_t* b1 = addr.IPv6.m_byte_view;
            const uint8_t* e1 = addr.IPv6.m_byte_view + sizeof(addr.IPv6.m_byte_view) / sizeof(*addr.IPv6.m_byte_view);
            
            return  objects::equals(b0, e0, b1, e1) && family == addr.family;
        }

        return false;
    }

    inet_family inet_address::get_family() const {
        return family;
    }

    const char* inet_address::get_host_name() const {
        return m_host_name;
    }

    /*static*/ int inet_address::IPv4ToString(char buf[], std::size_t bufsize, const inet_address& address) {
        if (buf == nullptr)
            throw_except<null_pointer_exception>("buf is null!");
        
        const uint32_t addr = address.IPv4.m_int_view;
        const uint8_t b1 =  (uint8_t) (addr           & 0xff);
        const uint8_t b2 =  (uint8_t) ((addr >> 8)    & 0xff);
        const uint8_t b3 =  (uint8_t) ((addr >> 16)   & 0xff);
        const uint8_t b4 =  (uint8_t) ((addr >> 24)   & 0xff);
        
        return snprintf(buf, bufsize, "%s/%d.%d.%d.%d", address.m_host_name, b1, b2, b3, b4);
    }
        
    /*static*/ int inet_address::IPv6ToString(char buf[], std::size_t bufsize, const inet_address& address) {
        
        const internal::net::ipv6_addr* addr = &address.IPv6;
        uint16_t values[8];
    
        for (int i = 0; i < 8; ++i)
            values[i] = ntohs(addr->m_short_view[i]);

        return snprintf(buf, bufsize, "%s/%x:%x:%x:%x:%x:%x:%x:%x", address.m_host_name, values[0], values[1], values[2], values[3], values[4], values[5], values[6], values[7]);
    }

    /*static*/ optional<inet_address> inet_address::get_by_name(const char* domain) {
        inet_address addr;
        std::size_t count = get_all_by_name(&addr, 1, domain);
        return count > 0 ? addr : optional<inet_address>::null_opt();
    }

    inet_address inet_address::as_in_addr(const in_addr* addr_in) {
        JSTD_DEBUG_CODE(
            if (addr_in == nullptr)
                throw_except<illegal_argument_exception>("addr_in == null");
        );
        inet_address addr;
        addr.family             = inet_family::IPV4;
        addr.IPv4.m_int_view    = addr_in->s_addr;
        return addr;
    }
    
    inet_address inet_address::as_in6_addr(const in6_addr* addr_in) {
        JSTD_DEBUG_CODE(
            if (addr_in == nullptr)
                throw_except<illegal_argument_exception>("addr_in == null");
        );
        inet_address addr;
        addr.family = inet_family::IPV6;
        for (int i = 0; i < 16; ++i)
            addr.IPv6.m_byte_view[i] = addr_in->s6_addr[i];
        return addr;
    }

    void inet_address::get_in_addr(in_addr* addr_out) const {
        JSTD_DEBUG_CODE(
            if (addr_out == nullptr)
                throw_except<illegal_argument_exception>("addr_out == null");
            if (family != inet_family::IPV4)
                throw_except<illegal_state_exception>("family is not IPv4");
        );
        addr_out->s_addr = IPv4.m_int_view;
    }

    void inet_address::get_in6_addr(in6_addr* addr_out) const {
        JSTD_DEBUG_CODE(
            if (addr_out == nullptr)
                throw_except<illegal_argument_exception>("addr_out == null");
            if (family != inet_family::IPV6)
                throw_except<illegal_state_exception>("family is not IPv6");
        );
        for (int i = 0; i < 16; ++i) 
            addr_out->s6_addr[i] = IPv6.m_byte_view[i];
    }

    /*static*/ std::size_t inet_address::get_all_by_name(inet_address dst[], std::size_t bufsize, const char* domain) {
        
        addrinfo addrInf;
        memset(&addrInf, 0, sizeof(addrInf));
        addrInf.ai_protocol     = AF_UNSPEC;
        addrInf.ai_socktype     = SOCK_STREAM;
        addrInf.ai_flags       |= AI_CANONNAME;

        addrinfo* begin = nullptr;
        int errcode   = getaddrinfo(domain, nullptr, &addrInf, &begin);
        
        if (errcode != 0)
            throw_except<illegal_state_exception>(getGaiStringError(errcode));    

        std::size_t off = 0;
        for (addrinfo* i = begin; i != nullptr && off < bufsize; i = i->ai_next)
        {

            inet_address addr;
            if (i->ai_family == inet_family::IPV4)
            {
                const sockaddr_in* sock4  = (const sockaddr_in*) i->ai_addr;
                addr = inet_address::as_in_addr(&sock4->sin_addr);
            }
            else if(i->ai_family == inet_family::IPV6)
            {
                const sockaddr_in6* sock6  = (const sockaddr_in6*) i->ai_addr;
                addr = inet_address::as_in6_addr(&sock6->sin6_addr);
            } 
            else
            {
                continue;
            }

            std::snprintf(addr.m_host_name, sizeof(addr.m_host_name), "%s", domain);

            dst[off++] = addr;
        }
        
        freeaddrinfo(begin);  
        return off;
    }


    /*static*/ inet_address inet_address::localhost(inet_family family) {
        if (family == inet_family::IPV4)
        {
            uint8_t ip[] {127, 0, 0, 1};
            return inet_address::as_ip4(ip, sizeof(ip));
        } 
        else if (family == inet_family::IPV6)
        {
            uint16_t ip[] {0, 0, 0, 0, 0, 0, 0, 1};
            return inet_address::as_ip6(ip, sizeof(ip));
        } 
        else
        {
            throw make_except<illegal_argument_exception>("Family %i is not family address", (int) family);
        }
    }

    tc::string inet_address::to_string(tca::allocator* allocator) const {
        tc::string result (allocator);

        char buf[64];

        if (family == inet_family::IPV4)
        {
            IPv4ToString(buf, sizeof(buf), *this);
        }
        else if (family == inet_family::IPV6)
        {
            IPv6ToString(buf, sizeof(buf), *this);
        }
        else
        {
            throw_except<illegal_state_exception>("Ip not v4 or v6");
        }

        result.append(buf);

        return result;
    }

    socket_address::socket_address() : m_address(), m_port(0) {

    }

    socket_address::socket_address(const inet_address& address, unsigned int port) : m_address(address), m_port(port) {

    }

    socket_address::socket_address(const socket_address& sa) : m_address(sa.m_address), m_port(sa.m_port) {

    }

    socket_address::socket_address(socket_address&& sa) : m_address(std::move(sa.m_address)), m_port(sa.m_port) {

    }

    socket_address& socket_address::operator= (const socket_address& sa) {
        if (&sa != this) {
            m_address    = sa.m_address;
            m_port       = sa.m_port;
        }
        return *this;
    }
    
    socket_address& socket_address::operator= (socket_address&& sa) {
        if (&sa != this) {
            m_address    = std::move(sa.m_address);
            m_port       = sa.m_port;
        }
        return *this;
    }

    const inet_address& socket_address::get_address() const {
        return m_address;
    }
    
    unsigned int socket_address::get_port() const {
        return m_port;
    }

    std::size_t socket_address::hashcode() const {
        std::size_t hash = m_address.hashcode();
        hash = hash + (std::size_t) 233 * m_port;
        return hash;
    }
    
    bool socket_address::equals(const socket_address& sock_addr) const {
        if (&sock_addr == this)
            return true;
        return m_port == sock_addr.m_port && m_address.equals(sock_addr.m_address);
    }

    string socket_address::to_string(tca::allocator* alloc) const {
        string result(alloc);
        result
            .append("[addr=").append(m_address.to_string(alloc))
            .append(", port=").append(tc::to_string(m_port, alloc))
            .append(']');
        return result;
    }
    
}

//#################################################### ./src/iostream.cpp ####################################################

#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/errcode.hpp>
#include <allocators/inline_linear_allocator.hpp>
#include <internal/io/io_helpers.hpp>

namespace tc
{
    std::size_t istream::skip(std::size_t n) {
        std::size_t skipped = 0;
        while (n > 0)
        {
            int readed = read();
            if (readed == -1)
                break;
            ++skipped;
            --n;
        }
        return skipped;
    }

    istream::~istream() {

    }

    ostream::~ostream() {
        
    }

    void ostream::write(char c) {
        write(&c, 1);
    }

    int istream::read() {
        char c;
        std::size_t readed = read(&c, 1);
        if (readed == istream::eof_value())
            return -1;
        return c & 0xFF;
    }

    void istream::close() {
        error_code err;
        close(err);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }

    void ostream::close() {
        error_code err;
        close(err);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }
} 

//#################################################### ./src/io_helpers.cpp ####################################################

#include <internal/io/io_helpers.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/errcode.hpp>
#include <allocators/inline_linear_allocator.hpp>

namespace tc
{
namespace internal
{
namespace io
{
    
    void throw_error_code(const error_code& err) {
        tca::inline_linear_allocator<1024> alloc;
        tc::string msg = err.get_message(&alloc);
        if (err == error_condition(errcode::permission_denied, generic_category()))
        {
            throw_except<security_exception>("%s", msg.c_str());    
        }
        throw_except<io_exception>("%s", msg.c_str());
    }

} //namespace io
} //namespace internal
} //namespace tc

//#################################################### ./src/ip_parser.cpp ####################################################

#include <internal/inet/ip_parser.h>
#include <cstdio>
#include <cctype>
#include <cassert>
#include <cstdlib>

namespace tc
{
namespace net
{
namespace internal
{
    ip_parser::ip_parser(const char* ip_string) : 
    str(ip_string), 
    cur(0),
    off(0),
    len(std::strlen(ip_string)),
    end(false) {
        
    }

    void ip_parser::next_char() {
        if (off >= len)
        {
            end = true;
            cur = 0;
            return;
        }
        cur = str[off++];
    }

    char ip_parser::get_char() const {
        return cur;
    }

    bool ip_parser::eof() const {
        return end;
    }

    token ip_parser::next_tok() {
        if (eof())  return token(token_kind::END);
        if (cur == '\0') next_char();
        
        const std::size_t BUF_SIZE = 64;
        char digits[BUF_SIZE];

        if (get_char() == '.')
        {
            next_char();
            return token(token_kind::DOT);
        }
        else if (get_char() == ':')
        {
            next_char();
            if (get_char() == ':')
            {
                next_char();
                return token(token_kind::COLON2);
            }
            return token(token_kind::COLON);
        }
        else if (std::isxdigit( get_char() ) || std::isdigit( get_char() ))
        {
            bool is_hex = false;
            std::size_t digits_pos = 0;
            while(std::isxdigit( get_char() ) || std::isdigit( get_char() ))
            {
                if (digits_pos + 1 >= BUF_SIZE)
                    throw_except<illegal_argument_exception>("'%s' is not ip", str);
                if ((get_char() >= 'a' && get_char() <= 'f') || (get_char() >= 'A' && get_char() <= 'F'))
                    is_hex = true;
                digits[digits_pos++] = get_char();
                next_char();
            }

            assert(digits_pos < BUF_SIZE);
            digits[digits_pos] = '\0';

            token number (token_kind::NUMBER);
            number.hex = -1;
            number.dec = -1;
            if (!is_hex)
                number.dec = strtol(digits, NULL, 10);
            number.hex = strtol(digits, NULL, 16);
            
            return number;
        }
        
        next_char();
        
        return token(token_kind::UNDEFINED);
    }

    inet_address ip_parser::parse_v4() {

        uint8_t parts[4];
        std::size_t part = 0;

        for (std::size_t i = 0; i < 8; ++i)
        {
            const token& tok = next_tok();
            if (i == 7)
            {
                if (tok.kind != token_kind::END)
                    goto ERR;
                break;
            }
            
            if (i % 2 != 0)
            {
                if (tok.kind != token_kind::DOT)
                    goto ERR;
            }
            else
            {
                if ((tok.kind != token_kind::NUMBER) || (tok.dec < 0 || tok.dec > 255))
                    goto ERR;
                parts[part++] = (uint8_t) (tok.dec & 0xFF);
            }
            
            continue;
            ERR:
                throw_except<illegal_argument_exception>("Invalid ipv4 address '%s'", str);
        }
        
        return inet_address::as_ip4(parts, 4);
    }
    
    inet_address ip_parser::parse_v6() {
        uint16_t parts[8] = {0};
        
        int double_colon_pos = -1;
        int pos = 0;
        
        while (!eof())
        {
            token tok = next_tok();
            if (tok.kind == token_kind::COLON2)
            {
                if (double_colon_pos != -1)
                    throw_except<illegal_argument_exception>("Invalid ipv6 address '%s'", str);
                double_colon_pos = pos;
            }
            else if (tok.kind == token_kind::NUMBER)
            {
                parts[pos++] = (uint16_t) (tok.hex & 0xFFFF);
            }
        }

        if (double_colon_pos != -1)
        {
            int shift = 8 - pos;
            for (int i = pos - 1; i >= double_colon_pos; --i)
                parts[i + shift] = parts[i];
            for (int i = 0; i < shift; ++i)
                parts[double_colon_pos + i] = 0;
        }
        else
        {
            if (pos != 8)
                throw_except<illegal_argument_exception>("Invalid ipv6 address '%s'", str);
        }

        return inet_address::as_ip6(parts, 8);
    }

    inet_address ip_parser::parse(inet_family family) {
        if (family == inet_family::IPV4) return parse_v4();
        if (family == inet_family::IPV6) return parse_v6();
        throw make_except<illegal_argument_exception>("%d is not inet family", (int) family);
    }
    

} //namespace internal
} //namespace net
} //namespace jstd

//#################################################### ./src/linear_allocator.cpp ####################################################

#include <allocators/linear_allocator.hpp>
#include <allocators/helpers.hpp>
#include <exception>
#include <cstdio>
#include <cstddef>

namespace tca
{

    linear_allocator::linear_allocator() : 
    allocator(nullptr),
    _buffer(nullptr),
    _capacity(0),
    _offset(0) {

    }

    linear_allocator::linear_allocator(void* baseBuffer, std::size_t capacity) : 
    allocator(nullptr),
    _buffer(baseBuffer), 
    _capacity(capacity), 
    _offset(0) {

    }

    linear_allocator::linear_allocator(std::size_t capacity, base_allocator* baseAllocator) : 
    allocator(baseAllocator), 
    _buffer(nullptr), 
    _capacity(capacity),
    _offset(0) {
        _buffer = m_parent->allocate_align(capacity, alignof(std::max_align_t));
    }

    linear_allocator::linear_allocator(linear_allocator&& alloc) : 
    allocator(std::move(alloc)), 
    _buffer(alloc._buffer), 
    _capacity(alloc._capacity), 
    _offset(alloc._offset) {
        alloc._buffer   = nullptr;
        alloc._capacity = 0;
        alloc._offset   = 0;
    }
    
    linear_allocator& linear_allocator::operator= (linear_allocator&& alloc) {
        if (&alloc != this) {
            if (m_parent != nullptr)
                m_parent->deallocate(_buffer, _capacity);
    
            base_allocator::operator=(std::move(alloc));

            _buffer     = alloc._buffer;
            _capacity   = alloc._capacity;
            _offset     = alloc._offset;
            alloc._buffer   = nullptr;
            alloc._capacity = 0;
            alloc._offset   = 0;

        }
        return *this;
    }

    linear_allocator::~linear_allocator() {
        if (m_parent != nullptr)
            m_parent->deallocate(_buffer, _capacity);
    }
    
    void* linear_allocator::allocate(std::size_t sz) {
        if ((_offset + sz > _capacity) || _buffer == nullptr){
            return nullptr;
        }
        char* buf = ((char*) _buffer) + _offset;
        _offset += sz;
        return (void*) buf;
    }

    void* linear_allocator::allocate_align(std::size_t sz, std::size_t align) {
        std::size_t off = align_up(_offset, align);
        if ((_offset + off + sz > _capacity) || _buffer == nullptr)
            return nullptr;
        _offset += off;
        char* buf = ((char*) _buffer) + _offset;
        _offset += sz;
        return (void*) buf;
    }
    
    void linear_allocator::deallocate(void*) {
    
    }

    int linear_allocator::to_string(char buf[], std::size_t buf_size) const {
        return snprintf(buf, buf_size, "[size %zu, offset %zu, free %zu]", _capacity, _offset, _capacity - _offset);
    }

    void linear_allocator::print() const {
        char strbuf[128];
        to_string(strbuf, sizeof(strbuf));
        std::printf("%s\n", strbuf);
    }
}

//#################################################### ./src/linear_compact_allocator.cpp ####################################################

// #include <allocators/linear_compact_allocator.hpp>
// #include <cpp/lang/exceptions.hpp>
// #include <exception>
// #include <iostream>

// namespace tca {

//     using namespace tc::internal::sptr;

//     /**
//      * 
//      * ############################################################################
//      *                           H E A D E R 
//      * ############################################################################
//      */
//     Header::Header() : _reference(nullptr), _movfunc(nullptr), _count(0) {}

//     Header::Header(Header&& h) : _reference(h._reference), _movfunc(h._movfunc), _count(h._count) {
//         if (_reference != nullptr) {
//             void* src = h.dataPointer();
//             void* dst = dataPointer();
//             _movfunc(dst, src, _count);
//             _reference->m_object = dst;
//         }
//         h._reference    = nullptr;
//         h._movfunc      = nullptr;
//         //h._size         = 0;
//         h._count        = 0;
//     }
    
//     Header::~Header() {
//         // if (_reference != nullptr) {
//         //     _reference->_data = nullptr;
//         //     _reference = nullptr;
//         // }
//     }

//     void* Header::dataPointer() {
//         char* _this = reinterpret_cast<char*>(this);
//         return _this + byteSize();
//     }

//     /**
//      * 
//      * ############################################################################
//      *                  L I N E A R _ C O M P A C T _ A L L O C A T O R
//      * ############################################################################
//      */

//     compact_linear_allocator::compact_linear_allocator(std::size_t capacity, base_allocator* allocator) : 
//     _allocator(allocator), 
//     m_ctrl_block_allocator(sizeof(tc::internal::sptr::shared_control_block), pool_allocator::DEFAULT_COUNT_BUCKETS, allocator), 
//     _capacity(capacity),
//     _offset(0) {
//         _data           = allocator->allocate_align(capacity, alignof(std::max_align_t));
//         _tmp            = nullptr;
//         _tmpCapacity    = 0;
//     }

//     compact_linear_allocator::compact_linear_allocator(compact_linear_allocator&& alloc) : 
//     _allocator(alloc._allocator),
//     m_ctrl_block_allocator(std::move(alloc.m_ctrl_block_allocator)),
//     _data(alloc._data),
//     _capacity(alloc._capacity),
//     _tmp(alloc._tmp),
//     _tmpCapacity(alloc._tmpCapacity),
//     _offset(alloc._offset)
//     {
//         alloc._allocator    = nullptr;
//         alloc._data         = nullptr;
//         alloc._capacity     = 0;
//         alloc._tmp          = nullptr;
//         alloc._tmpCapacity  = 0;
//         alloc._offset       = 0;
//     }
    
//     compact_linear_allocator& compact_linear_allocator::operator= (compact_linear_allocator&& alloc) {
//         if (&alloc != this) {
//             free();
//             _allocator              = alloc._allocator;
//             m_ctrl_block_allocator  = std::move(alloc.m_ctrl_block_allocator);
//             _data                   = alloc._data;
//             _capacity               = alloc._capacity;
//             _tmp                    = alloc._tmp;
//             _tmpCapacity            = alloc._tmpCapacity;
//             _offset                 = alloc._offset;

//             alloc._allocator    = nullptr;
//             alloc._data         = nullptr;
//             alloc._capacity     = 0;
//             alloc._tmp          = nullptr;
//             alloc._tmpCapacity  = 0;
//             alloc._offset       = 0;
//         }
//         return *this;
//     }

//     compact_linear_allocator::~compact_linear_allocator() {
//         free();
//     }

//     void compact_linear_allocator::free() {
//         if (_allocator != nullptr) {
//             if (_tmp != nullptr)
//                 _allocator->deallocate(_tmp,    _tmpCapacity);
//             if (_data != nullptr)
//                 _allocator->deallocate(_data,   _capacity);
//             _allocator  = nullptr;
//             _tmp        = nullptr;
//             _data       = nullptr;
//         }
//     }

//     void compact_linear_allocator::grow() {
//         grow(_capacity << 1);
//     }

//     void compact_linear_allocator::grow(std::size_t newCapacity) {
        
//         char* newData = reinterpret_cast<char*>(_allocator->allocate_align(newCapacity, alignof(std::max_align_t)));
//         if (newData == nullptr)
//             return;

//         for (std::size_t i = 0; i < _offset; ) {
//             Header* src     = reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) + i);
//             Header* dst = new(newData + i) Header(std::move(*src));
//             i += dst->_reference->m_blocksize;
//         }

//         _allocator->deallocate(_data, _capacity);
//         _data       = newData;
//         _capacity   = newCapacity;
//     }
    
//     void compact_linear_allocator::moveHeader(Header* dst, Header* src) {
        
//         /**
//          * Если размер объекта больше, чем буфер для временных объектов,
//          * то происходит его увеличение.
//          */
//         {
//             std::size_t sz = src->_reference->m_blocksize;
//             if (_tmpCapacity < sz) {
//                 if (_tmp != nullptr)
//                     _allocator->deallocate(_tmp, _tmpCapacity);
//                 _tmp            = _allocator->allocate_align(sz, alignof(std::max_align_t));
//                 _tmpCapacity    = sz;
//             }
//         }
//         Header* tmp = new(_tmp) Header(std::move(*src)); 
//         new (dst) Header(std::move(*tmp));
//     }

//     void compact_linear_allocator::compact() {
//         std::size_t readPointer     = 0;
//         std::size_t writePointer    = 0;
//         char* const data = reinterpret_cast<char*>(_data);
//         while (readPointer < _offset) {
//             Header* srcHeader = reinterpret_cast<Header*>(data + readPointer);
            
//             assert(srcHeader != nullptr);
//             assert(srcHeader->_reference != nullptr);

//             std::size_t size = srcHeader->_reference->m_blocksize;
//             if (srcHeader->_reference->m_strong_refs == 0 && srcHeader->_reference->m_weak_refs == 0 ) {
//                 readPointer += size;
//                 srcHeader->_reference->~shared_control_block();    
//                 m_ctrl_block_allocator.deallocate(srcHeader->_reference);
//                 continue;
//             }

//             Header* dstHeader = reinterpret_cast<Header*>(data + writePointer);
            
//             if (dstHeader != srcHeader)
//                 moveHeader(dstHeader, srcHeader);

//             writePointer    += size;
//             readPointer     += size;
//         }

//         _offset = writePointer;
//     }

//     tc::internal::sptr::shared_control_block* compact_linear_allocator::allocate(std::size_t sz, std::size_t count, void (*move_func)(void*, void*, std::size_t)) {
//         std::size_t total = align_up(Header::byteSize() + sz * count, alignof(std::max_align_t));
        
//         if (_offset + total > _capacity) {
//             compact();
//             if (_offset + total > _capacity) {
                
//                 std::size_t tmpCapacity = _capacity;
//                 //Попытка увеличить ёмкость, пока ёмкость не будет больше запрашиваемого блока памяти.
//                 while (tmpCapacity < (_capacity + total))
//                     tmpCapacity <<= 1; //aka tmpCapacity *= 2
                
//                 grow(tmpCapacity);
 
//                 if (_offset + total > _capacity) //Выделить не удалось, возврат нулевого shared_ptr
//                     return nullptr;
//             }
//         }
    
//         tc::internal::sptr::shared_control_block* ctrl_block = (tc::internal::sptr::shared_control_block*) m_ctrl_block_allocator.allocate();
//         if (ctrl_block == nullptr)  //если котролирующий блок не удалось выделить, возвращает нулевой shared_ptr
//             return nullptr;
        
//         char* start_new_object = reinterpret_cast<char*>(_data);
//         //Header* header = new(start_new_object + _offset) Header();
//         Header* header = (Header*) (start_new_object + _offset);
//         header->_movfunc                        = move_func;
//         header->_count                          = count;
//         header->_reference                      = ctrl_block;
//         _offset += total;
        
//         new (ctrl_block) tc::internal::sptr::shared_control_block(nullptr, header->dataPointer(), total);
//         return ctrl_block;
//     }

//     void compact_linear_allocator::print() const {
//         std::printf("================== C O M P A C T   A L L O C A T O R ==================\n");
//         std::printf("[capacity: %zu, tmpbuf %zu]\n", _capacity, _tmpCapacity);
//         for (std::size_t i = 0; i < _offset; ) {
//             Header* h = reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) + i);
//             //std::printf("[size: %llu, 0x%llx]\n", (unsigned long long) h->_reference->m_blocksize, (unsigned long long) h->_reference);
//             typedef unsigned long long ul;
//             std::printf("[size: %llu, 0x%llx, strong=%llu, weak=%llu]\n", (ul) h->_reference->m_blocksize, (ul) h->_reference, 
//                         (ul) h->_reference->m_strong_refs, (ul) h->_reference->m_weak_refs);
//             i += h->_reference->m_blocksize;
//         }
//     }
// }

//#################################################### ./src/logger.cpp ####################################################


#include <cpp/lang/logging/logger.hpp>
#include <cpp/lang/utils/date.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstring>

namespace tc
{

    namespace log
    {
        
        logger::logger() : m_out(nullptr) {

        }

        logger::logger(ostream* out) : m_out(out) {
            JSTD_DEBUG_CODE(check_non_null(out));
        }
        
        logger::logger(logger&& l) : m_out(l.m_out) {

        }
        
        logger& logger::operator= (logger&& l) {
            if (&l != this) {
                l       = l.m_out;
                l.m_out = nullptr;
            }
            return *this;
        }
        
        logger::~logger() {

        }

        void logger::message(level lvl, const char* msg) {
            JSTD_DEBUG_CODE(check_non_null(msg));

            const std::size_t RESULT_BUFFER_SIZE = 129;
            char result[RESULT_BUFFER_SIZE];
            int result_size = 0;

            tc::string strdate = date::now().to_string();

            switch (lvl) {
                case level::INFO : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[INFO] (%s) %s\n", strdate.c_str(), msg);
                    break;
                }
                case level::WARN : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[WARNING] (%s) %s\n", strdate.c_str(), msg);
                    break;
                }
                case level::ERROR : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[ERROR] (%s) %s\n", strdate.c_str(), msg);
                    break;
                }
                case level::EXCEPT : {
                    result_size = std::snprintf(result, RESULT_BUFFER_SIZE, "[EXCEPTION] (%s) %s\n", strdate.c_str(), msg);
                    break;
                }
            }

        if (m_out == nullptr)
        {
            system::tsprintf(result);
        }
        else
        {
            try {
                m_out->write(result, (std::size_t) result_size);
            } catch (const io_exception& e) {
                system::tsprintf("%s\n", e.cause());
                e.print_stack_trace();
            }
        }

        }

        void logger::warn(const char* msg) {
            message(level::WARN, msg);
        }
    
        void logger::info(const char* msg) {
            message(level::INFO, msg);
        }
        
        void logger::error(const char* msg) {
            message(level::ERROR, msg);
        }

        void logger::except(const throwable& t) {
            message(level::EXCEPT, t.cause());
        }
    }
}

//#################################################### ./src/lz4_compressor.cpp ####################################################

#include <cpp/lang/compress/lz4_compressor.hpp>
#include <cpp/lang/math.hpp>
#include <utility>
#include <cassert>

namespace tc
{
    
    lz4_compressor::lz4_compressor() : compressor(),
    rp(0),
    lit_start(0),
    state(state::FIND_MATCH),
    lit_len(0),
    lit_writed(0),
    offset(0),
    lit_rem(0),
    match_rem(0),
    match_len(0) {
        
    }

    lz4_compressor::lz4_compressor(lz4_compressor&& other) : compressor(std::move(other)),
    rp(0),
    lit_start(0),
    state(state::FIND_MATCH),
    lit_len(0),
    lit_writed(0),
    offset(0),
    lit_rem(0),
    match_rem(0),
    match_len(0) {
        std::swap(rp,           other.rp);
        std::swap(lit_start,    other.lit_start);
        std::swap(state,        other.state);
        std::swap(lit_len,      other.lit_len);
        std::swap(lit_writed,   other.lit_writed);
        std::swap(offset,       other.offset);
        std::swap(lit_rem,      other.lit_rem);
        std::swap(match_rem,    other.match_rem);
        std::swap(match_len,    other.match_len);
    }

    lz4_compressor& lz4_compressor::operator=(lz4_compressor&& other) {
        if (&other != this)
        {
            std::swap(rp,           other.rp);
            std::swap(lit_start,    other.lit_start);
            std::swap(state,        other.state);
            std::swap(lit_len,      other.lit_len);
            std::swap(lit_writed,   other.lit_writed);
            std::swap(offset,       other.offset);
            std::swap(lit_rem,      other.lit_rem);
            std::swap(match_rem,    other.match_rem);
            std::swap(match_len,    other.match_len);
        }
        return *this;
    }

    lz4_compressor::~lz4_compressor() {

    }
    
    lz4_compressor::u32 lz4_compressor::read_u32(const char* in) {
        return  ((u32) (unsigned char) in[0] << 0)   | 
                ((u32) (unsigned char) in[1] << 8)   | 
                ((u32) (unsigned char) in[2] << 16)  | 
                ((u32) (unsigned char) in[3] << 24);
    }

    /*static*/ std::size_t lz4_compressor::hash_idx(u32 val) {
        return (val * 0x85EBCA77u) & (TABLE_SIZE - 1);
    }

    void lz4_compressor::put_hash(u32 val, std::size_t pos) {
        hash_table[hash_idx(val)] = (u16) (pos & 0xffff);
    }

    std::size_t lz4_compressor::from_hash(u32 val) {
        std::size_t idx = hash_idx(val);
        return hash_table[idx] != 0xffffu ? hash_table[idx] : no_val;
    }

    void lz4_compressor::set_input(const char* in, std::size_t length) {
        compressor::set_input(in, length);
        state       = state::FIND_MATCH;
        for (std::size_t i = 0; i < TABLE_SIZE; ++i)
            hash_table[i] = 0xffffu;
    }

    void lz4_compressor::set_state(enum lz4_compressor::state state) {
        this->state = state;
    }

    static std::size_t extend_match(const char* x, std::size_t current, std::size_t prev, std::size_t len) 
    {
        std::size_t matched = 0;
        std::size_t apos = current;
        std::size_t bpos = prev;
        while ((apos < len))
        {
            if (x[apos] != x[bpos])
                break;
            ++apos;
            ++bpos;
            ++matched;
        }        
        return matched;
    }

    void lz4_compressor::find_match() {
        assert(state == state::FIND_MATCH);
        const char* in = input;

        lit_start = rp;

        const std::size_t NO_COMPRESS_SIZE  = 5;
        const std::size_t TAIL              = NO_COMPRESS_SIZE + MATCH_LENGTH;

        while ((input_size > TAIL) && (rp < (input_size - TAIL)))
        {
            assert(rp < input_size);
            u32 seq             = read_u32(in + rp);
            std::size_t prev    = from_hash(seq);
            put_hash(seq, rp);

            //Эта последовательность встретилась впервые.
            if (prev == lz4_compressor::no_val)
            {
                ++rp;
                continue;
            }
            
            assert(rp >= prev);
            u16 dst = (u16)((u16)rp - (u16)prev);

            if (dst == 0)
            {
                ++rp;
                continue;   
            }
            
            //Проверяем, чтобы текущая последовательность совпадала с той, которая хранится в хеш-карте.
            if (read_u32(in + rp - dst) != seq)
            {
                ++rp;
                continue;
            }
            
            std::size_t prev_match_pos  = rp - dst;
            std::size_t match_len       = MATCH_LENGTH + extend_match(in, rp + MATCH_LENGTH, prev_match_pos + MATCH_LENGTH, input_size - NO_COMPRESS_SIZE);

            this->lit_len   = rp - lit_start;
            this->match_len = match_len;
            this->offset    = dst;

            rp += match_len;
            
            //Установить следующее состояние, как запись токена.
            set_state(state::WRITE_TOKEN);

            return;
        }

        //Дописать оставшиеся, не сжатые, данные
        if (rp < input_size)
        {
            this->lit_len   = input_size - lit_start;
            this->match_len = 0;
            this->offset    = 0;
            rp              = input_size;
            set_state(state::WRITE_TOKEN);
        }

    }
    
    std::size_t lz4_compressor::write_tok(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
    
        char lit_tok    = 0;
        char match_tok  = 0;

        /*
            Если размер литерала или матча будет 15, то всё равно нужно будет прочитать нулевой байт, даже если rem будет равен нулю.
            При этом, если если длина литерала меньше 15, то rem даже не читается при записи

        */
        //длина литералов
        if (lit_len >= 15)
        {
            lit_tok = 15;
            lit_rem = lit_len - 15;
        }
        else
        {
            lit_tok |= (char) lit_len;
            lit_rem  = 0;
        }

        assert(match_len == 0 || match_len >= MATCH_LENGTH);
        std::size_t token_match = match_len == 0 ? 0 : match_len - MATCH_LENGTH;

        //длина мэтча
        if (token_match >= 15)
        {
            match_tok = 15;
            match_rem = token_match - 15;
        }
        else
        {
            match_tok |= (char) token_match;
            match_rem = 0;
        }
        
        //Если размер равен или больше 15, то нужно записать дополнительные байты, для кодирования длины.
        if (lit_len >= 15) set_state(state::WRITE_LIT_LEN_EXTRA);
        
        // Иначе просто записываем литералы.
        else                set_state(state::WRITE_LITERALS);

        *buf = (char) ((lit_tok << 4) | (match_tok << 0));

        return 1;
    }
    
    std::size_t lz4_compressor::write_lit_len_extra(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        std::size_t writed = 0;

        while ((writed < sz) && lit_rem >= 255)
        {
            buf[writed++] = (char) 255;
            lit_rem      -= 255;            
        }

        // Нужно записать остаток. Даже если 0, его всё равно нужно записать.
        if (writed < sz)
        {
            buf[writed++]   = (char) lit_rem;
            set_state(state::WRITE_LITERALS);
            lit_rem         = 0;
        }

        return writed;
    }
    
    std::size_t lz4_compressor::write_lit(char buf[], std::size_t sz) {
        if (sz == 0) return 0;

        assert(lit_len >= lit_writed);
        
        std::size_t write_rem   = lit_len - lit_writed;
        std::size_t writed      = math::min(sz, write_rem);
        std::memcpy(buf, input + lit_start + lit_writed, writed);
        lit_writed += writed;

        // Если записали все данные
        if (lit_writed == lit_len)
        {
            //Обнуляем для последующих записей.
            lit_writed = 0;
            
            // Если длина матча не нулевая, значит следом нужно записать смещение в little-endian порядке.
            if (match_len > 0)
            {
                //Начинаем с младшего слова.
                set_state(state::WRITE_OFFSET_L);
            }
            else
            {
                set_state(state::FIND_MATCH);
            }
        }

        return writed;
    }
    
    std::size_t lz4_compressor::write_offset(char buf[], std::size_t sz, offset_part part) {
        if (sz < 1) return 0;
        // std::printf("offset %zu\n", offset);
        switch (part) {
        case offset_part::LOW_WORD:
            *buf = (char) ((offset >> 0) & 0xFF);
            set_state(state::WRITE_OFFSET_H);
            break;
        case offset_part::HIGH_WORD:
            *buf = (char) ((offset >> 8) & 0xFF);
            //Если длина матча равна 15, то нужно записать дополнительные байты, иначе просто искать новый матч.
            assert(match_len >= MATCH_LENGTH);
            std::size_t token_match = match_len - MATCH_LENGTH;
            if (token_match >= 15)
                set_state(state::WRITE_MATCH_LEN_EXTRA);
            else
                set_state(state::FIND_MATCH);
            break;
        }
        return 1;
    }
    
    std::size_t lz4_compressor::write_match_len_extra(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        std::size_t writed = 0;

        while ((writed < sz) && match_rem >= 255)
        {
            buf[writed++] = (char) 255;
            match_rem    -= 255;
        }

        // Нужно записать остаток. Даже если 0, его всё равно нужно записать.
        if (writed < sz)
        {
            buf[writed++]   = (char) match_rem;
            set_state(state::FIND_MATCH);
            match_rem         = 0;
        }

        return writed;
    }

    bool lz4_compressor::finished() const {
        return rp >= input_size;
    }

    std::size_t lz4_compressor::compress(char out[], std::size_t outlen) {

        std::size_t writed  = 0;
        std::size_t rem     = outlen;

        while ((rem > 0) && (state != state::FIHISHED))
        {            
            std::size_t current_writed = 0;
            switch (state) {
                case state::FIND_MATCH:
                    if (finished())
                    {
                        set_state(state::FIHISHED);
                        finish();
                    }
                    else
                    {
                        find_match();
                    }
                    break;
                case state::WRITE_TOKEN:
                    current_writed = write_tok(out, rem);
                    break;
                case state::WRITE_LIT_LEN_EXTRA:
                    current_writed = write_lit_len_extra(out, rem);
                    break;
                case state::WRITE_LITERALS:
                    current_writed = write_lit(out, rem);
                    break;
                case state::WRITE_OFFSET_L:
                    current_writed = write_offset(out, rem, offset_part::LOW_WORD);
                    break;
                case state::WRITE_OFFSET_H:
                    current_writed = write_offset(out, rem, offset_part::HIGH_WORD);
                    break;
                case state::WRITE_MATCH_LEN_EXTRA:
                    current_writed = write_match_len_extra(out, rem);
                    break;
                default:
                    set_state(state::FIHISHED);
                    break;
            }
            //Увеличиваем коливество всех записей
            writed  += current_writed;
            //Двигаем указатель записи
            out     += current_writed;
            //Уменьшаем количество свободного места в выходном буфере
            assert(rem >= current_writed);
            rem     -= current_writed;
        }

        return writed;
    }
}

//#################################################### ./src/lz4_decompressor.cpp ####################################################

#include <cpp/lang/compress/lz4_decompressor.hpp>
#include <cpp/lang/math.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <utility>

namespace tc
{
    lz4_decompressor::lz4_decompressor() : decompressor(),
    window_pos(0),
    rp(0),
    state(state::READ_TOKEN),
    lit_len(0),
    match_len(0),
    off(0),
    lit_rem(0),
    match_off(0) {

    }

    lz4_decompressor::lz4_decompressor(lz4_decompressor&& other) : decompressor(std::move(other)),
    window_pos(0),
    rp(0),
    state(state::READ_TOKEN),
    lit_len(0),
    match_len(0),
    off(0),
    lit_rem(0),
    match_off(0) {
        std::swap(window_pos, other.window_pos);
        std::swap(rp, other.rp);
        std::swap(state, other.state);
        std::swap(lit_len, other.lit_len);
        std::swap(match_len, other.match_len);
        std::swap(off, other.off);
        std::swap(lit_rem, other.lit_rem);
        std::swap(match_off, other.match_off);
    }

    lz4_decompressor& lz4_decompressor::operator=(lz4_decompressor&& other) {
        if (&other != this)
        {
            decompressor::operator=(std::move(other));
            std::swap(window_pos, other.window_pos);
            std::swap(rp, other.rp);
            std::swap(state, other.state);
            std::swap(lit_len, other.lit_len);
            std::swap(match_len, other.match_len);
            std::swap(off, other.off);
            std::swap(lit_rem, other.lit_rem);
            std::swap(match_off, other.match_off);
        }
        return *this;
    }

    lz4_decompressor::~lz4_decompressor() {

    }
    
    void lz4_decompressor::set_state(enum lz4_decompressor::state state) {
        this->state = state;
    }

    void lz4_decompressor::set_input(const char* in, std::size_t len) {
        input       = in;
        input_size  = len;
        window_pos  = 0;
        set_state(state::READ_TOKEN);
    }
 
    void lz4_decompressor::read_tok() {
        unsigned char token     = read_char();
        unsigned char lit       = (token >> 4) & 0xf;
        unsigned char match     = (token >> 0) & 0xf;
        
        lit_len     = lit;
        match_len   = match + MATCH_LENGTH;
        lit_rem     = lit_len;
        match_off   = 0;

        // Если длина равна 15, значит нужно прочитать дополнительные байты длины
        if (lit == 15)  set_state(state::READ_LIT_EXTRA);
        else            set_state(state::READ_LITERALS);
    }

    void lz4_decompressor::read_lit_extra() {
        unsigned char readed;
        do {
            readed   = read_char();
            lit_len += readed;
            lit_rem += readed;
        } while (readed == 0xff);
        set_state(state::READ_LITERALS);
    }

    void lz4_decompressor::read_match_extra() {
        unsigned char readed;
        do {
            readed     = read_char();
            match_len += readed;
        } while (readed == 0xff);
        set_state(state::DECOMPRESS);
    }

    std::size_t lz4_decompressor::read_literals(char buf[], std::size_t sz) {
        if (sz == 0) return 0; 
        
        if (lit_rem > 0)
        {
            std::size_t readed = math::min(lit_rem, sz);

            for (std::size_t i = 0; i < readed; ++i)
            {
                char b = input[rp];
                
                //Сохранить литералы в окно.
                window[window_pos & WINDOW_MASK]  = b;
                
                //Сохранить литералы непосредственно в выходной буфер
                buf[i]                              = b;
                
                assert(lit_rem > 0);
                --lit_rem;
                ++rp;
                ++window_pos;
            }

            return readed;
        }
        else
        {
            if (rp < input_size)
            {
                off = 0;
                set_state(state::READ_OFFSET_L);
            }
            else
            {
                set_state(state::FINISHED);
            }
        }

        return 0;
    }
    
    void lz4_decompressor::read_offset(offset_part part) {
        switch (part) {
            case offset_part::LOW_WORD: 
                off |=  (u16) (((u16) read_char()) << 0);
                set_state(state::READ_OFFSET_H);
                break;
            case offset_part::HIGH_WORD:
                off |=  (u16) (((u16) read_char()) << 8);
                if (match_len >= 15 + MATCH_LENGTH)
                    set_state(state::READ_MATCH_EXTRA);
                else
                    set_state(state::DECOMPRESS);
                break;
        }
    }

    std::size_t lz4_decompressor::decompress_literals(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        assert(off != 0);

        if (match_off < match_len)
        {
            std::size_t readed = math::min(sz, match_len - match_off);
            for (std::size_t i = 0; i < readed; ++i)
            {
                char b = window[(window_pos - off) & WINDOW_MASK];
                buf[i] = b;
                window[window_pos & WINDOW_MASK] = b;
                ++window_pos;
                ++match_off;
            }
            return readed;
        }
        else
        {
            set_state(state::READ_TOKEN);
        }

        return 0;
    }
    
    std::size_t lz4_decompressor::decompress(char buf[], std::size_t sz) {
        std::size_t rem     = sz;
        std::size_t readed  = 0;
        while (rem > 0 && state != state::FINISHED)
        {
            std::size_t current_readed = 0;
            switch (state) {
                case state::READ_TOKEN:
                    read_tok();
                    break;
                case state::READ_LIT_EXTRA:
                    read_lit_extra();
                    break;
                case state::READ_LITERALS:
                    current_readed = read_literals(buf + readed, rem);
                    break;
                case state::READ_OFFSET_L:
                    read_offset(offset_part::LOW_WORD);
                    break;
                case state::READ_OFFSET_H:
                    read_offset(offset_part::HIGH_WORD);
                    break;
                case state::READ_MATCH_EXTRA:
                    read_match_extra();
                    break;
                case state::DECOMPRESS:
                    current_readed = decompress_literals(buf + readed, rem);
                    break;
                case state::FINISHED:
                    break;
            }
            assert(rem >= current_readed);
            rem     -= current_readed;
            readed  += current_readed;

            // std::printf("rem %zu\n", rem);
        }

        return readed;
    }
}

#if 0

#include <lz4.h>
#include <cpp/lang/ustring.hpp>

int main() {
    // const char* str = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
    // const char* str = "abracadabraabracadabraabracadabraabracadabraabracadabra";
    const std::size_t BUF_SZ = 1 << 17;
    char compressed[BUF_SZ];
    std::size_t compressed_length;


    tc::string s;
    for (int i = 0; i < 100000; ++i)
        s.append("The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.");

    {//compress
        compressed_length = LZ4_compress_default(s.c_string(), compressed, (int) s.length(), (int) BUF_SZ);
        std::printf("compressed: %zu\n", compressed_length);
        for (std::size_t i = 0; i < compressed_length; ++i)
        {
            std::printf("%x ", compressed[i] & 0xff);
        }
        std::printf("\n");
    }
    
    {//decompress
        tc::lz4_decompressor decompressor;
        decompressor.set_input(compressed, compressed_length);
        
        char decompressed[BUF_SZ];
        std::size_t decompressed_length = decompressor.decompress(decompressed, BUF_SZ);
        std::printf("decompressed: %zu\n", decompressed_length);

        for (std::size_t i = 0; i < decompressed_length; ++i)
        {
            std::printf("%c", decompressed[i]);
        }
    }
    
}

#endif

//#################################################### ./src/malloc_free_allocator.cpp ####################################################

#include <allocators/malloc_free_allocator.hpp>
#include <cstdlib>
#include <cstddef>
#include <malloc.h>

namespace tca {
    
    malloc_free_allocator::malloc_free_allocator() : allocator(nullptr) {

    }

    void* malloc_free_allocator::allocate(std::size_t sz) {
        return malloc(sz);
    }
    
    void* malloc_free_allocator::reallocate(void* p, std::size_t new_size) {
        return realloc(p, new_size);
    }

    void* malloc_free_allocator::allocate_align(std::size_t sz, std::size_t) {
        return malloc(sz);
    }
    
    void malloc_free_allocator::deallocate(void* p) {
        free(p);
    }
}

//#################################################### ./src/math.cpp ####################################################

#include <cpp/lang/math/math.hpp>
#include <cpp/lang/utils/random.hpp>

namespace tc
{
namespace math
{
    double random() {
        thread_local static class random rnd;
        return rnd.next<double>();
    }

}// namespace jstd
}// namespace math

//#################################################### ./src/memory.cpp ####################################################

#include <internal/memory.hpp>
#include <allocators/base_allocator.hpp>
#include <cpp/lang/math.hpp>
#include <allocators/helpers.hpp>
#include <cassert>
#include <cstdint>

namespace tca
{
namespace internal
{

    page_header* page_new(page_header* next, std::size_t page_size, std::size_t align, tca::base_allocator* alloc) {
        /*
            Увеличение размера заголовка, чтобы блок, начинающийся после него, 
            имел выравнивание не меньше, чем @param align
         */
        std::size_t header_size_with_padding = align_up(sizeof(internal::page_header), tc::math::max(alignof(internal::page_header), align));

        /*
            Общий размер страницы вместе с заголовком. 
        */
        std::size_t total_page_size = header_size_with_padding + page_size;

        void* block = alloc->allocate_align(total_page_size, tc::math::max(align, alignof(internal::page_header)));
        if (!block)
            return nullptr;
        
        assert((((std::uintptr_t) block) % align) == 0);
        
        internal::page_header* hder = reinterpret_cast<internal::page_header*>(block);
        
        assert((((std::uintptr_t) &hder->dat) % alignof(void*)) == 0);
        hder->dat    = static_cast<unsigned char*>(block) + header_size_with_padding;
        
        assert((((std::uintptr_t) &hder->next) % alignof(void*)) == 0);
        hder->next = next;
        
        assert((((std::uintptr_t) &hder->size) % alignof(std::size_t)) == 0);
        hder->size   = total_page_size;

        assert((((std::uintptr_t) &hder->user_size) % alignof(std::size_t)) == 0);
        hder->user_size = page_size;

        return hder;
    }

    void page_delete(internal::page_header* page, tca::base_allocator* alloc) {
        alloc->deallocate(page, page->size);
    }


} //namespace internal
} //namespace tca

//#################################################### ./src/mutex.cpp ####################################################

#include <cpp/lang/concurrency/mutex.hpp>
#include <cpp/lang/exceptions.hpp>
#include <utility>

namespace tc
{

    mutex::mutex() {

    }

    mutex::~mutex() {
        
    }
    
    void mutex::lock() {
        try {
            m_mutex_impl.lock();
        } catch (...) {
            throw_except<interrupted_exception>();
        }
    }
    
    void mutex::unlock() {
        try {
            m_mutex_impl.unlock();
        } catch (...) {
            throw make_except<interrupted_exception>("mutex interrupted");
        }
    }
    
    bool mutex::try_lock() {
        try {
            return m_mutex_impl.try_lock();
        } catch (...) {
            throw make_except<interrupted_exception>("mutex interrupted");
        }
    }

}// namespace jstd

//#################################################### ./src/null_allocator.cpp ####################################################

#include <allocators/null_allocator.hpp>
#include <utility>
#include <cassert>

namespace tca
{
    null_allocator::null_allocator() : base_allocator() {

    }

    null_allocator::null_allocator(null_allocator&& alloc) : base_allocator(std::move(alloc)) {

    }
    
    null_allocator& null_allocator::operator= (null_allocator&& alloc) {
        if (&alloc != this)
            base_allocator::operator=(std::move(alloc));
        return *this;
    }
    
    null_allocator::~null_allocator() {

    }
    
    void* null_allocator::allocate(std::size_t){
        return nullptr;
    }
    
    void* null_allocator::allocate_align(std::size_t, std::size_t) {
        return nullptr;
    }
    
    void null_allocator::deallocate(void* p, std::size_t) {
        assert(p == nullptr);
    }
}

//#################################################### ./src/numbers.cpp ####################################################

#include <cpp/lang/numbers.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cstring>

namespace tc 
{

namespace num 
{
    uint_float_bits float_to_uint_bits(float v) {
        static_assert(sizeof(v) == sizeof(uint_float_bits), "sizeof(v) == sizeof(uint_float_bits)");
        uint_float_bits value;
        std::memcpy(&value, &v, sizeof(float));
        return value;
    }
    
    uint_double_bits double_to_uint_bits(double v) {
        static_assert(sizeof(v) == sizeof(uint_double_bits), "sizeof(v) == sizeof(uint_double_bits)");
        uint_double_bits value;
        std::memcpy(&value, &v, sizeof(double));
        return value;
    }
}//namespace num

}//namespace jstd



//#################################################### ./src/obstream.cpp ####################################################

#include <cpp/lang/io/obstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/errcode.hpp>
#include <iostream>
#include <cassert>

namespace tc
{

    obstream::obstream() : m_allocator(nullptr), m_buffer(nullptr), m_capacity(0), m_offset(0), m_out(nullptr) {

    }

    obstream::obstream(ostream* stream, tca::allocator* allocator, std::size_t buf_size) : obstream() {
        JSTD_DEBUG_CODE
        (
            if (!stream)      throw_except<null_pointer_exception>("stream is null");
            if (!allocator)   throw_except<null_pointer_exception>("allocator is null");
        )
        char* data  = (char*) allocator->allocate_align(buf_size, alignof(char));
        if (!data)
            throw_except<out_of_memory_error>("out of memory");
        m_allocator  = allocator;
        m_buffer     = data;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_out        = stream;
    }
    
    obstream::obstream(ostream* stream, char* buffer, std::size_t buf_size) : obstream() {
        JSTD_DEBUG_CODE
        (
            if (stream == nullptr)  throw_except<null_pointer_exception>("stream is null");
            if (buffer == nullptr)  throw_except<null_pointer_exception>("buffer is null");
        )
        m_buffer     = buffer;
        m_capacity   = buf_size;
        m_offset     = 0;
        m_out        = stream;
    }

    obstream::obstream(obstream&& stream) : 
    m_allocator(stream.m_allocator), m_buffer(stream.m_buffer), m_capacity(stream.m_capacity), m_offset(stream.m_offset), m_out(stream.m_out) {
        stream.m_allocator   = nullptr;
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
        stream.m_out         = nullptr;
    }

    obstream& obstream::operator= (obstream&& stream) {
        if (&stream != this)
        {
            if (!m_out)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_allocator  = stream.m_allocator;
            m_buffer     = stream.m_buffer;
            m_capacity   = stream.m_capacity;
            m_offset     = stream.m_offset;
            m_out        = stream.m_out;

            stream.m_allocator   = nullptr;
            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
            stream.m_out         = nullptr;
        }
        return *this;
    }

    void obstream::free() {
        if (!m_allocator)
        {
            m_allocator->deallocate(m_buffer);
            m_allocator = nullptr;
        }
    }

    obstream::~obstream() {
        error_code dontcare;
        close(dontcare);
    }

    void obstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_out)
                throw_except<io_exception>("stream is null");
        )
        
        assert(m_capacity >= m_offset);
        std::size_t rem = m_capacity - m_offset;
        
        if (rem < sz)
        {
            flush();
        }
        
        if (sz > m_capacity)
        {
            m_out->write(data, sz);
        } 
        else
        {
            memcpy(m_buffer + m_offset, data, sz);
            m_offset += sz;
        }
    }
    
    void obstream::flush() {
        if (m_out)
        {
            if (m_offset > 0)
            {
                m_out->write(m_buffer, m_offset);
                m_offset = 0;
            }
        }
    }
    
    void obstream::close(error_code& err) {
        if (m_out)
        {
            
            error_code flush_err;
            error_code close_err;

            try {
                flush();
            } catch (...) {
                flush_err = error_code(errcode::io_error, generic_category());
            }
        
            m_out->close(close_err);
            
            free();
            m_out = nullptr;
            
            err = flush_err ? flush_err : close_err;
        }
    }
    
}

//#################################################### ./src/odstream.cpp ####################################################

#include <cpp/lang/io/odstream.hpp>
#include <cpp/lang/exceptions.hpp>

namespace tc
{
    odstream::odstream() : m_out(nullptr) {

    }

    odstream::odstream(ostream* out) : m_out(out) {
        JSTD_DEBUG_CODE
        (
            if (!out)
                throw_except<io_exception>("stream is null");
        )
    }

    odstream::odstream(odstream&& stream) : m_out(stream.m_out) {
        stream.m_out = nullptr;
    }
    
    odstream& odstream::operator= (odstream&& out) {
        if (&out != this)
        {
            if (m_out)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_out     = out.m_out;
            out.m_out = nullptr;
        }
        return *this;
    }
    
    odstream::~odstream() {
        error_code dontcare;
        close(dontcare);
    }

    void odstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_out)
                throw_except<io_exception>("stream is null");
        )
        m_out->write(data, sz);
    }
    
    void odstream::flush() {
        if (m_out)
        {
            m_out->flush();
        }
    }
    
    void odstream::close(error_code& err) {
        if (m_out)
        {
            m_out->close(err);
            m_out = nullptr;
        }
    }
}

//#################################################### ./src/ofstream.cpp ####################################################

#include <cpp/lang/io/ofstream.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <errno.h>
#include <cstring>
#include <iostream>
#include <internal/io/io_helpers.hpp>

namespace tc {

    ofstream::ofstream() : m_handle(nullptr) {

    }

    
    ofstream::ofstream(const string& path, bool append) : ofstream(path.c_str(), append) {
    
    }

    ofstream::ofstream(const char* path, bool append) : ofstream(file(path), append) {
        
    }
    
    ofstream::ofstream(const file& f, bool append) : ofstream() {    
        expected<FILE*, error_code> fhandle = filesystem::open(f.c_str(), append ? "ab" : "wb");
        if (!fhandle)
        {
            internal::io::throw_error_code(fhandle.error());
        }
        m_handle = fhandle.value();
    }

    ofstream::ofstream(ofstream&& stream) : m_handle(stream.m_handle) {
        stream.m_handle = nullptr;
    }
    
    ofstream& ofstream::operator= (ofstream&& out) {
        if (&out != this)
        {
            if (m_handle)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_handle     = out.m_handle;
            out.m_handle = nullptr;
        }
        return *this;
    }
    
    ofstream::~ofstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    void ofstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("stream is null");
        )
        std::size_t writed = fwrite(data, 1, sz, m_handle);
        if (writed != sz)
        {
            throw_except<io_exception>(strerror(errno));
        }
    }
    
    void ofstream::flush() {
        if (m_handle)
        {
            error_code err = filesystem::flush(m_handle);
            if (err)
            {
                internal::io::throw_error_code(err);
            }
        }
    }
    
    void ofstream::close(error_code& err) {
        if (m_handle)
		{
            err      = filesystem::close(m_handle);
            m_handle = nullptr;
        }
    }    
}

//#################################################### ./src/omstream.cpp ####################################################

#include <cpp/lang/io/omstream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <iostream>
#include <cassert>

namespace tc
{
    
    omstream::omstream(std::size_t init_buf_size, tca::allocator* allocator) : 
    m_allocator(allocator),
    m_buffer(nullptr),
    m_capacity(init_buf_size),
    m_offset(0) {

    }

    omstream::omstream(char* buf, std::size_t capacity) :
    m_allocator(nullptr),
    m_buffer(buf),
    m_capacity(capacity),
    m_offset(0) {

    }

    omstream::omstream(omstream&& stream) : 
    m_allocator(stream.m_allocator), 
    m_buffer(stream.m_buffer), 
    m_capacity(stream.m_capacity), 
    m_offset(stream.m_offset) {
        stream.m_allocator   = nullptr;
        stream.m_buffer      = nullptr;
        stream.m_capacity    = 0;
        stream.m_offset      = 0;
    }

    omstream& omstream::operator= (omstream&& stream) {
        if (&stream != this)
        {
            if (m_buffer)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_allocator   = stream.m_allocator;
            m_buffer      = stream.m_buffer;
            m_capacity    = stream.m_capacity;
            m_offset      = stream.m_offset;
            
            stream.m_allocator   = nullptr;
            stream.m_buffer      = nullptr;
            stream.m_capacity    = 0;
            stream.m_offset      = 0;
        }
        return *this;
    }

    void omstream::resize(std::size_t sz) {
        std::size_t s = m_capacity == 0 ? INIT_BUF_SIZE : m_capacity;
        
        if (s <= 1) s = 2;
        while (sz + m_offset > s)
            s = s + (s >> 1);

        char* new_buffer = (char*) m_allocator->allocate_align(s, alignof(char));
        if (!new_buffer)
            throw_except<out_of_memory_error>("out of memory");
        
        std::memcpy(new_buffer, m_buffer, m_offset);
        
        m_allocator->deallocate(m_buffer);
        m_buffer     = new_buffer;
        m_capacity   = s;
    }

    void omstream::write(const char* data, std::size_t sz) {
        JSTD_DEBUG_CODE
        (
            if (!m_allocator && !m_buffer)
                throw_except<io_exception>("Stream is null");
        )

        if ((m_capacity - m_offset < sz) || (!m_buffer))
        {
            if (!m_allocator)
                throw_except<overflow_exception>("omstream buffer is owerflow!");
            resize(sz);
        }
        
        std::memcpy(m_buffer + m_offset, data, sz);
        m_offset += sz;
    }

    void omstream::flush() {
        JSTD_DEBUG_CODE
        (
            if (!m_allocator && !m_buffer)
                throw_except<io_exception>("Stream is null");
        );
    }

    void omstream::close(error_code& err) {
        if (!m_buffer)
            return;
        if (m_allocator)
        {
            m_allocator->deallocate(m_buffer);
            m_allocator = nullptr;
        }
        m_buffer = nullptr;
    }

    omstream::~omstream() {
        error_code dontcare;
        close(dontcare);
    }

    const char* omstream::data() const {
        return m_buffer;
    }
    
    std::size_t omstream::offset() const {
        return m_offset;
    }
}

//#################################################### ./src/os_allocator.cpp ####################################################

#include <allocators/os_allocator.hpp>
#include <utility>

#ifdef __linux__
    #define LINUX_OS __linux__
#elif __APPLE__
    #define MAC_OS __APPLE__
#elif _WIN32
    #define WINDOWS_OS _WIN32
#else
#error Platform is not defined
#endif

#if defined(LINUX_OS) || defined(MAC_OS)
    #include <sys/mman.h>
    #include <cstring>
    #include <errno.h>
#elif WINDOWS_OS
    #include <memoryapi.h>    
    #include <windows.h>
#endif

#ifndef NDEBUG
    #include <cstdint>
    #include <cassert>
#endif

namespace tca {

    os_allocator::os_allocator(int protect) : base_allocator(), protect(protect), lastError(NO_ERRORS) {

    }

    os_allocator::os_allocator(os_allocator&& alloc) : base_allocator(std::move(alloc)), protect(alloc.protect), lastError(alloc.lastError) {

    }
    
    os_allocator& os_allocator::operator= (os_allocator&& alloc) {
        if (&alloc != this) {
            base_allocator::operator=(std::move(alloc));
            protect     = alloc.protect;
            lastError   = alloc.lastError;
        }
        return *this;
    }
    
    os_allocator::~os_allocator() {

    }
    
    void* os_allocator::allocate(std::size_t sz) {
        lastError = NO_ERRORS;
        return allocate_align(sz, alignof(char));
    }
    
    int os_allocator::getLastError() const {
        return lastError;
    }

#if defined(LINUX_OS) || defined(MAC_OS)
    
    void* os_allocator::allocate_align(std::size_t sz, std::size_t align) {
        lastError = NO_ERRORS;
        int prot = 0;
        prot |= protect & READ   ? PROT_READ  : 0;
        prot |= protect & WRITE  ? PROT_WRITE : 0;
        prot |= protect & EXEC   ? PROT_EXEC  : 0;
        
        void* block = mmap(nullptr, sz, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        
        if (block == MAP_FAILED) {
            lastError = errno;
            return nullptr;
        }
        
        return block;
    }
    
    void os_allocator::deallocate(void* ptr, std::size_t sz) {
        lastError = NO_ERRORS;
        if (ptr == nullptr)
            return;
        if (munmap(ptr, sz) != 0)
            lastError = errno;
    }

    const char* os_allocator::getErrorString(int error) {
        return std::strerror(error);        
    }

#elif WINDOWS_OS

    /**
     * @internal
     * Переопределяет маску защиты памяти платформо-независимой библиотеки в Windows маску.
     * 
     * @param prot
     *          Маска защиты библиотеки.
     * 
     * @return
     *          Маска защиты для операционной системы Windows.
     */
    DWORD libProtectToWindowsProtect(int prot) {
        DWORD native_protect = PAGE_NOACCESS;
        
        const int MEM_PROTECT_READ  = os_allocator::READ;
        const int MEM_PROTECT_WRITE = os_allocator::WRITE;
        const int MEM_PROTECT_EXEC  = os_allocator::EXEC;

        if ((prot & MEM_PROTECT_EXEC) && (prot & MEM_PROTECT_READ) && (prot & MEM_PROTECT_WRITE))
            native_protect = PAGE_EXECUTE_READWRITE;
        else if ((prot & MEM_PROTECT_EXEC) && (prot & MEM_PROTECT_READ))
            native_protect = PAGE_EXECUTE_READ;
        else if (((prot & MEM_PROTECT_READ) && (prot & MEM_PROTECT_WRITE)) || (prot & MEM_PROTECT_WRITE))
            native_protect = PAGE_READWRITE;
        else if (prot & MEM_PROTECT_EXEC)
            native_protect = PAGE_EXECUTE;
        else if (prot & MEM_PROTECT_READ)
            native_protect = PAGE_READONLY;
        else 
            return PAGE_NOACCESS;
        
        return native_protect;
    }

    void* os_allocator::allocate_align(std::size_t sz, std::size_t) {
        lastError = NO_ERRORS;
        DWORD prot = libProtectToWindowsProtect(protect);
        void* block = VirtualAlloc(nullptr, sz, MEM_COMMIT | MEM_RESERVE, prot);
        if (block == nullptr)
            lastError = GetLastError();
        return block;
    }

    void os_allocator::deallocate(void* ptr, std::size_t) {
        lastError = NO_ERRORS;
        if (ptr == nullptr)
            return;
        BOOL result = VirtualFree(ptr, 0, MEM_RELEASE);
        if (!result) 
            lastError = GetLastError();
    }

    const char* os_allocator::getErrorString(int error) {
        switch(error) {
            case ERROR_SUCCESS:
                return "The operation completed successfully.";
            case ERROR_INVALID_PARAMETER:
                return "The parameter is incorrect.";
            case ERROR_INVALID_HANDLE:
                return "The handle is invalid.";
            case ERROR_NOT_ENOUGH_MEMORY: 
                return "Not enough memory resources are available to process this command.";
            case ERROR_OUTOFMEMORY:
                return "Not enough storage is available to complete this operation.";            
            case ERROR_INVALID_ADDRESS:
                return "Attempt to access invalid address.";
            default:
                return "Undefined error";
        }
    }

#endif
}

//#################################################### ./src/pool_allocator.cpp ####################################################

#include <allocators/pool_allocator.hpp>
#include <allocators/helpers.hpp>
#include <cpp/lang/math.hpp>
#include <cassert>
#include <cstdint>
#include <utility>

namespace tca
{
    using pa = pool_allocator;

    pa:: pool_allocator() : allocator(),
    pages(nullptr),
    free_list(nullptr),
    align(0),
    bucket_size(0),
    cnt_buckets_per_page(0) {

    }

    pa::pool_allocator(std::size_t bucket_size, std::size_t count_buckets, std::size_t align, allocator* alloc) :  allocator(alloc),
    pages(nullptr),
    free_list(nullptr),
    align( tc::math::max(align, alignof(internal::bucket)) ),
    bucket_size( align_up(bucket_size, this->align) ),
    cnt_buckets_per_page(count_buckets) {

    }

    pa:: pool_allocator(pool_allocator&& alloc) : allocator(std::move(alloc)) {
        std::swap(pages,        alloc.pages);
        std::swap(free_list,    alloc.free_list);
        std::swap(align,        alloc.align);
        std::swap(bucket_size,  alloc.bucket_size);
        std::swap(cnt_buckets_per_page, alloc.cnt_buckets_per_page);
    }

    pool_allocator& pa:: operator=(pool_allocator&& alloc) {
        if (&alloc != this)
        {
            allocator::operator=(std::move(alloc));
            std::swap(pages,        alloc.pages);
            std::swap(free_list,    alloc.free_list);
            std::swap(align,        alloc.align);
            std::swap(bucket_size,  alloc.bucket_size);
            std::swap(cnt_buckets_per_page, alloc.cnt_buckets_per_page);
        }
        return *this;
    }

    void pa:: allocate_page() {
        if (m_parent == nullptr)
            return;

        internal::page_header* page = internal::page_new(pages, cnt_buckets_per_page * bucket_size, align, m_parent);
        if (!page)
            return;

        unsigned char* buckets = static_cast<unsigned char*>(page->dat);
        for (std::size_t i = 0; i < cnt_buckets_per_page; ++i)
        {
            // What the fuck
            internal::bucket* bucket = static_cast<internal::bucket*>(
                static_cast<void*>(buckets + i * bucket_size)
            );
            
            assert((((std::uintptr_t) bucket) % align) == 0);
            
            bucket->next = free_list;
            free_list    = bucket;
        }

    }

    void* pa:: allocate() {
        
        for (std::size_t i = 0; i < 2; ++i)
        {
            if (free_list != nullptr)
            {
                
                internal::bucket* node = free_list;
                free_list = free_list->next;
                
                assert((((std::uintptr_t) node) % align) == 0);
                
                return static_cast<void*>(node);
            }
    
            if (i == 0)
            {
                allocate_page();
            }
        }

        return nullptr;
    }
    
    void* pa:: allocate(std::size_t sz) {
        if (sz > bucket_size || alignof(std::max_align_t) > align)
            return nullptr;
        return allocate();
    }

    void* pa:: allocate_align(std::size_t sz, std::size_t align_) {
        if (sz > bucket_size || align_ > align)
            return nullptr;
        return allocate();
    }

    void pa ::deallocate(void* p) {
        if (p == nullptr)
            return;
        internal::bucket* node = static_cast<internal::bucket*>(p);
        node->next = free_list;
        free_list  = node;
    }

    pa:: ~pool_allocator() {
        for (internal::page_header* page = pages; page != nullptr; )
        {
            internal::page_header* next = page->next;
            internal::page_delete(page, m_parent);
            page = next;
        }
    }
}

//#################################################### ./src/properties.cpp ####################################################

#include <cpp/lang/io/properties.hpp>
#include <cpp/lang/utils/date.hpp>

namespace tc {

    properties::properties(tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_values(allocator) {

    }

    bool properties::is_empty() const {
        return m_values.is_empty();
    }

    void properties::set(const string& key, const string& value) {
        m_values.put(key, value);
    }

    void properties::set(const char* key, const char* value) {   
        m_values.put(string(key, m_allocator), string(value, m_allocator));
    }

    const string& properties::get(const string& key) const {
        return m_values.get(key);
    }

    const string& properties::get_or_default(const string& key, string& _default) const {
        return m_values.get_or_default(key, _default);
    }

    const string& properties::get(const char* key) const {
        return get(string(key, m_allocator));
    }

    void properties::save(ostream& out) const {
        const char NEW_LINE = '\n';
        const char ASSIGN   = '=';

        {//store date
            date now = date::now();
            string date = now.to_string(m_allocator);
            out.write(date.c_str(), date.length() + 1);
            out.write(&NEW_LINE, 1);
        }

        for (const pair<tc::string, tc::string>& e : m_values)
        {    
            const string& key     = e.first();
            const string& value   = e.second();
            
            out.write(key.c_str(), key.length());
         
            out.write(&ASSIGN, 1);                      //add "="
         
            out.write(value.c_str(), value.length());
         
            out.write(&NEW_LINE, 1);                    //add "\n"
        }
    }

    void properties::put_property(string& key, string& value) {
        key.trim();
        value.trim();

        if (!key.is_empty())
            m_values.put(key, value);

        key.clear();
        value.clear();
    }

    void properties::load(istream& in)
    {
        enum struct state {KEY, VALUE, COMMENT};
        
        int reader;
        state state = state::KEY;

        string key (m_allocator);
        string val (m_allocator);

        while ((reader = in.read()) != -1)
        {
            if (state == state::KEY)
            {
                if (reader == '#')
                {
                    state = state::COMMENT;
                    continue;
                }
                if (reader == '=')
                {
                    state = state::VALUE;
                    continue;
                }
                key.append((char) reader);
            }
            else if (state == state::VALUE)
            {
                if (reader == '\n')
                {
                    put_property(key, val);
                    state = state::KEY;
                    continue;
                }
                val.append((char) reader);
            }
            else if (state == state::COMMENT)
            {
                if (reader == '\n')
                {
                    state = state::KEY;
                    continue;
                }
            }
        }
        
        put_property(key, val);
        
    }
}

//#################################################### ./src/rafstream.cpp ####################################################

#include <cpp/lang/io/rafstream.hpp>
#include <internal/io/io_helpers.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/traits/primitive_traits.hpp>
#include <cpp/lang/io/basebuf.hpp>
#include <cstdio>
#include <cerrno>

#if defined(JSTD_OS_WINDOWS)
# include <io.h>
# include <sys/stat.h>
# define STAT_STRUCT struct _stat
# define STAT_FUNC(fd, ptr) _fstat(fd, ptr)
# define FTRUNCATE(fd, size) _chsize_s(fd, size)
#elif defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC)
# include <sys/stat.h>
# include <unistd.h>
# define STAT_STRUCT struct stat
# define STAT_FUNC(fd, ptr) fstat(fd, ptr)
# define FTRUNCATE(fd, size) ftruncate(fd, size)
#endif

namespace tc
{

    typedef typename int_of<32>::utype len_type;

    static void fill_stat(int fd, STAT_STRUCT* s) {
        if (STAT_FUNC(fd, s) != 0)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }

    static int FILE_to_fd(FILE* f) {
        int fd = fileno(f);
        if (fd == -1)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
        return fd;  
    } 
}

namespace tc
{

    rafstream::rafstream() : m_handle(nullptr) {
    
    }
    
    rafstream::rafstream(const char* f, const char* m) : rafstream(file(f), m) {
        
    }
    
    rafstream::rafstream(const file& file, const char* mode) {
        
        if (strcmp(mode, "rw") == 0)
        {
            mode = "w+b";
        }
        else if (strcmp(mode, "r") == 0)
        {
            mode = "rb";
        }
        else if (strcmp(mode, "w") == 0)
        {
            mode = "wb";
        }
        
        expected<FILE*, error_code> fhandle = filesystem::open(file.c_str(), mode);
        if (!fhandle)
        {
            internal::io::throw_error_code(fhandle.error());
        }

        m_handle = static_cast<void*>(fhandle.value());
    }
    
    rafstream::rafstream(const file& file, const string& mode) : rafstream(file, mode.c_str()) {
    
    }
    
    rafstream::rafstream(const string& file, const string& mode) : rafstream(file.c_str(), mode.c_str()) {
    
    }
    
    rafstream::rafstream(rafstream&& s) : m_handle(s.m_handle) {
        s.m_handle = nullptr;
    } 

    rafstream& rafstream::operator=(rafstream&& s) {
        if (&s != this)
        {
            if (m_handle)
            {
                error_code dontcare;
                close(dontcare);
            }
            m_handle    = s.m_handle;
            s.m_handle  = nullptr;
        }
        return *this;
    }
    
    rafstream::~rafstream() {
        error_code dontcare;
        close(dontcare);
    }
    
    void rafstream::close() {
        error_code err;
        close(err);
        if (err)
        {
            internal::io::throw_error_code(err);
        }
    }
    
    void rafstream::close(error_code& err) {
        if (m_handle)
        {
            err = filesystem::close(static_cast<FILE*>(m_handle));
            m_handle = nullptr;
        }
    }
    
    void rafstream::set_length(std::size_t len) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        int fd = FILE_to_fd(static_cast<FILE*>(m_handle));
        if (FTRUNCATE(fd, len) != 0)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }
    
    std::size_t rafstream::length() {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        int fd = FILE_to_fd(static_cast<FILE*>(m_handle));
        STAT_STRUCT stat;
        fill_stat(fd, &stat);
        return static_cast<std::size_t>(stat.st_size);
    }
    
    void rafstream::seek(std::size_t pos) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        int err = std::fseek(static_cast<FILE*>(m_handle), static_cast<long>(pos), SEEK_SET);
        if (err != 0)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }
    
    std::size_t rafstream::get_file_pointer() {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        long pos = std::ftell(static_cast<FILE*>(m_handle));
        if (pos == -1L)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
        return static_cast<std::size_t>(static_cast<unsigned long>(pos));
    }
    
    void rafstream::skip_bytes(std::size_t n) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        
        const std::size_t BUF_SIZE = 128;
        char buf[BUF_SIZE];
        while (n)
        {
            std::size_t need_skip   = math::min(BUF_SIZE, n);
            std::size_t readed      = read(buf, need_skip);
            if (readed < need_skip)
                break;
            n -= readed;
        }
    }
    
    void rafstream::write(const char* arr, std::size_t len) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        std::size_t writed = fwrite(arr, sizeof(char), len, static_cast<FILE*>(m_handle));
        if (writed != len)
        {
            internal::io::throw_error_code(error_code(errno, generic_category()));
        }
    }
    
    void rafstream::write(char ch) {
        write(&ch, 1);
    }
    
    std::size_t rafstream::read(char buf[], std::size_t len) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        
        FILE* handle = static_cast<FILE*>(m_handle);
        
        std::size_t readed = std::fread(buf, sizeof(char), len, handle);
        if (readed == 0)
        {
            if (std::feof(handle))
            {
                return eof_value();
            }
            else if (std::ferror(handle))
            {
                internal::io::throw_error_code(error_code(errno, generic_category()));
            }
        }

        return readed;
    }
    
    int rafstream::read() {
        char c;
        std::size_t r = read(&c, 1);
        if (r == 0)
            return -1;
        return c & 0xFF;
    }

    void rafstream::write_string(const string& x) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        len_type len = static_cast<len_type>(x.length());
        write<len_type>(len);
        write(x.c_str(), len);
    }

    string rafstream::read_string(tca::allocator* alloc) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )
        
        string result(alloc);

        len_type len = read<len_type>();
        
        result.set_length(len);
        
        std::size_t readed = read(result.c_str(), len);
    
        if (readed != len)
        {
            throw_except<eof_exception>("cannot read type");
        }

        result.c_str()[len] = 0;

        return result;
    }

    void rafstream::force(bool metadata) {
        JSTD_DEBUG_CODE
        (
            if (!m_handle)
                throw_except<io_exception>("file not open");
        )

        if (std::fflush(static_cast<FILE*>(m_handle)) != 0)
        {
            internal::io::throw_error_code( error_code(errno, generic_category()) );
        }

        #if 1
        JSTD_WIN_CODE
        (
            HANDLE fhandle = reinterpret_cast<HANDLE>(
                _get_osfhandle(fileno( static_cast<FILE*>(m_handle) ))
            );
            
            if (!FlushFileBuffers(fhandle))
            {
                internal::io::throw_error_code(error_code(GetLastError(), system_category()));
            }
        )

        JSTD_UNIX_CODE
        (
            int fhandle = fileno(static_cast<FILE*>(m_handle));
            int err  = 0;
            
            if (metadata) err = fsync(fhandle);
            else          err = fdatasync(fhandle);
            
            if (err)
            {
                internal::io::throw_error_code(error_code(errno, generic_category()));
            }
        )
        #endif
    }
}

//#################################################### ./src/random.cpp ####################################################

#include <cpp/lang/utils/random.hpp>
#include <cpp/lang/math/math.hpp>

namespace tc
{

    unsigned long long random::random_seed() {
        return (unsigned long long) (system::current_time_millis() & 0xffff);
    }
    
    unsigned long long random::next() {
        const unsigned long long a = 0x2363239423U;
        const unsigned long long c = 17U;
        m_seed = (m_seed * a + c);
        unsigned long long result = m_seed;
        result ^= result >> 16;
        result *= 0xa68bU;
        return result;
    }
    
    random::random(unsigned long long seed) : m_seed(seed) {

    }
    
    random::random(const random& rnd) : m_seed(rnd.m_seed)  {

    }
    
    random::random(random&& rnd) : m_seed(rnd.m_seed) {

    }
    
    random& random::operator=(const random& rnd) {
        m_seed = rnd.m_seed;
        return *this;
    }
    
    random& random::operator=(random&& rnd) {
        m_seed = rnd.m_seed;
        return *this;
    }
    
    random::~random() {

    }

    template<>
    float random::next() {
        unsigned long long v = next();
        return (float) math::abs(v) / (float) num_limits<unsigned long long>::max();
    }

    template<>
    double random::next() {
        unsigned long long v = next();
        return (double) math::abs(v) / (double) num_limits<unsigned long long>::max();
    }

    template<>
    long double random::next() {
        unsigned long long v = next();
        return (long double) math::abs(v) / (long double) num_limits<unsigned long long>::max();
    }

    template<>
    bool random::next() {
        return ((next() >> 31) & 1) != 0;
    }
}

//#################################################### ./src/smooth_noise.cpp ####################################################

#include <cpp/lang/utils/smooth_noise.hpp>
#include <cpp/lang/math/math.hpp>

namespace tc
{

    smooth_noise::smooth_noise(curve_func curve) : m_curve_func(curve) {

    }

    smooth_noise::smooth_noise(unsigned long long seed, curve_func curve) : smooth_noise(curve) {
        set_seed(seed);
        generate_values();
    }

    unsigned long long smooth_noise::get_seed() const {
        return m_seed;
    }
    
    void smooth_noise::set_seed(unsigned long long seed) {
        m_seed = seed;
    }

    void smooth_noise::generate_values() {
        random(m_seed).values<signed char>(m_buffer, BUF_SIZE, num_limits<signed char>::max());
    }

    static unsigned long hash2d(long x, long y) {
        const unsigned long ux = (unsigned  long) x;
        const unsigned long uy = (unsigned  long) y;
        unsigned long hash = (ux * 0x0B00B135U) + (uy * 0x50FFC001U);
        hash ^= hash >> 13U;
        hash += hash & 832749825U;
        hash ^= hash >> 16U;
        return hash;
    }

    signed char smooth_noise::value_at(long x, long y) const {
        unsigned long hash = hash2d(x, y);
        return m_buffer[hash & (BUF_SIZE - 1)];
    }

    static long floor_div(long num, long denom) {
        long   r = num / denom;
        return num >= 0 ? r : r - 1;
    }

    signed char smooth_noise::get0(long x, long y, int scale) const {
        if (scale == 0)
            return 0;

        long xChunk = floor_div(x, scale);
        long yChunk = floor_div(y, scale);

        long xStart = xChunk * scale;
        long yStart = yChunk * scale;

        long xc = x - xStart;
        long yc = y - yStart;

        signed char v0 = value_at(xChunk,     yChunk);
        signed char v1 = value_at(xChunk + 1, yChunk);

        signed char v2 = value_at(xChunk,     yChunk + 1);
        signed char v3 = value_at(xChunk + 1, yChunk + 1);

        signed char a = (signed char) ( ((v1 - v0) * xc / scale) + v0 );
        signed char b = (signed char) ( ((v3 - v2) * xc / scale) + v2 );        
        signed char c = (signed char) ( (( b - a ) * yc / scale) + a  );

        return c;
    }

    num::q16 smooth_noise::get1(long x, long y, int scale) const {
        if (scale == 0)
            return 0;

        long xChunk = floor_div(x, scale);
        long yChunk = floor_div(y, scale);

        long xStart = xChunk * scale;
        long yStart = yChunk * scale;

        long xc = x - xStart;
        long yc = y - yStart;

        signed char v0 = value_at(xChunk,     yChunk);
        signed char v1 = value_at(xChunk + 1, yChunk);

        signed char v2 = value_at(xChunk,     yChunk + 1);
        signed char v3 = value_at(xChunk + 1, yChunk + 1);

        using num::q16;

        q16 tx = q16(xc) / q16(scale);
        q16 ty = q16(yc) / q16(scale);

        tx = m_curve_func(tx);
        ty = m_curve_func(ty);

        q16 a = math::lerp<q16>(v0, v1, tx);
        q16 b = math::lerp<q16>(v2, v3, tx);
        q16 c = math::lerp<q16>(a,  b,  ty);

        return c / q16(127);
    }

    float smooth_noise::get(long x, long y, int scale) const {
        return get1(x, y, scale);
    }

    float smooth_noise::get(long x, long y, int scale, int octaves, int scale_factor /* = 2 */, int freq_factor /* = 2 */) const {
        JSTD_DEBUG_CODE
        (
            if (scale        <= 0) throw_except<illegal_argument_exception>("Invalid scale: %i",scale);
            if (octaves      <= 0) throw_except<illegal_argument_exception>("Invalid count octaves: %i", octaves);
            if (scale_factor < 0 ) throw_except<illegal_argument_exception>("Invalid scale_factor octaves: %f", scale_factor);
            if (freq_factor  < 0 ) throw_except<illegal_argument_exception>("Invalid freeq_factor octaves: %f", freq_factor);
        );

        using num::q16;

        int freq       = 1;
        q16 result     = 0;
        q16 amplitude  = 1;
        q16 weigth_sum = 0;

        while (octaves > 0)
        {
            int octave_scale = scale / freq;
            if (octave_scale == 0) break;
            
            result     += get1(x, y, octave_scale) * amplitude;
            weigth_sum += amplitude;
            amplitude  /= q16(scale_factor);
            freq       *= freq_factor;
            
            --octaves;
        }
        
        return (float) (result / weigth_sum);
    }

    void smooth_noise::set_curve_fuc(curve_func func) {
        JSTD_DEBUG_CODE(
            if (func == nullptr)
                throw_except<illegal_argument_exception>("func pointer == null");
        );
        m_curve_func = func;
    }

    /*static*/ num::q16 smooth_noise::no_smooth(const num::q16& x) {
        return x;
    }
    
}

// #include <cpp/lang/utils/images/image.hpp>
// #include <cpp/lang/utils/images/imageio.hpp>

// #include <stdio.h>
// #include <math.h>
// #include <stdlib.h>

// int main() {
//     using namespace tc;
//     int size = 1024;
//     image img(size, size, 1);
    
//     smooth_noise noise(system::current_time_millis(), math::quintic);

//     timepoint last = system::nano_time();
//     for (int i = 0; i < size; ++i)
//     {
//         int xo = -10000000;
//         int yo = 10000000;
        
//         for (int j = 0; j < size; ++j)
//         {
//             // float h = noise.get(xo + i, yo + j, 128, 8, 4, 4);
//             float h = noise.get(xo + i, yo + j, 1024, 8, 2, 2);
//             img.get_gray(i, j) = image::gray((unsigned char) (h * 255));

//         }
//     }
//     timepoint now = system::nano_time();
//     printf("passed: %f\n", (double) (now - last) / 1000000.0);

//     imageio::write_image(tc::file("./rnd.png"), &img, ".png");
// }

//#################################################### ./src/stacktrace.cpp ####################################################

#include <cpp/lang/stacktrace/stacktrace.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/system.hpp>
#include <utility>
#include <iostream>
#include <string>

namespace tc
{

    calltrace::calltrace() JSTD_CALLTRACE_CODE( : m_stacktrace() ) {

    }

    calltrace::calltrace(const calltrace& ct) JSTD_CALLTRACE_CODE( : m_stacktrace(ct.m_stacktrace) )   {

    }
    
    calltrace::calltrace(calltrace&& ct) JSTD_CALLTRACE_CODE( : m_stacktrace(std::move(ct.m_stacktrace)) )   {

    }
    
    calltrace& calltrace::operator=(const calltrace& ct) {
        if (&ct != this)
        {
            JSTD_CALLTRACE_CODE
            (
                m_stacktrace = ct.m_stacktrace;
            );
        }
        return *this;
    }
    
    calltrace& calltrace::operator=(calltrace&& ct) {
        if (&ct != this)
        {
            JSTD_CALLTRACE_CODE
            (
                m_stacktrace = std::move(ct.m_stacktrace);
            );
        }
        return *this;
    }
    
    calltrace::~calltrace() {

    }
    
    /*static*/ calltrace calltrace::current() {
        JSTD_CALLTRACE_CODE (
            calltrace current;
            current.m_stacktrace = std::stacktrace::current();
            return current;
        );
        return calltrace();
    }

    void calltrace::print() const {
        JSTD_CALLTRACE_CODE (
            if (m_stacktrace.size() > 0)
            {
                for (unsigned short i = 0; i < m_stacktrace.size() - 1; ++i)
                {
                    std::string description      = m_stacktrace.at(i).description();
                    std::string callable_file    = m_stacktrace.at(i + 1).source_file();
                    unsigned int callable_line   = m_stacktrace.at(i + 1).source_line();
                    system::tsprintf("      at %s(\033[36m%s:%u\033[0m)\n", description.c_str(), callable_file.c_str(), callable_line);
                }
            }
        );
    }
    
    void calltrace::write_log(class ostream* out) const {
        JSTD_CALLTRACE_CODE(
            try {
                for (const std::stacktrace_entry& entry : m_stacktrace)
                {
                    const std::string& desc = entry.description();
                    out->write(desc.c_str(), desc.length());
                }
            } catch (const throwable& t) {
                std::cout << t.cause() << "\n";
            }
        );
    }

}



//#################################################### ./src/string.cpp ####################################################

#include <cpp/lang/string.hpp>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <cwchar>

namespace tc
{
    template<typename TCHAR>
    tstring<TCHAR>::tstring(tca::allocator* allocator) : data(nullptr), allocator(allocator), cap(INLINE_BUFFER_SIZE), size(0) {

    }

    template<typename TCHAR>
    tstring<TCHAR>::tstring(const TCHAR* s, tca::allocator* alloc) : data(nullptr), allocator(alloc), cap(INLINE_BUFFER_SIZE), size(0) {
        append(0, s);
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::tstring(const tstring<TCHAR>& s) : tstring<TCHAR>(s.c_str(), s.get_allocator()) {
        
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::tstring(tstring<TCHAR>&& s) : allocator(s.allocator), cap(s.cap), size(s.size) {
        if (s.is_inline_string())
        {
            std::memcpy(inline_data, s.inline_data, (s.size + 1) * sizeof(TCHAR));
        }
        else
        {
            data = s.data;
            s.data = nullptr;
        }

        s.cap   = INLINE_BUFFER_SIZE;
        s.size  = 0;
        s.inline_data[0] = TCHAR(0);
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::operator=(const tstring<TCHAR>& s) {
        if (&s != this)
        {
            if (s.length() < INLINE_BUFFER_SIZE)
            {
                if (!is_inline_string())
                    allocator->deallocate(data);
                
                assert(s.length() < INLINE_BUFFER_SIZE);
                
                std::memcpy(inline_data, s.c_str(), (s.length() + 1) * sizeof(TCHAR));
                cap  = INLINE_BUFFER_SIZE;
                size = s.length();
            }
            else
            {
                assert(allocator != nullptr);
                
                std::size_t new_cap = (s.length() + 1);
                TCHAR* new_data = static_cast<TCHAR*>(
                                                        allocator->allocate_align(new_cap * sizeof(TCHAR), alignof(TCHAR))
                                                    );
                if (!new_data)
                    throw_except<out_of_memory_error>("out of memory");
                
                std::memcpy(new_data, s.c_str(), (s.length() + 1) * sizeof(TCHAR));

                if (!is_inline_string())
                    allocator->deallocate(data);
                
                data    = new_data;
                cap     = new_cap;
                size    = s.length();
            }
        }
        return *this;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::operator=(tstring<TCHAR>&& s) {
        if (this == &s) return *this;

        if (get_allocator() == s.get_allocator())
        {
            if (!is_inline_string())
            {
                allocator->deallocate(data);
            }
            
            if (s.is_inline_string())
            {
                cap  = s.cap;
                size = s.length();
                std::memcpy(inline_data, s.c_str(), sizeof(TCHAR) * (s.length() + 1));
            }
            else
            {
                data   = s.data;
                size   = s.size;
                cap    = s.cap;
            }
            s.size           = 0;
            s.inline_data[0] = 0;
            s.cap            = INLINE_BUFFER_SIZE;
        }
        else
        {
            *this = s;
        }
        return *this;
    }

    template<typename TCHAR>
    void tstring<TCHAR>::ensure_cap(std::size_t new_size) {
        std::size_t new_cap = new_size + 1;
        if (new_cap <= INLINE_BUFFER_SIZE)
            return;

        TCHAR* new_data     = static_cast<TCHAR*>( allocator->allocate_align(sizeof(TCHAR) * new_cap, alignof(TCHAR)) );
        if (!new_data)
            throw_except<out_of_memory_error>("out of memory");

        std::memcpy(new_data, c_str(), (size + 1) * sizeof(TCHAR));

        //Если предыдущий буфер строки не встроен в строку, освобождаем
        if (!is_inline_string())
            allocator->deallocate(data);
        
        data = new_data;
        cap  = new_cap;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>::~tstring() {
        if (!is_inline_string() && data)
        {
            allocator->deallocate(data);
        }
    }

    template<typename TCHAR>
    TCHAR* tstring<TCHAR>::c_str() {
        return is_inline_string() ? inline_data : data;
    }

    template<typename TCHAR>
    const TCHAR* tstring<TCHAR>::c_str() const {
        return is_inline_string() ? inline_data : data;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::reserve(std::size_t sz) {
        if (sz >= INLINE_BUFFER_SIZE)
        {
            std::size_t new_cap = sz + 1;
            std::size_t len     = math::min(length(), sz);

            TCHAR* new_data = static_cast<TCHAR*>( allocator->allocate_align(new_cap * sizeof(TCHAR), alignof(TCHAR)) );
            if (!new_data)
                throw_except<out_of_memory_error>("out of memory");
            
            std::memcpy(new_data, c_str(), len * sizeof(TCHAR));
            new_data[len] = 0;
            
            if (!is_inline_string())
                allocator->deallocate(data);
            
            data    = new_data;
            cap     = new_cap;
            size    = len;
        }
        else
        {
            if (!is_inline_string())
            {
                TCHAR* data = c_str();
                std::memcpy(inline_data, data, math::min(static_cast<std::size_t>(INLINE_BUFFER_SIZE), size) * sizeof(TCHAR));
                cap = static_cast<std::size_t>(INLINE_BUFFER_SIZE);
                allocator->deallocate(data);
            }
            size    = math::min(size, sz);
            inline_data[sz] = 0;
        }
        return *this;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::append(std::size_t idx, const TCHAR* s, std::size_t len) {
        JSTD_DEBUG_CODE(
            if (idx > size)
                throw_except<index_out_of_bound_exception>("Index %zu out of bound for length %zu!", idx, size);
        )
        
        len = normalize_length(s, len);

        if (rem() <= len)
            ensure_cap(size + len);
        
        TCHAR* str = c_str();
        
        std::memmove(
                    str + (idx + len), 
                    str + idx, 
                    (size - idx) * sizeof(TCHAR)
        );
        
        std::memcpy(
            str + idx, 
            s, 
            len * sizeof(TCHAR)
        );
        
        size += len;
        str[size] = 0;

        return *this;
    }

    template<typename TCHAR>
    std::size_t tstring<TCHAR>::index_of(const TCHAR* s, std::size_t from_index, std::size_t len) const {
        JSTD_DEBUG_CODE(
            if (from_index >= length()) throw_except<illegal_argument_exception>("from_index can't be greater length");
        )
        
        len = normalize_length(s, len);
        if (len == 0 || len > (length() - from_index)) return npos();

        for (std::size_t i = from_index; i <= length() - len; ++i)
        {
            bool match = true;
            for (std::size_t j = 0; j < len; ++j)
            {
                assert(j < len);
                assert(i + j < size);
                if (c_str()[i + j] != s[j])
                {
                    match = false;
                    break;
                }
            }   
            if (match) return i;
        }

        return npos();
    }
    
    template<typename TCHAR>
    std::size_t tstring<TCHAR>::last_index_of(const TCHAR* s, std::size_t len) const {
        len = normalize_length(s, len);
    
        if (len == 0 || len > length()) return npos();
        
        for (std::size_t i = length() - len + 1; i > 0;)
        {
            --i;
            bool match = true;
            for (std::size_t j = 0; j < len; ++j)
            {
                if (c_str()[i + j] != s[j])
                {
                    match = false;
                    break;
                }
            }
            
            if (match) return i;
        }
        
        return npos();
    }

    template<typename TCHAR>
    void tstring<TCHAR>::clear() {
        c_str()[0]   = 0;
        size        = 0;
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::is_empty() const {
        return length() == 0;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::remove(std::size_t start, std::size_t end) {
       JSTD_DEBUG_CODE(
            if (end < start)    throw_except<illegal_argument_exception>("'start' can't less 'end' where [start: %zu, end: %zu]", start, end);
            if (end > length()) throw_except<illegal_argument_exception>("'end' must be less or equal 'length' where [start: %zu, length: %zu]", start, length());
        )
        std::size_t len = end - start;
        TCHAR* str      = c_str();
        
        std::size_t s = start;                          //start
        std::size_t e = end;                            //end
        std::size_t l = (size - end) * sizeof(TCHAR);   //length

        if (l > 0)
        {
            std::memmove(str + s, str + e, l);
        }

        size -= len;    
        str[size] = 0;

        return *this;
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::contains(const TCHAR* s, std::size_t len) const {
        return index_of(s, len) != npos();
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::starts_with(std::size_t offset, const TCHAR* s, std::size_t len) const {

        len = normalize_length(s, len);
        
        if (len == 0 || len > length()) return false;
        if (offset >= length())         return false;
        if (length() - offset < len)    return false;

        for (std::size_t i = 0; i < len; ++i)
        {
            assert( 
                (offset < length()) && 
                (i < length() - offset) 
            );
            if (c_str()[i + offset] != s[i])
                return false;
        }

        return true;
    }
    
    template<typename TCHAR>
    bool tstring<TCHAR>::ends_with(const TCHAR* s, std::size_t len) const {
        len = normalize_length(s, len);
        if (len == 0 || len > length())
            return false;
        
        std::size_t start   = length() - len;
    
        for (std::size_t i = start, j = 0; j < len; ++i, ++j)
            if (c_str()[i] != s[j])
                return false;

        return true;
    }

    template<typename TCHAR>
    std::size_t count_match(const TCHAR* str, std::size_t slen, const TCHAR* match, std::size_t mlen) {
        if (slen < mlen)
            return 0;
        
        std::size_t count = 0;
        for (std::size_t i = 0; i <= slen - mlen;)
        {
            
            bool matched = true;
            for (std::size_t j = 0; j < mlen; ++j)
            {
                assert(i + j < slen);
                assert(j < mlen);
                if (str[i + j] != match[j])
                {
                    matched = false;
                    break;
                }
            }

            if (matched)
            {
                i += mlen;
                ++count;
            }
            else
            {
                ++i;
            }
            
        }

        return count;
    }

    template<typename TCHAR>
    std::size_t unsafe_append(TCHAR* str, std::size_t slen, std::size_t idx, const TCHAR* a, std::size_t alen) {
        std::size_t rem = slen - idx;
        if (rem > 0)
        {
            std::memmove(str + idx + alen, str + idx, rem * sizeof(TCHAR));
        }
        std::memcpy(str + idx, a, sizeof(TCHAR) * alen);

        std::size_t newlen = slen + alen;

        str[newlen] = 0;

        return newlen;
    }
    
    template<typename TCHAR>
    std::size_t unsafe_remove(TCHAR* str, std::size_t slen, std::size_t start, std::size_t end) {
        assert(start < end);
        std::size_t rem = slen - end;
        if (rem > 0)
        {
            std::memmove(str + start, str + end, sizeof(TCHAR) * rem);
        }

        std::size_t newlen = slen - (end - start);
        str[newlen] = 0;

        return newlen;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::replace_all(const TCHAR* matcher, const TCHAR* replacement) {
        std::size_t matcher_len     = str_len(matcher);
        std::size_t replacement_len = str_len(replacement);

        std::size_t count = count_match(c_str(), length(), matcher, matcher_len);
        
        std::size_t newlen = (length() - (matcher_len * count)) + (replacement_len * count);

        if (newlen >= capacity())
        {
            reserve(newlen);
        }

        if (matcher_len == 0 || matcher_len > length())
            return *this;

        std::size_t len = length();

        for (std::size_t i = 0; i <= len - matcher_len; )
        {
            bool matched = match(c_str() + i, matcher, matcher_len);
            if (matched)
            {
                len = unsafe_remove(c_str(), len, i, i + matcher_len);
                len = unsafe_append(c_str(), len, i, replacement, replacement_len);
                i += replacement_len;
            }
            else
            {
                ++i;
            }
        }

        assert(len == newlen);
        size = newlen;

        return *this;
    }

    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::replace(std::size_t start, std::size_t end, const TCHAR* replacement) {
        JSTD_DEBUG_CODE(
            if (start >= length())  throw_except<illegal_argument_exception>("start can't be equal or greater length()");
            if (end   >  length())  throw_except<illegal_argument_exception>("end can't be greater length()");
            if (end   <  start)     throw_except<illegal_argument_exception>("end can't be less start");
        );

        std::size_t rep_len = str_len(replacement);
        std::size_t newlen = (length() - (end - start)) + rep_len;
        if (newlen >= capacity())
        {
            reserve(newlen);
        }

        std::size_t len = length();

        len = unsafe_remove(c_str(), len, start, end);
        len = unsafe_append(c_str(), len, start, replacement, rep_len);

        size = len;
        c_str()[len] = 0;

        assert(len == newlen);

        return *this;
    }

    template<typename TCHAR>
    tstring<TCHAR> tstring<TCHAR>::sub_string(std::size_t start, std::size_t end, tca::allocator* allocator) const {
        JSTD_DEBUG_CODE(
            if (end < start)    throw_except<illegal_argument_exception>("'start' can't less 'end' where [start: %zu, end: %zu]", start, end);
            if (end > length()) throw_except<illegal_argument_exception>("'end' must be less or equal 'length' where [start: %zu, length: %zu]", start, length());
        )
        
        std::size_t len = end - start;

        tstring<TCHAR> sub(allocator);
        sub.reserve(len);
        sub.append(c_str() + start, len);
        
        return tstring<TCHAR>(std::move(sub));
    }

    template<typename TCHAR>
    bool tstring<TCHAR>::equals(const tstring<TCHAR>& s) const {
		if (length() != s.length()) return false;
        return objects::equals(begin(), end(), s.begin(), s.end());
    }
    
    template<typename TCHAR>
    bool tstring<TCHAR>::equals(const TCHAR* s) const {
		if (length() != str_len(s)) return false;
        return objects::equals(begin(), end(), s, s + length());
    }

    template<typename TCHAR>
    TCHAR* tstring<TCHAR>::begin() {
        return c_str();
    }
    
    template<typename TCHAR>
    TCHAR* tstring<TCHAR>::end() {
        return c_str() + length();
    }

    template<typename TCHAR>
    const TCHAR* tstring<TCHAR>::begin() const {
        return c_str();
    }
    
    template<typename TCHAR>
    const TCHAR* tstring<TCHAR>::end() const {
        return c_str() + length();
    }

    template<typename TCHAR>
    std::size_t tstring<TCHAR>::hashcode() const {
        return objects::hashcode(c_str(), c_str() + length(), hash_for<TCHAR>());
    }
    
    template<typename TCHAR>
    int tstring<TCHAR>::compare_to(const tstring<TCHAR>& r) {
        std::size_t len = math::min(length(), r.length());
        for (std::size_t i = 0; i < len; ++i)
            if (char_at(i) < r.char_at(i))
                return -1;
            else if (char_at(i) > r.char_at(i))
                return 1;
        if (length() < r.length()) return -1;
        if (length() > r.length()) return 1;
        return 0;
    }
    
    template<typename TCHAR>
    tstring<TCHAR>& tstring<TCHAR>::trim() {
        std::size_t start = 0;
        std::size_t end = length();

        while (start < end && char_at(start) <= 0x20)
            ++start;

        while (end > start && char_at(end - 1) <= 0x20)
            --end;

        const std::size_t len = end - start;

        if (start != 0 && len != 0)
            std::memmove(c_str(), c_str() + start, len * sizeof(TCHAR));

        size = len;
        c_str()[size] = 0;

        return *this;
    }

    template<typename TCHAR>
    void tstring<TCHAR>::set_length(std::size_t newlen, const TCHAR& ch) {
        if (newlen >= capacity())
        {
            reserve(newlen);
        }

        if (newlen > length())
        {
            while (size < newlen)
                c_str()[size++] = ch;
            c_str()[size] = 0;
        }
        else
        {
            c_str()[newlen] = 0;
            size = newlen;
        }
    }
} //namespace tc

namespace tc
{
namespace internal
{
    static const std::size_t CHAR_BUF = 32;
    template<typename E>
    tstring<char> to_string0(const E& e, const char* ext, tca::allocator* allocator) {
        tstring<char> str(allocator);
        char buf[CHAR_BUF];    
        int len = std::snprintf(buf, CHAR_BUF, ext, e);
        if (len > 0)
            str.append(buf, static_cast<std::size_t>(len));
        return tstring<char>( std::move(str) );
    }

    template<typename E>
    tstring<wchar_t> to_wstring0(const E& e, const wchar_t* ext, tca::allocator* allocator) {
        tstring<wchar_t> str(allocator);
        wchar_t buf[CHAR_BUF];    
        int len = std::swprintf(buf, CHAR_BUF, ext, e);
        if (len > 0)
            str.append(buf, static_cast<std::size_t>(len));
        return tstring<wchar_t>( std::move(str) );
    }

    // explicit instancing to_string
    template tstring<char> to_string0<char>                 (const char&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned char>        (const unsigned char&, const char*, tca::allocator*);
    template tstring<char> to_string0<signed char>          (const signed char&, const char*, tca::allocator*);
    template tstring<char> to_string0<short>                (const short&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned short>       (const unsigned short&, const char*, tca::allocator*);
    template tstring<char> to_string0<int>                  (const int&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned int>         (const unsigned int&, const char*, tca::allocator*);
    template tstring<char> to_string0<long>                 (const long&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned long>        (const unsigned long&, const char*, tca::allocator*);
    template tstring<char> to_string0<long long>            (const long long&, const char*, tca::allocator*);
    template tstring<char> to_string0<unsigned long long>   (const unsigned long long&, const char*, tca::allocator*);
    template tstring<char> to_string0<float>                (const float&, const char*, tca::allocator*);
    template tstring<char> to_string0<double>               (const double&, const char*, tca::allocator*);
    template tstring<char> to_string0<long double>          (const long double&, const char*, tca::allocator*);
    template tstring<char> to_string0<void*>                (void* const&, const char*, tca::allocator*);
    template tstring<char> to_string0<const void*>          (const void* const&, const char*, tca::allocator*);
    
    // explicit instancing to_wstring
    template tstring<wchar_t> to_wstring0<wchar_t>              (const wchar_t&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned char>        (const unsigned char&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<signed char>          (const signed char&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<short>                (const short&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned short>       (const unsigned short&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<int>                  (const int&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned int>         (const unsigned int&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<long>                 (const long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned long>        (const unsigned long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<long long>            (const long long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<unsigned long long>   (const unsigned long long&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<float>                (const float&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<double>               (const double&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<long double>          (const long double&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<void*>                (void* const&, const wchar_t*, tca::allocator*);
    template tstring<wchar_t> to_wstring0<const void*>          (const void* const&, const wchar_t*, tca::allocator*);

} //namespace internal
} //namespace tc

// Explicit instaincing strings
namespace tc
{
    template class tstring<char>;
    template class tstring<unsigned char>;
    template class tstring<signed char>;
    
    template class tstring<wchar_t>;
    
    template class tstring<short>;
    template class tstring<unsigned short>;
    
    template class tstring<int>;
    template class tstring<unsigned int>;
    
    template class tstring<long>;
    template class tstring<unsigned long>;
    
    template class tstring<long long>;
    template class tstring<unsigned long long>;
} //namespace tc

//#################################################### ./src/synchronized_allocator.cpp ####################################################

#include <allocators/synchronized_allocator.hpp>

namespace tca
{

    synchronized_allocator::synchronized_allocator(allocator* alloc) : m_alloc(alloc), m_mutex() {
    
    }
    
    synchronized_allocator::synchronized_allocator(synchronized_allocator&& a) : m_alloc(a.m_alloc), m_mutex() {
        
    }
    
    synchronized_allocator& synchronized_allocator::operator= (synchronized_allocator&& a) {
        m_alloc = a.m_alloc;
        return *this;
    }
    
    void* synchronized_allocator::allocate(std::size_t sz) {
        tc::unique_lock lock(m_mutex);
        return m_alloc->allocate(sz);
    }
    
    void* synchronized_allocator::allocate_align(std::size_t sz, std::size_t align) {
        tc::unique_lock lock(m_mutex);
        return m_alloc->allocate_align(sz, align);
    }
    
    void synchronized_allocator::deallocate(void* p) {
        tc::unique_lock lock(m_mutex);
        m_alloc->deallocate(p);
    }
    
    void synchronized_allocator::deallocate(void* p, std::size_t) {
        tc::unique_lock lock(m_mutex);
        m_alloc->deallocate(p);
    }

}

//#################################################### ./src/system.cpp ####################################################

#include <cpp/lang/system.hpp>
#include <cpp/lang/concurrency/mutex.hpp>
#include <cstdarg>
#include <cstdio>
#include <cpp/lang/utils/cond_compile.hpp>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <cstring>
    #include<sys/time.h>
	#include<time.h>
#endif

namespace tc
{   
namespace system
{

namespace internal
{

    byte_order init_native_byte_order() {
        const unsigned long c   = 1;
        const unsigned char* ip = reinterpret_cast<const unsigned char*>(&c);
        return *ip != 0 ? byte_order::LE : byte_order::BE;
    }
    
    const byte_order system_order = init_native_byte_order();

}

    
    timepoint current_time_millis() {
    #if defined(__linux__) || defined(__APPLE__)
        timeval time;
        gettimeofday(&time, NULL);
        return (timepoint) ((time.tv_sec * 1000) + (time.tv_usec / 1000));
    #elif _WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER time;
        time.LowPart    = ft.dwLowDateTime;
        time.HighPart   = ft.dwHighDateTime;
        ULONGLONG millesec = time.QuadPart / (timepoint) 10000;
        return (timepoint) (millesec - (timepoint) 11644473600000);
    #else
        #error Undefined platform
    #endif
    }
    
    timepoint current_time_seconds() {
        return current_time_millis() / 1000;
    }

    timepoint nano_time() {
        const long NS_SECOND = 1000000000L;
    #if defined(JSTD_OS_WINDOWS)
		
		LARGE_INTEGER freeq, counter;
        
        QueryPerformanceFrequency(&freeq);
        QueryPerformanceCounter(&counter);
        
        return (timepoint) ((counter.QuadPart * NS_SECOND) / freeq.QuadPart);
    #elif defined(JSTD_OS_MAC) || defined(JSTD_OS_LINUX)
		timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        return (timepoint) ((time.tv_sec * NS_SECOND) + time.tv_nsec);
    #else
        #error Undefined OS
    #endif
    }


#if defined(_WIN32)
    const char* error_string(int err) {
        thread_local char no_specified_error_buffer[48];
        switch(err){
			case ERROR_SUCCESS              : return "No error";
			case ERROR_INSUFFICIENT_BUFFER  : return "The buffer size is insufficient to store the full path to the file";
			case ERROR_INVALID_HANDLE       : return "Invalid handle";
			case ERROR_MOD_NOT_FOUND        : return "Module not found";
			case ERROR_ACCESS_DENIED        : return "Access denied";
			case ERROR_INVALID_PARAMETER    : return "Invalid parameter";
			case ERROR_NOT_ENOUGH_MEMORY    : return "There is not enough memory to perform the operation";
			case ERROR_INVALID_FUNCTION     : return "The wrong was called to perform the operation";
			case ERROR_BAD_PATHNAME         : return "Bad path name";
			case ERROR_FILE_NOT_FOUND       : return "File not found";
			case ERROR_INVALID_NAME         : return "Invalid name";
			case ERROR_BAD_FORMAT           : return "Bad format";
            case ERROR_NOACCESS             : return "Invalid access to memory location";
            case ERROR_MAPPED_ALIGNMENT     : return "The base address or the file offset specified does not have the proper alignment";
            case ERROR_USER_MAPPED_FILE     : return "The requested operation cannot be performed on a file with a user-mapped section open";
			default:
                    std::snprintf(no_specified_error_buffer, sizeof(no_specified_error_buffer), "Windows error: %i", err);
                    return no_specified_error_buffer;
		}
    }
#elif defined(__linux__)
    const char* error_string(int err) {
        return strerror(err);
    }
#endif

    int tsprintf(const char* format, ...) {
        static tc::mutex ls_mutex;
        ls_mutex.lock();
            std::va_list args;
            va_start(args, format);
            int result = std::vprintf(format, args);
            va_end(args);
        ls_mutex.unlock();
        return result;
    }

}//namespace system

}//namespace jstd

//#################################################### ./src/thread.cpp ####################################################

#include <cpp/lang/concurrency/thread.hpp>
#include <cpp/lang/concurrency/mutex.hpp>
#include <cpp/lang/concurrency/runnable.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/system.hpp>
#include <cstring>
#include <exception>
#include <iostream>

namespace tc {
    
    /*static*/ const thread::state thread::state::NEW         = {0};
    /*static*/ const thread::state thread::state::RUNNABLE    = {1};
    /*static*/ const thread::state thread::state::TERMINATED  = {2};
    
    const char* thread::state::to_string() const {
        if (*this == NEW)               return "NEW";
        else if (*this == RUNNABLE)     return "RUNNABLE";
        else if (*this == TERMINATED)   return "TERMINATED";
        else                            return "Illegal state!";
    }
    
    std::size_t thread::state::hashcode() const {
        return (std::size_t) m_state;
    }
    
    bool thread::state::equals(const state& other) const {
        return m_state == other.m_state;
    }

    bool thread::state::operator==(const thread::state& state) const {
        return equals(state);
    }
    
    bool thread::state::operator!=(const thread::state& state) const {
        return !equals(state);
    }


    /**
     * Функция для запуска нового потока.
     * 
     * @param _this
     *      Указатель на объект потока, который владеет реальным потоком ОС.
     */
    void new_thread_start_func(thread* _this);

    /*static */ std::size_t thread::total_threads = 0;

    /*static */ std::mutex thread::global_lock;

    thread::thread() : thread(nullptr, nullptr, 0) {

    }

    thread::thread(runnable* task, const char* thread_name, std::size_t) : m_thread(), m_runnable(nullptr), m_thread_id(0) {
        global_lock.lock(); {
            m_thread_id = total_threads++;
        } global_lock.unlock();
          
        m_runnable  = task;    
        m_state     = thread::state::NEW;

        if (thread_name != nullptr)
            set_name(thread_name);
        else
            std::snprintf(m_name, sizeof(m_name), "Thread-%llu", (unsigned long long ) m_thread_id);
    }
    
    thread::thread(thread&& t) : m_thread(), m_runnable(nullptr), m_thread_id(0), m_state(state::NEW) {
        *this = std::move(t);
    }
    
    thread& thread::operator= (thread&& t) {
        if (&t != this) {
            if (joinable())
                throw_except<illegal_state_exception>("Move assignment is invalid! Target thread must not be joinable");
            
            m_thread    = std::move(t.m_thread);
            m_thread_id = t.m_thread_id;
            m_state     = t.get_state();
            set_name(t.get_name());
            m_runnable  = t.m_runnable;

        }
        return *this;
    }

    void thread::start() {
        m_state     = thread::state::RUNNABLE;
        m_thread    = std::thread(new_thread_start_func, this);
    }

    void new_thread_start_func(thread* _this) {
        try {
            _this->run();
        } catch (const throwable& e1) {
 #if defined(JSTD_ENABLE_CALLTRACE)
            static tc::mutex mutex;
            mutex.lock();
                system::tsprintf("Exception in thread\"%s\"\n", _this->get_name());
                e1.print_stack_trace();
            mutex.unlock();
#else
            system::tsprintf("Exception in thread \"%s\": %s\n", _this->get_name(), e1.cause());
#endif
        } catch (const std::exception& e2) {
            system::tsprintf("Exception in thread \"%s\": %s\n", _this->get_name(), e2.what());
        }
        
        _this->m_state = thread::state::TERMINATED;
        
    }

    void thread::run() {
        if (m_runnable != nullptr)
            m_runnable->run();
    }
    
    thread::~thread() {
        
    }

    const char* thread::get_name() const {
        return m_name;
    }

    void thread::set_name(const char* new_name) {
        ncopy(m_name, new_name, sizeof(m_name));
    }
    
    void thread::join() {
        m_thread.join();
    }

    bool thread::joinable() const {
        return m_thread.joinable();
    }

    thread::state thread::get_state() const {
        return m_state;
    }
}

//#################################################### ./src/wav_data.cpp ####################################################

#include <cpp/lang/utils/audio/wav_data.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/io/bytebuf.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/iostream.hpp>
#include <iostream>


#if 0
#define TC_MAKE_INT(buf) ((static_cast<long>(buf[0]) & 0xFF) | ((static_cast<long>(buf[1]) & 0xFF) << CHAR_BIT) | ((static_cast<long>(buf[2]) & 0xFF) << (CHAR_BIT * 2)) | ((static_cast<long>(buf[3]) & 0xFF) << (CHAR_BIT * 3)));
#define TC_MAKE_SHORT(buf) ((static_cast<short>(buf[0]) & 0xFF) | ((static_cast<short>(buf[1]) & 0xff) << CHAR_BIT));
#endif

#define TC_MAKE_INT(buf) static_cast<long>(\
    static_cast<unsigned long>(static_cast<unsigned char>(buf[0]) << (0 * CHAR_BIT)) |\
    static_cast<unsigned long>(static_cast<unsigned char>(buf[1]) << (1 * CHAR_BIT)) |\
    static_cast<unsigned long>(static_cast<unsigned char>(buf[2]) << (2 * CHAR_BIT)) |\
    static_cast<unsigned long>(static_cast<unsigned char>(buf[3]) << (3 * CHAR_BIT)) \
)\

#define TC_MAKE_SHORT(buf) static_cast<short>(\
    static_cast<unsigned short>(static_cast<unsigned char>(buf[0]) << (0 * CHAR_BIT)) |\
    static_cast<unsigned short>(static_cast<unsigned char>(buf[1]) << (1 * CHAR_BIT)) \
)\

    

namespace tc 
{

    wav_data::wav_data(tca::allocator* alloc) : m_allocator(alloc), data(nullptr) {

    }

    wav_data::wav_data(const wav_data& wav) : m_allocator(wav.m_allocator) {
        data = (char*) m_allocator->allocate( static_cast<std::size_t>(wav.subchunk2Size) );
        if (!data)
            throw_except<out_of_memory_error>("Out of memory");
        
        std::memcpy(data, wav.data, wav.subchunk2Size);
        
        chunk_size      = wav.chunk_size;
        subchunk1Size   = wav.subchunk1Size;
        sampleRate      = wav.sampleRate;
        byteRate        = wav.byteRate;
        subchunk2Size   = wav.subchunk2Size;
        block_align     = wav.block_align;
        audioFormat     = wav.audioFormat;
        numChannels     = wav.numChannels;
        bits_per_sample = wav.bits_per_sample;

        std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
        std::memcpy(format, wav.format, sizeof(format));
        std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
        std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
    }
    
    wav_data::wav_data(wav_data&& wav) : wav_data(wav.m_allocator) {
        std::swap(data, wav.data);
        std::swap(chunk_size, wav.chunk_size);
        std::swap(subchunk1Size, wav.subchunk1Size);
        std::swap(sampleRate, wav.sampleRate);
        std::swap(byteRate, wav.byteRate);
        std::swap(subchunk2Size, wav.subchunk2Size);
        std::swap(block_align, wav.block_align);
        std::swap(audioFormat, wav.audioFormat);
        std::swap(numChannels, wav.numChannels);
        std::swap(bits_per_sample, wav.bits_per_sample);
        std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
        std::memcpy(format, wav.format, sizeof(format));
        std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
        std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
    }
    
    wav_data& wav_data::operator=(const wav_data& wav) {
        if (&wav != this)
        {
            char* new_data = (char*) m_allocator->allocate( static_cast<std::size_t>(wav.subchunk2Size) );
            if (!new_data)
                throw_except<out_of_memory_error>("Out of memory");
                
            std::memcpy(new_data, wav.data, wav.subchunk2Size);
            
            if (data)
                m_allocator->deallocate(data);
        
            data            = new_data;
            chunk_size      = wav.chunk_size;
            subchunk1Size   = wav.subchunk1Size;
            sampleRate      = wav.sampleRate;
            byteRate        = wav.byteRate;
            subchunk2Size   = wav.subchunk2Size;
            block_align     = wav.block_align;
            audioFormat     = wav.audioFormat;
            numChannels     = wav.numChannels;
            bits_per_sample = wav.bits_per_sample;
    
            std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
            std::memcpy(format, wav.format, sizeof(format));
            std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
            std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
        }
        return *this;
    }
    
    wav_data& wav_data::operator=(wav_data&& wav) {
        if (&wav != this)
        {
            if (get_allocator() == wav.get_allocator())
            {
                std::swap(data, wav.data);
                std::swap(chunk_size, wav.chunk_size);
                std::swap(subchunk1Size, wav.subchunk1Size);
                std::swap(sampleRate, wav.sampleRate);
                std::swap(byteRate, wav.byteRate);
                std::swap(subchunk2Size, wav.subchunk2Size);
                std::swap(block_align, wav.block_align);
                std::swap(audioFormat, wav.audioFormat);
                std::swap(numChannels, wav.numChannels);
                std::swap(bits_per_sample, wav.bits_per_sample);
                std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
                std::memcpy(format, wav.format, sizeof(format));
                std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
                std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
            }
            else
            {
                *this = wav;
            }
        }
        return *this;
    }

    wav_data::wav_data(const file& path, tca::allocator* allocator) : m_allocator(allocator) {    
        if (!path.exists())
            throw_except<file_not_found_exception>("file not found!");
        ifstream in(path);
        load_from(&in);
    }

    wav_data::wav_data(istream* in, tca::allocator* allocator) : wav_data(allocator) {
        JSTD_DEBUG_CODE(
            if (in == nullptr)
                throw_except<illegal_argument_exception>("in must be != null");
        );
        load_from(in);
    }

    static void fill_buf_or_except(char buf[], std::size_t sz, /*non_null*/ istream* in) {
        std::size_t readed = in->read(buf, sz);
        if (readed < sz)
            throw_except<invalid_data_format_exception>("Invalid wav data!");
    }

    void wav_data::load_from(/*!non null!*/istream* in) {

        {//read "RIFF"
            fill_buf_or_except(chunk_id, sizeof(chunk_id), in);

            const char RIFF[] = "RIFF";
            for (std::size_t i = 0; i < sizeof(chunk_id); ++i)
                if (chunk_id[i] != RIFF[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'RIFF' wav");
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            chunk_size = TC_MAKE_INT(buf);
        }

        {//read "WAVE"
            fill_buf_or_except(format, sizeof(format), in);

            const char WAVE[] = "WAVE";
            for (std::size_t i = 0; i < sizeof(format); ++i)
                if (format[i] != WAVE[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'WAVE' wav");
        }
        
        {//read "fmt "
            
            fill_buf_or_except(subchunk1Id, sizeof(subchunk1Id), in);

            const char FMT[] = "fmt ";
            for (std::size_t i = 0; i < sizeof(subchunk1Id); ++i)
                if (subchunk1Id[i] != FMT[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'fmt ' wav");
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);        
            subchunk1Size = TC_MAKE_INT(buf);
        }
        
        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            audioFormat = TC_MAKE_SHORT(buf);
        }
        
        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            numChannels = TC_MAKE_SHORT(buf);
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            sampleRate = TC_MAKE_INT(buf);
        }
        
        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            byteRate = TC_MAKE_INT(buf);
        }

        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            block_align = TC_MAKE_SHORT(buf);
        }
        
        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            bits_per_sample = TC_MAKE_SHORT(buf);
        }

        {
            fill_buf_or_except(subchunk2Id, sizeof(subchunk2Id), in);
            const char data[] = "data";
            for (std::size_t i = 0; i < sizeof(subchunk2Id); ++i)
                if (subchunk2Id[i] != data[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'subchunk2Id' wav");
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            subchunk2Size = TC_MAKE_INT(buf);
        }

        {      
            data = (char*) m_allocator->allocate(static_cast<std::size_t>(subchunk2Size));
            if (!data)
            {
                throw_except<out_of_memory_error>("Out of memory");
            }

            try {
                fill_buf_or_except(data, static_cast<std::size_t>(subchunk2Size), in);
            } catch (...) {
                m_allocator->deallocate(data);
                throw;
            }
        }
    }

    wav_data::~wav_data() {
        if (m_allocator != nullptr && data != nullptr)
        {
            m_allocator->deallocate(data, subchunk2Size);
            data = nullptr;
        }
    }

    const char* wav_data::get_data() const {
        return data;
    }
    
    long wav_data::get_sample_rate() const {
        return sampleRate;
    }
    
    long wav_data::get_byte_rate() const {
        return byteRate;
    }
    
    long wav_data::get_length() const {
        return subchunk2Size;
    }
    
    short wav_data::get_num_channels() const {
        return numChannels;
    }

    short wav_data::get_bits_per_sample() const {
        return bits_per_sample;
    }
}

