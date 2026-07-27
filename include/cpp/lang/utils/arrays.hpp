#ifndef JSTD_CPP_LANG_UTILS_ARRAYS_H
#define JSTD_CPP_LANG_UTILS_ARRAYS_H

#include <cctype>
#include <type_traits>
#include <cstring>
#include <initializer_list>
#include <cstdint>

#ifndef NDEBUG
    #include <cpp/lang/exceptions.hpp>
#endif

#define JSTD_TRIVIAL_COPY_CHECK 

namespace jstd {

    /**
     * Конструирует массив объектов типа T в заранее выделенной области памяти с использованием placement new.
     *
     * Эта функция последовательно вызывает конструктор по умолчанию для каждого элемента массива,
     * начиная с заданного адреса array. 
     * 
     * Если при конструировании одного из объектов выбрасывается исключение,
     * ранее успешно сконструированные объекты будут уничтожены в обратном порядке.
     *
     * @tparam T 
     *      Тип объектов, которые будут размещены.
     * 
     * @param array 
     *      Указатель на предварительно выделенную память, достаточную для хранения `length` объектов типа T.
     * 
     * @param length 
     *      Количество объектов, которые требуется сконструировать.
     * 
     * @throws null_pointer_exception 
     *      Eсли array == nullptr
     * 
     * @throws Любое исключение, выброшенное конструктором T.
     *
     * 
     * @note Если включён JSTD_TRIVIAL_COPY_CHECK и тип T является тривиально копируемым,
     *       функция завершает выполнение без создания объектов.
     */
    template<typename T>
    void placement_new(T* array, std::size_t length) {
        JSTD_DEBUG_CODE(
        if (array == nullptr)
            throw_except<null_pointer_exception>("array is null");
        );
#ifdef JSTD_TRIVIAL_COPY_CHECK
        if (std::is_trivially_copyable<T>::value)
            return;
#endif//JSTD_TRIVIAL_COPY_CHECK

        std::size_t constructed = 0;
        try {
            for (;constructed < length; ++constructed)
                new (array + constructed) T();
        } catch (...) {
            while (constructed-- != 0)
                array[constructed].~T();
            throw;
        }
    }

    /**
     * Явно вызывает деструкторы для массива объектов типа T, размещённых в указанной области памяти.
     *
     * Функция последовательно вызывает деструктор для каждого объекта, начиная с конца массива.
     *
     * @tparam T 
     *      Тип объектов, чьи деструкторы необходимо вызвать.
     * 
     * @param array 
     *      Указатель на массив объектов.
     * 
     * @param length 
     *      Количество объектов, для которых вызываются деструкторы.
     *
     * @throws null_pointer_exception 
     *      Eсли array == nullptr.
     *
     * @note Если включён JSTD_TRIVIAL_COPY_CHECK и тип T является тривиально разрушимым,
     *       функция завершает выполнение без вызова деструкторов.
     */
    template<typename T>
    void placement_destroy(const T* array, std::size_t length) {
        JSTD_DEBUG_CODE(
            if (array == nullptr)
                throw_except<null_pointer_exception>("array is null");
        );
        
#ifdef JSTD_TRIVIAL_COPY_CHECK
        if (std::is_trivially_destructible<T>::value)
            return;
#endif//JSTD_TRIVIAL_COPY_CHECK
        
        std::size_t destructs = length;
        while (destructs > 0)
            array[--destructs].~T();
    }

