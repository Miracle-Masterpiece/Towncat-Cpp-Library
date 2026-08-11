#ifndef JSTD_CPP_LANG_UTILS_HASH_SET_H
#define JSTD_CPP_LANG_UTILS_HASH_SET_H

#include <cpp/lang/utils/hash_map.hpp>

namespace tc {

/**
 * Представляет собой множество уникальных элементов на основе хеш-таблицы.
 *
 * Класс hash_set реализован поверх hash_map, где значения элементов не играют роли
 * (используется значение по умолчанию '\0'. 
 * 
 * Предназначен для эффективного хранения и поиска уникальных ключей. 
 * Поддерживает пользовательские хеш-функции и функции сравнения.
 *
 * @tparam K 
 *      Тип ключей.
 * 
 * @tparam KEY_HASH 
 *      Тип хеш-функции для ключей (по умолчанию hash_for<K>).
 * 
 * @tparam KEY_EQUAL 
 *      Тип функции сравнения ключей (по умолчанию equal_to<K>).
 */
template<typename K, typename KEY_HASH = hash_for<K>, typename KEY_EQUAL = equal_to<K>>
class hash_set {
    
    /**
     * 
     */
    static const std::size_t INIT_CAPACITY  = 16;
    
    /**
     * 
     */
    static const char DEFAULT_VALUE     = '\0';
    
    /**
     * 
     */
    hash_map<K, char, KEY_HASH, KEY_EQUAL> m_storage;

public:
    /**
     * 
     */
    typedef typename hash_map<K, char, KEY_HASH, KEY_EQUAL>::entry entry;

    /**
     * Создаёт пустое множество с заданным аллокатором и (опционально) начальной ёмкостью.
     *
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     * 
     * @param init_capacity 
     *      Начальная ёмкость.
     */
    hash_set(tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Создаёт пустое множество с заданным аллокатором и (опционально) начальной ёмкостью.
     *
     * @param init_capacity 
     *      Начальная ёмкость.
     *
     * @param load_factor
     *      Соотношение количества элементов и ёмкостью при котором будет расширение хеш-сета.
     *  
     * @param allocator 
     *      Указатель на пользовательский аллокатор.
     * 
     */
    hash_set(std::size_t init_capacity, float load_factor = 0.75f, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Конструктор копирования.
     *
     * @param other 
     *      Множество, из которого будет скопировано содержимое.
     */
    hash_set(const hash_set<K, KEY_HASH, KEY_EQUAL>& other);

    /**
     * Конструктор перемещения.
     *
     * @param other 
     *      Множество, содержимое которого будет перемещено.
     */
    hash_set(hash_set<K, KEY_HASH, KEY_EQUAL>&& other);

    /**
     * Оператор копирующего присваивания.
     *
     * @param other 
     *      Множество, содержимое которого будет скопировано.
     * 
     * @return 
     *      Ссылка на текущее множество.
     */
    hash_set<K, KEY_HASH, KEY_EQUAL>& operator=(const hash_set<K, KEY_HASH, KEY_EQUAL>& other);

    /**
     * Оператор перемещающего присваивания.
     *
     * @param other 
     *      Множество, содержимое которого будет перемещено.
     * 
     * @return 
     *      Ссылка на текущее множество.
     */
    hash_set<K, KEY_HASH, KEY_EQUAL>& operator=(hash_set<K, KEY_HASH, KEY_EQUAL>&& other);

    /**
     * Деструктор.
     */
    ~hash_set();

    /**
     * Добавляет ключ в множество.
     *
     * Если такой ключ уже есть, множество не изменяется.</p>
     *
     * @tparam _K 
     *      Тип передаваемого ключа (поддерживаются rvalue и lvalue).
     * 
     * @param key 
     *      Ключ для добавления.
     */
    template<typename _K>
    void add(_K&& key);

    /**
     * Удаляет ключ из множества.
     *
     * @param key 
     *      Ключ, который требуется удалить.
     * @return 
     *      true, если ключ был найден и удалён, иначе false.
     */
    bool remove(const K& key);

    /**
     * Удаляет из данного множества все элементы, содержащиеся в передаваемом множестве.
     * 
     * @param set
     *      Передаваемое множество.
     * 
     * @return
     *      true, если был удалён хотя бы один элемент.
     */
    bool remove_all(const hash_set<K>& set);

    /**
     * Проверяет, содержится ли ключ в множестве.
     *
     * @param key 
     *      Проверяемый ключ.
     * 
     * @return 
     *      true, если ключ найден, иначе false.
     */
    bool contains(const K& key) const;

    /**
     * Проверяет, содержатся ли все элементы переданного множества в текущем.
     *
     * @param set 
     *      Множество, элементы которого нужно проверить.
     * 
     * @return 
     *      true, если все ключи присутствуют, иначе false.
     */
    bool contains_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) const;

    /**
     * Добавляет в текущее множество все элементы из другого множества.
     *
     * @param set 
     *      Множество, ключи которого будут добавлены.
     */
    void add_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set);

