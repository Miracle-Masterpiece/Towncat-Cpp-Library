#ifndef JSTD_INTERNAL_BSD_SOCKET_CLASS_H
#define JSTD_INTERNAL_BSD_SOCKET_CLASS_H

#include <cpp/lang/net/base_socket.hpp>
#include <cstdint>

namespace tc
{

namespace bsd_socket
{

/**
 * 
 */
struct sock_handle {
    
    /**
     * 
     */
    std::intptr_t handle;

    /**
     * 
     */
    sock_handle();
    
    /**
     * 
     */
    sock_handle(std::intptr_t handle);
    
    /**
     * 
     */
    sock_handle(sock_handle&&);
    
    /**
     * 
     */
    sock_handle& operator= (sock_handle&&);
    
    /**
     * 
     */
    ~sock_handle();

private:
    /**
     * 
     */
    void close();
    
    /**
     * 
     */
    sock_handle(const sock_handle&) = delete;
    
    /**
     * 
     */
    sock_handle& operator= (const sock_handle&) = delete;
};

/**
 * @note
 *      Деструктор не закрывает сокет.
 *      Для закрытия сокета, необходимо вызвать {@code socket_impl::close()}
 * 
 * @since 1.0
 */
class socket_impl : public base_socket<sock_handle> {
    /**
     * Является ли сокет неблокирующим.
     */
    bool _blocking;    
    
public:
    /**
     * Создаёт непривязанный сокет
     */
    socket_impl();
    
    /**
     * Перемещает данные из другого сокета в этот.
     */
    socket_impl(socket_impl&&);

    /**
     * Перемещает данные из одного сокета в этот.
     * Если этот сокет открыт, закрывает его.
     * 
     * @throws socket_exception
     *      Если при закрытие этого сокета произошла ошибка.
     * 
     * @return
     *      Этот сокет.
     */
    socket_impl& operator= (socket_impl&&);

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
     *      Если сокет не был открыт.
     *      Если сокет уже закрыт.
     */
    void set_socket_option(int opt_id, const socket_option& value) override;
    
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
     *      Если сокет не был открыт.
     *      Если сокет уже закрыт.
     */
    socket_option get_socket_option(int opt_id) const override;
    
    /**
     * @throws connect_exception 
     *      Если произошла ошибка при подключении.
     */
    void connect(const char* host, unsigned int port) override;
    
    /**
     * @throws connect_exception 
     *      Если произошла ошибка при подключении.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается
     */
    void connect(const inet_address& host, unsigned int port) override;
    
    /**
     * @throws bind_exception 
     *      Если произошла ошибка при привязке сокета.
     * 
     * @throws illegal_argument_exception
     *      Если тип семейства адреса не поддерживается.
     */
    void bind(const inet_address& address, unsigned int port) override;
    
    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установки размера максимальной очереди.
     */
    void listen(int backlog) override;
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка сокета.
     * 
     * @throws unsupported_operation_exception
     *      Если имя сокета не поддерживает семейство адресов отличное от IPv4 и IPv6
     */
    bool accept(base_socket<sock_handle>* client) override;
    
    /**
     * Создаёт дескриптор сокета для указанного семейства адресов.
     */
    void create(inet_family family) override;
    
    /**
     * Сознал ли дескриптор сокета.
     */
    bool is_created() const override;

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при закрытии сокета.
     */
    void close() override;
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении чтения.
     */
    void shutdown_in() override;
    
    /**
     * @throws socket_exception
     *      Если произошла ошибка при заглушении записи.
     */
    void shutdown_out() override;

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
    std::size_t read(char buf[], std::size_t len) override;
    
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
    std::size_t write(const char* data, std::size_t len) override;

    /**
     * @note
     *      Деструктор не закрывает сокет.
     *      Для закрытия сокета, необходимо вызвать {@code socket_impl::close()}
     */
    ~socket_impl();

    /**
     * @throws socket_exception 
     *      Если произошла ошибка при установке блокирующего/неблокирущего режима.
     */
    void set_blocking(bool non_block) override;
    
    /**
     * Является ли сокет блокирующим.
     * @return true - если блокирующий, false - неблокирующий.
     */
    bool is_blocking() const override;
};

}//bsd_socket
}//jstd
#endif// JSTD_INTERNAL_BSD_SOCKET_CLASS_H