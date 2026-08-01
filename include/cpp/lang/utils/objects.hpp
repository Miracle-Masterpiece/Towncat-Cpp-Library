#ifndef JSTD_CPP_LANG_UTILS_OBJECTS_H
#define JSTD_CPP_LANG_UTILS_OBJECTS_H

#include <cstdint>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/cond_compile.hpp>

namespace tc
{

class null_pointer_exception;
class illegal_argument_exception;

template<typename T>
void throw_except(const char* format, ...);

namespace objects
{

    /**
     * Вычисляет хеш-код для массива элементов типа T.
     * 
     * Эта функция принимает указатель на массив и вычисляет его хеш-код.
     * 
     * Если длина массива не указана (len == -1), функция будет
     * вычислять хеш-код, пока не встретит нулевой байт (подразумевается, что массив заканчивается нулём, как C-строка).
     * 
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param array 
     *      Указатель на массив элементов типа T.
     * 
     * @param len 
     *      Длина массива.
     * 
     * @return 
     *      Хеш-код массива.
     * 
     * @example
     * 
     * char str[] = "Hello, C++";
     * uint64_t hash = hashcode(str, std::strlen(str));
     * std::cout << "Hash: " << hash << "\n";
     */
    template<typename T, typename HASH_FOR = hash_for<T>>
    std::size_t hashcode(const T* array, std::size_t len) {
        JSTD_DEBUG_CODE(
            if (array == nullptr)
                throw_except<null_pointer_exception>("array must be != null");        
        );
        std::size_t hash = (std::size_t) 0xcbf29ce484222325;
        const HASH_FOR hash_calculater;
        for (std::size_t i = 0; i < len; ++i)
            hash = (hash ^ hash_calculater(array[i])) * 0x100000001b3;
        return hash;
    }
    
    /**
     * @see 
     *      template<typename T>
     *      std::size_t hashcode(const T* array, std::size_t len);
     */
    template<>
    std::size_t hashcode<float>(const float* array, std::size_t len);

    /**
     * @see 
     *      template<typename T>
     *      std::size_t hashcode(const T* array, int64_t len);
     */
    template<>
    std::size_t hashcode<double>(const double* array, std::size_t len);

    /**
     * Сравнивает два массива элементов типа T на равенство.
     * 
     * Функция принимает два указателя на массивы и их длину,
     * после чего посимвольно сравнивает их содержимое.
     * Если массивы полностью совпадают, возвращает true.
     * Если есть хоть одно несовпадение — false.
     * 
     * @tparam T 
     *      Тип элементов массива.
     * 
     * @param a1 
     *      Указатель на первый массив. Должен быть != nullptr.
     * 
     * @param a2 
     *      Указатель на второй массив. Должен быть != nullptr.
     * 
     * @param len 
     *      Длина массивов, подлежащая сравнению.
     * 
     * @return 
     *      true, если массивы идентичны, иначе false.
     * 
     * @throws null_pointer_exception 
     *      Если один из указателей равен nullptr.
     * 
     * @example
     *      char str1[] = "Hello, C++";
     *      char str2[] = "Hello, C++";
     *      bool result = equals(str1, str2, 10);
     *      std::cout << (result ? "equal" : "not equal") << std::endl;
     */
    template<typename T, typename EQUAL_TO = equal_to<T>>
    bool equals(const T* a1, const T* a2, std::size_t len) {
        JSTD_DEBUG_CODE(
            if (a1 == nullptr) throw_except<null_pointer_exception>("a1 must be != null");
            if (a2 == nullptr) throw_except<null_pointer_exception>("a2 must be != null");
        );
        const EQUAL_TO equals_to;
        for (std::size_t i = 0; i < len; ++i)
            if (!equals_to(a1[i], a2[i]))
                return false;
        return true;
    }
}//namespace objects

}//namespace jstd

#endif//JSTD_CPP_LANG_UTILS_OBJECTS_H