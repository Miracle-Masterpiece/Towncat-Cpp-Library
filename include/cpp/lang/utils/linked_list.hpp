#ifndef _JSTD_CPP_LANG_UTIL_LINKED_LIST_H
#define _JSTD_CPP_LANG_UTIL_LINKED_LIST_H

#include <cpp/lang/traits/cv_traits.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/hash.hpp>
#include <cpp/lang/common.hpp>
#include <utility>
#include <cstdint>
#include <cassert>

namespace tc
{

namespace list
{

/**
 * @brief Iterator for linked list nodes
 * 
 * @tparam NODE_TYPE
 *      Type of node (const or non-const)
 * 
 * @tparam VALUE_TYPE
 *      Type of value (const or non-const)
 * 
 * Provides forward iteration over list elements with pointer-like syntax.
 * Supports pre-increment, post-increment, and dereference operators.
 * 
 * @example
 * for (auto it = list.begin(); it != list.end(); ++it) {
 *     std::cout << *it << std::endl;
 * }
 */
template<typename NODE_TYPE, typename VALUE_TYPE>
class node_iterator {
    NODE_TYPE* _n;
public:
    node_iterator() : _n(nullptr) {

    }
    
    node_iterator(NODE_TYPE* n) : _n(n) {

    }

    bool operator!= (const node_iterator<NODE_TYPE, VALUE_TYPE>& it) const {
        return _n != it._n;
    }
    
    bool operator== (const node_iterator<NODE_TYPE, VALUE_TYPE>& it) const {
        return _n == it._n;
    }

    node_iterator<NODE_TYPE, VALUE_TYPE>& operator++() {
        JSTD_DEBUG_CODE(check_non_null(_n, "Iterator out of bound"));
        _n = _n->get_next();
        return *this;
    }

    node_iterator<NODE_TYPE, VALUE_TYPE> operator++(int) {
        JSTD_DEBUG_CODE(check_non_null(_n, "Iterator out of bound"));
        node_iterator<NODE_TYPE, VALUE_TYPE> tmp(*this);
        _n = _n->get_next();
        return tmp;
    }

    VALUE_TYPE& operator*() const {
        JSTD_DEBUG_CODE(check_non_null(_n, "Iterator out of bound"));
        return _n->get_value();
    }
};
}


/**
 * Internal node structure for doubly-linked list
 * 
 * @tparam T
 *      Type of stored value
 * 
 */
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
     * @brief Default constructor value-initializes stored object
     */
    list_node();
    
    /**
     * @brief Constructs node with forwarded value
     * 
     * @tparam _T
     *      Type of value (deduced from argument)
     * 
     * @param t
     *      Value to store (forwarded perfectly)
     */
    template<typename _T>
    list_node(_T&&);

    /**
     * @brief Move constructor transfers ownership of node content
     * 
     * @param n
     *      Source node to move from
     */
    list_node(list_node<T>&&);
    
    /**
     * @brief Move assignment operator
     * 
     * @param n
     *      Source node to move from
     * 
     * @return
     *      Reference to this node
     */
    list_node<T>& operator= (list_node<T>&&);
    
    /**
     * @brief Sets value using perfect forwarding
     * 
     * @tparam _T
     *      Type of value (deduced from argument)
     * 
     * @param v
     *      Value to store
     */
    template<typename _T>
    void set_value(_T&& v);

    /**
     * @brief Returns reference to stored value (non-const)
     * 
     * @return
     *      T& Reference to value
     */
    T& get_value();
    
    /**
     * @brief Returns const reference to stored value
     * 
     * @return
     *      const T& Const reference to value
     */
    const T& get_value() const;

    /**
     * @brief Sets pointer to next node
     * 
     * @param
     *      next Pointer to next node
     */
    void set_next(list_node<T>* next);
    
    /**
     * @brief Sets pointer to previous node
     * 
     * @param prev
     *      Pointer to previous node
     */
    void set_prev(list_node<T>* prev);

    /**
     * @brief Returns pointer to next node (non-const)
     * 
     * @return
     *      list_node<T>* Next node pointer
     */
    list_node<T>* get_next();
    
    /**
     * @brief Returns pointer to previous node (non-const)
     * 
     * @return
     *      list_node<T>* Previous node pointer
     */
    list_node<T>* get_prev();

    /**
     * @brief Returns const pointer to next node
     * 
     * @return
     *      const list_node<T>* Const next node pointer
     */
    const list_node<T>* get_next() const;
    
