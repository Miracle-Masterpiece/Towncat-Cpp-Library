#ifndef D6696066_D7E0_45B6_973C_F5F1C6645AB7
#define D6696066_D7E0_45B6_973C_F5F1C6645AB7

#include <tc/internal/cpp/lang/utils/comparator.hpp>
#include <tc/internal/cpp/lang/math/math.hpp>
#include <cassert>
#include <utility>

namespace tc
{
namespace detail
{

template<typename K>
class avlt_entry {
    /**
     * 
     */
    avlt_entry<K>* m_left;
    
    /**
     * 
     */
    avlt_entry<K>* m_right;
    
    /**
     * 
     */
    K m_key;

    /**
     * 
     */
    signed char m_height;

    /**
     * 
     */
    avlt_entry();

    /**
     * 
     */
    template<typename T_>
    avlt_entry(T_&& k);

    /**
     * 
     */
    avlt_entry(avlt_entry<K>&& n);
    
    /**
     * 
     */
    avlt_entry<K>& operator= (avlt_entry<K>&& n);

    /**
     * 
     */
    avlt_entry<K>*& left() {
        return m_left;
    }
    
    /**
     * 
     */
    avlt_entry<K>* left() const {
        return m_left;
    }
    
    /**
     * 
     */
    avlt_entry<K>*& right() {
        return m_right;
    }
    
    /**
     * 
     */
    avlt_entry<K>* right() const {
        return m_right;
    }
    
    /**
     * 
     */
    K& key() {
        return m_key;
    }
    
    /**
     * 
     */
    const K& key() const {
        return m_key;
    }

    /**
     * 
     */
    signed char& height() {
        return m_height;
    }
    
    /**
     * 
     */
    signed char height() const {
        return m_height;
    }

    /**
     * 
     */
    ~avlt_entry() {

    }
};

    template<typename K>    
    avlt_entry<K>::avlt_entry() : 
        m_left(nullptr), 
        m_right(nullptr), 
        m_key(), 
        m_height(0) {
        
    }

    template<typename K>
    template<typename T_>
    avlt_entry<K>::avlt_entry(T_&& k) : 
        m_left(nullptr), 
        m_right(nullptr),
        m_key(std::forward<T_>(k)),
        m_height(0) {

    }

    template<typename K>
    avlt_entry<K>::avlt_entry(avlt_entry<K>&& n) : 
        m_left(n.m_left), 
        m_right(n.m_right), 
        m_key(std::move(n.m_key)), 
        m_height(n.m_height)
    {
        n.m_left    = nullptr;
        n.m_right   = nullptr;
        n.m_height  = 0;
    }

    template<typename K>
    avlt_entry<K>& avlt_entry<K>::operator= (avlt_entry<K>&& n) {
        if (&n != this)
        {
            std::swap(m_left,   n.m_left);
            std::swap(m_right,  n.m_right);
            std::swap(m_key,    n.m_key);
            std::swap(m_height, n.m_height);
        }
        return *this;
    }

} // namespace detail
} // namespace tc

namespace tc
{

/**
 * Класс для низкоуровневой работы с типом данных AVL-Tree.
 * Класс не выделяет память под узлы дерева, а только принимает указатель на узел для связывания в дерево.
 * Так же класс не учищает память, выделенную под узел. Это должен делать сторонний код.
 * 
 * Для того, чтобы указать свой тип данных узла, необходима класс или структура с публичными полями
 * {
 *      struct my_node {
 *          my_node*    m_left;
 *          my_node*    m_right;
 *          int         m_key; <- этот ключ может быть любого типа, главное, чтобы он поддерживал сравнение через компаратор.
 *          signed char m_height;    
 *          
 *          avlt_entry<K>*& left() {return m_left;}
 *          avlt_entry<K>* left() const {return m_left;}
 *          
 *          avlt_entry<K>*& right() {return m_right;}
 *          avlt_entry<K>* right() const {return m_right;}
 *          
 *          int& key() {return m_key;}
 *          const T& key() const {return m_key;}
 *          
 *          signed char& height() {return m_height;}
 *          const signed char height() const {return m_height;}
 *      };
 *      Данная структура уже может использоваться в качестве узла дерева.
 * }
 * 
 */
template<typename K, typename COMPARATOR_T = compare_to<K>, typename NODE_T = detail::avlt_entry<K>>
class raw_avl_tree {
public:
    /**
     * 
     */
    typedef NODE_T node;
private:
    /**
     * 
     */
    node* m_root;

