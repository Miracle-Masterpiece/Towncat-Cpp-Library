#ifndef JSTD_CPP_LANG_NET_SOCKET_H
#define JSTD_CPP_LANG_NET_SOCKET_H

#include <cpp/lang/net/socket_option.hpp>
#include <cpp/lang/net/inetaddr.hpp>
#include <internal/bsd_socket_class.hpp>
#include <cpp/lang/io/bytebuffer.hpp>
#include <cstddef>

namespace tc {

/**
 * Класс, реализующий сокетное соединение.
 * 
 * @note
 *      Для закрытия сокета, необходимо вызвать функция {@code tsocket<T>::close()}
 *      Деструктор не закрывает соединение!
 * 
 * @since 1.0
 */
template<typename SOCK_T>
class tsocket {
    /**
     * 
     */
    static const int DEFAULT_BACKLOG = 16;

    /**
     * 
     */
    static const int IS_CREATED         = 1 << 0;
    
    /**
     * 
     */
    static const int IS_BINDED          = 1 << 1;
    
    /**
     * 
     */
    static const int IS_CONNECTED       = 1 << 2;
    
    /**
     * 
     */
    static const int IS_NONBLOCKING     = 1 << 3;
    
    /**
     * 
     */
    static const int IS_CLOSED          = 1 << 4;
    
    /**
     * 
     */
    static const int IS_SHUTDOWN_IN     = 1 << 5;
    
    /**
     * 
     */
    static const int IS_SHUTDOWN_OUT    = 1 << 6;
    
    /**
     * 
     */
    static const int IS_LISTENING       = 1 << 7;
    
    /**
     * Cокет, реализующий функции
     */
    SOCK_T _impl;
    
    /**
     * Конфигурация сокета
     */
    unsigned int _configure;

    /**
     * 
     */
    tsocket(const tsocket<SOCK_T>&) = delete;
    
    /**
     * 
     */
    tsocket<SOCK_T>& operator= (const tsocket<SOCK_T>&) = delete;

    /**
     * 
     */
    void set_config(int conf, bool value);
    
    /**
     * 
     */
    bool get_config(int conf) const; 
    
    /**
     * 
     */
    void create(inet_family family);
    
    /**
     * 
     */
    bool is_created();
    
    inline void check_state_or_except() const {
        if (get_config(IS_CLOSED))
            throw_except<illegal_state_exception>("Socket is closed");
        if (!get_config(IS_CREATED))
            throw_except<illegal_state_exception>("Socket not created");
    }
public:
    /**
     * nullsocket
     */
    tsocket();
    
    /**
     * @brief Created server-socket
     * 
     * @throws illegal_state_exception
     *      Если тип семейства адреса не поддерживается.
     * 
     * @throws bind_exception
     *      Если произошла ошибка сокета.
     */
    tsocket(unsigned int port, int backlog = DEFAULT_BACKLOG);

    /**
     * @brief Created client-socket
     * 
     * @throws unknow_host_exception
     *      Если указанный хост не известен.
     * 
     * @throws illegal_state_exception
     *      Если произошла ошибка при получении адреса по указанному доменному имени.
     * 
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     */
    tsocket(const char* domain, unsigned int port);

    /**
     * @brief Created client-socket
     * 
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     */
    tsocket(const socket_address& addr);

    /**
     * @brief Created client-socket
     * 
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     */
    tsocket(const inet_address& addr, unsigned int port);

    /**
     * Возвращает ссылку на объект, реализующий функции сокета.
     */
    SOCK_T& get_implsock();

    /**
     * @throws illegal_state_exception
     *      Если сокет уже подключен.
     *      Если сокет уже закрыт.
     *      Если сокет уже привязан.
     * 
     * @throws connect_exception
     *      Если произошла ошибка при подключении сокета.
     */
    void connect(const socket_address& address);
    