    /**
     * @brief Returns const pointer to previous node
     * 
     * @return
     *      const list_node<T>* Const previous node pointer
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

template<typename T>
class linked_list {
    
    typedef typename remove_cv<T>::type         Tvalue;
    typedef list_node<Tvalue>                   Node;
    typedef list::node_iterator<Node, T>        Iterator;
    typedef list::node_iterator<Node, const T>  ConstIterator;

    tca::allocator* _allocator; 
    Node* _head;
    Node* _tail;
    std::size_t _size;

    template<typename _T>
    Node* new_node(_T&& value);

    void delete_node(Node* node);
    Node* node_at(std::size_t idx) const;
    void unlink(Node* node);

public:

    linked_list(tca::allocator* allocator = tca::get_default_allocator());
    linked_list(std::size_t cnt, tca::allocator* allocator = tca::get_default_allocator());
    linked_list(std::initializer_list<T> init_list, tca::allocator* allocator = tca::get_default_allocator());
    linked_list(const linked_list<T>& other);
    linked_list(linked_list<T>&& other);
    linked_list<T>& operator=(const linked_list<T>& other);
    linked_list& operator=(linked_list<T>&& other);
    ~linked_list();
    void clear();
    
    template<typename _T>
    void add(_T&& value);

    template<typename _T>
    void add_first(_T&& value);

    template<typename _T>
    void add_last(_T&& value);

    template<typename _T>
    void add(std::size_t idx, _T&& value);

    void remove_at(std::size_t idx, Tvalue* _return = nullptr);
    void remove_first(Tvalue* _return = nullptr);
    void remove_last(Tvalue* _return = nullptr);
    void remove(const T& v, Tvalue* _return = nullptr);
    
    template<typename _T>
    void push(_T&& value);
    
    void pop(Tvalue* _return = nullptr);
    std::size_t size() const;

    tca::allocator* get_allocator() const {
        return _allocator;
    }

    bool is_empty() const;

    T& at(std::size_t idx);

    const T& at(std::size_t idx) const;

    bool contains(const T& value) const;
    
    std::size_t index_of(const T& value) const;
    
    bool equals(const linked_list<T>& list) const;
    
    std::size_t hashcode() const;

    ConstIterator begin() const {
        return ConstIterator(_head);
    }

    ConstIterator end() const {
        return ConstIterator(nullptr);
    }

    Iterator begin() {
        return Iterator(_head);
    }

    Iterator end() {
        return Iterator(nullptr);
    }
};

    template<typename T>
    template<typename _T>
    typename linked_list<T>::Node* linked_list<T>::new_node(_T&& t) {
        
        Node* node = (Node*) _allocator->allocate_align(sizeof(Node), alignof(Node));
        if (!node)
            throw_except<out_of_memory_error>("Out of memory");
        
        try {
            new (node) Node(std::forward<_T>(t));
        } catch (...) {
            _allocator->deallocate(node);
            throw;
        }
        
        return node;
    }
    
    template<typename T>
    void linked_list<T>::delete_node(Node* n) {
        n->~list_node();
        _allocator->deallocate(n);
    }

    template<typename T>
    linked_list<T>::linked_list(tca::allocator* allocator) :
    _allocator(allocator),
    _head(nullptr),
    _tail(nullptr),
    _size(0) {

    }

    template<typename T>
    linked_list<T>::linked_list(std::size_t cnt, tca::allocator* allocator) : linked_list<T>(allocator) {
        if (cnt == 0)
            return;   
        try {
            while (cnt > 0)
            {
                add(T());            
                --cnt;
            }
        } catch (...) {
            for (Node* n = _tail; n != nullptr;)
            {
                Node* prev = n->get_prev();
                delete_node(prev);
                n = prev;
            }
            throw;
        }
    }

    template<typename T>
    linked_list<T>::linked_list(std::initializer_list<T> init_list, tca::allocator* allocator) : linked_list<T>(allocator) {
        try {
            for (const T& e : init_list)
            {
                add(e);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename T>
    linked_list<T>::linked_list(const linked_list<T>& other) : linked_list<T>(other._allocator) {
        try {
            for (const T& e : other)
            {
                add(e);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename T>
    linked_list<T>::linked_list(linked_list<T>&& other) : 
    _allocator(other._allocator),
    _head(other._head),
    _tail(other._tail),
    _size(other._size) {
        other._head         = nullptr;
        other._tail         = nullptr;
        other._size         = 0;
    }

    template<typename T>
    linked_list<T>& linked_list<T>::operator=(const linked_list<T>& other) {
        if (&other != this)
        {
            Node* head = nullptr;
            Node* tail = nullptr;
            try {

                for (const T& e : other)
                {
                    Node* new_ = new_node(e);
                    if (head == nullptr)
                    {
                        head = tail = new_;
                    }
                    else
                    {
                        new_->set_prev(tail);
                        tail->set_next(new_);
                        tail = new_;
                    }
                }

                clear();
                
                _head = head;
                _tail = tail;
                _size = other.size();

            } catch (...) {
                for (Node* n = tail; n != nullptr; )
                {
                    Node* current = n;
                    n = n->get_prev();
                    delete_node(current);
                }
                throw;
            }
        }
        return *this;
    }

    template<typename T>
    linked_list<T>& linked_list<T>::operator=(linked_list<T>&& other) {
        if (&other != this) 
        {
            _allocator = other._allocator;
            std::swap(_head,        other._head);
            std::swap(_tail,        other._tail);
            std::swap(_size,        other._size);
        }
        return *this;
    }

    template<typename T>
    linked_list<T>::~linked_list() {
        clear();
    }

    template<typename T>
    void linked_list<T>::clear() {
        for (Node* i = _tail; i != nullptr; ) {
            Node* current = i;
            i = i->get_prev();
            delete_node(current);
        }
        _head = nullptr;
        _tail = nullptr;
        _size = 0;
    }

    template<typename T>
    template<typename _T>
    void linked_list<T>::add_last(_T&& t) {
        Node* node = new_node(std::forward<_T>(t)); 
        if (_tail == nullptr)
        {
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
        Node* node = new_node(std::forward<_T>(t)); 
        if (_head == nullptr)
        {
            _head = _tail = node;
        }
        else
        {
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
            Node* node = new_node(std::forward<_T>(t));
            Node* current = node_at(idx);
            assert(current != nullptr);

            Node* prev = current->get_prev();
            assert(prev != nullptr);
            
            // prev node next
            node->set_prev(prev);
            node->set_next(current);

            current->set_prev(node);
            prev->set_next(node);
        }
    }

    template<typename T>
    typename linked_list<T>::Node* linked_list<T>::node_at(std::size_t idx) const {
        assert(idx < size());
        if (idx < size() >> 1)
        {
            std::size_t i = 0;
            for (Node* n = _head; n != nullptr; n = n->get_next())
            {
                if (i == idx)
                    return n;
                ++i;
            }
        } 
        else
        {
            std::size_t i = size() - 1;
            for (Node* n = _tail; n != nullptr; n = n->get_prev())
            {
                if (i == idx)
                    return n;
                --i;
            }
        }
        
        return nullptr;
    }
    
    template<typename T>
    void linked_list<T>::remove_first(Tvalue* _return) {
        if (_size == 0)
            throw_except<no_such_element_exception>();
        Node* n = _head;
        assert(n != nullptr);
        
        _head = n->get_next();
        
        if (_head != nullptr)
        {
            _head->set_prev(nullptr);
        } 
        else
        {
            _head = _tail = nullptr;
        }
        
        if (_return != nullptr)
            *_return = std::move(n->get_value());

        --_size;
        delete_node(n);
    }

    template<typename T>
    void linked_list<T>::remove_last(Tvalue* _return) {
        if (_size == 0)
            throw_except<no_such_element_exception>();
        Node* n = _tail;
        assert(n != nullptr);
        
        _tail = n->get_prev();
        
        if (_tail != nullptr)
        {
            _tail->set_next(nullptr);
        } 
        else 
        {
            _head = _tail = nullptr;
        }

        if (_return != nullptr)
            *_return = std::move(n->get_value());

        --_size;
        delete_node(n);
    }

    template<typename T>
    void linked_list<T>::unlink(Node* node) {
        assert(node != nullptr);
        
        Node* prev = node->get_prev();
        Node* next = node->get_next();
        
        if (prev == nullptr)
        {
            _head = node->get_next();
            if (_head == nullptr)
            {
                _head = _tail = nullptr;
            } 
            else
            {
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
    void linked_list<T>::remove_at(std::size_t idx, Tvalue* _return) {
        JSTD_DEBUG_CODE(
            if (idx > _size)
                throw_except<index_out_of_bound_exception>("Index %li out of bound for length %li", (long int) idx, (long int) _size);
        )

        if (idx == 0)
        {
            remove_first(_return);
            return;
        }
        else if (idx == _size)
        {
            remove_last(_return);
            return;
        }

        Node* current = node_at(idx);
        
        if (_return != nullptr)
            *_return = std::move(current->get_value());
        
        unlink(current);

        --_size;
    }

    template<typename T>
    void linked_list<T>::remove(const T& v, Tvalue* _return) {
        equal_to<T> equals;
        for (Node* i = _head; i != nullptr; i = i->get_next()) {
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
    void linked_list<T>::pop(Tvalue* _return) {
        remove_last(_return);
    }

    template<typename T>
    T& linked_list<T>::at(std::size_t idx) {
        check_index(idx, _size);
        Node* node = node_at(idx);
        assert(node != nullptr);
        return node->get_value();
    }
    
    template<typename T>
    const T& linked_list<T>::at(std::size_t idx) const {
        check_index(idx, _size);
        const Node* node = node_at(idx);
        assert(node != nullptr);
        return node->get_value();
    }

    template<typename T>
    bool linked_list<T>::contains(const T& t) const {
        equal_to<T> equals;
        for (const Node* i = _head; i != nullptr; i = i->get_next())
            if (equals(t, i->get_value()))
                return true;
        return false;
    }

    template<typename T>
    std::size_t linked_list<T>::index_of(const T& t) const {
        std::size_t idx = 0;
        equal_to<T> equals;
        for (const Node* i = _head; i != nullptr; i = i->get_next(), ++idx) {
            if (equals(t, i->get_value()))
                return idx;
        }
        return npos();
    }

    template<typename T>
    bool linked_list<T>::equals(const linked_list<T>& list) const {
        if (size() != list.size())
            return false;
        return objects::equals(begin(), end(), list.begin(), list.end(), equal_to<T>());
    }
    
    template<typename T>
    std::size_t linked_list<T>::hashcode() const {
        return objects::hashcode(begin(), end(), hash_for<T>());
    }
}
#endif//_JSTD_CPP_LANG_UTIL_LINKED_LIST_H