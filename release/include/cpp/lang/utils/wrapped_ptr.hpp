#ifndef JSTD_CPP_LANG_UTILS_WRAPPED_PTR_H
#define JSTD_CPP_LANG_UTILS_WRAPPED_PTR_H

#include <cpp/lang/exceptions.hpp>
#include <cstdint>

namespace tc
{

/**
 * Класс для хранения сырого указателя.
 * 
 * @warning
 *      Класс не владеет указателем! Он лишь нужен для безопасной работы, во время разыменования.
 *      Так как проверяет, не является ли хранимый указатель - нулём.
 * 
 * @note
 *      Макрос NDEBUG отключает все проверки в классе.
 * 
 * @since 2.1
 */
template<typename T>
class wrapped_ptr {
    /**
     * 
     */
    T* m_pointer;
public:
    
    /**
     * Создаёт нулевой указатель.
     */
    wrapped_ptr();
    
    /**
     * Инициализирует объект передаваемым указателем.
     */
    wrapped_ptr(T* pointer);

    /**
     *
     */
    wrapped_ptr(const wrapped_ptr<T>&) = default;
    
    /**
     * Перемещает указатель из передаваемого объекта, в этот.
     * При этом указатель внутри старого объекта указывает на 0. 
     */
    wrapped_ptr(wrapped_ptr<T>&&);
    
    /**
     * 
     */
    wrapped_ptr<T>& operator= (const wrapped_ptr<T>&) = default;
    
    /**
     * Перемещает указатель из передаваемого объекта, в этот.
     * При этом указатель внутри старого объекта указывает на 0. 
     */
    wrapped_ptr<T>& operator= (wrapped_ptr<T>&&);
    
    /**
     * @note
     *      Деструктор не освобождает данные!
     */
    ~wrapped_ptr() = default;

    /**
     * Разыменовывает указатель и возвращает ссылку на значение.
     * 
     * @throws null_pointer_exception
     *      Если попытка разыменовать нулевой указатель.
     */
    T& operator*() const;
    
    /**
     * Разыменовывает указатель на член структуры или класса.
     * 
     * @throws null_pointer_exception
     *      Если попытка разыменовать нулевой указатель.
     */
    T* operator->() const;
    
    /**
     * Оператор индексирования.
     * 
     * @return
     *      Ссылка на значение, хранящаася по индексу.
     * 
     * @throws null_pointer_exception
     *      Если попытка разыменовать нулевой указатель.
     */
    T& operator[](std::size_t idx) const;
    
    /**
     * Добавляет смещение к указателю и возвращает новый объект.
     * 
     * @param off
     *      Смещение, добавляемое к указателю.
     * 
     * @return
     *      Смещённый указатель.
     */
    wrapped_ptr<T> operator+(std::size_t off) const;
    
    /**
     * Возвращает разницу между этим указателем и передаваемым.
     * 
     * @return
     *      Разница индексов меджду указателями.
     */
    std::ptrdiff_t operator-(const wrapped_ptr<T>&) const;
    
    /**
     * @return
     *      true тогда, и только тогда, когда указатель не равен nullptr.
     */
    operator bool() const;

    /**
     * Возвращает сырой указатель, хранящийся внутри этого объекта.
     * 
     * @return
     *      Сырой указатель.
     */
    operator T*() const;

    /**
     * 
     */
    template<typename E>
    explicit operator wrapped_ptr<E>() const;
};

    template<typename T>
    wrapped_ptr<T>::wrapped_ptr() : wrapped_ptr<T>(nullptr) {

    }

    template<typename T>
    wrapped_ptr<T>::wrapped_ptr(wrapped_ptr<T>&& p) : m_pointer(p.m_pointer) {
        p.m_pointer = nullptr;
    }

    template<typename T>
    wrapped_ptr<T>& wrapped_ptr<T>::operator= (wrapped_ptr<T>&& p) {
        if (&p != this) {
            m_pointer   = p.m_pointer;
            p.m_pointer = nullptr;
        }
        return *this;
    }

    template<typename T>
    wrapped_ptr<T>::wrapped_ptr(T* pointer) : m_pointer(pointer) {

    }

    template<typename T>
    T& wrapped_ptr<T>::operator*() const {
        JSTD_DEBUG_CODE(
            check_non_null(m_pointer);
        );
        return *m_pointer;
    }

    template<typename T>
    T* wrapped_ptr<T>::operator->() const {
        JSTD_DEBUG_CODE(
            check_non_null(m_pointer);
        );
        return m_pointer;
    }

    template<typename T>
    T& wrapped_ptr<T>::operator[](std::size_t idx) const {
        JSTD_DEBUG_CODE(
            check_non_null(m_pointer);
        );
        return m_pointer[idx];
    }

    template<typename T>
    wrapped_ptr<T> wrapped_ptr<T>::operator+(std::size_t off) const {
        return wrapped_ptr<T>(m_pointer + off);
    }

    template<typename T>
    wrapped_ptr<T>::operator bool() const {
        return m_pointer != nullptr;
    }

    template<typename T>
    wrapped_ptr<T>::operator T*() const {
        return m_pointer;
    }

    template<typename T>
    std::ptrdiff_t wrapped_ptr<T>::operator-(const wrapped_ptr<T>& p) const {
        return m_pointer - p.m_pointer;
    }
    
    template<typename T>
    template<typename E>
    wrapped_ptr<T>::operator wrapped_ptr<E>() const {
        JSTD_DEBUG_CODE(
            if (!dynamic_cast<E*>(m_pointer))
                throw_except<class_cast_exception>();
        );
        return wrapped_ptr<E>(reinterpret_cast<E*>(m_pointer));
    }

    template<typename T>
    wrapped_ptr<T> make_wrapped(T* pointer) {
        return wrapped_ptr<T>(pointer);
    }

}


#endif//JSTD_CPP_LANG_UTILS_WRAPPED_PTR_H