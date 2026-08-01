#ifndef _JSTD_CPP_LANG_UTIL_LINKED_LIST_H
#define _JSTD_CPP_LANG_UTIL_LINKED_LIST_H

#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <utility>
#include <cstdint>
#include <cassert>

namespace tc {

template<typename T>
class list_node {
    
    /**
     * 
     */
    list_node<T>* _prev;
    
    /**
     * 
     */
    list_node<T>* _next;
    
    /**
     * 
     */
    T _value;

    /**
     * deleted
     */
    list_node(const list_node&) = delete;
    
    /**
     * deleted
     */
    list_node& operator= (const list_node&) = delete;

public:
    
    /**
     * 
     */
    list_node();
    
    /**
     * 
     */
    template<typename _T>
    list_node(_T&&);

    /**
     * 
     */
    list_node(list_node<T>&&);
    
    /**
     * 
     */
    list_node<T>& operator= (list_node<T>&&);
    
    /**
     * 
     */
    template<typename _T>
    void set_value(_T&& v);

    /**
     * 
     */
    T& get_value();
    
    /**
     * 
     */
    const T& get_value() const;

    /**
     * 
     */
    void set_next(list_node<T>* next);
    
    /**
     * 
     */
    void set_prev(list_node<T>* prev);

    /**
     * 
     */
    list_node<T>* get_next();
    
    /**
     * 
     */
    list_node<T>* get_prev();

    /**
     * 
     */
    const list_node<T>* get_next() const;
    
    /**
     * 
     */
    const list_node<T>* get_prev() const;
};

    template<typename T>
    list_node<T>::list_node() : 
    _prev(nullptr),
    _next(nullptr),
    _value() {

    }
    
    template<typename T>
    template<typename _T>
    list_node<T>::list_node(_T&& t) : 
    _prev(nullptr),
    _next(nullptr),
    _value(std::forward<_T>(t)) {

    }

    template<typename T>
    list_node<T>::list_node(list_node<T>&& n) :
    _prev(n._prev),
    _next(n._next),
    _value(std::move(n._value)) {
        n._prev = nullptr;
        n._next = nullptr;
    }
    
    template<typename T>
    list_node<T>& list_node<T>::operator= (list_node<T>&& n) {
        if (&n != this) {
            _prev = n._prev;
            _next = n._next;
            _value = std::move(n._value);

            n._prev = nullptr;
            n._next = nullptr;
        }
        return *this;
    }

    template<typename T>
    template<typename _T>
    void list_node<T>::set_value(_T&& v) {
        _value = std::forward<_T>(v);
    }

    template<typename T>
    T& list_node<T>::get_value() {
        return _value;
    }

    template<typename T>
    const T& list_node<T>::get_value() const {
        return _value;
    }

    template<typename T>
    void list_node<T>::set_next(list_node<T>* next) {
        _next = next;
    }

    template<typename T>
    void list_node<T>::set_prev(list_node<T>* prev) {
        _prev = prev;
    }

    template<typename T>
    list_node<T>* list_node<T>::get_next() {
        return _next;
    }

    template<typename T>
    list_node<T>* list_node<T>::get_prev() {
        return _prev;
    }

    template<typename T>
    const list_node<T>* list_node<T>::get_next() const {
        return _next;
    }

    template<typename T>
    const list_node<T>* list_node<T>::get_prev() const {
        return _prev;
    }

/**
 * @class linked_list
 * Шаблонный класс двусвязного списка с поддержкой пользовательского аллокатора.
 * 
 * Предоставляет функциональность стандартного двусвязного списка, включая вставку, удаление,
 * доступ по индексу, а также стековые операции. Поддерживает выделение памяти через внешний аллокатор.
 * 
 * @tparam T Тип элементов, хранящихся в списке.
 */
template<typename T>
class linked_list {
    
    /**
     * Указатель на аллокатор, используемый для выделения памяти под узлы.
     */
    tca::base_allocator*    _allocator; 
    
    /**
     * Указатель на первый элемент списка.
     */
    list_node<T>*           _head;
    
    /**
     * Указатель на последний элемент списка.
     */
    list_node<T>*           _tail;
    
