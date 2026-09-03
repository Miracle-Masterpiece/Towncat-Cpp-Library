#ifndef JSTD_CPP_LANG_UTILS_RANDOM_H
#define JSTD_CPP_LANG_UTILS_RANDOM_H

#include <cstdint>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/limits.hpp>
#include <cpp/lang/exceptions.hpp>

namespace tc
{

/**
 * Генератор псевдослучайных чисел с возможностью задания начального зерна.
 * 
 * Основан на линейном конгруэнтном методе (LCG) с переменным приращением.
 * Один и тот же seed гарантирует
 * детерминированную последовательность чисел.
 */
class random {

    /**
     * 
     */
    unsigned long long m_seed;

    /**
     * Внутренний генератор псевдослучайных чисел.
     * @return 
     *      Следующее псевдослучайное 64-битное значение.
     */
    unsigned long long next();

    /**
     * 
     */
    static unsigned long long random_seed();

    /**
     * 
     */
    unsigned long long get_seed() const {
        return m_seed;
    }

public:
    /**
     * Создаёт новый генератор случайных чисел с заданным или автоматически определённым зерном.
     * 
     * @param seed 
     *      Начальное значение генератора.
     */
    random(unsigned long long seed = random_seed());

    /**
     * Конструктор копирования.
     * Копирует внутреннее состояние генератора.
     * 
     * @param other 
     *      Генератор, из которого копируется состояние.
     */
    random(const random& other);

    /**
     * Конструктор перемещения.
     * Перемещает внутреннее состояние генератора.
     * 
     * @param other 
     *      Генератор, состояние которого перемещается.
     */
    random(random&& other);

    /**
     * Оператор копирующего присваивания.
     * 
     * @param other 
     *      Генератор, из которого копируется состояние.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    random& operator=(const random& other);

    /**
     * Оператор перемещающего присваивания.
     * 
     * @param other 
     *      Генератор, из которого перемещается состояние.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    random& operator=(random&& other);

    /**
     */
    ~random();

    /**
     * Генерирует случайное значение указанного типа.
     * Для целых типов значение выбирается из полного диапазона типа {@code T}.
     * Для типов с плавающей точкой (float, double, long double) результат в диапазоне [0.0, 1.0).
     *
     * @tparam T 
     *      Тип возвращаемого значения.
     * 
     * @return 
     *      Случайное значение указанного типа.
     */
    template<typename T>
    T next();

    /**
     * Генерирует случайное значение типа T в диапазоне от 0 (включительно) до max (не включительно).
     * Значение {@code max} должно быть строго положительным.
     * 
     * @tparam T 
     *      Тип возвращаемого значения.
     * 
     * @param max 
     *      Верхняя граница диапазона (не включительно).
     * 
     * @return 
     *      Случайное значение в диапазоне [0, max).
     * 
     * @throws illegal_argument_exception 
     *      Eсли max < 0.
     */
    template<typename T>
    T next(const T max);

    /**
     * Заполняет указанный массив случайными значениями типа T.
     * Каждый элемент заполняется вызовом {@code next<T>()}.
     *
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param buf 
     *      Указатель на массив.
     * 
     * @param bufsize 
     *      Количество элементов, которые нужно заполнить.
     * 
     * @throws illegal_argument_exception 
     *      Eсли buf == nullptr
     */
    template<typename T>
    void values(T buf[], std::size_t bufsize);

    /**
     * Заполняет указанный массив случайными значениями типа T в диапазоне [0, max).
     * Каждое значение генерируется вызовом {@code next<T>(max)}.
     * 
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param buf 
     *      Указатель на массив.
     * 
     * @param bufsize 
     *      Количество элементов, которые нужно заполнить.
     * 
     * @param max 
     *      Верхняя граница диапазона (не включительно).
     * 
     * @throws illegal_argument_exception 
     *      Eсли buf == nullptr
     */
    template<typename T>
    void values(T buf[], std::size_t bufsize, T max);
};

    template<>
    float random::next();

    template<>
    double random::next();

    template<>
    long double random::next();
    
    template<>
    bool random::next();

    template<typename T>
    T random::next() {
        return (T) next();
    }

    template<typename T>
    T random::next(const T max) {
        T v = (T) (next<T>() % max);
        return v < 0 ? static_cast<T>(-v) : v;
    }

    template<typename T>
    void random::values(T buf[], std::size_t bufsize) {
        JSTD_DEBUG_CODE(check_non_null(buf));
        for (std::size_t i = 0; i < bufsize; ++i)
            buf[i] = next<T>();
    }

    template<typename T>
    void random::values(T buf[], std::size_t bufsize, T max) {
        JSTD_DEBUG_CODE(check_non_null(buf));
        for (std::size_t i = 0; i < bufsize; ++i)
            buf[i] = next<T>(max);
    }
}

#endif//JSTD_CPP_LANG_UTILS_RANDOM_H