    /**
     * 
     */
    COMPARATOR_T m_key_comparator;

    /**
     * 
     */
    raw_avl_tree(const raw_avl_tree<K, COMPARATOR_T>&) = delete;
    
    /**
     * 
     */
    raw_avl_tree<K, COMPARATOR_T>& operator= (const raw_avl_tree<K, COMPARATOR_T>&) = delete;

    /**
     * 
     */
    node* insert_entry0(node* n, node* in);

    /**
     * 
     */
    node* find_min(node* n) const;

    /**
     * 
     */
    node* remove_min(node* n);

    /**
     * 
     */
    node* find_remove_min(node* n, node** result);

    /**
     * 
     */
    node* remove_entry0(node* n, const K& key, node** deleted);

    /**
     * 
     */
    node* remove_entry0(node* n, node* entry, node** deleted);

    /**
     * 
     */
    node* remove_ceil_entry0(node* n, K& key, node** deleted);

    /**
     * 
     */
    template<typename VISITOR_T>
    void visit0(const node* node, const VISITOR_T& visitor) const;

    /**
     * 
     */
    static node* right_rotate(node* b);

    /**
     * 
     */
    static node* left_rotate(node* b);

    /**
     * 
     */
    static node* balance(node* n);

    /**
     * 
     */
    static signed char get_height(node* n) {
        return n ? n->height() : 0;
    }

    /**
     * 
     */
    static signed char get_balance_factor(node* entry) {
        assert(entry != nullptr);
        return get_height(entry->left()) - get_height(entry->right());
    }

    /**
     * 
     */
    static void calc_height(node* entry) {
        assert(entry != nullptr);
        entry->height() = (math::max(get_height(entry->left()), get_height(entry->right())) + 1);
    }

public:
    /**
     * 
     */
    raw_avl_tree();
    
    /**
     * 
     */
    raw_avl_tree(const COMPARATOR_T& comparator);

    /**
     * 
     */
    raw_avl_tree(raw_avl_tree<K, COMPARATOR_T, NODE_T>&& rbt);
    
    /**
     * 
     */
    raw_avl_tree<K, COMPARATOR_T, NODE_T>& operator=(raw_avl_tree<K, COMPARATOR_T, NODE_T>&& t);

    /**
     * 
     */
    ~raw_avl_tree() {

    }

    /**
     * 
     */
    void insert_entry(node* in);

    /**
     * 
     */
    node* remove_entry(const K& key);

    /**
     * 
     */
    node* remove_entry(node* entry);

    /**
     * 
     */
    node* search_entry(const K& key) const;

    /**
     * 
     */
    node* floor_entry(const K& key) const;

    /**
     * 
     */
    node* ceil_entry(const K& key) const;
    
    /**
     * 
     */
    node* remove_ceil_entry(const K& key);

    /**
     * 
     */
    node* first_entry() const;

    /**
     * 
     */
    node* last_entry() const;

    /**
     * 
     */
    template<typename VISITOR_T>
    void visit(const VISITOR_T& visitor) const {
        if (m_root != nullptr)
            visit0(m_root, visitor);
    }

    /**
     * 
     */
    template<typename COMPARATOR_T_>
    void set_comparator(COMPARATOR_T_&& comparator) {
        m_key_comparator = std::forward<COMPARATOR_T_>(comparator);
    }

    /**
     * 
     */
    COMPARATOR_T& get_comparator() {
        return m_key_comparator;
    }

    /**
     * 
     */
    const COMPARATOR_T& get_comparator() const {
        return m_key_comparator;
    }

