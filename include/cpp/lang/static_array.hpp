#ifndef JSTD_CPP_LANG_STATIC_ARRAY_H
#define JSTD_CPP_LANG_STATIC_ARRAY_H

#include <initializer_list>
#include <cstdint>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/exceptions.hpp>

namespace tc
{
    
/**
 * @since 2.0
 */
template<typename T, std::size_t LENGTH>
class static_array {
    T m_data[LENGTH];
public:
    /**
     * Размер массива.
     */
    static const std::size_t length = LENGTH;
    
    /**
     * 
     */
    static_array();
    
    /**
     * Копирует данные из входящего массива в этот.
     * 
     * @param arr
     *      Массив, из которого буду скопированы данные.
     */
    static_array(const static_array<T, LENGTH>& arr);
    
    /**
     * Перемещает данные из входящего массива в этот.
     * 
     * @param arr
     *      Массив, из которого будут перемещены данные.
     */
    static_array(static_array<T, LENGTH>&& arr);
    
    /**
     * Инициализирует массив значениеями из инициализируюшего списка.
     * 
     * @param init_list
     *      Инициализирующий список для инициализации массива.
     * 
     * @throws overflow_exception
     *      Если инициализирующий список больше размера массива.
     */
    static_array(const std::initializer_list<T>& init_list);
    
    /**
     * Копирует данные из другого массива в этот.
     */
    static_array<T, LENGTH>& operator= (const static_array<T, LENGTH>&);
    
    /**
     * Перемещает данные из другого массива в этот.
     */
    static_array<T, LENGTH>& operator= (static_array<T, LENGTH>&&);
    
    /**
     * Оператор индексирования. (Константный)
     * 
     * @param idx
     *      Индекс.
     * 
     * @throws index_out_of_bound_exception
     *      Если индекс выходит за границу массива.
     */
    const T& operator[] (std::size_t idx) const;
    
    /**
     * Оператор индексирования.
     * 
     * @param idx
     *      Индекс.
     * 
     * @throws index_out_of_bound_exception
     *      Если индекс выходит за границу массива.
     */
    T& operator[] (std::size_t idx);
    
    /**
     * Возвращает сырой указатель на данные. (Константный)
     * 
     * @return
     *      Указатель на начало массива.
     */
    const T* data() const;
    
    /**
     * Возвращает сырой указатель на данные.
     * 
     * @return
     *      Указатель на начало массива.
     */
    T* data();
};

    template<typename T, std::size_t LENGTH>
    static_array<T, LENGTH>::static_array() {
        
    }
    
    template<typename T, std::size_t LENGTH>
    static_array<T, LENGTH>::static_array(const static_array<T, LENGTH>& array) {
        using NON_CONST_T = typename tc::remove_cv<T>::type;
        uninitialized_copy_n(const_cast<NON_CONST_T*>(m_data), array.m_data, LENGTH);
    }
    
    template<typename T, std::size_t LENGTH>
    static_array<T, LENGTH>::static_array(static_array<T, LENGTH>&& array) {
        for (std::size_t i = 0; i < LENGTH; ++i)
            m_data[i] = std::move(array.m_data[i]);
    }
    
    template<typename T, std::size_t LENGTH>
    static_array<T, LENGTH>::static_array(const std::initializer_list<T>& init_list) {
        JSTD_DEBUG_CODE(
            if (init_list.size() > length)
                throw_except<overflow_exception>("Initializer list size > length array");
        );
        std::size_t i = 0;
        for (const T& v : init_list) {
            m_data[i++] = v;
        }
    }
    
    template<typename T, std::size_t LENGTH>
    static_array<T, LENGTH>& static_array<T, LENGTH>::operator= (const static_array<T, LENGTH>& array) {
        if (&array != this) {
            using NON_CONST_T = typename tc::remove_cv<T>::type;
            uninitialized_copy_n(const_cast<NON_CONST_T*>(m_data), array.m_data, LENGTH);
        }
        return *this;
    }
    
    template<typename T, std::size_t LENGTH>
    static_array<T, LENGTH>& static_array<T, LENGTH>::operator= (static_array<T, LENGTH>&& array) {
        if (&array != this) {
            for (std::size_t i = 0; i < LENGTH; ++i)
                m_data[i] = std::move(array.m_data[i]);
        }
        return *this;
    }
    
    template<typename T, std::size_t LENGTH>
    const T& static_array<T, LENGTH>::operator[] (std::size_t idx) const {
        JSTD_DEBUG_CODE(
            check_index(idx, length);
        );
        return m_data[idx];
    }
    
    template<typename T, std::size_t LENGTH>
    const T* static_array<T, LENGTH>::data() const {
        return m_data;
    }
    
    template<typename T, std::size_t LENGTH>
    T& static_array<T, LENGTH>::operator[] (std::size_t idx) {
        JSTD_DEBUG_CODE(
            check_index(idx, length);
        );
        return m_data[idx];
    }
    
    template<typename T, std::size_t LENGTH>
    T* static_array<T, LENGTH>::data() {
        return m_data;
    }
}

#endif//JSTD_CPP_LANG_STATIC_ARRAY_H