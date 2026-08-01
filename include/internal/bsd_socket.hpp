#ifndef JSTD_INTERNAL_BSD_SOCKET_FUNCS_H
#define JSTD_INTERNAL_BSD_SOCKET_FUNCS_H

#include <cstdint>
#include <cpp/lang/net/inetaddr.hpp>
#include <cpp/lang/net/socket_option.hpp>

#if defined(__linux__) || defined(__APPLE__)
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    using SOCK_TYPE             = int;
    const SOCK_TYPE NULL_SOCKET = -1;
#elif _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    using SOCK_TYPE             = SOCKET;
    const SOCK_TYPE NULL_SOCKET = INVALID_SOCKET;
#else 
    #error Unsupported platform!
#endif

namespace tc
{

namespace bsd_socket
{
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при открытие сокета.
     */
    SOCK_TYPE open_tcp(inet_family family);
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при закрытии сокета.
     */
    void close(SOCK_TYPE sock);

    /**
     * @throws bind_exception 
     *      Если произошла ошибка при привязке сокета.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается.
     */
    void bind(SOCK_TYPE sock, const inet_address& address, unsigned int port);

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     */
    void backlog(SOCK_TYPE sock, int maxq);

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     */
    inline void listen(SOCK_TYPE sock , int maxq) {
        return backlog(sock, maxq);
    }

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при приняти клиента.
     */
    SOCK_TYPE accept(SOCK_TYPE serv_sock, socket_address* client_addr);

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установке блокирующего/неблокирущего режима.
     */
    void set_blocking(SOCK_TYPE sock, bool block_mode);

    /**
     * @throws connect_exception 
     *      Если произошла ошибка при подключении.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается.
     */
    void connect(SOCK_TYPE client_sock, const inet_address& address, unsigned int port);

    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     */
    void set_sock_opt(SOCK_TYPE sock, int opt, const socket_option& value);

    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     */
    void get_sock_opt(SOCK_TYPE sock, int opt, socket_option& value);

    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода-вывода.
     * 
     * @throws socket_timeout_exception
     *      Если произошёл таймаут.
     * 
     * @throw connect_exception
     *      Если соединение было разорвано.
     * 
     * @return
     *      Сколько фактически отправилось байт.     
     */
    std::size_t send(SOCK_TYPE sock, const char* data, std::size_t length, bool is_blocking);

    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода-вывода.
     * 
     * @throws socket_timeout_exception
     *      Если произошёл таймаут.
     * 
     * @throw connect_exception
     *      Если соединение было разорвано.
     * 
     * @return
     *      Сколько фактически прочиталось байт.
     */
    std::size_t recv(SOCK_TYPE sock, char* data, std::size_t length, bool is_blocking);

    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если имя сокета не поддерживает семейство адресов отличное от IPv4 и IPv6
     */
    void get_sock_name(SOCK_TYPE sock, socket_address& address);

    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении чтения.
     */
    void shutdown_in(SOCK_TYPE sock);
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении записи.
     */
    void shutdown_out(SOCK_TYPE sock);

}// namespace bsd_socket

}// namespace jstd

#endif//_JSTD_INTERNAL_BSD_SOCKET_FUNCS_H_