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