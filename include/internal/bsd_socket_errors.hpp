#ifndef _JSTD_INTERNAL_BSD_SOCKET_ERRORS_H_
#define _JSTD_INTERNAL_BSD_SOCKET_ERRORS_H_
namespace tc {
namespace bsd_socket {

    /**
     * Возвращает строковое описание ошибки связанную с сокетом.
     * 
     * Linux:
     *  Используется просто strerror(erro) для описания последней ошибки.
     * 
     * Windows
     *  Используется WSAGetLastError() для описания последней ошибки.
     * 
     * @internal
     *  Реализация находится в файле bsd_socket.cpp
     * 
     */
    const char* socket_error_string();

}
}
#endif//_JSTD_INTERNAL_BSD_SOCKET_ERRORS_H_