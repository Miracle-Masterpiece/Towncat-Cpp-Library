#ifndef JSTD_CPP_LANG_STRING_POOL_H
#define JSTD_CPP_LANG_STRING_POOL_H

#include <cpp/lang/concurrency/mutex.hpp>
#include <cpp/lang/utils/hash_map.hpp>
#include <cpp/lang/utils/shared_ptr.hpp>
#include <cpp/lang/cstr.hpp>
#include <cpp/lang/string.hpp>

namespace tc 
{

/**
 * Класс, реализующий строковой пул константных строк в однобайтовом формате UTF-8 и ASCII.
 * 
 * @see jstd::concurrency_string_pool
 */
class string_const_pool {
    /**
     * Аллокатор для распределения памяти под строки и хеш-карты.
     */
    tca::allocator* m_allocator;
    
    /**
     * Хеш-карта, хранящая константную строку ввиде ключа и объект string ввиде значения.
     */
    hash_map<cstr, shared_ptr<const string>> m_map;

public:
    /**
     * @param allocator
     *      Распределитель памяти для пула (Опционально).
     */
    string_const_pool(tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * 
     */
    ~string_const_pool();
    
    /**
     * Возвращает указатель на tc::string из пула.
     * Если строки в пуле нет, создаёт её и добавляет в пул.
     * 
     * @warning
     *      Строка обязана быть литералом, либо освобождаться строго после вызова деструктора этого пула!
     *      Объект работает напрямую с этим указателем!
     * 
     * @param str
     *      Константная си-строка.
     * 
     * @return 
     *      Строка идентичная переданной строке, но ввиде объекта jstd::string
     */
    shared_ptr<const string>& get(const char* str);
    
    /**
     * Добавляет строку в пул.
     * 
     * @return
     *      true - если строка успешно добавлена.
     *      false - если строка уже существует.
     */
    bool intern(const char* str);
    
    /**
     * Удаляет строку из пула.
     * 
     * @return
     *      true - если строка успешно удалена.
     *      false - если строки в пуле нет.
     */
    bool deintern(const char* str);
    
    /**
     * Удаляет строку из пула с использованием указателя.
     * 
     * @return
     *      true - если строка успешно удалена.
     *      false - если строки в пуле нет.
     */
    bool deintern(shared_ptr<const string> str);
    
    /**
     * Распечатывает на консоль строки, содержищиеся в пуле.
     */
    void print_log() const;
    
    /**
     * Полностью очищает пул от всех строк.
     */
    void clear();
};

/**
 * 
 * Класс, реализующий многопоточный строковой пул константных строк в однобайтовом формате UTF-8 и ASCII.
 * 
 * @see jstd::string_const_pool
 */
class concurrency_string_const_pool {
    /**
     * Строковой пул, хранящий строки.
     */
    string_const_pool m_strpool;
    
    /**
     */
    mutex m_mutex;
public:
    /**
     * @param allocator
     *      Распределитель памяти для пула (Опционально).
     */
    concurrency_string_const_pool(tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     */
    ~concurrency_string_const_pool();
    
    /**
     * @see jstd::string_const_pool::get(const char*)
     */
    shared_ptr<const string>& get(const char* str);
    
    /**
     * @see jstd::string_const_pool::intern(const char*)
     */
    bool intern(const char* str);
    
    /**
     * @see jstd::string_const_pool::deintern(const char*)
     */
    bool deintern(const char* str);
    
    /**
     * @see jstd::string_const_pool::deintern(jstd::shared_ptr<const string>)
     */
    bool deintern(shared_ptr<const string> str);
    
    /**
     * @see jstd::string_const_pool::print_log()
     */
    void print_log() const;
    
    /**
     * @see jstd::string_const_pool::clear()
     */
    void clear();
};

}


#endif//JSTD_CPP_LANG_STRING_POOL_H