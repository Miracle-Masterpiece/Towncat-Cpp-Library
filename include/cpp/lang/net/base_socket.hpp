#ifndef JSTD_CPP_LANG_BASE_SOCKET_H
#define JSTD_CPP_LANG_BASE_SOCKET_H
#include <cpp/lang/net/inetaddr.hpp>
#include <cpp/lang/net/socket_option.hpp>
#include <cpp/lang/string.hpp>
#include <cstdint>

namespace tc
{

/**
 * Абстрактный базовый класс для работы с сокетами.
 * 
 * @tparam 
 *      SOCKET_T Тип дескриптора сокета.
 * 
 * Этот класс предоставляет интерфейс для работы с сокетами, включая подключение,
 * приём и отправку данных, управление параметрами сокета и обработку соединений.
 * Класс не поддерживает копирование, но допускает перемещение.
 */
template<typename SOCKET_T>
class base_socket {
    base_socket(const base_socket&) = delete;
    base_socket& operator= (const base_socket&) = delete;
protected:
    //Значение для нулевого порта.    
    static const int NULL_PORT = 0;

    //Дескриптор сокета.
    SOCKET_T sock_id;         
    
    // Адрес сокета.
    inet_address address;    
    
    // Порт сокета.
    unsigned int port; 
    
    // Локальный порт сокета.
    unsigned int localport;

    /**
     * Конструктор по умолчанию.
     */
    base_socket();

    /**
     * Конструктор перемещения.
     * 
     * @param other 
     *      Перемещаемый объект.
     */
    base_socket(base_socket&& other);

    /**
     * 
     *      Оператор присваивания (перемещение).
     * 
     * @param other 
     *      Перемещаемый объект.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_socket& operator= (base_socket&& other);

    /**
     * Виртуальный деструктор, обязательный для абстрактного класса.
     */
    virtual ~base_socket() = 0;

    /**
     * Устанавливает параметр сокета.
     * 
     * @param opt_id 
     *      Идентификатор параметра.
     * 
     * @param value 
     *      Значение параметра.
     */
    virtual void set_socket_option(int opt_id, const socket_option& value) = 0;

    /**
     * Получает параметр сокета.
     * 
     * @param opt_id 
     *      Идентификатор параметра.
     * 
     * @return 
     *      Значение параметра.
     */
    virtual socket_option get_socket_option(int opt_id) const = 0;

    /**
     * Подключается к удалённому хосту.
     * 
     * @param host 
     *      Имя хоста или IP-адрес.
     * 
     * @param port 
     *      Порт хоста.
     * 
     * @throws io_exception
     *      Если произошла ошибка при подключении.
     */
    virtual void connect(const char* host, unsigned int port) = 0;

    /**
     * Подключается к удалённому хосту по объекту inet_address.
     * 
     * @param host 
     *      Адрес хоста.
     * @param port 
     *      Порт хоста.
     * 
     * @throws io_exception
     *      Если произошла ошибка при подключении.
     */
    virtual void connect(const inet_address& host, unsigned int port) = 0;

    /**
     * Привязывает сокет к указанному адресу и порту.
     * 
     * @param address 
     *      Адрес, к которому будет привязан сокет.
     * 
     * @param port 
     *      Порт для привязки.
     * 
     * @throws io_exception
     *      Если произошла ошибка при связывании сокета.
     */
    virtual void bind(const inet_address& address, unsigned int port) = 0;

    /**
     * Переводит сокет в режим прослушивания входящих подключений.
     * 
     * @param backlog 
     *      Размер очереди ожидающих подключений.
     * 
     * @throws io_exception
     *      Если произошла ошибка при изменении очереди ожидающих подключения.
     */
    virtual void listen(int backlog) = 0;

    /**
     * Принимает входящее соединение.
     * 
     * @param client 
     *      Указатель на новый клиентский сокет.
     * 
     * @return 
     *      true, если подключение успешно принято, иначе false.
     * 
     * @throws io_exception
     *      Если произошла ошибка при принятии сокета.
     */
    virtual bool accept(base_socket<SOCKET_T>* client) = 0;

    /**
     * Закрывает сокет.
     * 
     * @throws io_exception
     *      Если произошла ошибка при закрытии сокета.
     */
    virtual void close() = 0;

    /**
     * Останавливает приём данных на сокет.
     * 
     * @throws io_exception
     *      Если произошла ошибка при остановке приёма данных.
     */
    virtual void shutdown_in() = 0;

    /**
     * Останавливает отправку данных через сокет.
     * 
     * @throws io_exception
     *      Если произошла ошибка при остановке отправки данных.
     */
    virtual void shutdown_out() = 0;

    /**
     * Преобразует информацию о сокете в строку.
     */
    string to_string(tca::allocator* alloc = tca::get_default_allocator()) const;

    /**
     * Читает данные из сокета.
     * 
     * @param buf 
     *      Буфер для хранения прочитанных данных.
     * 
     * @param len 
     *      Максимальный размер данных для чтения.
     * 
     * @return 
     *      Количество прочитанных байт.
     * 
     * @throws io_exception
     *      Если произошла ошибка при чтении данных.
     */
    virtual std::size_t read(char buf[], std::size_t len) = 0;