    /**
     * 
     */
    node* get_root() {
        return m_root;
    }
    
    /**
     * 
     */
    const node* get_root() const {
        return m_root;
    }
};
  
    template<typename K, typename COMPARATOR_T, typename NODE_T>
    raw_avl_tree<K, COMPARATOR_T, NODE_T>::raw_avl_tree() :
        m_root(nullptr),
        m_key_comparator()
    {

    }
    
    template<typename K, typename COMPARATOR_T, typename NODE_T>
    raw_avl_tree<K, COMPARATOR_T, NODE_T>::raw_avl_tree(const COMPARATOR_T& comparator) :
        m_root(nullptr),
        m_key_comparator(comparator)
    {

    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    raw_avl_tree<K, COMPARATOR_T, NODE_T>::raw_avl_tree(raw_avl_tree<K, COMPARATOR_T, NODE_T>&& rbt) :
        m_root(rbt.m_root),
        m_key_comparator(std::move(rbt.m_key_comparator))
    {
        rbt.m_root = nullptr;
    }
    
    template<typename K, typename COMPARATOR_T, typename NODE_T>
    raw_avl_tree<K, COMPARATOR_T, NODE_T>& raw_avl_tree<K, COMPARATOR_T, NODE_T>::operator=(raw_avl_tree<K, COMPARATOR_T, NODE_T>&& t) {
        if (&t != this)
        {
            std::swap(m_root,           t.m_root);
            std::swap(m_key_comparator, t.m_key_comparator);
        }
        return *this;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    template<typename VISITOR_T>
    void raw_avl_tree<K, COMPARATOR_T, NODE_T>::visit0(const node* node, const VISITOR_T& visitor) const {
        assert(node != nullptr);
        if (node->left())
        {
            visit0(node->left(), visitor);
        }
        visitor(node);
        if (node->right())
        {
            visit0(node->right(), visitor);
        }
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::insert_entry0(node* n, node* in) {
        assert(in != nullptr);
        if (n == nullptr)
            return in;
        
        const int cmp = m_key_comparator(in->key(), n->key());
        if (cmp >= 0)
        {
            n->right() = insert_entry0(n->right(), in);
        } 
        else
        {
            n->left() = insert_entry0(n->left(), in);
        }

        return balance(n);
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::find_min(node* n) const {
        assert(n != nullptr);
        while (n->left())
        {
            n = n->left();
        }
        return n;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_min(node* n) {
        assert(n != nullptr);
        if (!n->left())
        {
            return n->right();
        }
        n->left() = remove_min(n->right());
        return balance(n);
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::find_remove_min(node* n, node** result) {
        assert(n != nullptr);
        assert(result != nullptr);
        if (n->left() == nullptr)
        {
            *result = n;
            return n->right();
        }
        n->left() = find_remove_min(n->left(), result);
        return balance(n);
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_entry0(node* n, const K& key, node** deleted) {
        if (n == nullptr)
            return nullptr;
        int cmp = m_key_comparator(key, n->key());
        if (cmp > 0)
        {
            n->right() = remove_entry0(n->right(), key, deleted);
        }
        else if (cmp < 0)
        {
            n->left() = remove_entry0(n->left(), key, deleted);
        }
        else
        {
            node* left  = n->left();
            node* right = n->right();
            *deleted = n;
            if (right == nullptr) 
                return left;    
            node* min = nullptr;
            node* right_from_min = find_remove_min(right, &min);
            min->right() = right_from_min;
            min->left()  = left();
            n = min;
        }
        return balance(n);
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_ceil_entry(const K& key) {
        assert(false);
        node* result = nullptr;
        m_root = remove_ceil_entry0(m_root, key, &result);
        return result;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_ceil_entry0(node* n, K& key, node** deleted) {
        assert(false);
        return nullptr;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_entry0(node* n, node* entry, node** deleted) {
      if (n == nullptr)
            return nullptr;
        int cmp = m_key_comparator(entry->key(), n->key());
        if (cmp > 0)
        {
            n->right() = remove_entry0(n->right(), entry, deleted);
        }
        else if (cmp < 0)
        {
            n->left() = remove_entry0(n->left(), entry, deleted);
        }
        else
        {
            //такое может быть, если в дереве хранятся дубликаты
            if (n != entry)
            {
                node* const before = *deleted;
                n->right() = remove_entry0(n->right(), entry, deleted);
                if (before == *deleted)
                    n->left() = remove_entry0(n->left(), entry, deleted);
            }
            else
            {
                node* left  = n->left();
                node* right = n->right();
                *deleted = n;
                if (right == nullptr) 
                    return left;
                node* min = nullptr;
                node* right_from_min = find_remove_min(right, &min);
                min->right()  = right_from_min;
                min->left()   = left;
                n = min;
            }

        }
        return balance(n);
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    void raw_avl_tree<K, COMPARATOR_T, NODE_T>::insert_entry(node* in) {
        assert(in != nullptr);
        m_root = insert_entry0(m_root, in);
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_entry(const K& key) {
        node* deleted = nullptr;
        m_root = remove_entry0(m_root, key, &deleted);
        return deleted;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::remove_entry(node* entry) {
        assert(entry != nullptr);
        node* deleted = nullptr;
        m_root = remove_entry0(m_root, entry, &deleted);
        return deleted;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::search_entry(const K& key) const {
        node* node = m_root;
        while (node)
        {
            if (node->key() == key)
                return node;
            if (key > node->key())
            {
                node = node->right();
            }
            else
            {
                node = node->left();
            }
        }
        return nullptr;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::floor_entry(const K& key) const {
        node* root          = m_root;
        node* floor_node    = nullptr;
        const COMPARATOR_T& compare = m_key_comparator;
        while (root)
        {
            int cmp = compare(key, root->key());
            if (cmp == 0)
                return root;
            if (cmp < 0)
            {
                root = root->left();
            }
            else
            {
                floor_node = root;
                root = root->right();
            }
        }
        return floor_node;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::ceil_entry(const K& key) const {
        node* root = m_root;
        node* ceil = nullptr;
        const COMPARATOR_T& compare = m_key_comparator;
        while (root)
        {
            int cmp = compare(key, root->key());
            if (cmp == 0)
                return root;
            if (cmp > 0)
            {
                root = root->right();
            }
            else
            {
                ceil = root;
                root = root->left();
            }
        }
        return ceil;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::first_entry() const {
        node* root = m_root;
        node* first = root;
        while (root)
        {
            first = root;
            root  = root->left();
        }
        return first;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::last_entry() const {
        node* root = m_root;
        node* last = root;
        while (root)
        {
            last = root;
            root = root->right();
        }
        return last;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::right_rotate(node* b) {
        assert(b != nullptr);
        node* const c = b->left();
        node* const N = c ? c->right() : nullptr;
        b->left()  = N;
        c->right() = b;
        calc_height(b);
        calc_height(c);
        return c;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::left_rotate(node* b) {
        assert(b != nullptr);
        node* const c = b->right();
        node* const N = c ? c->left() : nullptr;
        b->right() = N;
        c->left()  = b;
        calc_height(b);
        calc_height(c);
        return c;
    }

    template<typename K, typename COMPARATOR_T, typename NODE_T>
    typename raw_avl_tree<K, COMPARATOR_T, NODE_T>::node* raw_avl_tree<K, COMPARATOR_T, NODE_T>::balance(node* n) {
        assert(n != nullptr);
        calc_height(n);

        signed char balance_factor = get_balance_factor(n);
    
        //left case
        if (balance_factor > 1) {
            if (get_balance_factor(n->left()) < 0)
                n->left() = left_rotate(n->left());
            return right_rotate(n);
        }
        
        //right case
        if (balance_factor < -1) {
            if (get_balance_factor(n->right()) > 0)
                n->right() = right_rotate(n->right());
            return left_rotate(n);
        }

        return n;
    }

}

#endif /* D6696066_D7E0_45B6_973C_F5F1C6645AB7 */
