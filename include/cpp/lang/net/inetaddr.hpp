#ifndef _JSTD_CPP_LANG_NET_INET_ADDRESS_H_
#define _JSTD_CPP_LANG_NET_INET_ADDRESS_H_


#include <cpp/lang/utils/optional.hpp>
#include <cpp/lang/string.hpp>
#include <cstdint>

/**
 * BSD types
 */
struct in_addr;
struct in6_addr;

namespace tc
{

class inet_family {
    int m_value;
public:
    // 
    // 
    // 
    constexpr inet_family(int value = 0) : m_value(value) {}
    
    // 
    // 
    // 
    inet_family(const inet_family& f) : m_value(f.m_value) {}
    
    // 
    // 
    // 
    inet_family(inet_family&& f) : m_value(f.m_value) {}
    
    // 
    // 
    // 
    inet_family& operator= (const inet_family& f) { m_value = f.m_value; return *this;}
    
    // 
    // 
    // 
    inet_family& operator= (inet_family&& f) { m_value = f.m_value; return *this;}
    
    // 
    // 
    // 
    bool operator==(const inet_family& f) const {return m_value == f.m_value;}
    
    // 
    // 
    // 
    bool operator!=(const inet_family& f) const {return m_value != f.m_value;};
    
    // 
    // 
    // 
    operator int() const {return m_value;}

    // 
    // NO IP
    // 
    static const inet_family NONE;
    
    // 
    // Internet protocol v4
    // 
    static const inet_family IPV4;
    
    // 
    // Internet protocol v6
    // 
    static const inet_family IPV6;
};



namespace internal
{
namespace net
{

/**
 * C-структура для представления IPv4-адреса.
 * Обеспечивает доступ к данным как в виде массива байт, так и целого числа.
 */
struct ipv4_addr {
    union {
        /**
         * Представление адреса как массива 4 байт.
         */
        uint8_t m_byte_view[4];

        /**
         * Представление адреса как одного 32-битного целого числа.
         */
        uint32_t m_int_view;
    };
};

/**
 * C-структура для представления IPv6-адреса.
 * Обеспечивает доступ к данным в разных разрядностях.
 */
struct ipv6_addr {
    union {
        /**
         * Представление адреса как массива из 16 байт.
         */
        uint8_t m_byte_view[16];

        /**
         * Представление адреса как массива из 8 16-битных слов.
         */
        uint16_t m_short_view[8];

        /**
         * Представление адреса как массива из 4 32-битных слов.
         */
        uint32_t m_int_view[4];
    };
};

}//namespace net

}//namespace internal

/**
 * Представляет IP-адрес (IPv4 или IPv6).
 * Поддерживает преобразование, парсинг, доступ к имени хоста и сравнение.
 */
class inet_address {
protected:
    /**
     * Размер буфера для хранения имени хоста.
     */
    static const int64_t HOST_BUFFER_SIZE = 64;

    /**
     * Объединение для хранения IPv4 или IPv6-адреса.
     */
    union {
        union {
            /**
             * IPv4 представление адреса.
             */
            internal::net::ipv4_addr IPv4;

            /**
             * IPv6 представление адреса.
             */
            internal::net::ipv6_addr IPv6;
        };

        /**
         * Универсальный буфер для хранения IP-данных.
         */
        char m_data_buffer[(sizeof(IPv4) < sizeof(IPv6)) ? sizeof(IPv6) : sizeof(IPv4)];
    };

    /**
     * Имя хоста (если было разрешено).
     */
    char m_host_name[HOST_BUFFER_SIZE];

    /**
     * Семейство IP (IPv4, IPv6 или NONE).
     */
    inet_family family;

public:
    /**
     * Конструктор по умолчанию.
     * Все данные адреса равны нулю.
     */
    inet_address();

    /**
     * Конструктор с указанием семейства IP.
     */
    inet_address(inet_family family);

    /**
     * Конструктор, принимающий строку IP-адреса.
     */
    inet_address(const char* ip);

    /**
     * Конструктор копирования.
     */
    inet_address(const inet_address& addr);

    /**
     * Конструктор перемещения.
     */
    inet_address(inet_address&& addr);

    /**
     * Оператор копирующего присваивания.
     */
    inet_address& operator= (const inet_address& addr);

    /**
     * Оператор перемещающего присваивания.
     */
    inet_address& operator= (inet_address&& addr);

    /**
     * Деструктор.
     */
    ~inet_address();

    /**
     * Возвращает семейство IP (IPv4, IPv6 или NONE).
     */
    inet_family get_family() const;

    /**
     * Возвращает имя хоста, если оно определено.
     */
    const char* get_host_name() const;

