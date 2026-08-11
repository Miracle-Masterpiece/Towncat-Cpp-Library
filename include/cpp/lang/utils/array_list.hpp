#ifndef JSTD_CPP_LANG_UTILS_ARRAY_LIST_H
#define JSTD_CPP_LANG_UTILS_ARRAY_LIST_H

#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/utils/comparator.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/traits/cv_traits.hpp>
#include <cstdint>
#include <utility>
#include <initializer_list>
#include <cassert>

namespace tc
{

/**
 * Динамический массив с возможностью управления аллокатором.
 * 
 * Класс представляет собой обобщённую реализацию списка на основе массива, аналогичную {@code std::vector}. 
 * Поддерживает автоматическое расширение, вставку, удаление, доступ по индексу, а также сортировку.
 *
 * @tparam E 
 *      Тип элементов, хранящихся в списке.
 */
template<typename E>
class array_list {
    /**
     * Значение по умолчанию для начальной вместимости массива.
     */
    static const int DEFAULT_CAPACITY = 10;

    /**
     * Указатель на пользовательский аллокатор памяти.
     */
    tca::allocator* m_allocator;

    /**
     * Указатель на массив элементов.
     */
    E* m_data;

    /**
     * Текущая вместимость массива (сколько элементов может быть размещено без перераспределения).
     */
    std::size_t m_capacity;

    /**
     * Текущее количество элементов в списке.
     */
    std::size_t m_size;

    /**
     * Вызывает деструкторы для элементов в диапазоне [start, end).
     *
     * @param start Начальный индекс (включительно).
     * @param end Конечный индекс (не включительно).
     */
    void call_destructors(std::size_t start, std::size_t end);

    /**
     * Освобождает память и сбрасывает внутреннее состояние.
     */
    void cleanup();

    /**
     * Увеличивает вместимость массива, обычно с коэффициентом роста 1.5x.
     */
    void grow();

public:

    /**
     * 
     */
    static const std::size_t null_val = ~((std::size_t) 0);

