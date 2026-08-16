#ifndef AEAB2A40_A1EB_41B4_BDCC_0FACEDA58362
#define AEAB2A40_A1EB_41B4_BDCC_0FACEDA58362

#include <cpp/lang/utils/cond_compile.hpp>

#ifndef NDEBUG
 #include <cpp/lang/exceptions.hpp>
#endif //NDEBUG

namespace tc
{

/**
 * Шаблонный класс для представления массива.
 * 
 * Класс предоставляет функции для безопасного доступа с проверкой границ в DEBUG сборке.
 */
template<typename T>
class array_view {
    /**
     * Указатель на первый элемент массива.
     */
    T* m_data;
    
    /**
     * Размер массива.
     */
    std::size_t m_length;
public:

    /**
     * Инициализирует этот view значениями по-умолчанию.
     * Указатель устанавливается в nullptr, а длина 0.
     */
    array_view() : m_data(nullptr), m_length(0) {}

    /**
     * Создаёт view над существующим массивом элементов.
     *
     * @param p
     *      Указатель на первый элемент массива.
     *
     * @param length
     *      Количество элементов массива.
     * 
     * @example
     *      int ints[] = {1, 2, 3, 4}
     *      tc::array_view<int> arr(ints, sizeof(ints) / sizeof(*ints));
     *      std::cout << arr[2] << "\n"; //output 3
     */
    array_view(T* p, std::size_t length) : m_data(p), m_length(length) {}

    /**
     * @return
     *     Длина view массива.
     */
    std::size_t length() const {
        return m_length;
    }

    /**
     * Оператор доступа по индексу.
     * 
     * @param idx
     *      Индекс элемента.
     * 
     * @return
     *      Ссылка на элемент массива.
     * 
     * @throws index_out_of_bound_exception (в DEBUG сборке)
     *      Если idx >= length()
     * 
     * @throws null_pointer_exception
     *      Если view не содержит указатель на массив.
     */
    T& operator[] (std::size_t idx) {
        JSTD_DEBUG_CODE(
            check_non_null(m_data);
            check_index(idx, length());
        );
        return m_data[idx];
    }
    
    /**
     * Оператор доступа по индексу.
     * 
     * @param idx
     *      Индекс элемента.
     * 
     * @return
     *      Константная ссылка на элемент массива.
     * 
     * @throws index_out_of_bound_exception (в DEBUG сборке)
     *      Если idx >= length()
     * 
     * @throws null_pointer_exception
     *      Если view не содержит указатель на массив.
     */
    const T& operator[] (std::size_t idx) const {
        JSTD_DEBUG_CODE(
            check_non_null(m_data);
            check_index(idx, length());
        );
        return m_data[idx];
    }
};

}

#endif /* AEAB2A40_A1EB_41B4_BDCC_0FACEDA58362 */