    /**
     * Преобразует адрес в строку.
     *
     * @param buf 
     *      Выходной буфер
     * 
     * @param bufsize 
     *      Размер буфера
     * 
     * @return 
     *      Количество записанных символов (без учёта завершающего '\0')
     */
    int to_string(char buf[], std::size_t bufsize) const;

    /**
     * Вычисляет хеш-код адреса.
     */
    std::size_t hashcode() const;

    /**
     * Проверяет равенство двух адресов.
     */
    bool equals(const inet_address& addr) const;

    /**
     * Разрешает доменное имя в один IP-адрес.
     */
    static optional<inet_address> get_by_name(const char* domain);

    /**
     * Получает все IP-адреса по имени хоста.
     *
     * @param buf 
     *      Массив для записи адресов
     * 
     * @param bufsize 
     *      Размер массива
     * 
     * @param domain 
     *      Имя хоста
     * 
     * @return 
     *      Количество найденных адресов
     *
     * @throws illegal_state_exception 
     *      Eсли не удалось получить адреса
     */
    static std::size_t get_all_by_name(inet_address buf[], std::size_t bufsize, const char* domain);

    /**
     * Создаёт IPv4-адрес из массива байт.
     */
    static inet_address as_ip4(const unsigned char buf[], std::size_t bufsize);

    /**
     * Создаёт IPv6-адрес из массива 16-битных значений.
     */
    static inet_address as_ip6(const unsigned short buf[], std::size_t bufsize);

    /**
     * Парсит строку как IPv4-адрес.
     */
    static inet_address parse_IPv4(const char* ip);

    /**
     * Парсит строку как IPv6-адрес.
     */
    static inet_address parse_IPv6(const char* ip);

    /**
     * Возвращает адрес localhost.
     *
     * @param family семейство IP
     */
    static inet_address localhost(inet_family family);

    /**
     * Преобразует IPv4-адрес в строку.
     */
    static int IPv4ToString(char buf[], std::size_t bufsize, const inet_address& address);

    /**
     * Преобразует IPv6-адрес в строку.
     */
    static int IPv6ToString(char buf[], std::size_t bufsize, const inet_address& address);

    /**
     * Создаёт объект из структуры in_addr.
     */
    static inet_address as_in_addr(const in_addr* addr_in);

    /**
     * Создаёт объект из структуры in6_addr.
     */
    static inet_address as_in6_addr(const in6_addr* addr_in);

    /**
     * Копирует IPv4-адрес в структуру in_addr.
     */
    void get_in_addr(in_addr* addr_out) const;

    /**
     * Копирует IPv6-адрес в структуру in6_addr.
     */
    void get_in6_addr(in6_addr* addr_out) const;

    /**
     * Возвращает строковое представление IP адреса.
     */
    tc::string to_string(tca::allocator* = tca::get_default_allocator()) const;
};

/**
 * Представляет сетевой адрес сокета: IP-адрес + порт.
 * Используется для установления соединений и привязки.
 */
class socket_address {
    // The ip socket address
    inet_address m_address;
    // The port 
    unsigned int m_port;
public:
    /**
     * Конструктор по умолчанию.
     */
    socket_address();

    /**
     * Конструктор с IP-адресом и портом.
     */
    socket_address(const inet_address& address, unsigned int port);

    /**
     * Конструктор копирования.
     */
    socket_address(const socket_address&);

    /**
     * Конструктор перемещения.
     */
    socket_address(socket_address&&);

    /**
     * Оператор копирующего присваивания.
     */
    socket_address& operator= (const socket_address&);

    /**
     * Оператор перемещающего присваивания.
     */
    socket_address& operator= (socket_address&&);

    /**
     * Возвращает порт.
     */
    unsigned int get_port() const;

    /**
     * Возвращает IP-адрес.
     */
    const inet_address& get_address() const;

    /**
     * Вычисляет хеш-код.
     */
    std::size_t hashcode() const;

    /**
     * Проверяет равенство двух адресов сокета.
     */
    bool equals(const socket_address& sock_addr) const;

    /**
     * Преобразует адрес сокета в строку.
     *
     * @param buf 
     *      Буфер для строки
     * 
     * @param bufsize 
     *      Размер буфера
     * 
     * @return 
     *      Количество записанных символов (не считая '\0').
     */
    string to_string(tca::allocator* alloc = tca::get_default_allocator()) const;
};


template<typename T>
struct hash_for;

template<typename T>
struct equal_to;

template<>
struct equal_to<inet_address> {
    bool operator() (const inet_address& i1, const inet_address& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<inet_address> {
    std::size_t operator() (const inet_address& i) {
        return i.hashcode();
    }
};

template<>
struct equal_to<socket_address> {
    bool operator() (const socket_address& i1, const socket_address& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<socket_address> {
    std::size_t operator() (const socket_address& i) {
        return i.hashcode();
    }
};

}

#endif//_JSTD_CPP_LANG_NET_INET_ADDRESS_H_