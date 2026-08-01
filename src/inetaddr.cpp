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
#include <allocators/ArrayList.h>
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
            return objects::hashcode<char>(reinterpret_cast<const char*>(&IPv4), sizeof(IPv4));
        } 
        else if (family == inet_family::IPV6)
        {
            return objects::hashcode<char>(reinterpret_cast<const char*>(&IPv6), sizeof(IPv6));
        }
        return 0;
    }
    
    bool inet_address::equals(const inet_address& addr) const {
        if (&addr == this)
            return true;
        
        if (family == inet_family::IPV4 && addr.family == inet_family::IPV4)
        {
            return 
                    objects::equals<char>(reinterpret_cast<const char*>(&IPv4), reinterpret_cast<const char*>(&addr.IPv4), sizeof(IPv4)) 
                    &&
                    family == addr.family;
        } 
        
        else if (family == inet_family::IPV6 && addr.family == inet_family::IPV6)
        {
            return 
                    objects::equals<char>(reinterpret_cast<const char*>(&IPv6), reinterpret_cast<const char*>(&addr.IPv6), sizeof(IPv6))
                    &&
                    family == addr.family;
        }

        return false;
    }

    int inet_address::to_string(char buf[], std::size_t bufsize) const {
        if (family == inet_family::IPV4)
            return IPv4ToString(buf, bufsize, *this);
        else
            return IPv6ToString(buf, bufsize, *this);
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
        int errcode   = getaddrinfo(domain, null, &addrInf, &begin);
        
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

        return tc::string( std::move(result) );
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

    int socket_address::to_string(char buf[], std::size_t bufsize) {
        int off = m_address.to_string(buf, bufsize);
        if (off > 0)
        {
            std::size_t offset = (std::size_t) off;
            if (bufsize >= offset && bufsize - offset > 0)
                off += snprintf(buf + offset, bufsize - offset, "/%i", m_port);
        }
        return off;
    }
    
}