    /**
     * Создаёт пустой список без аллокатора.
     */
    array_list(tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Создаёт список с заданным аллокатором и начальной вместимостью.
     *
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     * 
     * @param init_capacity 
     *      Начальная вместимость (по умолчанию — 10).
     */
    explicit array_list(std::size_t init_capacity, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Создаёт список с заданным аллокатором и инициализирующим листом.
     *
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     * 
     * @param init_list
     *      Инициилизирующий список из которого будут добавлены элементы в этот список.
     */
    array_list(const std::initializer_list<E>& init_list, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Создаёт новый список, копируя содержимое из переданного списка.
     *
     * @param list 
     *      Список, содержимое которого будет скопировано.
     */
    array_list(const array_list<E>& list);

    /**
     * Создаёт новый список, перемещая содержимое из переданного списка.
     * 
     * @param list 
     *      Список, содержимое которого будет перемещено.
     */
    array_list(array_list<E>&& list);

    /**
     * Копирует содержимое из другого списка в текущий.
     * 
     * @param list 
     *      Список, содержимое которого будет скопировано.
     * 
     * @return 
     *      Ссылка на текущий объект после копирования.
     */
    array_list<E>& operator=(const array_list<E>& list);

    /**
     * Перемещает содержимое из другого списка в текущий.
     *
     * @param list 
     *      Список, содержимое которого будет перемещено.
     * 
     * @return 
     *      Ссылка на текущий объект после перемещения.
     */
    array_list<E>& operator=(array_list<E>&& list);

    /**
     * Удаляет все элементы и освобождает ресурсы.
     */
    ~array_list();

    /**
     * Добавляет элемент в конец списка.
     *
     * @tparam _E 
     *      Тип добавляемого объекта (может быть rvalue).
     * 
     * @param e 
     *      Элемент для добавления.
     */
    template<typename _E>
    void add(_E&& e);

    /**
     * Вставляет элемент по заданному индексу.
     *
     * @tparam _E 
     *      Тип добавляемого объекта (может быть rvalue).
     * 
     * @param idx 
     *      Индекс, по которому нужно вставить.
     * 
     * @param e 
     *      Элемент для вставки.
     */
    template<typename _E>
    void add(std::size_t idx, _E&& e);

    /**
     * Возвращает последний индекс указанного элемента, 
     * используя компаратор jstd::equal_to<E> для проверки равенства.
     * 
     * @param e 
     *      Элемент для поиска.
     * 
     * @return 
     *      Индекс, если найден; иначе array_list<E>::null_val.
     */
    std::size_t last_index_of(const E& e) const;

    /**
     * Возвращает первый индекс указанного элемента, 
     * используя компаратор jstd::equal_to<E> для проверки равенства.
     *
     * @param e 
     *      Элемент для поиска.
     * 
     * @return 
     *      Индекс, если найден; иначе array_list<T>::null_val.
     */
    std::size_t index_of(const E& e) const;

    /**
     * Проверяет, содержится ли указанный элемент в списке, 
     * используя компаратор jstd::equal_to<E> для проверки равенства.
     * 
     * @param e 
     *      Элемент, наличие которого нужно проверить.
     * 
     * @return 
     *      true, если элемент присутствует в спискe, иначе false.
     */
    bool contains(const E& e) const {
        return index_of(e) != null_val;
    }

    /**
     * Удаляет первое вхождение указанного элемента.
     *
     * @param e 
     *      Элемент для удаления.
     * 
     * @return 
     *      true, если удаление произошло.
     */
    bool remove(const E& e);

    /**
     * Удаляет элемент по индексу, с возможностью сохранить удалённое значение.
     *
     * @param idx 
     *      Индекс для удаления.
     * 
     * @param ret 
     *      (Необязательно) указатель, куда сохранить удалённый элемент.
     * 
     * @return 
     *      true, если удаление прошло успешно.
     */
    bool remove_at(std::size_t idx, E* ret = nullptr);

    /**
     * Быстрое удаление по индексу без сдвига элементов, может нарушить порядок.
     *
     * @param idx 
     *      Индекс для удаления.
     * 
     * @param ret 
     *      (Необязательно) указатель, куда сохранить удалённый элемент.
     * 
     * @return 
     *      true, если удаление прошло успешно.
     * 
     * @throws index_out_of_bound_exception 
     *      Если индекс не допустим.
     */
    bool fast_remove_at(std::size_t idx, E* ret = nullptr);

    /**
     * Заменяет элемент по указанному индексу новым значением.
     * 
     * @tparam _E 
     *      Тип присваиваемого значения, допускающий как rvalue-, так и lvalue-ссылки.
     * 
     * @param idx 
     *      Индекс заменяемого элемента (должен находиться в диапазоне от 0 до {@code size()}).
     * 
     * @param e 
     *      Новое значение, которое будет присвоено элементу.
     * 
     * @param ret_old_value 
     *      (Необязательно) Указатель, по которому сохраняется старое значение, если не {@code nullptr}.
     * 
     * @return 
     *      true, если элемент был успешно заменён, иначе false.
     * 
     * @throws index_out_of_bound_exception 
     *      Если индекс не допустим.
     */
    template<typename _E>
    bool set(std::size_t idx, _E&& e, E* ret_old_value = nullptr);

    /**
     * Возвращает ссылку на элемент по индексу.
     *
     * @param idx 
     *      Индекс элемента.
     * 
     * @return 
     *      Ссылка на элемент.
     * 
     * @throws index_out_of_bound_exception 
     *      Если индекс не допустим.
     */
    E& at(std::size_t idx) const;

    /**
     * Возвращает количество элементов в списке.
     *
     * @return 
     *      Размер списка.
     */
    std::size_t size() const;

    /**
     * Гарантирует, что список может вместить не менее {@code new_capacity} элементов без перераспределения.
     *
     * @param new_capacity 
     *      Желаемая вместимость.
     * 
     * @throws illegal_state_exception 
     *      Eсли аллокатор не задан.
     * 
     * @throws illegal_argument_exception 
     *      Eсли вместимость отрицательная.
     * 
     * @throws out_of_memory_error 
     *      Eсли не удалось выделить память.
     */
    void reserve(std::size_t new_capacity);

    /**
     * Удаляет все элементы, но не освобождает память.
     */
    void clear();

    /**
     * Проверяет, пустой ли контейнер.
     * true - если список пуст, иначе - false.
     */
    bool is_empty() const;

    /**
     * Проверяет, являются ли элементы внутри списка равны.
     * 
     * @param other
     *      Список, с которым будет производиться сравнение.
     */
    bool equals(const array_list<E>& other) const;
    
    /**
     * Возвращает хеш-код списка.
     * 
     * @note
     *      Для вычесления хеш-кода необходимо, чтобы объекты, которые хранятся внутри списка,
     *      определяли специализацию структуры jstd::hash_for;
     * 
     * Прототип:
     *      template<>
     *      struct jstd::hash_for<T> {
     *          std::size_t operator() (const T& obj) const;
     *      };
     * 
     * @return
     *      Хеш-код всех элементов.
     */
    std::size_t hashcode() const;

    /**
     * Возвращает аллокатор, связанный с этим array_list
     */
    tca::allocator* get_allocator() const {
        return m_allocator;
    }

    /**
     * Возвращает указатель на сырые данные.
     */
    E* data() {
        return m_data;
    }

    /**
     * Возвращает указатель на сырые данные. (Константная версия)
     */
    const E* data() const {
        return m_data;   
    }

    /**
     * Сортирует элементы списка с помощью простого алгоритма вставки.
     * 
     * По умолчанию используется компаратор {@code compare_to<E>}, но может быть передан и пользовательский.
     *
     * @tparam COMPARATOR_T 
     *      Тип компаратора, реализующего оператор {@code ()(const E&, const E&)}.
     */
    template<typename COMPARATOR_T = compare_to<E>>
    void intersect_sort();

    /**
     * Выполняет бинарный поиск заданного элемента в отсортированном списке.
     *
     * Метод предполагает, что список отсортирован в порядке, определённом компаратором {@code COMPARATOR_T},
     * который по умолчанию равен {@code compare_to<E>}.
     * Поиск осуществляется методом деления пополам. Возвращает индекс найденного элемента
     * или array_list<E>::null_val, если элемент не найден.
     *
     * Требование: список должен быть предварительно отсортирован тем же компаратором,
     * иначе результат будет неопределён.
     *
     * @tparam COMPARATOR_T 
     *      Тип компаратора, поддерживающего вызов {@code compare(a, b)}, 
     *      возвращающий отрицательное значение, если {@code a < b}, 
     *      ноль — если {@code a == b}, и положительное — если {@code a > b}.
     * 
     * @param searched 
     *      Элемент, который необходимо найти.
     * 
     * @return 
     *      Индекс найденного элемента, или array_list<T>::null_val, если элемент не найден.
     */
    template<typename COMPARATOR_T = compare_to<E>>
    std::size_t binary_search(const E& searched) const;
    
    template<typename DATA_TYPE, typename VALUE_TYPE>
    class iterator {
        DATA_TYPE*  m_begin;
        DATA_TYPE*  m_end;
    public:
        iterator(DATA_TYPE* begin, DATA_TYPE* end);
        iterator(const iterator<DATA_TYPE, VALUE_TYPE>&);
        iterator(iterator<DATA_TYPE, VALUE_TYPE>&&);
        iterator<DATA_TYPE, VALUE_TYPE>& operator= (const iterator<DATA_TYPE, VALUE_TYPE>&);
        iterator<DATA_TYPE, VALUE_TYPE>& operator= (iterator<DATA_TYPE, VALUE_TYPE>&&);
        ~iterator();
        VALUE_TYPE& operator*() const;
        iterator<DATA_TYPE, VALUE_TYPE>& operator++();
        iterator<DATA_TYPE, VALUE_TYPE> operator++(int);
        bool operator!=(const iterator<DATA_TYPE, VALUE_TYPE>& it) const;
        bool operator==(const iterator<DATA_TYPE, VALUE_TYPE>& it) const;
    };


    iterator<const E, const E&> begin() const {
        return iterator<const E, const E&>(m_data, m_data + m_size);
    }

    iterator<const E, const E&> end() const {
        return iterator<const E, const E&>(m_data + m_size, m_data + m_size);
    }

    iterator<E, E&> begin() {
        return iterator<E, E&>(m_data, m_data + m_size);
    }

    iterator<E, E&> end() {
        return iterator<E, E&>(m_data + m_size, m_data + m_size);
    }

};

    template<typename E>
    array_list<E>::array_list(tca::allocator* allocator) : m_allocator(allocator), m_data(nullptr), m_capacity(0), m_size(0) {

    }

    template<typename E>
    array_list<E>::array_list(std::size_t init_capacity, tca::allocator* allocator) : 
    m_allocator(allocator), 
    m_data(nullptr), 
    m_capacity(0), 
    m_size(0) {
        if (init_capacity > 0)
            reserve(init_capacity);
    }

    template<typename E>
    array_list<E>::array_list(const std::initializer_list<E>& init_list, tca::allocator* allocator) : array_list<E>(0, allocator) {
        if (init_list.size() > 0)
            reserve(init_list.size());
        try {
            for (const E& e : init_list)
                add(e);
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename E>
    array_list<E>::array_list(const array_list<E>& list) : array_list<E>() {
        m_allocator = list.m_allocator;
        
        E* data = allocate_and_copy_n(list.data(), list.size(), m_allocator);
        if (!data)
            throw_except<out_of_memory_error>("Out of memory");

        m_data      = data;
        m_capacity  = list.size();
        m_size      = list.size();
    }
    
    template<typename E>
    array_list<E>::array_list(array_list<E>&& list) : 
    m_allocator(list.m_allocator),
    m_data(list.m_data),
    m_capacity(list.m_capacity),
    m_size(list.m_size) {
        list.m_allocator    = nullptr;
        list.m_data         = nullptr;
        list.m_capacity     = 0;
        list.m_size         = 0;
    }
    
    template<typename E>
    array_list<E>& array_list<E>::operator= (const array_list<E>& list) {
        if (&list != this)
        {
            E* data = allocate_and_copy_n(list.data(), list.size(), m_allocator);
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
            m_allocator->deallocate(const_cast<typename remove_cv<E>::type*>(m_data));
            m_data      = data;
            m_capacity  = list.size();
            m_size      = list.size();
        }
        return *this;
    }
    
    template<typename E>
    array_list<E>& array_list<E>::operator= (array_list<E>&& list) {
        if (&list != this) {
            std::swap(m_allocator,  list.m_allocator);
            std::swap(m_data,       list.m_data);
            std::swap(m_capacity,   list.m_capacity);
            std::swap(m_size,       list.m_size);
        }
        return *this;
    }
    
    template<typename E>
    template<typename COMPARATOR_T>
    std::size_t array_list<E>::binary_search(const E& searched) const {
        if (is_empty())
            return array_list<E>::null_val;
        
        COMPARATOR_T compare_to;
        std::size_t start   = 0;
        std::size_t end     = m_size - 1;
        while (start <= end) {
            
            const std::size_t mid       = (end - start) / 2 + start;
            const E& mid_value          = at(mid);
            const int compare_result    = compare_to(searched, mid_value);
            
            if (compare_result == 0)
                return mid;
            else if (compare_result < 0)
                end     = mid - 1;
            else
                start   = mid + 1;
        }
        
        return array_list<E>::null_val;
    }

    template<typename E>
    template<typename SORT_TYPE>
    void array_list<E>::intersect_sort() {
        SORT_TYPE compare_to;
        for (std::size_t i = 1; i < m_size; ++i) {
            std::size_t j = i;
            while (j > 0) {
                E& a = m_data[j - 1];
                E& b = m_data[j - 0];
                if (compare_to(a, b) > 0) {
                    std::swap(a, b);
                    --j;
                    continue;
                }
                break;
            }
        }
    }

    template<typename E>
    template<typename _E>
    bool array_list<E>::set(std::size_t idx, _E&& e, E* ret_old_value) {
        check_index(idx, m_size);
        if (ret_old_value != nullptr)
            *ret_old_value = std::move(m_data[idx]);
        m_data[idx] = std::forward<_E>(e);
        return true;
    }

    template<typename E>
    void array_list<E>::grow() {
        const std::size_t new_capacity = m_capacity > 0 ? m_capacity + (m_capacity >> 1) : DEFAULT_CAPACITY;
        reserve(new_capacity);
    }

    template<typename E>
    std::size_t array_list<E>::size() const {
        return m_size;
    }

    template<typename E>
    E& array_list<E>::at(std::size_t idx) const {
        check_index(idx, m_size);
        return m_data[idx];
    }
    
    template<typename E>
    void array_list<E>::clear() {
        call_destructors(0, m_size);
        m_size = 0;
    }

    template<typename E>
    std::size_t array_list<E>::last_index_of(const E& e) const {
        equal_to<E> equals;
        for (std::size_t i = m_size; i > 0; --i)
            if (equals(e, m_data[i - 1]))
                return i - 1;
        return array_list<E>::null_val;
    }

    template<typename E>
    std::size_t array_list<E>::index_of(const E& e) const {
        equal_to<E> equals;
        for (std::size_t i = 0; i < m_size; ++i)
            if (equals(e, m_data[i]))
                return i;
        return array_list<E>::null_val;
    }
    
    template<typename E>
    bool array_list<E>::remove(const E& e) {
        std::size_t finded_index = index_of(e);
        if (finded_index == array_list<E>::null_val)
            return false;
        return remove_at(finded_index, nullptr);
    }
    
    template<typename E>
    bool array_list<E>::remove_at(std::size_t idx, E* ret) {
        check_index(idx, m_size);

        if (ret != nullptr)
            *ret = std::move(m_data[idx]);

        m_data[idx].~E();

        for (std::size_t i = idx; i < m_size - 1; ++i)
            new(m_data + i) E(std::move(m_data[i + 1]));

        --m_size;
        
        return true;
    }
    
    template<typename E>
    bool array_list<E>::fast_remove_at(std::size_t idx, E* ret) {
        check_index(idx, m_size);
        if (idx == m_size - 1)
            return remove_at(idx, ret);
        
        std::swap(m_data[idx], m_data[m_size - 1]);
        
        if (ret != nullptr)
            *ret = std::move(m_data[m_size - 1]);
        
        m_data[m_size - 1].~E();
        --m_size;
        
        return true;
    }

    template<typename E>
    template<typename _E>
    void array_list<E>::add(_E&& e) {
        if (m_size + 1 > m_capacity) 
            grow();
        new (m_data + m_size) E(std::forward<_E>(e));
        ++m_size;
    }

    template<typename E>
    template<typename _E>
    void array_list<E>::add(std::size_t idx, _E&& e) {
        check_index(idx, m_size + 1);
        
        if (m_size + 1 > m_capacity) 
            grow();
        
        for (std::size_t i = m_size; i > idx; --i)
            new(m_data + i) E(std::move(m_data[i - 1]));
        
        new(m_data + idx) E(std::forward<_E>(e));
        
        ++m_size;
    }

    template<typename E>
    bool array_list<E>::is_empty() const {
        return m_size == 0;
    }

    template<typename E>
    void array_list<E>::reserve(std::size_t new_capacity) {
        JSTD_DEBUG_CODE(
            if (m_allocator == nullptr)
                throw_except<illegal_state_exception>("allocator is null");
        )

        if (new_capacity == 0) {
            cleanup();
            return;
        }

        E* new_data = reinterpret_cast<E*>(
                                            m_allocator->allocate_align(sizeof(E) * new_capacity, alignof(E)) 
                                                                                                            );
        if (new_data == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");
        
        if (m_data != nullptr)
        {
            if (new_capacity >= m_size)
            {
                for (std::size_t i = 0; i < m_size; ++i)
                    new(new_data + i) E(std::move(m_data[i]));
            }
            else
            {
                for (std::size_t i = 0; i < new_capacity; ++i)
                    new(new_data + i) E(std::move(m_data[i]));
                call_destructors(new_capacity, m_size);
                m_size = new_capacity;
            }
        }
    
        m_allocator->deallocate(m_data);
        m_capacity  = new_capacity;
        m_data      = new_data;
    }
    
    template<typename E>
    void array_list<E>::call_destructors(std::size_t start, std::size_t end) {
        assert(start <= end);
        assert(end >= start);
        while (end > start)
            m_data[--end].~E();
    }

    template<typename E>
    void array_list<E>::cleanup() {
        if (m_allocator != nullptr && m_data != nullptr)
        {
            call_destructors(0, m_size);
            m_allocator->deallocate((void*) m_data);
        }   
        m_data      = nullptr;
        m_capacity  = 0;
        m_size      = 0;
    }

    template<typename E>
    bool array_list<E>::equals(const array_list<E>& other) const {
        if (size() != other.size())
            return false;
        if (data() && other.data())
        {
            return objects::equals(begin(), end(), other.begin(), other.end(), equal_to<E>());
        }
        else
        {
            return data() == other.data();
        }
    }

    template<typename E>
    std::size_t array_list<E>::hashcode() const {
        if (size() == 0)
            return 0;
        return objects::hashcode(begin(), end(), hash_for<E>());
    }

    template<typename E>
    array_list<E>::~array_list() {
        cleanup();
    }

    /**
     * ==========================================================================================================================================
     */
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::iterator(DATA_TYPE* begin, DATA_TYPE* end) :
    m_begin(begin),
    m_end(end) {

    }

    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::iterator(const iterator<DATA_TYPE, VALUE_TYPE>& it) :
    m_begin(it.m_begin),
    m_end(it.m_end) {

    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::iterator(iterator<DATA_TYPE, VALUE_TYPE>&& it) :
    m_begin(it.m_begin),
    m_end(m_end) {
        it.m_begin  = nullptr;
        it.m_end    = nullptr;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    typename array_list<E>:: template iterator<DATA_TYPE, VALUE_TYPE>& 
    array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator= (const iterator<DATA_TYPE, VALUE_TYPE>& it) {
        if (&it != this)
        {
            m_begin = it.m_begin;
            m_end   = it.m_end;
        }
        return *this;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    typename array_list<E>:: template iterator<DATA_TYPE, VALUE_TYPE>& 
    array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator= (iterator<DATA_TYPE, VALUE_TYPE>&& it) {
        if (&it != this)
        {
            std::swap(m_begin, it.m_begin);
            std::swap(m_end, it.m_end);
        }
        return *this;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::~iterator() {
        
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    VALUE_TYPE& array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator*() const {
        if (m_begin >= m_end)
            throw_except<index_out_of_bound_exception>("iterator out of bound");
        
        JSTD_DEBUG_CODE(check_non_null(m_begin, "data is null"););
        
        return *m_begin;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    typename array_list<E>:: template iterator<DATA_TYPE, VALUE_TYPE>& array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator++() {
        ++m_begin;
        return *this;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    typename array_list<E>:: template iterator<DATA_TYPE, VALUE_TYPE> array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator++(int) {
        iterator<DATA_TYPE, VALUE_TYPE> it = *this;
        ++m_begin;
        return it;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    bool array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator!=(const iterator<DATA_TYPE, VALUE_TYPE>& it) const {
        return m_begin != it.m_begin || m_end != it.m_end;
    }
    
    template<typename E>
    template<typename DATA_TYPE, typename VALUE_TYPE>
    bool array_list<E>::iterator<DATA_TYPE, VALUE_TYPE>::operator==(const iterator<DATA_TYPE, VALUE_TYPE>& it) const {
        return m_begin == it.m_begin && m_end == it.m_end;
    }
}
#endif//JSTD_CPP_LANG_UTILS_ARRAY_LIST_H