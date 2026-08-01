#ifndef JSTD_CPP_LANG_CSTRING_H
#define JSTD_CPP_LANG_CSTRING_H

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/objects.hpp>

namespace tc {

/**
 * Представляет собой неизменяемую строку в стиле C (null-terminated), с кэшируемой длиной и удобными методами доступа.
 *
 * Класс cstr предоставляет обёртку над const char*, позволяя безопасно и удобно работать со строками, 
 * не копируя данные. Длина строки кэшируется после первого запроса, что ускоряет повторные вызовы length().
 * Поддерживает сравнение, хеширование и доступ к символам по индексу.
 */
class cstr {
    const char* m_cstr;             // Указатель на C-строку (null-terminated).
    mutable std::size_t m_length;   // Кэшированная длина строки. Вычисляется при первом вызове length().
public:
    /**
     * Конструктор из C-строки.
     *
     * @param s 
     *      Указатель на null-terminated строку. 
     *      Должен быть действительным на всё время жизни объекта.
     */
    cstr(const char* s);

    /**
     * Неявное преобразование к const char*.
     * Позволяет использовать объект cstr в контексте, где ожидается C-строка, например при передаче в стандартные функции C.
     *
     * @return 
     *      Указатель на исходную C-строку.
     */
    operator const char* () const;

    /**
     * Возвращает длину строки (без учёта завершающего нуля).
     *
     * Длина строки вычисляется один раз и кэшируется для последующих вызовов.
     *
     * @return 
     *      Количество символов в строке.
     */
    std::size_t length() const;

    /**
     * Возвращает символ по индексу.
     * 
     * @param idx 
     *      Индекс символа.
     * 
     * @return 
     *      Константная ссылка на символ по указанному индексу.
     * 
     * @throws index_out_of_bound_exception
     *      Если индекс меньше нуля или больше длины. (Если не определён NDEBUG).
     */
    const char& operator[] (std::size_t idx) const;

    /**
     * Сравнивает две строки на равенство содержимого.
     *
     * Сравниваются символы строк посимвольно до первого отличия или конца строк.
     *
     * @param s 
     *      Другая строка для сравнения.
     * 
     * @return 
     *      true, если строки идентичны, иначе — false.
     */
    bool equals(const cstr& s) const;

    /**
     * Оператор сравнения на равенство.
     *
     * Делегирует выполнение методу equals().
     *
     * @param s 
     *      Другая строка для сравнения.
     * 
     * @return 
     *      true, если строки равны, иначе false.
     */
    bool operator==(const cstr& s) const;

    /**
     * @brief Оператор сравнения на неравенство.
     *
     * Противоположность operator==.
     *
     * @param s 
     *      Другая строка для сравнения.
     * 
     * @return 
     *      true, если строки не равны, иначе false.
     */
    bool operator!=(const cstr& s) const;

    /**
     * Вычисляет хеш-код строки.
     * 
     * @return 
     *      Хеш-код строки.
     */
    std::size_t hashcode() const;

    /**
     * Проверяет, является ли строка пустой.
     *
     * Возвращает true, если строка не содержит символов (т.е. длина равна нулю).
     * Поведение идентично сравнению length() == 0. 
     * 
     * Это касается как пустых строк, так и случая, когда строка равна nullptr. 
     * В обоих случаях метод вернёт true.
     *
     * @return 
     *      true, если строка пуста, иначе false.
     */
    bool is_empty() const;
};

template<>
struct hash_for<cstr> {
    std::size_t operator ()(const cstr& s) const {
        return s.hashcode();
    }
};

template<>
struct equal_to<cstr> {
    bool operator ()(const cstr& s0, const cstr& s1) const {
        return s0.equals(s1);
    }
};


}

#endif//JSTD_CPP_LANG_CSTRING_H