    /**
     * Количество элементов в списке.
     */
    std::size_t                 _size;

    /**
     * Выделяет память и создаёт новый узел списка.
     * 
     * @tparam _T 
     *      Тип значения (поддерживает perfect forwarding).
     * 
     * @param value 
     *      Значение, которое будет помещено в новый узел.
     * 
     * @return 
     *      Указатель на созданный узел.
     */
    template<typename _T>
    list_node<T>* new_node(_T&& value);

    /**
     * Удаляет узел и освобождает память.
     * 
     * @param node 
     *      Узел, который следует удалить.
     */
    void delete_node(list_node<T>* node);

    /**
     * Возвращает указатель на узел по индексу.
     * 
     * @param idx 
     *      Индекс (0-based).
     * @return 
     *      Указатель на найденный узел.
     */
    list_node<T>* node_at(std::size_t idx);

    /**
     * Константная версия node_at.
     * 
     * @param idx 
     *      Индекс (0-based).
     * @return 
     *      Константный указатель на найденный узел.
     */
    const list_node<T>* node_at(std::size_t idx) const;

    /**
     * Удаляет узел из связей (не освобождает память).
     * 
     * @param node 
     *      Узел, который необходимо удалить из списка.
     */
    void unlink(list_node<T>* node);

public:

    /**
     * 
     */
    static const std::size_t null_val = ~((std::size_t) 0);

    /**
     * Конструктор с пользовательским аллокатором.
     * 
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     */
    linked_list(tca::base_allocator* allocator = tca::get_default_allocator());

    /**
     * Конструктор с инициализирующим листом..
     * 
     * @param init_list
     *      Ссылка на инициализирующий список для заполнения этого связного списка.
     * 
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     */
    linked_list(const std::initializer_list<T>& init_list, tca::base_allocator* allocator = tca::get_default_allocator());

    /**
     * Копирующий конструктор.
     * 
     * @param other 
     *      Список для копирования.
     */
    linked_list(const linked_list<T>& other);

    /**
     * Перемещающий конструктор.
     * @param other 
     *      Список, ресурсы которого будут перемещены.
     */
    linked_list(linked_list<T>&& other);

    /**
     * Копирующее присваивание.
     * 
     * @param other 
     *      Список для копирования.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    linked_list<T>& operator=(const linked_list<T>& other);

    /**
     * Перемещающее присваивание.
     * 
     * @param other 
     *      Список, ресурсы которого будут перемещены.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    linked_list& operator=(linked_list<T>&& other);

    /**
     * Деструктор. Очищает список и освобождает память.
     */
    ~linked_list();

    /**
     * 
     *      Удаляет все элементы из списка.
     */
    void clear();

    /**
     * Добавляет элемент в конец списка.
     * 
     * @tparam _T 
     *      Тип передаваемого значения.
     * @param value 
     *      Значение, которое нужно добавить.
     */
    template<typename _T> void add(_T&& value);

    /**
     * Добавляет элемент в начало списка.
     * 
     * @tparam _T 
     *      Тип передаваемого значения.
     * 
     * @param value 
     *      Значение, которое нужно добавить.
     */
    template<typename _T> void add_first(_T&& value);

    /**
     * Добавляет элемент в конец списка (эквивалент add()).
     * 
     * @tparam _T 
     *      Тип передаваемого значения.
     * 
     * @param value 
     *      Значение, которое нужно добавить.
     */
    template<typename _T> void add_last(_T&& value);

    /**
     * Добавляет элемент по указанному индексу.
     * 
     * @tparam _T 
     *      Тип передаваемого значения.
     * 
     * @param idx 
     *      Индекс, по которому нужно вставить значение.
     * 
     * @param value 
     *      Значение для вставки.
     */
    template<typename _T> void add(std::size_t idx, _T&& value);

    /**
     * Удаляет элемент по индексу.
     * 
     * @param idx 
     *      Индекс элемента.
     * @param _return 
     *      [optional] Указатель, куда будет записано удалённое значение.
     */
    void remove_at(std::size_t idx, T* _return = nullptr);

