#ifndef JSTDLIB_CPP_LANG_ARRAY_H_
#define JSTDLIB_CPP_LANG_ARRAY_H_

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <allocators/allocator.hpp>
#include <utility>
#include <new>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <initializer_list>
#include <cpp/lang/traits/cv_traits.hpp>

namespace tc
{
    
/**
 *  Динамический массив с поддержкой пользовательского аллокатора.
 *  Позволяет не только аллоцировать массив, 
 *  но и оборачивать уже существующий указатель, предствляя его, как массив.
 * 
 * @tparam T Тип элементов массива.
 */
template<typename T>
class array {
protected:
    /**
     * Аллокатор, управляющий памятью.
     */
    tca::allocator* _allocator;
    
    /**
     * Указатель на выделенный блок памяти.
     */
    T* _data;
    
    /**
     * Освобождает память, вызывает деструкторы элементов и сбрасывает всё в ноль.
     *
     * Если массив пуст или уже освобождён, ничего не делает.
     */
    void free();    

public:
    /**
     * Количество элементов в массиве.
     */
    std::size_t length;
    
    /**
     * Создаёт пустой массив.
     *
     * Память не выделяется, данные отсутствуют, просто заглушка.
     */
    array();

    /**
     * Создаёт обёртку над уже выделенной памятью.
     * Новая память не выделяется, просто обёртка над уже существующим блоком памяти.
     * При уничтожении объектов деструкторы вызваны не будут.
     */
    array(T* buf, std::size_t bufsize);

