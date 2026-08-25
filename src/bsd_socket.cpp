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