    /**
     * Удаляет первый элемент.
     * 
     * @param _return 
     *      [optional] Указатель, куда будет записано удалённое значение.
     */
    void remove_first(T* _return = nullptr);

    /**
     * Удаляет последний элемент.
     * 
     * @param _return 
     *      [optional] Указатель, куда будет записано удалённое значение.
     */
    void remove_last(T* _return = nullptr);

    /**
     * Удаляет первое вхождение значения из списка.
     * 
     * @param v Значение для удаления.
     * 
     * @param _return 
     *      [optional] Указатель, куда будет записано удалённое значение.
     */
    void remove(const T& v, T* _return = nullptr);

    /**
     * Добавляет элемент в конец (аналог push_back).
     * 
     * @tparam _T 
     *      Тип передаваемого значения.
     * @param value 
     *      Значение для добавления.
     */
    template<typename _T> void push(_T&& value);

    /**
     * Удаляет последний элемент (аналог pop_back).
     * 
     * @param _return 
     *      [optional] Указатель, куда будет записано удалённое значение.
     */
    void pop(T* _return = nullptr);

    /**
     * Возвращает количество элементов в списке.
     * 
     * @return 
     *      Размер списка.
     */
    std::size_t size() const;

    /**
     * Проверяет, пуст ли список.
     * 
     * @return 
     *      true, если список пуст.
     */
    bool is_empty() const;

    /**
     * Возвращает ссылку на элемент по индексу.
     * 
     * @param idx 
     *      Индекс (0-based).
     * @return 
     *      Ссылка на элемент.
     */
    T& at(std::size_t idx);

    /**
     * Константная версия at().
     * 
     * @param idx 
     *      Индекс (0-based).
     * @return 
     *      Константная ссылка на элемент.
     */
    const T& at(std::size_t idx) const;

    /**
     * Проверяет, содержится ли значение в списке.
     *
     * Для сравнения используется структура jstd::equal_to<T>. 
     * Чтобы метод работал корректно для пользовательских типов, 
     * необходимо специализировать jstd::equal_to<T>.
     *
     * @param value 
     *      Значение для поиска.
     * 
     * @return 
     *      true, если элемент найден, иначе false.
     */
    bool contains(const T& value) const;

    /**
     * Возвращает индекс первого вхождения элемента в списке.
     *
     * Для сравнения используется jstd::equal_to<T>. Если элемент не найден, возвращается linked_list<T>::null_val.
     *
     * @param value 
     *      Значение для поиска.
     * 
     * @return 
     *      Индекс (0-based), либо linked_list<T>::null_val, если элемент не найден.
     */
    std::size_t index_of(const T& value) const;

    /**
     * Создаёт глубокую копию списка.
     *
     * Все элементы копируются в новый список. Если передан пользовательский аллокатор,
     * он будет использован для выделения памяти в новом списке. В противном случае —
     * используется текущий аллокатор списка.
     *
     * @param allocator 
     *      (опционально) Указатель на пользовательский аллокатор.
     * 
     * @return 
     *      Новый список, содержащий копии всех элементов текущего.
     */
    linked_list<T> clone(tca::base_allocator* allocator = nullptr) const;

    template<typename NODE_TYPE, typename VALUE_TYPE>
    class iterator {
        NODE_TYPE* _n;
    public:
        iterator();
        iterator(NODE_TYPE* n);
        iterator(const iterator&);
        iterator(iterator&&);
        iterator& operator= (const iterator&);
        iterator& operator= (iterator&&);
        bool operator!=(const iterator&) const;
        iterator& operator++();
        iterator operator++(int);
        VALUE_TYPE operator*() const;
    };
    
    iterator<const list_node<T>, const T&> begin() const {
        return iterator<const list_node<T>, const T&>(_head);
    }

    iterator<const list_node<T>, const T&> end() const {
        return iterator<const list_node<T>, const T&>(nullptr);
    }

    iterator<list_node<T>, T&> begin() {
        return iterator<list_node<T>, T&>(_head);
    }

    iterator<list_node<T>, T&> end() {
        return iterator<list_node<T>, T&>(nullptr);
    }
};

