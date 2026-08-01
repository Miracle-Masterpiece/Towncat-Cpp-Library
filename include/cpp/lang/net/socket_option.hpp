#ifndef JSTD_CPP_LANG_NET_SOCKET_OPTIONS_H
#define JSTD_CPP_LANG_NET_SOCKET_OPTIONS_H

#include <cpp/lang/types.hpp>

namespace tc {

/**
 * Структура для настройки параметров сокета.
 * Используется для установки различных опций сокета через setsockopt.
 * Поддерживает параметры для управления буферами, таймаутами, keep-alive и другими опциями.
 */
struct socket_option {
    /** 
     * Максимальный размер буфера для установки опции SO_RCVBUF и SO_SNDBUF.
     */
    static const int OPT_BUFFER_SIZE = 16;

    /**
     * Объединение (union) для различных типов значений опции.
     * Позволяет задавать значения разного типа в зависимости от выбранной опции.
     */
    union {
        char    byte_value;
        short   short_value;
        int     int_value;
        float   float_value;
        long    long_value;
        double  double_value;
        
        /** 
         * Параметры linger (SO_LINGER).
         * Управляет поведением сокета при закрытии.
         */
        struct {
            bool         on_off;    /* Включение/выключение linger (true = включено, false = выключено).     */
            unsigned int sec_time;  /* Время ожидания перед закрытием соединения (0 = закрытие немедленно).  */
        } linger;

        /** 
         * Таймаут операций (SO_RCVTIMEO, SO_SNDTIMEO).
         * Определяет максимальное время ожидания операций.
         */
        struct {
            timepoint millis; /* Время ожидания в миллисекундах. */
        } timeout;

        /** 
         * Параметры keep-alive (SO_KEEPALIVE и TCP_KEEP*).
         * Используются для поддержания соединения.
         */
        struct {
            bool on_off;        /* Включение keep-alive (true = включено, false = выключено).   */
            unsigned int  time_to_first; /* Время до первой проверки активности (в секундах).            */
            unsigned int  time_interval; /* Интервал между проверками (в секундах).                      */
            unsigned int  try_count;     /* Количество попыток перед закрытием соединения.               */
        } keepalive;
    };

    /**
     * Параметры опций BSD-сокета.
     * Константы для указания различных опций в setsockopt.
     */
    struct bsd {
        /**
         * Отключение алгоритма Nagle (TCP_NODELAY).
         * Тип параметра: int (0 = включён Nagle, 1 = отключён Nagle).
         */
        static const int TCP_NODELAY_ = 0;

        /**
         * Повторное использование локального адреса (SO_REUSEADDR).
         * Тип параметра: int (0 = отключено, 1 = включено).
         */
        static const int SO_REUSEADDR_ = 1;

        /**
         * Разрешение широковещательной отправки (SO_BROADCAST).
         * Тип параметра: int (0 = запрещено, 1 = разрешено).
         */
        static const int SO_BROADCAST_ = 2;

        /**
         * Установка типа обслуживания IP (IP_TOS).
         * Тип параметра: int, возможные значения:
         *   - 0x00 — Обычный трафик (Normal-Service)
         *   - 0x04 — Минимальная задержка (Minimize-Delay)
         *   - 0x08 — Максимальная пропускная способность (Maximize-Throughput)
         *   - 0x10 — Максимальная надёжность (Maximize-Reliability)
         *   - 0x1C — Минимальная стоимость (Minimize-Cost)
         */
        static const int IP_TOS_ = 3;

        /**
         * Управление linger (SO_LINGER).
         * Тип параметра: структура linger { int l_onoff, int l_linger }.
         *   - l_onoff = 0 → немедленное закрытие без отправки оставшихся данных.
         *   - l_onoff = 1, l_linger > 0 → ожидание перед закрытием (секунды).
         */
        static const int SO_LINGER_ = 4;

        /**
         * Таймаут приёма (SO_RCVTIMEO).
         * Тип параметра: структура timeval { long sec, long usec }.
         *   - sec  = секунды ожидания.
         *   - usec = микросекунды ожидания.
         */
        static const int SO_RCVTIMEO_ = 5;

        /**
         * Таймаут отправки (SO_SNDTIMEO).
         * Тип параметра: структура timeval { long sec, long usec }.
         */
        static const int SO_SNDTIMEO_ = 6;

        /**
         * Размер буфера отправки (SO_SNDBUF).
         * Тип параметра: int (размер в байтах).
         */
        static const int SO_SNDBUF_ = 7;

        /**
         * Размер буфера приёма (SO_RCVBUF).
         * Тип параметра: int (размер в байтах).
         */
        static const int SO_RCVBUF_ = 8;

        /**
         * Включение keep-alive (SO_KEEPALIVE).
         * Тип параметра: int (0 = выключено, 1 = включено).
         */
        static const int SO_KEEPALIVE_ = 9;

        /**
         * Время перед первой keep-alive проверкой (TCP_KEEPIDLE).
         * Тип параметра: int (время в секундах).
         */
        static const int TCP_KEEPIDLE_ = 10;

        /**
         * Количество keep-alive попыток перед разрывом соединения (TCP_KEEPCNT).
         * Тип параметра: int (количество попыток).
         */
        static const int TCP_KEEPCNT_ = 11;

        /**
         * Интервал между keep-alive проверками (TCP_KEEPINTVL).
         * Тип параметра: int (время в секундах).
         */
        static const int TCP_KEEPINTVL_ = 12;

        /**
         * Обработка данных вне полосы (SO_OOBINLINE).
         * Тип параметра: int (0 = отключено, 1 = включено).
         * Если включено, данные вне полосы (OOB) обрабатываются в общем потоке данных.
         */
        static const int SO_OOBINLINE_ = 13;
    };
};


}

#endif //JSTD_CPP_LANG_NET_SOCKET_OPTIONS_H