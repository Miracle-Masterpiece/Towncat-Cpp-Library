#ifndef JSTD_CPP_LANG_UTIL_DATE_H
#define JSTD_CPP_LANG_UTIL_DATE_H

#include <ctime>
#include <cstdint>
#include <cpp/lang/types.hpp>
#include <cpp/lang/string.hpp>

namespace tc {

/**
 * Класс для работы с датой и временем.
 * Предоставляет функциональность для создания, сравнения, форматирования и манипуляции датами.
 * Внутренне использует структуру time_t для хранения времени и tm для локального представления.
 * 
 * @note Класс поддерживает копирование, перемещение и сравнение дат.
 *       Также предоставляет методы для получения отдельных компонентов даты (день, месяц, год и т.д.).
 */
class date { 
    
    /**
     *  Время в формате time_t (секунды с начала Unix-эпохи).
     */
    std::time_t _time;
    
    /**
     * Локальное представление времени в структуре tm.
     */
    std::tm _localTime;
public:
    
    enum struct month {
        JANUARY = 1,   // Январь
        FEBRUARY,      // Февраль
        MARCH,         // Март
        APRIL,         // Апрель
        MAY,           // Май
        JUNE,          // Июнь
        JULY,          // Июль
        AUGUST,        // Август
        SEPTEMBER,     // Сентябрь
        OCTOBER,       // Октябрь
        NOVEMBER,      // Ноябрь
        DECEMBER       // Декабрь
    };
    
    /**
     * 
     */
    static const int DAY_DEFAULT    = 1;
    
    /**
     * 
     */
    static const int MONTH_DEFAULT  = 1;
    
    /**
     * 
     */
    static const int YEAR_DEFAULT   = 1990;
    
    /**
     * 
     */
    static const int SECOND_DEFAULT = 0;
    
    /**
     * 
     */
    static const int MINUTE_DEFAULT = 0;
    
    /**
     * 
     */
    static const int HOUR_DEFAULT   = 0;

    /**
     * Конструктор, создающий объект даты с указанными компонентами.
     * 
     * @param day
     *      День месяца (1-31).
     * 
     * @param month
     *      Месяц (1-12).
     * 
     * @param year
     *      Год (например, 2023).
     * 
     * @param second
     *      Секунды (0-59).
     * 
     * @param minute
     *      Минуты (0-59).
     * 
     * @param hour
     *      Часы (0-23).
     */
    date(
        int day     = DAY_DEFAULT,
        int month   = MONTH_DEFAULT,
        int year    = YEAR_DEFAULT,
        int second  = SECOND_DEFAULT,
        int minute  = MINUTE_DEFAULT,
        int hour    = HOUR_DEFAULT
    );

    /**
     * Конструктор, создающий объект даты из времени в формате time_t.
     * 
     * @param date Время в формате time_t (секунды с начала Unix-эпохи).
     */
    date(std::time_t date);

    /**
     * Возвращает текущую дату и время.
     * 
     * @return
     *      Объект date, представляющий текущий момент времени.
     */
    static date now();

    /**
     * 
     */
    static date of_seconds(timepoint sec);
    
    /**
     * 
     */
    static date of_milliseconds(timepoint ms);

    /**
     * Преобразует дату в строковое представление.
     */
    tc::string to_string(tca::allocator* = tca::get_default_allocator()) const;

    /**
     * Сравнивает текущую дату с другой датой на равенство.
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return
     *      true, если даты равны, иначе false.
     */
    bool equals(const date& other) const;

    /**
     * Возвращает хэш-код даты.
     * 
     * @return
     *      Хэш-код в формате int64_t.
     */
    std::size_t hashcode() const;

    /**
     * Сравнивает текущую дату с другой датой.
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return 
     *      0, если даты равны; отрицательное число, если текущая дата меньше; положительное число, если больше.
     */
    int compare_to(const date& other) const;

    /**
     * Оператор сравнения на равенство.
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return
     *      true, если даты равны, иначе false.
     */
    bool operator==(const date& other) const;

    /**
     * Оператор сравнения на неравенство.
     * 
     * @param other
     *  Дата для сравнения.
     * 
     * @return
     *      true, если даты не равны, иначе false.
     */
    bool operator!=(const date& other) const;

    /**
     * Оператор сравнения "больше".
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return
     *      true, если текущая дата больше, иначе false.
     */
    bool operator>(const date& other) const;

    /**
     * Оператор сравнения "меньше".
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return
     *      true, если текущая дата меньше, иначе false.
     */
    bool operator<(const date& other) const;

    /**
     * Оператор сравнения "больше или равно".
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return
     *      true, если текущая дата больше или равна, иначе false.
     */
    bool operator>=(const date& other) const;

    /**
     * Оператор сравнения "меньше или равно".
     * 
     * @param other
     *      Дата для сравнения.
     * 
     * @return
     *      true, если текущая дата меньше или равна, иначе false.
     */
    bool operator<=(const date& other) const;

    /**
     * Устанавливает время в формате time_t.
     * 
     * @param date
     *      Время в формате time_t.
     */
    void set_time(std::time_t date);

    /**
     * Возвращает время в формате time_t.
     * 
     * @return
     *      Время в формате time_t.
     */
    std::time_t get_time() const;

    /**
     * Возвращает день месяца.
     * 
     * @return
     *      День месяца (1-31).
     */
    int get_day() const;

    /**
     * Возвращает месяц.
     * 
     * @return
     *      Месяц (1-12).
     */
    int get_month() const;

    /**
     * Возвращает год.
     * 
     * @return
     *      Год (например, 2023).
     */
    int get_year() const;

    /**
     * Возвращает час.
     * 
     * @return
     *      Час (0-23).
     */
    int get_hour() const;

    /**
     * Возвращает минуты.
     * 
     * @return
     *      Минуты (0-59).
     */
    int get_minute() const;

    /**
     * Возвращает секунды.
     * 
     * @return
     *      Секунды (0-59).
     */
    int get_second() const;

    /**
     * Возвращает номер недели в году.
     * 
     * @return
     *      Номер недели (1-53).
     */
    int get_week() const;
};

template<typename T>
struct hash_for;

template<typename T>
struct equal_to;

template<>
struct equal_to<date> {
    bool operator() (const date& i1, const date& i2) {
        return i1.equals(i2);
    }
};

template<>
struct hash_for<date> {
    uint64_t operator() (const date& i) {
        return i.hashcode();
    }
};

}
#endif //JSTD_CPP_LANG_UTIL_DATE_H