    template<typename T>
    template<typename _T>
    list_node<T>* linked_list<T>::new_node(_T&& t) {
#ifndef NDEBUG
        if (_allocator == nullptr)
            throw_except<illegal_state_exception>("allocator must be != null");
#endif//NDEBUG
        void* mem = _allocator->allocate_align(sizeof(list_node<T>), alignof(list_node<T>));
        if (mem == nullptr)
            throw_except<out_of_memory_error>("Out of memory");
        return new (mem) list_node<T>(std::forward<_T>(t));
    }
    
    template<typename T>
    void linked_list<T>::delete_node(list_node<T>* n) {
#ifndef NDEBUG
        if (_allocator == nullptr)
            throw_except<illegal_state_exception>("allocator must be != null");
#endif//NDEBUG
        n->~list_node();
        _allocator->deallocate(n, sizeof(list_node<T>));
    }

    template<typename T>
    linked_list<T>::linked_list(tca::base_allocator* allocator) :
    _allocator(allocator),
    _head(nullptr),
    _tail(nullptr),
    _size(0) {

    }

    template<typename T>
    linked_list<T>::linked_list(const std::initializer_list<T>& init_list, tca::base_allocator* allocator) : linked_list<T>(allocator) {
        for (const T& value : init_list)
            add(value);
    }

    template<typename T>
    linked_list<T>::linked_list(const linked_list<T>& other) : linked_list<T>() {
        this->operator=(other);
    }

    template<typename T>
    linked_list<T>::linked_list(linked_list<T>&& other) : 
    _allocator(other._allocator),
    _head(other._head),
    _tail(other._tail),
    _size(other._size) {
        other._allocator    = nullptr;
        other._head         = nullptr;
        other._tail         = nullptr;
        other._size         = 0;
    }

    template<typename T>
    linked_list<T>& linked_list<T>::operator=(const linked_list<T>& other) {
        if (&other != this) {
            this->operator=(std::move(other.clone(_allocator)));
        }
        return *this;
    }

    template<typename T>
    linked_list<T>& linked_list<T>::operator=(linked_list<T>&& other) {
        if (&other != this) {
            clear();
            _allocator  = other._allocator;
            _head       = other._head;
            _tail       = other._tail;
            _size       = other._size;

            other._allocator    = nullptr;
            other._head         = nullptr;
            other._tail         = nullptr;
            other._size         = 0;
        }
        return *this;
    }

    template<typename T>
    linked_list<T>::~linked_list() {
        clear();
    }

    template<typename T>
    void linked_list<T>::clear() {
        if (_allocator != nullptr) {
            for (list_node<T>* i = _head; i != nullptr; ) {
                list_node<T>* current = i;
                i = i->get_next();
                delete_node(current);
            }
            _head = nullptr;
            _tail = nullptr;
            _size = 0;
        }
    }

    template<typename T>
    template<typename _T>
    void linked_list<T>::add_last(_T&& t) {
        list_node<T>* node = new_node(std::forward<_T>(t)); 
        if (_tail == nullptr) {
            _head = _tail = node;
        } 

        else {
            _tail->set_next(node);
            node->set_prev(_tail);
            _tail = node;
        }
        ++_size;
    }

    template<typename T>
    template<typename _T>
    void linked_list<T>::add_first(_T&& t) {
        list_node<T>* node = new_node(std::forward<_T>(t)); 
        if (_head == nullptr) {
            _head = _tail = node;
        }

        else {
            node->set_next(_head);
            _head->set_prev(node);
            _head = node;
        }
        ++_size;
    }

    template<typename T>
    template<typename _T>
    void linked_list<T>::add(_T&& t) {
        add_last(std::forward<_T>(t));
    }

    template<typename T>
    template<typename _T>
    void linked_list<T>::add(std::size_t idx, _T&& t) {
        if (idx == _size) {
            add_last(std::forward<_T>(t));
            return;
        }
        
        if (idx == 0) {
            add_first(std::forward<_T>(t));
        } 
        
        else {
            list_node<T>* node = new_node(std::forward<_T>(t));
            list_node<T>* current = node_at(idx);
            assert(current != nullptr);

            list_node<T>* prev = current->get_prev();
            assert(prev != nullptr);
            
            // prev node next
            node->set_prev(prev);
            node->set_next(current);

            current->set_prev(node);
            prev->set_next(node);
        }
    }