    /**
     * Удаляет все элементы из множества.
     */
    void clear();

    /**
     * Возвращает количество элементов в множестве.
     *
     * @return 
     *      Текущее количество элементов.
     */
    int64_t size() const;

    /**
     * Проверяет, пусто ли множество.
     *
     * @return 
     *      true, если множество не содержит ни одного элемента.
     */
    bool is_empty() const;

    /**
     * Создаёт полную копию множества.
     *
     * @param allocator (необязательно) 
     *      Аллокатор, который будет использоваться для нового множества.
     *      Если аллокатор не указан, будет использоваться текущий.
     * 
     * @return 
     *      Новый экземпляр множества, содержащий те же элементы.
     */
    hash_set<K, KEY_HASH, KEY_EQUAL> clone(tca::allocator* allocator = nullptr) const;

    /**
     * 
     */
    typename hash_map<K, char, KEY_HASH, KEY_EQUAL>:: template iterator<const entry> begin() const {
        return m_storage.begin();
    }
    
    /**
     * 
     */
    typename hash_map<K, char, KEY_HASH, KEY_EQUAL>:: template iterator<const entry> end() const {
        return m_storage.end();
    }
    
    /**
     * 
     */
    typename hash_map<K, char, KEY_HASH, KEY_EQUAL>:: template iterator<entry> begin() {
        return m_storage.begin();
    }
    
    /**
     * 
     */
    typename hash_map<K, char, KEY_HASH, KEY_EQUAL>:: template iterator<entry> end() {
        return m_storage.end();
    }
};

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(tca::allocator* allocator) :
        m_storage(allocator) {
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(std::size_t init_capacity, float load_factor, tca::allocator* allocator) :
        m_storage(init_capacity, load_factor, allocator) {
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) : m_storage(set.m_storage) {

    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::hash_set(hash_set<K, KEY_HASH, KEY_EQUAL>&& set) : m_storage(std::move(set.m_storage)) {

    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>& hash_set<K, KEY_HASH, KEY_EQUAL>::operator= (const hash_set<K, KEY_HASH, KEY_EQUAL>& set) {
        m_storage = set.m_storage;
        return *this;
    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>& hash_set<K, KEY_HASH, KEY_EQUAL>::operator= (hash_set<K, KEY_HASH, KEY_EQUAL>&& set) {
        m_storage = std::move(set.m_storage);
        return *this;
    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL>::~hash_set() {

    }
    
    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    template<typename _K>
    void hash_set<K, KEY_HASH, KEY_EQUAL>::add(_K&& key) {
        const char value = DEFAULT_VALUE;
        m_storage.put(std::forward<_K>(key), value);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::remove(const K& key) {
        return m_storage.remove(key);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::remove_all(const hash_set<K>& set) {
        bool changed = false;
        for (const entry& e : set)
        {
            if (remove(e.get_key()))
                changed = true;
        }
        return changed;
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::contains(const K& key) const {
        return m_storage.contains_key(key);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    void hash_set<K, KEY_HASH, KEY_EQUAL>::add_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) {
        m_storage.put_all(set.m_storage);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    void hash_set<K, KEY_HASH, KEY_EQUAL>::clear() {
        m_storage.clear();
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    int64_t hash_set<K, KEY_HASH, KEY_EQUAL>::size() const {
        return m_storage.size();
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::is_empty() const {
        return m_storage.is_empty();
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    bool hash_set<K, KEY_HASH, KEY_EQUAL>::contains_all(const hash_set<K, KEY_HASH, KEY_EQUAL>& set) const {
        return m_storage.contains_all(set.m_storage);
    }

    template<typename K, typename KEY_HASH, typename KEY_EQUAL>
    hash_set<K, KEY_HASH, KEY_EQUAL> hash_set<K, KEY_HASH, KEY_EQUAL>::clone(tca::allocator* allocator) const {
        hash_map<K, char, KEY_HASH, KEY_EQUAL> storage = m_storage.clone(allocator);
        hash_set<K, KEY_HASH, KEY_EQUAL> result;
        result.m_storage = std::move(storage);
        return hash_set<K, KEY_HASH, KEY_EQUAL>(std::move(result));
    }

}
#endif//JSTD_CPP_LANG_UTILS_HASH_SET_H