    /**
     * @throws illegal_state_exception
     *      Если сокет уже подключен.
     *      Если сокет уже закрыт.
     *      Если сокет уже привязан.
     * 
     * @throws connect_exception
     *      Если произошла ошибка при подключении сокета.
     */
    void connect(const inet_address& address, unsigned int port);

    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет уже подключен.
     *      Если сокет уже привязан.
     *      Если тип семейства адреса не поддерживается.
     * 
     * @throws bind_exception
     *      Если произошла ошибка сокета.
     */
    void bind(unsigned int port);
    
    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет уже подключен.
     *      Если сокет уже привязан.
     *      Если тип семейства адреса не поддерживается.
     * 
     * @throws bind_exception
     *      Если произошла ошибка сокета.
     */
    void bind(const inet_address& address, unsigned int port);
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     * 
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет уже подключен.
     *      Если сокет не привязан.
     */
    void listen(int backlog = DEFAULT_BACKLOG);
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при закрытии сокета.
     * 
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     */
    void close();
    
    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет не подключен.
     * 
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
     */
    std::size_t write(const char* data, std::size_t size);
    
    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет не подключен.
     * 
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
     */
    std::size_t write(byte_buffer& buff);

    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет не подключен.
     * 
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
     */
    std::size_t read(char buf[], std::size_t size);
    
    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет не подключен.
     * 
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
     */
    std::size_t read(byte_buffer& buff);

    /**
     * @throws illegal_state_exception
     *      Если сокет уже закрыт.
     *      Если сокет не привязан.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при приняти клиента.
     */
    bool accept(tsocket<SOCK_T>* client);

    /**
     * @internal TCP_NODELAY
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_tcp_no_delay(bool on_off);
    
    /**
     * @internal TCP_NODELAY
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    bool get_tcp_no_delay() const;

    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_blocking(bool block);
    
    /**
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    bool is_blocking() const;

    /**
     * @internal IP_TOS
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_tos(int value);
    
    /**
     * @internal IP_TOS
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    int get_tos() const;

    /**
     * @internal SO_SNDBUF
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     *      Если size меньше нуля. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_send_buf_size(std::size_t size);

    /**
     * @internal SO_SNDBUF
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    std::size_t get_send_buf_size() const;

    /**
     * @internal SO_RCVBUF
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     *      Если size меньше нуля. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_receive_buf(std::size_t size);
    
    /**
     * @internal SO_RCVBUF
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    std::size_t get_receive_buf() const;

    /**
     * @internal SO_LINGER
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_so_linger(timepoint ms);
    
    /**
     * @internal SO_LINGER
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым. 
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    timepoint get_so_linger() const;

    /**
     * @internal SO_REUSEADDR
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_reuse_address(bool reuse);
    
    /**
     * @internal SO_REUSEADDR
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    bool get_reuse_address() const;

    /**
     * @internal SO_KEEPALIVE
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_keep_alive(bool on_off, timepoint first = 60, timepoint interval = 60, timepoint try_count = 4);
    
    /**
     * @internal SO_KEEPALIVE
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    bool get_keep_alive() const;

    /**
     * @internal SO_SNDTIMEO
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_so_send_timeout(timepoint ms);
    
    /**
     * @internal SO_SNDTIMEO
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    timepoint get_so_send_timeout() const;

    /**
     * @internal SO_RCVTIMEO
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_so_receive_timeout(timepoint ms);
    
    /**
     * @internal SO_RCVTIMEO
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    timepoint get_so_receive_timeout() const;

    /**
     * @internal SO_OOBINLINE
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws socket_exception 
     *      Если произошла ошибка при установке опции сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void set_oob_inline(bool on_off);
    
    /**
     * @internal SO_OBBINLINE
     * 
     * @throws illegal_argument_exception
     *      Если переданное значение не является допустимым.
     * 
     * @throws unsupported_operation_exception
     *      Если опция сокета не поддерживается.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    bool get_oob_inline() const;

    /**
     * 
     * @throw socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void shutdown_input();
    
    /**
     * @return
     *   true - если чтение из сокета было заглушено, иначе false.
     */
    bool is_input_shutdown() const;

    /**
     * 
     * @throw socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws illegal_state_exception
     *      Если сокет не был создан. (Созданный сокет - это сокет, который либо привязан, либо подключен к другому сокету.)
     *      Если сокет уже закрыт.
     */
    void shutdown_output();
    
    /**
     * @return
     *   true - если запись в сокет была заглушена, иначе false.
     */
    bool is_output_shutdown() const;

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
    void set_option(int optID, const socket_option& opt);
    
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
    socket_option get_option(int optID) const;

    /**
     * Привязан ли сокет.
     */
    bool is_bound() const;
    
    /**
     * Подключен ли сокет.
     */
    bool is_connected() const;
    
    /**
     * Закрыт ли сокет.
     */
    bool is_closed() const;