    template<typename T>
    list_node<T>* linked_list<T>::node_at(std::size_t idx) {
        assert(idx >= 0 && idx < _size);
        if (idx < _size >> 1) {
            std::size_t i = 0;
            for (list_node<T>* n = _head; n != nullptr; n = n->get_next()) {
                if (i == idx)
                    return n;
                ++i;
            }
        } 
        
        else {
            std::size_t i = _size - 1;
            for (list_node<T>* n = _tail; n != nullptr; n = n->get_prev()) {
                if (i == idx)
                    return n;
                --i;
            }
        }
        
        return nullptr;
    }
    
    template<typename T>
    void linked_list<T>::remove_first(T* _return) {
        if (_size == 0)
            throw_except<no_such_element_exception>();
        list_node<T>* n = _head;
        assert(n != nullptr);
        
        _head = n->get_next();
        
        if (_head != nullptr) {
            _head->set_prev(nullptr);
        } 
        
        else {
            _head = _tail = nullptr;
        }
        
        if (_return != nullptr)
            *_return = std::move(n->get_value());

        --_size;
        delete_node(n);
    }

    template<typename T>
    void linked_list<T>::remove_last(T* _return) {
        if (_size == 0)
            throw_except<no_such_element_exception>();
        list_node<T>* n = _tail;
        assert(n != nullptr);
        
        _tail = n->get_prev();
        
        if (_tail != nullptr) {
            _tail->set_next(nullptr);
        } 

        else {
            _head = _tail = nullptr;
        }


        if (_return != nullptr)
            *_return = std::move(n->get_value());

        --_size;
        delete_node(n);
    }

    template<typename T>
    void linked_list<T>::unlink(list_node<T>* node) {
        assert(node != nullptr);
        
        list_node<T>* prev = node->get_prev();
        list_node<T>* next = node->get_next();
        
        if (prev == nullptr)
        {
            _head = node->get_next();
            if (_head == nullptr) {
                _head = _tail = nullptr;
            } 
            
            else {
                _head->set_prev(nullptr);
            }
        } 
        else if (next == nullptr)
        {
            _tail = node->get_prev();
            if (_tail != nullptr)
            {
                _tail->set_next(nullptr);
            }
            else
            {
                _head = _tail = nullptr;
            }
        }
        else
        {
            prev->set_next(next);
            next->set_prev(prev);
        }


        delete_node(node);
        
        --_size;
    }

    template<typename T>
    void linked_list<T>::remove_at(std::size_t idx, T* _return) {
#ifndef NDEBUG
        if (idx < 0 || idx > _size)
            throw_except<index_out_of_bound_exception>("Index %li out of bound for length %li", (long int) idx, (long int) _size);
#endif//NDEBUG
        
        if (idx == 0) {
            remove_first(_return);
            return;
        }

        else if (idx == _size) {
            remove_last(_return);
            return;
        }

        list_node<T>* current = node_at(idx);
        
        if (_return != nullptr)
            *_return = std::move(current->get_value());
        
        unlink(current);

        --_size;
    }

    template<typename T>
    void linked_list<T>::remove(const T& v, T* _return) {
        equal_to<T> equals;
        for (list_node<T>* i = _head; i != nullptr; i = i->get_next()) {
            if (equals(v, i->get_value())) {
                if (_return != nullptr){
                    *_return = std::move(i->get_value());
                    unlink(i);
                    return;
                }
            }
        }
    }

    template<typename T>
    const list_node<T>* linked_list<T>::node_at(std::size_t idx) const {
        linked_list<T>* _this = const_cast<linked_list<T>*>(this);
        return _this->node_at(idx);
    }

    template<typename T>
    std::size_t linked_list<T>::size() const {
        return _size;
    }

