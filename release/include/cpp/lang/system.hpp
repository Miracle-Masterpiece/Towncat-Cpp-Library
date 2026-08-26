#ifndef _ALLOCATORS_SYSTEM_H
#define _ALLOCATORS_SYSTEM_H

#include <cstdint>
#include <cpp/lang/types.hpp>

namespace tc 
{

enum byte_order {
    LE, //little-endian
    BE, //big-endian
};

namespace system 
{

namespace internal
{
    /**
     * Системный порядок байт.
     * */    
    extern const byte_order system_order;
}
    /**
     * Возвращает константу порядка байтов на выполняемой системе.
     * 
     * @return
     *      byte_order::LE - если программа выполняется на процессоре с порядом байтов Little-Endian
     *      byte_order::BE - если программа выполняется на процессоре с порядом байтов Big-Endian
     */
    inline byte_order native_byte_order() {
        return internal::system_order;
    }

    inline bool native_is_little_endian() {
        return native_byte_order() == byte_order::LE;
    }

    inline bool native_is_big_endian() {
        return native_byte_order() == byte_order::BE;
    }

    /**
     * Возвращает текущее время в миллисекундах, прошедших с начала Unix-эпохи (1 января 1970 года).
     * Функция предоставляет кросс-платформенный способ получения времени, работая на Linux, macOS и Windows.
     * 
     * @return 
     *      Количество миллисекунд, прошедших с начала Unix-эпохи.
     * 
     * @note На Windows время изначально представлено в 100-наносекундных интервалах.
     *       Для перевода в миллисекунды значение делится на 10000.
     * 
     * @warning Время, возвращаемое функцией, зависит от системных часов.
     *          Изменение системного времени может повлиять на результат.
     */
    timepoint current_time_millis();

    /**
     * Возвращает текущее время в секундах, прошедших с начала Unix-эпохи (1 января 1970 года).
     * Функция использует результат 'current_time_millis()' и делит его на 1000 для получения времени в секундах.
     * 
     * @return 
     *      Количество секунд, прошедших с начала Unix-эпохи.
     * 
     * @warning
     *      Время, возвращаемое функцией, зависит от системных часов.
     *      Изменение системного времени может повлиять на результат.
     */
    timepoint current_time_seconds();

    /**
     * Возвращает текущее время в наносекундах, используя монотонные часы.
     * 
     * @return 
     *      Количество наносекунд, прошедших с момента, определённого системой как начало отсчёта.
     * 
     * @warning
     *      Возвращаемое значение не привязано к календарному времени.
     *      Оно предназначено исключительно для измерения интервалов.
     */
    timepoint nano_time();

    /**
     * Возвращает описание системной ошибки.
     */
    const char* error_string(int err);

    /**
     * Thread-Safe printf
     */
    int tsprintf(const char* format, ...);

}//namespace system

}//namespace jstd



#endif//_ALLOCATORS_SYSTEM_H