    /**
     * Возвращает удалённый порт сокета, который подключен к этому сокету.
     */
    unsigned int get_port() const;
    
    /**
     * Возвращает локальный порт этого сокета.
     */
    unsigned int get_localport() const;
    
    /**
     * Возвращает адрес сокета, к которому подключен этот сокет.
     */
    const inet_address& get_address() const;
};

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_option(int optID, const socket_option& opt) {
        check_state_or_except();
        _impl.set_socket_option(optID, opt);
    }
    
    template<typename SOCK_T>
    socket_option tsocket<SOCK_T>::get_option(int optID) const {
        check_state_or_except();
        return _impl.get_socket_option(optID);
    }

    template<typename SOCK_T>
    unsigned int tsocket<SOCK_T>::get_port() const {
        return _impl.get_port();
    }
    
    template<typename SOCK_T>
    unsigned int tsocket<SOCK_T>::get_localport() const {
        return _impl.get_localport();
    }
    
    template<typename SOCK_T>
    const inet_address& tsocket<SOCK_T>::get_address() const {
        return _impl.get_address();
    }

    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_bound() const {
        return get_config(IS_BINDED);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_connected() const {
        return get_config(IS_CONNECTED);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_closed() const {
        return get_config(IS_CLOSED);
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::shutdown_output() {
        check_state_or_except();
        if (get_config(IS_SHUTDOWN_OUT))
            throw_except<illegal_state_exception>("Socket already output shutdown");
        _impl.shutdown_out();
        set_config(IS_SHUTDOWN_OUT, true);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_output_shutdown() const {
        return get_config(IS_SHUTDOWN_OUT);
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::shutdown_input() {
        check_state_or_except();
        if (get_config(IS_SHUTDOWN_IN))
            throw_except<illegal_state_exception>("Socket already input shutdown");
        _impl.shutdown_in();
        set_config(IS_SHUTDOWN_IN, true);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_input_shutdown() const {
        return get_config(IS_SHUTDOWN_IN);
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_oob_inline(bool on_off) {
        check_state_or_except();
        socket_option opt;
        opt.int_value = on_off;
        _impl.set_socket_option(socket_option::bsd::SO_OOBINLINE_, opt);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::get_oob_inline() const {
        check_state_or_except();
        return _impl.get_socket_option().int_value;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_so_send_timeout(timepoint ms) {
        check_state_or_except();
        socket_option opt;
        opt.timeout.millis = ms;
        _impl.set_socket_option(socket_option::bsd::SO_SNDTIMEO_, opt);
    }
    
    template<typename SOCK_T>
    timepoint tsocket<SOCK_T>::get_so_send_timeout() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_SNDTIMEO_).timeout.millis;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_so_receive_timeout(timepoint ms) {
        check_state_or_except();
        socket_option opt;
        opt.timeout.millis = ms;
        _impl.set_socket_option(socket_option::bsd::SO_RCVTIMEO_, opt);
    }
    
    template<typename SOCK_T>
    timepoint tsocket<SOCK_T>::get_so_receive_timeout() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_RCVTIMEO_).timeout.millis;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_keep_alive(bool on_off, timepoint first /* = 60*/, timepoint interval /* = 60*/, timepoint try_count /* = 4*/) {
        check_state_or_except();
        socket_option opt;
        if (on_off)
        {
            opt.keepalive.on_off        = 1;
            opt.keepalive.time_to_first = first;
            opt.keepalive.time_interval = interval;
            opt.keepalive.try_count     = try_count;
        }
        else
        {
            opt.keepalive.on_off        = 0;
            opt.keepalive.time_to_first = 0;
            opt.keepalive.time_interval = 0;
            opt.keepalive.try_count     = 0;
        }
        _impl.set_socket_option(socket_option::bsd::SO_KEEPALIVE_, opt);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::get_keep_alive() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_KEEPALIVE_).keepalive.on_off;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_reuse_address(bool reuse) {
        check_state_or_except();
        socket_option opt;
        opt.int_value = reuse ? 1 : 0;
        _impl.set_socket_option(socket_option::bsd::SO_REUSEADDR_, opt);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::get_reuse_address() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_REUSEADDR_).int_value != 0;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_so_linger(timepoint ms) {
        check_state_or_except();
        socket_option opt;
        if (ms == 0) {
            opt.linger.on_off   = 0;
            opt.linger.sec_time = 0;
        } else {
            opt.linger.on_off   = 1;
            opt.linger.sec_time = ms / 1000;
        }
        _impl.set_socket_option(socket_option::bsd::SO_LINGER_, opt);
    }
    
    template<typename SOCK_T>
    timepoint tsocket<SOCK_T>::get_so_linger() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_LINGER_).linger.sec_time * 1000;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_receive_buf(std::size_t size) {
        check_state_or_except();
        if (size < 0)
            throw_except<illegal_argument_exception>("receive buffer size can't be negative: %li", (long int) size);
        socket_option opt;
        opt.int_value = size;
        _impl.set_socket_option(socket_option::bsd::SO_RCVBUF_, opt);
    }

    template<typename SOCK_T>
    std::size_t tsocket<SOCK_T>::get_receive_buf() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_RCVBUF_).int_value;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_send_buf_size(std::size_t size) {
        check_state_or_except();
        if (size < 0)
            throw_except<illegal_argument_exception>("send buffer size can't be negative: %li", (long int) size);
        socket_option opt;
        opt.int_value = size;
        _impl.set_socket_option(socket_option::bsd::SO_SNDBUF_, opt);
    }
    
    template<typename SOCK_T>
    std::size_t tsocket<SOCK_T>::get_send_buf_size() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::SO_SNDBUF_).int_value;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_config(int conf, bool value) {
        if (value)
            _configure |= conf;
        else 
            _configure &= ~conf;
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::get_config(int conf) const {
        return _configure & conf;
    }

    template<typename SOCK_T>
    tsocket<SOCK_T>::tsocket() : _impl(), _configure(0) {

    }

    template<typename SOCK_T>
    tsocket<SOCK_T>::tsocket(unsigned int port, int backlog) : _impl(), _configure(0) {
        bind(inet_address(inet_family::IPV4), port);
        listen(backlog);
    }

    template<typename SOCK_T>
    tsocket<SOCK_T>::tsocket(const char* domain, unsigned int port) : _configure(0) {
        optional<inet_address> address = inet_address::get_by_name(domain);
        if (address.is_value())
        {
            connect(address.get_value(), port);
        }
        else
        {
            throw_except<unknow_host_exception>("Host \'%s\' not exists)", domain);
        }
    }
        
    template<typename SOCK_T>
    tsocket<SOCK_T>::tsocket(const socket_address& addr) : tsocket<SOCK_T>(addr.get_address(), addr.get_port()) {
        
    }

    template<typename SOCK_T>
    tsocket<SOCK_T>::tsocket(const inet_address& addr, unsigned int port) : _impl(), _configure(0) {
        connect(addr, port);
    }

    template<typename SOCK_T>
    SOCK_T& tsocket<SOCK_T>::get_implsock() {
        return _impl;
    }
    
    template<typename SOCK_T>
    void tsocket<SOCK_T>::create(inet_family family) {
        _impl.create(family);
        set_config(IS_CREATED, true);
    }

    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_created() {
        return get_config(IS_CREATED);
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::connect(const socket_address& address) {
        connect(address.get_address(), address.get_port());
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::connect(const inet_address& address, unsigned int port) {
        
        if (get_config(IS_CLOSED))    throw_except<illegal_state_exception>("Socket is closed");
        if (get_config(IS_CONNECTED)) throw_except<illegal_state_exception>("Socket already connected");
        if (get_config(IS_BINDED))    throw_except<illegal_state_exception>("Server socket can't connected");
        
        if (!is_created())
            create(address.get_family());
        
        _impl.connect(address, port);
        set_config(IS_CONNECTED, true);
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::bind(unsigned int port) {
        bind(inet_address(inet_family::IPV4), port);
    }
    
    template<typename SOCK_T>
    void tsocket<SOCK_T>::bind(const inet_address& address, unsigned int port) {
        
        if (get_config(IS_CLOSED))    throw_except<illegal_state_exception>("Socket is closed");
        if (get_config(IS_BINDED))    throw_except<illegal_state_exception>("Socket already bound");
        if (get_config(IS_CONNECTED)) throw_except<illegal_state_exception>("Client socket can't bind");
        
        if (!is_created())
            create(address.get_family());
        _impl.bind(address, port);
        set_config(IS_BINDED, true);

    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::listen(int32_t backlog) {
        
        if (get_config(IS_CLOSED))    throw_except<illegal_state_exception>("Socket is closed");
        if (get_config(IS_CONNECTED)) throw_except<illegal_state_exception>("Client socket can't listening");
        if (!get_config(IS_BINDED))   throw_except<illegal_state_exception>("Socket is not bound");
        
        _impl.listen(backlog);
        set_config(IS_LISTENING, true);

    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::close() {
        if (get_config(IS_CLOSED))
            throw_except<illegal_state_exception>("Socket already closed");
        
        _impl.close();
        set_config(IS_CLOSED,       true);
        set_config(IS_CONNECTED,    false);
        set_config(IS_BINDED,       false);
    }

    template<typename SOCK_T>
    std::size_t tsocket<SOCK_T>::write(const char* data, std::size_t size) {
        if (get_config(IS_CLOSED))
            throw_except<illegal_state_exception>("Socket is closed");
        if (!get_config(IS_CONNECTED))
            throw_except<illegal_state_exception>("Socket not connected");
        if (get_config(IS_SHUTDOWN_OUT))
            throw_except<illegal_state_exception>("Socket output shutdown");
        return _impl.write(data, size);
    }
    
    template<typename SOCK_T>
    std::size_t tsocket<SOCK_T>::write(byte_buffer& buff) {
        std::size_t rem = buff.remaining();
        if (rem > 0)
        {
            std::size_t writed = write(buff.data() + buff.position(), buff.remaining());
            if (writed > 0)
            {
                buff.position(buff.position() + writed);
            }
            return writed;
        }
        return 0;
    }

    template<typename SOCK_T>
    std::size_t tsocket<SOCK_T>::read(char buf[], std::size_t size) {
        
        if (get_config(IS_CLOSED))      throw_except<illegal_state_exception>("Socket is closed");
        if (!get_config(IS_CONNECTED))  throw_except<illegal_state_exception>("Socket not connected");
        if (get_config(IS_SHUTDOWN_IN)) throw_except<illegal_state_exception>("Socket input shutdown");
        
        return _impl.read(buf, size);
    }

    template<typename SOCK_T>
    std::size_t tsocket<SOCK_T>::read(byte_buffer& buff) {
        std::size_t rem = buff.remaining();
        if (rem > 0)
        {
            std::size_t readed = read(buff.data(), rem);
            if (readed > 0)
            {
                buff.position(buff.position() + readed);
            }
            return readed;
        }
        return 0;
    }

    template<typename SOCK_T>
    bool tsocket<SOCK_T>::accept(tsocket<SOCK_T>* client) {
        JSTD_DEBUG_CODE(
            if (client == nullptr)
                throw_except<null_pointer_exception>("client is null");
        );
        
        if (get_config(IS_CLOSED))     throw_except<illegal_state_exception>("Socket is closed");
        if (!get_config(IS_BINDED))    throw_except<illegal_state_exception>("Socket is not bound");
        if (!get_config(IS_LISTENING)) throw_except<illegal_state_exception>("Socket is not listening");
        
        SOCK_T client_impl;
        bool accepted = _impl.accept(&client_impl);
        if (accepted)
        {
            client->_impl = std::move(client_impl);
            client->set_config(IS_CREATED,      true);
            client->set_config(IS_CONNECTED,    true);
            return true;
        }
        
        return false;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_tcp_no_delay(bool on_off) {
        check_state_or_except();
        socket_option opt;
        opt.int_value = on_off;
        _impl.set_socket_option(socket_option::bsd::TCP_NODELAY_, opt);
    }
    
    template<typename SOCK_T>
    bool tsocket<SOCK_T>::get_tcp_no_delay() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::TCP_NODELAY_).int_value != 0;
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_blocking(bool block) {
        check_state_or_except();
        _impl.set_blocking(block);
    }

    template<typename SOCK_T>
    bool tsocket<SOCK_T>::is_blocking() const {
        check_state_or_except();
        return _impl.is_blocking();
    }

    template<typename SOCK_T>
    void tsocket<SOCK_T>::set_tos(int value) {
        check_state_or_except();
        socket_option opt;
        opt.int_value = value;
        _impl.set_socket_option(socket_option::bsd::IP_TOS_, opt);
    }
    
    template<typename SOCK_T>
    int tsocket<SOCK_T>::get_tos() const {
        check_state_or_except();
        return _impl.get_socket_option(socket_option::bsd::IP_TOS_).int_value;
    }

typedef tsocket<bsd_socket::socket_impl> socket;

}
#endif//JSTD_CPP_LANG_NET_SOCKET_H