    /**
     * Создаёт массив заданного размера, используя переданный аллокатор.
     *
     * @param allocator 
     *      Аллокатор для управления памятью.
     * 
     * @param sz 
     *      Количество элементов в массиве.
     */
    array(std::size_t sz, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Создаёт массив размером инициализирующего листа. 
     * И копирует элементы в массив.
     * 
     * @param allocator 
     *      Аллокатор для управления памятью.
     * 
     * @param init_list
     *      Список для инициализации массива.
     */
    array(const std::initializer_list<T>& init_list, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Конструктор копирования.
     *
     * Создаёт копию массива. Выделяет новую память и копирует элементы.
     *
     * @param a
     *      Исходный массив.
     * 
     * @throws out_of_memory_error
     *      Если не удалось выделить память.
     */
    array(const array<T>& a);

    /**
     * Конструктор перемещения.
     *
     * Забирает данные из другого массива, оставляя его пустым.
     * Быстро, потому что ничего не копирует.
     *
     * @param a
     *      Исходный массив.
     */
    array(array<T>&& a);

    /**
     * Оператор присваивания (копирование).
     *
     * Освобождает текущие данные и создаёт копию переданного массива.
     * Если памяти не хватает — кидает исключение.
     *
     * @param a
     *      Массив для копирования.
     * 
     * @return
     *      Ссылка на текущий массив.
     */
    array<T>& operator=(const array<T>& a);

    /**
     * Оператор присваивания (перемещение).
     *
     * Освобождает текущие данные и просто забирает всё у другого массива.
     * Никакого копирования, просто перекидывание указателей.
     *
     * @param a
     *      Массив, откуда забираем данные.
     * 
     * @return
     *      Ссылка на текущий массив.
     */
    array<T>& operator=(array<T>&& a);

    /**
     * Деструктор.
     *
     * Освобождает память, уничтожает элементы.
     */
    ~array();

    /**
     * Оператор доступа по индексу.
     *
     * Возвращает элемент массива. Если индекс неверный — будет плохо.
     *
     * @param idx
     *      Индекс элемента.
     * 
     * @return
     *      Ссылка на элемент.
     * 
     * @throws index_out_of_bound_exception
     *      Если индекс выходит за границы массива.
     */
    T& operator[](std::size_t idx);

    /**
     * Оператор доступа по индексу (константный).
     *
     * Возвращает элемент, но не даёт его изменять.
     *
     * @param idx
     *      Индекс элемента.
     * 
     * @return
     *      Константная ссылка на элемент.
     * 
     * @throws index_out_of_bound_exception
     *      Если индекс выходит за границы массива.
     */
    const T& operator[](std::size_t idx) const;

    /**
     * Возвращает указатель на начало массива.
     * 
     * @return
     *      Указатель на первый элемент массива.
     */
    T* data() const;

    /**
     * Устанавливает весь массив в переданное значение.
     * 
     * @param value 
     *      Значение, которым будет заполнен массив.
     */
    void set(const T& value);

    /**
     * Возвращает аллокатор, владеющей памятью этого массива.
     * 
     * @return
     *      Указатель на аллокатор.
     */
    tca::allocator* get_allocator() const;

    /**
     * Возвращает хеш-код массива.
     * Элементы массива должны определять специализацию структуры для подсчёта хеш-кода.
     * 
     * //Шаблон
     * template<typename T>
     * struct hash_for;
     * 
     * //Специализация
     * template<>
     * struct hash_for<T>{
     *  std::size_t operator()(const T& t) const;
     * };
     * 
     * @return 
     *      Хеш-код массива.
     */
    std::size_t hashcode() const;

    /**
     * Проверяет, является ли переданный массив равен этому массиву.
     * Элементы массива должны определять специализацию структуры для подсчёта хеш-кода.
     * 
     * //Шаблон
     * template<typename T>
     * struct equal_to;
     * 
     * //Специализация
     * template<>
     * struct equal_to<T>{
     *  bool operator()(const T& t1, const T& t2) const;
     * };
     * 
     * @return 
     *      true - если массивы равны, иначе false.
     */
    bool equals(const array<T>& a) const;
    
    /**
     * Возвращает итератор на первый элемент.
     */
    const T* begin() const;
    
    /**
     * Возвращает итератор на последний элемент.
     */
    const T* end() const;
    
    /**
     * Возвращает итератор на первый элемент.
     */
    T* begin();
    
    /**
     * Возвращает итератор на последний элемент.
     */
    T* end();
};

    template<typename T>
    array<T>::array() : _allocator(nullptr), _data(nullptr), length(0) {}

    template<typename T>
    array<T>::array(T* buf, std::size_t bufsize) : _allocator(nullptr), _data(buf), length(bufsize) {

    }

    template<typename T>
    array<T>::array(std::size_t sz, tca::allocator* allocator) : array<T>() {
        JSTD_DEBUG_CODE(check_non_null(allocator));
        
        _allocator = allocator;
        
        if (sz > 0) {
            T* data = (T*) allocator->allocate_align(sizeof(T) * sz, alignof(T));
            if (data == nullptr)
                throw_except<out_of_memory_error>("Out of memory!");
            uninitialized_construct_n(const_cast<typename remove_cv<T>::type*>(data), sz);
            _data       = data;
            length      = sz;
        }
    }

    template<typename T>
    array<T>::array(const std::initializer_list<T>& init_list, tca::allocator* allocator) {
        JSTD_DEBUG_CODE(check_non_null(allocator));
        
        _allocator  = allocator;
        T* data     = nullptr;
        std::size_t sz = init_list.size();

        if (sz > 0)
        {
            data = allocate_and_copy_n(init_list, _allocator);
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
        }

        _data       = data;
        length      = sz;  
    }

    template<typename T>
    array<T>::array(const array<T>& a) : array<T>() {
        _allocator  = a._allocator;
        
        T* data         = nullptr;
        std::size_t len = a.length;

        if (len > 0)
        {
        
            data = allocate_and_copy_n(a.data(), a.length, _allocator); 
            if (!data)
                throw_except<out_of_memory_error>("Out of memory");
        }
        
        _data   = data;
        length  = len;
    }
    
    template<typename T>
    array<T>::array(array<T>&& a) : _allocator(a._allocator), _data(a._data), length(a.length) {
        a._allocator    = nullptr;
        a._data         = nullptr;
        a.length        = 0;
    }
    
    template<typename T>
    array<T>& array<T>::operator= (const array<T>& a) {
        if (&a == this)
            return *this;

        std::size_t new_len = a.length;
        T* new_data         = nullptr;
        if (a.length > 0)
        {
            new_data = allocate_and_copy_n(a.data(), new_len, _allocator);
            if (!new_data)
                throw_except<out_of_memory_error>("Out of memory");
        }
        
        if (_data)
        {
            deallocate_and_destroy_n(_data, length, _allocator);
        }

        _data   = new_data;
        length  = new_len;
        
        return *this;
    }
    
    template<typename T>
    array<T>& array<T>::operator= (array<T>&& a) {
        if (&a != this)
        {
            std::swap(_allocator,   a._allocator);
            std::swap(_data,        a._data);
            std::swap(length,       a.length);
        }
        return *this;
    }
    
    template<typename T>
    array<T>::~array() {
        if (_allocator != nullptr && _data != nullptr)
        {
            deallocate_and_destroy_n(_data, length, _allocator);
        }
    }

    template<typename T>
    T& array<T>::operator[] (std::size_t idx) {
        check_index(idx, length);
        return _data[idx];
    }
    
    template<typename T>
    const T& array<T>::operator[] (std::size_t idx) const {
        check_index(idx, length);
        return _data[idx];
    }

    template<typename T>
    T* array<T>::data() const {
        return _data;
    }

    template<typename T>
    void array<T>::set(const T& value) {
        for (std::size_t i = 0, len = length; i < len; ++i)
            _data[i] = value;
    }

    template<typename T>
    std::size_t array<T>::hashcode() const {
        return (_data == nullptr || length == 0) ? 
                                                    0 : objects::hashcode(begin(), end(), hash_for<T>());
    }

    template<typename T>
    tca::allocator* array<T>::get_allocator() const {
        return _allocator;
    }

    template<typename T>
    bool array<T>::equals(const array<T>& a) const {
        if (length != a.length)
        {
            return false;
        }
        if (data() != nullptr && a.data() != nullptr)
        {
            return objects::equals(begin(), end(), a.begin(), a.end(), equal_to<T>());
        }
        else
        {
            return data() == a.data();
        }
    }

    template<typename T>
    const T* array<T>::begin() const {
        return _data;
    }
    
    template<typename T>
    const T* array<T>::end() const {
        return _data + length;
    }
    
    template<typename T>
    T* array<T>::begin() {
        return _data;
    }
    
    template<typename T>
    T* array<T>::end() {
        return _data + length;
    }
}
#endif//JSTDLIB_CPP_LANG_ARRAY_H_