    /**
     * Копирует массив объектов в ранее выделенную память с помощью конструктора копирования и placement new.
     *
     * Эта функция создаёт новые объекты типа T в массиве dst, копируя содержимое из src.
     * При возникновении исключения во время копирования все уже сконструированные объекты будут уничтожены.
     *
     * @tparam T 
     *      Тип объектов для копирования.
     * 
     * @param dst 
     *      Указатель на ранее выделенную память, куда будет производиться копирование.
     * 
     * @param src 
     *      Указатель на массив исходных объектов, которые будут скопированы.
     * 
     * @param length 
     *      Количество элементов, которые требуется скопировать.
     *
     * @throws null_pointer_exception 
     *      Eсли dst == nullptr.
     *      Если src == nullptr.
     * 
     * @throws Любое исключение, выброшенное копирующим конструктором T.
     *
     * @note Если включён JSTD_TRIVIAL_COPY_CHECK и T является тривиально копируемым,
     *       используется std::memcpy вместо конструктора копирования.
     */
    template<typename T>
    void placement_copy(T* dst, const T* src, std::size_t length) {
        JSTD_DEBUG_CODE(
            if (dst == nullptr) throw_except<null_pointer_exception>("dst is null");
            if (src == nullptr) throw_except<null_pointer_exception>("src is null");
        );
#ifdef JSTD_TRIVIAL_COPY_CHECK
        if (std::is_trivially_copyable<T>::value) {
            std::memcpy((void*) dst, (const void*) src, sizeof(T) * length);
            return;
        }
#endif//JSTD_TRIVIAL_COPY_CHECK
        
        std::size_t copied = 0;
        try {
            for (;copied < length; ++copied)
                new (dst + copied) T(src[copied]);
        } catch (...) {
            while (copied > 0)
                dst[--copied].~T();
            throw;
        }
    }

    template<typename T, typename E>
    void placement_copy(T* array, const std::initializer_list<E>& init_list) {
        JSTD_DEBUG_CODE(
            if (array == nullptr)
                throw_except<null_pointer_exception>("array is null");
        );
        std::size_t copied = 0;
        try {
            for (const E& e : init_list) {
                new(array + copied) T(e); 
                ++copied;
            }
        } catch (...) {
            std::size_t i = copied;
            while (i-- != 0)
                array[i].~T();
            throw;
        }
    }

    /**
     * Копирует элементы одного массива в другой, используя оператор присваивания.
     *
     * В отличие от placement_copy, эта функция предполагает, что объекты уже были сконструированы.
     * Копирование производится при помощи оператора присваивания operator=.
     *
     * @tparam T 
     *      Тип копируемых объектов.
     * 
     * @param dst 
     *      Указатель на целевой массив, куда будут скопированы значения.
     * 
     * @param src 
     *      Указатель на исходный массив, откуда будут взяты значения.
     * 
     * @param length 
     *      Количество копируемых элементов.
     *
     * @throws null_pointer_exception 
     *      Eсли dst == nullptr. 
     *      Если src == nullptr.
     *
     * @note Если включён JSTD_TRIVIAL_COPY_CHECK и тип T тривиально копируемый,
     *       используется std::memcpy вместо вызова `operator=`.
     */
    template<typename T>
    void copy(T* dst, const T* src, std::size_t length) {
        JSTD_DEBUG_CODE(
            if (dst == nullptr) throw_except<null_pointer_exception>("dst is null");
            if (src == nullptr) throw_except<null_pointer_exception>("src is null");
        );
#ifdef JSTD_TRIVIAL_COPY_CHECK
        if (std::is_trivially_copyable<T>::value) {
            std::memcpy(dst, src, sizeof(T) * length);
            return;
        }
#endif//JSTD_TRIVIAL_COPY_CHECK
        for (std::size_t i = 0; i < length; ++i)
            dst[i] = src[i];
    }