    template<typename T>
    bool linked_list<T>::is_empty() const {
        return _head == nullptr && _tail == nullptr;
    }
    template<typename T>
    template<typename _T> void linked_list<T>::push(_T&& t) {
        add_last(std::forward<_T>(t));
    }
    
    template<typename T>
    void linked_list<T>::pop(T* _return) {
        remove_last(_return);
    }

    template<typename T>
    T& linked_list<T>::at(std::size_t idx) {
        check_index(idx, _size);
        list_node<T>* node = node_at(idx);
        assert(node != nullptr);
        return node->get_value();
    }
    
    template<typename T>
    const T& linked_list<T>::at(std::size_t idx) const {
        check_index(idx, _size);
        const list_node<T>* node = node_at(idx);
        assert(node != nullptr);
        return node->get_value();
    }

    template<typename T>
    bool linked_list<T>::contains(const T& t) const {
        equal_to<T> equals;
        for (const list_node<T>* i = _head; i != nullptr; i = i->get_next())
            if (equals(t, i->get_value()))
                return true;
        return false;
    }

    template<typename T>
    std::size_t linked_list<T>::index_of(const T& t) const {
        std::size_t idx = 0;
        equal_to<T> equals;
        for (const list_node<T>* i = _head; i != nullptr; i = i->get_next(), ++idx) {
            if (equals(t, i->get_value()))
                return idx;
        }
        return linked_list<T>::null_val;
    }

    template<typename T>
    linked_list<T> linked_list<T>::clone(tca::base_allocator* allocator) const {
        if (allocator == nullptr) {
            if (_allocator == nullptr)
                return linked_list<T>();
            allocator = _allocator;
        }
        linked_list<T> result(allocator);

        for (const list_node<T>* i = _head; i != nullptr; i = i->get_next())
            result.add(i->get_value());

        return linked_list<T>(std::move(result));
    }












    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::iterator() : _n(nullptr) {

    }

    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::iterator(NODE_TYPE* n) : _n(n) {

    }

    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::iterator(const iterator<NODE_TYPE, VALUE_TYPE>& it) : _n(it._n) {

    }
    
    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::iterator(iterator<NODE_TYPE, VALUE_TYPE>&& it) : _n(it._n) {
        it._n = nullptr;
    }

    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    typename linked_list<T>:: template iterator<NODE_TYPE, VALUE_TYPE>& linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::operator= (const iterator<NODE_TYPE, VALUE_TYPE>& it) {
        if (&it != this) {
            _n = it._n;
        }
        return *this;
    }

    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    typename linked_list<T>:: template iterator<NODE_TYPE, VALUE_TYPE>& linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::operator= (iterator<NODE_TYPE, VALUE_TYPE>&& it) {
        if (&it != this) {
            _n = it._n;
            it._n = nullptr;
        }
        return *this;
    }
    
    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    bool linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::operator!=(const iterator<NODE_TYPE, VALUE_TYPE>& it) const {
        return _n != it._n;
    }

    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    typename linked_list<T>:: template iterator<NODE_TYPE, VALUE_TYPE>& linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::operator++() {
#ifndef NDEBUG
        if (_n == nullptr) 
            throw_except<null_pointer_exception>("_n must be != null");
#endif//NDEBUG
        _n = _n->get_next();
        return *this;
    }
    
    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    typename linked_list<T>:: template iterator<NODE_TYPE, VALUE_TYPE> linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::operator++(int) {
#ifndef NDEBUG
        if (_n == nullptr) 
            throw_except<null_pointer_exception>("_n must be != null");
#endif//NDEBUG
        iterator<NODE_TYPE, VALUE_TYPE> tmp(*this);
        _n = _n->get_next();
        return tmp;
    }
    
    template<typename T>
    template<typename NODE_TYPE, typename VALUE_TYPE>
    VALUE_TYPE linked_list<T>::iterator<NODE_TYPE, VALUE_TYPE>::operator*() const {
#ifndef NDEBUG
        if (_n == nullptr) 
            throw_except<null_pointer_exception>("_n must be != null");
#endif//NDEBUG
        return _n->get_value();
    }
}
#endif//_JSTD_CPP_LANG_UTIL_LINKED_LIST_H