    /**
     * Отправляет данные через сокет.
     * 
     * @param data 
     *      Указатель на отправляемые данные.
     * 
     * @param len 
     *      Размер отправляемых данных в байтах.
     * 
     * @return 
     *      Количество отправленных байт.
     * 
     * @throws io_exception
     *      Если произошла ошибка при отправке данных.
     */
    virtual std::size_t write(const char* data, std::size_t len) = 0;

    /**
     * Устанавливает режим блокировки сокета.
     * 
     * @param block 
     *      true — блокирующий режим, false — неблокирующий.
     * 
     * @throws io_exception
     *      Если произошла ошибка при установке режима сокета.
     */
    virtual void set_blocking(bool block) = 0;

    /**
     * Проверяет, является ли сокет блокирующим.
     * 
     * @return 
     *      true, если сокет работает в блокирующем режиме, иначе false.
     */
    virtual bool is_blocking() const = 0;

public:
    /**
     * Создаёт сокет указанного семейства.
     * 
     * @param family 
     *      Семейство адресов (например, AF_INET или AF_INET6).
     * 
     * @throws io_exception
     *      Если произошла ошибка при создании сокета.
     */
    virtual void create(inet_family family) = 0;

    /**
     * Проверяет, был ли сокет создан.
     * 
     * @return 
     *      true, если сокет создан, иначе false.
     */
    virtual bool is_created() const = 0;

    /**
     * Получает дескриптор сокета.
     * 
     * @return 
     *      Ссылка на дескриптор сокета.
     */
    SOCKET_T& get_descriptor();

    /**
     * Получает дескриптор сокета (константная версия).
     * 
     * @return 
     *      Константная ссылка на дескриптор сокета.
     */
    const SOCKET_T& get_descriptor() const;

    /**
     * Получает адрес сокета.
     * 
     * @return 
     *      Ссылка на объект inet_address.
     */
    inet_address& get_address();

    /**
     * Получает адрес сокета (константная версия).
     * 
     * @return 
     *      Константная ссылка на объект inet_address.
     */
    const inet_address& get_address() const;

    /**
     * Получает порт сокета.
     */
    unsigned int get_port() const;

    /**
     * Получает локальный порт сокета.
     */
    unsigned int get_localport() const;

    /**
     * Устанавливает адрес сокета.
     */
    void set_address(const inet_address& addr);

    /**
     * Устанавливает порт сокета.
     */
    void set_port(unsigned int port);

    /**
     * Устанавливает локальный порт сокета.
     */
    void set_localport(unsigned int localport);

    /**
     * Устанавливает дескриптор сокета.
     */
    void set_descriptor(SOCKET_T&& sID);
};


    template<typename SOCKET_T>
    base_socket<SOCKET_T>::base_socket() : sock_id(), address(), port(0), localport(0) {

    }

    template<typename SOCKET_T>
    base_socket<SOCKET_T>::base_socket(base_socket<SOCKET_T>&& sock) : 
    sock_id(std::move(sock.sock_id)), 
    address(std::move(sock.address)), 
    port(sock.port), 
    localport(sock.localport) {
        sock.port      = NULL_PORT;
        sock.localport = NULL_PORT;
    }

    template<typename SOCKET_T>
    base_socket<SOCKET_T>& base_socket<SOCKET_T>::operator= (base_socket<SOCKET_T>&& sock) {
        if (&sock != this) {
            std::swap(sock_id,   sock.sock_id);
            std::swap(address,   sock.address);
            std::swap(port,      sock.port);
            std::swap(localport, sock.localport);
        }
        return *this;
    }

    template<typename SOCKET_T>
    base_socket<SOCKET_T>::~base_socket() {
        
    }

    template<typename SOCKET_T>
    const SOCKET_T& base_socket<SOCKET_T>::get_descriptor() const {
        return sock_id;
    }

    template<typename SOCKET_T>
    SOCKET_T& base_socket<SOCKET_T>::get_descriptor() {
        return sock_id;
    }

    template<typename SOCKET_T>
    inet_address& base_socket<SOCKET_T>::get_address() {
        return address;
    }

    template<typename SOCKET_T>
    const inet_address& base_socket<SOCKET_T>::get_address() const {
        return address;
    }

    template<typename SOCKET_T>
    unsigned int base_socket<SOCKET_T>::get_port() const {
        return port;
    }
    
    template<typename SOCKET_T>
    unsigned int base_socket<SOCKET_T>::get_localport() const {
        return localport;
    }

    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_address(const inet_address& addr) {
        address = addr;
    }
    
    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_port(unsigned int port_in) {
        port = port_in;
    }
    
    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_localport(unsigned int localport_in) {
        localport = localport_in;
    }
    
    template<typename SOCKET_T>
    void base_socket<SOCKET_T>::set_descriptor(SOCKET_T&& sID) {
        sock_id = std::move(sID);
    }
    
    template<typename SOCKET_T>
    string base_socket<SOCKET_T>::to_string(tca::allocator* alloc) const {
        string result(alloc);
        result
            .append("[addr=").append(address.to_string(alloc))
            .append(", port=").append(tc::to_string(port, alloc))
            .append(", localport=").append(tc::to_string(localport, alloc))
            .append(']');
        return result;
    }
}
#endif //JSTD_CPP_LANG_BASE_SOCKET_H