    /**
     * Копирует заданный диапазон элементов из одного массива в другой с дополнительными проверками границ в режиме отладки.
     * Этот метод обрабатывает как случай, когда исходный и целевой массивы перекрываются, так и когда они не перекрываются.
     * Он предназначен для предотвращения конфликтов памяти и обеспечивает безопасное копирование.
     *
     * Подробное поведение в зависимости от положения диапазонов источника и назначения:
     *      Если исходный и целевой массивы не перекрываются, элементы копируются напрямую из источника в назначение.
     *      Если массивы перекрываются, элементы копируются таким образом, чтобы данные не были перезаписаны в процессе. 
     * 
     * @tparam T
     *      Тип элементов массива. Ожидается, что T поддерживает семантику копирования и перемещения.
     * 
     * @param src 
     *      Исходный массив, из которого копируются элементы. Не должен быть равен null.
     * 
     * @param src_pos 
     *      Позиция начала копирования в исходном массиве. Должна быть допустимым индексом.
     * 
     * @param dest 
     *      Целевой массив, в который копируются элементы. Не должен быть равен null.
     * 
     * @param dst_pos 
     *      Позиция начала копирования в целевом массиве. Должна быть допустимым индексом.
     * 
     * @param length 
     *      Количество элементов, которые нужно скопировать из исходного массива в целевой массив. Не должно превышать границы ни одного из массивов.
     * 
     * @param dst_length 
     *      Длина целевого массива. Используется для проверки корректности позиции назначения.
     * 
     * @param src_length 
     *      Длина исходного массива. Используется для проверки корректности позиции исходных данных.
     * 
     * @throws null_pointer_exception 
     *      Eсли один из массивов src или dest равен null.
     * 
     * @throws index_out_of_bound_exception 
     *      Eсли src_pos или dst_pos выходят за границы соответствующих массивов.
     *      Eсли указанные диапазоны копирования выходят за пределы исходного или целевого массива.
     */
    template<typename T>
    void arraycopy(T* src, int64_t src_pos, 
                    T* dest, int64_t dst_pos, int64_t length, int64_t dst_length, int64_t src_length) {
    #ifndef NDEBUG
        check_non_null(src, "src must be != null");
        check_non_null(dest, "dest must be != null");
        check_index(src_pos,  src_length);
        check_index(dst_pos,  dst_length);
        if (src_length - length < src_pos)
            throw_except<index_out_of_bound_exception>("src range out of bounds");
        if (dst_length - length < dst_pos)
            throw_except<index_out_of_bound_exception>("dest range out of bounds");
    #endif//NDEBUG
        if (src == dest) {
            if (dst_pos > src_pos) {
                const int64_t off = dst_pos - src_pos;
                for (int64_t i = dst_pos + length; i >= src_pos + off; --i) {
                    dest[i] = src[i - off];
                }
            } 
            
            else if (dst_pos < src_pos){
                const int64_t off = src_pos - dst_pos;
                for (int64_t i = dst_pos; i < dst_pos + length; ++i) {
                    dest[i] = src[i + off];
                } 
            }
        }
    
        else {
            for (int64_t i = 0; i < length; ++i)
                dest[i + dst_pos] = src[i + src_pos];
        }
    }

    /**
     * Копирует строку типа из буфера src в буфер dst, ограничивая количество элементов dst_max.
     *
     * Функция предназначена для безопасного копирования нуль-терминированных строк любых типов (например, char, wchar_t, char16_t и т.п.).
     * 
     * Копирование прекращается, когда:
     *      Достигнут максимальный размер dst_max - 1 — в этом случае результат всегда завершается нулем.
     *      В исходной строке найден нулевой символ (конец строки).
     *
     * В режиме отладки (когда не определен NDEBUG) производится проверка указателей на nullptr и недопустимых значений dst_max.
     *
     * @tparam T 
     *      Тип символа (например, char, wchar_t, char16_t.
     * 
     * @param src 
     *      Указатель на исходную нуль-терминированную строку. 
     *      Не должен быть nullptr.
     * 
     * @param dst 
     *      Указатель на буфер назначения. 
     *      Не должен быть nullptr.
     * 
     * @param dst_max 
     *      Максимальное количество элементов, которые можно записать в @code dst, включая завершающий нулевой символ.
     * 
     * @return 
     *      Количество записанных символов. (Не включая нуль-терминатор).
     * 
     * @throws illegal_argument_exception 
     *      Если dst_max < 0.
     * 
     * @throws null_pointer_exception 
     *      Если src равен nullptr.
     *      Если dst равен nullptr.
     * 
     * @note Если dst_max == 0, функция немедленно возвращает 0, не выполняя копирования.
     */
    template<typename T>
    std::size_t ncopy(T* dst, const T* src, std::size_t dst_max) {
        JSTD_DEBUG_CODE(
            check_non_null(src, "src must be != null");
            check_non_null(dst, "dst must be != null");
            if (dst_max == 0)
                return 0;
        )
        
        std::size_t i;
        for (i = 0; i < dst_max; ++i)
        {
            
            if (i == dst_max - 1)
            {
                dst[i] = 0;
                return i;
            }
            
            dst[i] = src[i];
            
            if (src[i] == 0)
            {
                break;
            }
        }

        return i;
    }

}

#endif//JSTD_CPP_LANG_UTILS_ARRAYS_H