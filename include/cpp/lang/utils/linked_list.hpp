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
 * Iterator for linked list nodes
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
     * Default constructor value-initializes stored object
     */
    list_node();
    
    /**
     * Constructs node with forwarded value
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
     * Move constructor transfers ownership of node content
     * 
     * @param n
     *      Source node to move from
     */
    list_node(list_node<T>&&);
    
    /**
     * Move assignment operator
     * 
     * @param n
     *      Source node to move from
     * 
     * @return
     *      Reference to this node
     */
    list_node<T>& operator= (list_node<T>&&);
    
    /**
     * Sets value using perfect forwarding
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
     * Returns reference to stored value (non-const)
     * 
     * @return
     *      T& Reference to value
     */
    T& get_value();
    
    /**
     * Returns const reference to stored value
     * 
     * @return
     *      const T& Const reference to value
     */
    const T& get_value() const;

    /**
     * Sets pointer to next node
     * 
     * @param
     *      next Pointer to next node
     */
    void set_next(list_node<T>* next);
    
    /**
     * Sets pointer to previous node
     * 
     * @param prev
     *      Pointer to previous node
     */
    void set_prev(list_node<T>* prev);

    /**
     * Returns pointer to next node (non-const)
     * 
     * @return
     *      list_node<T>* Next node pointer
     */
    list_node<T>* get_next();
    
    /**
     * Returns pointer to previous node (non-const)
     * 
     * @return
     *      list_node<T>* Previous node pointer
     */
    list_node<T>* get_prev();

    /**
     * Returns const pointer to next node
     * 
     * @return
     *      const list_node<T>* Const next node pointer
     */
    const list_node<T>* get_next() const;
    
    /**
     * Returns const pointer to previous node
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

/**
 * A class that implements a doubly linked list using a polymorphic allocator.
 * 
 * Supported Features:
 *      Adding (to the end, to the beginning, to the middle)
 *      Delete (from the end, from the beginning, from the middle)
 *      Checking an existing element.
 *      Size check.
 */
template<typename T>
class linked_list {    
    /**
     * Non const T value
     */
    typedef typename remove_cv<T>::type         Tvalue;
public:
    /**
    * The node type used to store linked_list<T> elements.
    *
    * The type is declared publicly so it can be used.
    * when creating specialized allocators, such as pool_allocator,
    * customized allocation of linked_list<T> nodes.
    */
    typedef list_node<Tvalue> node;
private:
    /**
     * 
     */
    typedef list::node_iterator<node, T>        Iterator;

    /**
     * 
     */
    typedef list::node_iterator<node, const T>  ConstIterator;

    /**
     * Memory allocator for memory management
     */
    tca::allocator* const _allocator; 
    
    /**
     * Start of list
     */
    node* _head;

    /**
     *End of list
     */
    node* _tail;

    /**
     * List size
     */
    std::size_t _size;

    /**
     * Allocates memory for a list node and initializes it.
     * 
     * A strong guarantee of exceptions is maintained.
     * 
     * @return
     *      Pointer to node.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any exception from the element's move or copy constructor.
     */
    template<typename _T>
    node* new_node(_T&& value);

    /**
     * Deinitializes the list node.
     * Calls the destructor and frees the memory.
     * 
     * @param n
     *      Pointer to the node that needs to be deallocate.
     */
    void delete_node(node* node);
    
    /**
     * Returns the node by index.
     * If the index is greater than or equal to the size, assert() is triggered
     */
    node* node_at(std::size_t idx) const;
    
    /**
     * Removes a node from the list.
     */
    void unlink(node* node);

public:

    /**
     * Initializes this linked_list with default values.
     * 
     * @param allocator (Optional)
     *      Memory allocator that will be used to allocate and free memory.
     *      If allocator is not specified, 'tca::get_default_allocator()' will be used
     */
    linked_list(tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Initializes this linked_list and adds the passed number of elements,
     * which are initialized by the default constructor.
     * 
     * @param cnt
     *      Number of items added to the list.
     * 
     * @param allocator
     *      Memory allocator, to allocate memory and free memory of this linked_list
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any element constructor exception.
     */
    linked_list(std::size_t cnt, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Initializes this linked_list with std::initializer_list
     * 
     * @param init_list
     *      Initializing list from which elements will be copied.
     * 
     * @param allocator
     *      Memory allocator for allocating and freeing memory of this linked_list.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any element copy constructor exception.
     */
    linked_list(std::initializer_list<T> init_list, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Copies the allocator and data from 'list' to this linked_list
     * 
     * @param list
     *      List from which elements will be copied.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::linked_list<int> ints = {1, 2, 3, 4, 5};
     *      tc::linked_list<int> copied = ints;
     * 
     *      assert(copied.size() == ints.size());
     *      assert(copied.get_allocator() == ints.get_allocator());
     */
    linked_list(const linked_list<T>& other);
    
    /**
     * Passes to the current linked_list the linked resources from 'list'
     * 
     * After moving 'list' retains its allocator and remains valid,
     * but in unspecified state.
     * 
     * @param list
     *      List from where the data will be moved.
     * 
     * @example
     *      tc::linked_list<int> ints = {1, 2, 3, 4, 5};
     *      tc::linked_list<int> moved = std::move(ints);
     * 
     *      assert(moved.size() == 5);
     *      assert(moved.get_allocator() == ints.moved());
     */
    linked_list(linked_list<T>&& other);
    
    /**
     * Copies data from 'list' to this linked_list
     * 
     * The allocator of the current object does not change!
     * 
     * Strong exceptions guarantee.
     * If an exception occurs during copying, the object is not changed.
     * 
     * @param list
     *      List from which elements will be copied.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::linked_list<int> ints = {1, 2, 3, 4, 5};
     *      tc::linked_list<int> copied;
     * 
     *      copied = ints;
     * 
     *      assert(copied.size() == ints.size());
     */
    linked_list<T>& operator=(const linked_list<T>& other);
    
    /**
     * Moves content from 'list' to this linked_list
     * 
     * When moving, the allocator of the current object does not change!
     * After moving, the 'list' allocator does not change.
     * The 'list' object remains in a valid but unspecified state.
     * 
     * If the current object's allocator and 'list' are equal, 
     * memory is transferred to this object without copying.
     * 
     * If the allocators are different,
     * copying is performed using the current object's allocator.
     * 
     * Strong exceptions guarantee.
     * If an exception occurs, the object is not modified.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any exception thrown by the element's copy constructor.
     * 
     * @example
     *      tc::linked_list<int> ints = {1, 2, 3, 4, 5};
     *      tc::linked_list<int> moved;
     *      tca::allocator* alloc = moved.get_allocator();     
     * 
     *      moved = std::move(ints);
     * 
     *      assert(moved.size() == 5);
     *      assert(moved.get_allocator() == alloc);
     */
    linked_list& operator=(linked_list<T>&& other);

    /**
     * 
     */
    ~linked_list();


    /**
     * Clear this list.
     * 
     * @example
     *      tca::linked_list<int> ints = {1, 2, 3, 4, 5};
     *      std::cout << ints.size() << "\n"; //output 5
     *      ints.clear();
     *      std::cout << ints.size() << "\n"; //output 0
     */
    void clear();
    
    /**
     * Adds a new element to the end of the list.
     * 
     * Strong exceptions guarantee.
     * If an exception is thrown when adding, this linked_list is not modified.
     * 
     * @param t
     *      The object to be added to this list.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any element copy or move constructor exception.
     */
    template<typename _T>
    void add(_T&& value);

    /**
     * Adds a new element to the beginning of the list.
     * 
     * Strong exceptions guarantee.
     * If an exception is thrown when adding, this linked_list is not modified.
     * 
     * @param t
     *      The object to be added to this list.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any element copy or move constructor exception.
     */
    template<typename _T>
    void add_first(_T&& value);

    /**
    * Adds a new element to the end of the list.
    *
    * Strong exception guarantee!
    * If an exception is thrown during the addition, this linked_list will be left unchanged.
    *
    * @param t
    *       The element to add to the list.
    *
    * @param out_of_memory_error
    *       If there is not enough memory.
    *
    * @param
    *       Any exception thrown by the element's copy constructor.
    */
    template<typename _T>
    void add_last(_T&& value);

    /**
     * Adds a new element to the list at the passed index.
     * 
     * Strong exceptions guarantee.
     * If an exception is thrown when adding, this linked_list is not modified.
     * 
     * @param idx
     *      Index at which the new element will be added.
     *      If index is equal to size, the new element will be added to the end of the list.
     * 
     * @param t
     *      The object to be added to this list.
     * 
     * @throws index_out_of_bound_exception (in DEBUG build)
     *      If 'idx' > size()
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any element copy or move constructor exception.
     */
    template<typename _T>
    void add(std::size_t idx, _T&& value);

    /**
     * Removes a list element at the passed index.
     * 
     * @param idx
     *      Index by which you want to delete the element
     *      If the index is equal to the size of the list, the last element will be removed.
     * 
     * @throws index_out_of_bound_exception (In DEBUG build)
     *      If 'idx' >= size()
     * 
     */
    void remove_at(std::size_t idx);

    /**
     * Removes the first element of the list.
     * 
     * @throws no_such_element_exception (in DEBUG build)
     *      If the list is empty.
     */
    void remove_first();

    /**
     * Removes the last element of the list.
     * 
     * @throws no_such_element_exception (in DEBUG build)
     *      If the list is empty.
     */
    void remove_last();

    /**
     * Removes the first occurrence of the passed element.
     * 
     * To compare elements, tc::equal_to<T> is used
     * 
     * @return
     *      true - if the element was deleted, otherwise - false.
     */
    bool remove(const T& v);
    
    /**
     * Adds a new element to the end of the list.
     * 
     * Strong exceptions guarantee.
     * If an exception is thrown when adding, this list does not change!
     * 
     * @param t
     *      The element that is added to the list.
     * 
     * @throws out_of_memory_error
     *      If there is not enough memory.
     * 
     * @throws
     *      Any exception, copy constructor or move element.
     */
    template<typename _T>
    void push(_T&& value);

    /**
    * Removes the last element from the list.
    */
    void pop();
    
    /**
    * Returns a reference to the last element of the list.
    *
    * @return
    *       A reference to the last element of the list.
    *
    * @throws no_such_element_exception (in DEBUG build)
    *       If the list is empty.
    */
    Tvalue& get_last();

    /**
    * Returns a constant reference to the last element of the list.
    *
    * @return
    *       A reference to the last element of the list.
    *
    * @throws no_such_element_exception (in DEBUG build)
    *       If the list is empty.
    */
    const Tvalue& get_last() const;
    
    /**
    * Returns a reference to the first element of the list.
    *
    * @return
    *       A reference to the first element of the list.
    *
    * @throws no_such_element_exception (in DEBUG build)
    *       If the list is empty.
    */
    Tvalue& get_first();

    /**
    * Returns a constant reference to the first element of the list.
    *
    * @return
    *       A reference to the first element of the list.
    *
    * @throws no_such_element_exception (in DEBUG build)
    *       If the list is empty.
    */
    const Tvalue& get_first() const;

    /**
     * Returns the size of this list.
     */
    std::size_t size() const;

    /**
    * Returns the allocator for this linked_list.
    *
    * @return
    *       The allocator responsible for allocating and freeing memory for this list.
    */
    tca::allocator* get_allocator() const {
        return _allocator;
    }

    /**
     * Whether this list is empty.
     * 
     * @return
     *      Return true only if size() == 0
     */
    bool is_empty() const;

    /**
    * Returns a reference to the element at the given index.
    *
    * @param idx
    *       The element's index.
    *
    * @return
    *       A reference to the object at index 'idx'
    *
    * @throws index_out_of_bound (in DEBUG build)
    *       If 'idx' >= size()
    */
    T& at(std::size_t idx);

    /**
    * Returns a constant reference to the element at the given index.
    *
    * @param idx
    *       The element's index.
    *
    * @return
    *       A reference to the object at index 'idx'
    *
    * @throws index_out_of_bound (in DEBUG build)
    *       If 'idx' >= size()
    */
    const T& at(std::size_t idx) const;

    /**
    * Checks whether this linked_list contains the specified element.
    *
    * tc::equal_to<T> is used for comparison.
    *
    * @param t
    *       The element whose presence is to be checked.
    *
    * @return
    *       true if the list contains the 't' element, otherwise false
    */
    bool contains(const T& value) const;
    
    /**
    * Checks the index of the first occurrence of the specified element.
    *
    * tc::equal_to<T> is used for comparison.
    *
    * @param t
    *       The element to search for.
    *
    * @return
    *       The index of the element, if it is in the list.
    *       Or tc::npos() if it is not present.
    */
    std::size_t index_of(const T& value) const;
    
    /**
    * Checks if this linked_list is equal to the passed list.
    *
    * tc::equal_to<T> is used for comparison.
    *
    * @param list
    *       Another list to check.
    *
    * @return
    *       true - only if the lists have the same sizes,
    *       and the same element contents. False - otherwise.
    *
    * @example
    *       tc::linked_list<int> ints_1 = {1, 2, 3, 4, 5};
    *       tc::linked_list<int> ints_2 = {1, 2, 3, 4, 5};
    *       tc::linked_list<int> ints_3 = {2, 5, 1 5, 4};
    * 
    *       std::cout << ints_1.equals(ints_2) << "\n"; //output true 
    *       std::cout << ints_1.equals(ints_3) << "\n"; //output false 
    * 
    */
    bool equals(const linked_list<T>& list) const;
    
    /**
    * Returns the hash code of this list.
    * The hash code is generated based on all stored elements.
    *
    * tca::hash_for<T> is used to generate the hash code of elements.
    *
    * @return
    *       The hash code of this list.
    */
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
    typename linked_list<T>::node* linked_list<T>::new_node(_T&& t) {
        
        node* n = (node*) _allocator->allocate_align(sizeof(node), alignof(node));
        if (!n)
            throw_except<out_of_memory_error>("Out of memory");
        
        try {
            new (n) node(std::forward<_T>(t));
        } catch (...) {
            _allocator->deallocate(n);
            throw;
        }
        
        return n;
    }

    template<typename T>
    void linked_list<T>::delete_node(node* n) {
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
            for (std::size_t i = 0; i < cnt; ++i)
            {
                add(T());
            }
        } catch (...) {
            clear();
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
    linked_list<T>::linked_list(const linked_list<T>& list) : linked_list<T>(list.get_allocator()) {
        try {
            for (const T& e : list)
            {
                add(e);
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    template<typename T>
    linked_list<T>::linked_list(linked_list<T>&& list) : 
    _allocator(list._allocator),
    _head(list._head),
    _tail(list._tail),
    _size(list._size) {
        list._head         = nullptr;
        list._tail         = nullptr;
        list._size         = 0;
    }

    template<typename T>
    linked_list<T>& linked_list<T>::operator=(const linked_list<T>& list) {
        if (&list != this)
        {
            node* head = nullptr;
            node* tail = nullptr;
            try {

                for (const T& e : list)
                {
                    node* new_ = new_node(e);
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
                _size = list.size();

            } catch (...) {
                for (node* n = tail; n != nullptr; )
                {
                    node* current = n;
                    n = n->get_prev();
                    delete_node(current);
                }
                throw;
            }
        }
        return *this;
    }

    template<typename T>
    linked_list<T>& linked_list<T>::operator=(linked_list<T>&& list) {
        if (&list == this)
            return *this;
        if (get_allocator() == list.get_allocator()) 
        {
            std::swap(_head,        list._head);
            std::swap(_tail,        list._tail);
            std::swap(_size,        list._size);
        }
        else
        {
            *this = list;
        }
        return *this;
    }

    template<typename T>
    linked_list<T>::~linked_list() {
        clear();
    }

    template<typename T>
    void linked_list<T>::clear() {
        for (node* i = _tail; i != nullptr; ) {
            node* current = i;
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
        node* node = new_node(std::forward<_T>(t)); 
        if (_tail == nullptr)
        {
            _head = _tail = node;
        } 
        else
        {
            _tail->set_next(node);
            node->set_prev(_tail);
            _tail = node;
        }
        ++_size;
    }

    template<typename T>
    template<typename _T>
    void linked_list<T>::add_first(_T&& t) {
        node* node = new_node(std::forward<_T>(t)); 
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
        JSTD_DEBUG_CODE(
            if (idx > size())
                throw_except<index_out_of_bound_exception>("index %zu out of bound size %zu", idx, size());
        );
        if (idx == _size) {
            add_last(std::forward<_T>(t));
            return;
        }
        if (idx == 0) {
            add_first(std::forward<_T>(t));
        } 
        else
        {
            node* n = new_node(std::forward<_T>(t));
            node* current = node_at(idx);
            assert(current != nullptr);

            node* prev = current->get_prev();
            assert(prev != nullptr);
            
            // prev node next
            n->set_prev(prev);
            n->set_next(current);

            current->set_prev(n);
            prev->set_next(n);
        }
    }

    template<typename T>
    typename linked_list<T>::node* linked_list<T>::node_at(std::size_t idx) const {
        assert(idx < size());
        if (idx < size() >> 1)
        {
            std::size_t i = 0;
            for (node* n = _head; n != nullptr; n = n->get_next())
            {
                if (i == idx)
                    return n;
                ++i;
            }
        } 
        else
        {
            std::size_t i = size() - 1;
            for (node* n = _tail; n != nullptr; n = n->get_prev())
            {
                if (i == idx)
                    return n;
                --i;
            }
        }
        
        return nullptr;
    }
    
    template<typename T>
    void linked_list<T>::remove_first() {
        JSTD_DEBUG_CODE(if (size() == 0) throw_except<no_such_element_exception>("list is empty");)
        
        node* n = _head;
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
    
        --_size;
        delete_node(n);
    }

    template<typename T>
    void linked_list<T>::remove_last() {
        JSTD_DEBUG_CODE(if (size() == 0) throw_except<no_such_element_exception>("list is empty");)
        
        node* n = _tail;
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
        --_size;
        delete_node(n);
    }

    template<typename T>
    void linked_list<T>::unlink(node* n) {
        assert(n != nullptr);
        
        node* prev = n->get_prev();
        node* next = n->get_next();
        
        if (prev == nullptr)
        {
            _head = n->get_next();
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
            _tail = n->get_prev();
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


        delete_node(n);
        
        --_size;
    }

    template<typename T>
    void linked_list<T>::remove_at(std::size_t idx) {
        JSTD_DEBUG_CODE(check_index(idx, size());)
        if (idx == 0)
        {
            remove_first();
            return;
        }
        else if (idx == size() - 1)
        {
            remove_last();
            return;
        }

        node* current = node_at(idx);
        
        assert(current != nullptr);
        unlink(current);

        --_size;
    }

    template<typename T>
    bool linked_list<T>::remove(const T& v) {
        equal_to<T> equals;
        for (node* i = _head; i != nullptr; i = i->get_next()) {
            if (equals(v, i->get_value())) {
                unlink(i);
                return true;
            }
        }
        return false;
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
    void linked_list<T>::pop() {
        remove_last();
    }

    template<typename T>
    typename linked_list<T>::Tvalue& linked_list<T>::get_last() {
        JSTD_DEBUG_CODE(if(size() == 0) throw_except<no_such_element_exception>("list is empty");)
        assert(_tail != nullptr);
        return _tail->get_value();
    }
    
    
    template<typename T>
    const typename linked_list<T>::Tvalue& linked_list<T>::get_last() const {
        JSTD_DEBUG_CODE(if(size() == 0) throw_except<no_such_element_exception>("list is empty");)
        assert(_tail != nullptr);
        return _tail->get_value();   
    }
    
    template<typename T>
    typename linked_list<T>::Tvalue& linked_list<T>::get_first() {
        JSTD_DEBUG_CODE(if(size() == 0) throw_except<no_such_element_exception>("list is empty");)
        assert(_head != nullptr);
        return _head->get_value();
    }
    
    
    template<typename T>
    const typename linked_list<T>::Tvalue& linked_list<T>::get_first() const {
        JSTD_DEBUG_CODE(if(size() == 0) throw_except<no_such_element_exception>("list is empty");)
        assert(_head != nullptr);
        return _head->get_value();
    }

    template<typename T>
    T& linked_list<T>::at(std::size_t idx) {
        JSTD_DEBUG_CODE(check_index(idx, size()));
        node* node = node_at(idx);
        assert(node != nullptr);
        return node->get_value();
    }
    
    template<typename T>
    const T& linked_list<T>::at(std::size_t idx) const {
        JSTD_DEBUG_CODE(check_index(idx, size()));
        const node* node = node_at(idx);
        assert(node != nullptr);
        return node->get_value();
    }

    template<typename T>
    bool linked_list<T>::contains(const T& t) const {
        equal_to<T> equals;
        for (const node* i = _head; i != nullptr; i = i->get_next())
            if (equals(t, i->get_value()))
                return true;
        return false;
    }

    template<typename T>
    std::size_t linked_list<T>::index_of(const T& t) const {
        std::size_t idx = 0;
        equal_to<T> equals;
        for (const node* i = _head; i != nullptr; i = i->get_next(), ++idx) {
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