#ifndef _JSTD_CPP_LANG_UTILS_OPTIONAL_H_
#define _JSTD_CPP_LANG_UTILS_OPTIONAL_H_

#include <utility>
#include <cpp/lang/exceptions.hpp>

namespace tc {

/**
 * Класс optional<T> предоставляет контейнер для хранения объекта типа T, 
 * который может присутствовать или отсутствовать. Это аналог std::optional, 
 * но реализован вручную.
 *
 * @tparam T Тип хранимого значения.
 */
template<typename T>
class optional {
    union {
        
        /**
         * Выравнивание для корректного хранения объекта.
         */
        T _align;
        
        /**
         * Буфер для хранения объекта типа T.
         */
        char _object[sizeof(T)];
    };
    
    bool _is_value; ///< Флаг, указывающий, содержит ли optional значение.

    /**
     * Освобождает хранимое значение, если оно присутствует.
     */
    void dispose();

    /**
     * Проверяет, возможен ли доступ к значению.
     * Если this->_is_value == false, то кидает исключение.
     */
    void check_access() const {
        if (!_is_value)
            throw_except<illegal_state_exception>("Optional is empty!");
    }

public:
    /**
     * Создаёт пустой optional.
     */
    optional();

    /**
     * Создаёт optional с копией переданного значения.
     *
     * @param value Значение, которое будет скопировано в optional.
     */
    optional(const T& value);

    /**
     * Создаёт optional с перемещённым значением.
     *
     * @param value Значение, которое будет перемещено в optional.
     */
    optional(T&& value);

    /**
     * Конструктор копирования.
     *
     * @param other Другой optional, из которого будет скопировано значение.
     */
    optional(const optional<T>& other);

    /**
     * Конструктор перемещения.
     *
     * @param other Другой optional, из которого будет перемещено значение.
     */
    optional(optional<T>&& other);

    /**
     * Оператор копирующего присваивания.
     *
     * @param other Другой optional, из которого будет скопировано значение.
     * @return Ссылка на текущий объект.
     */
    optional<T>& operator= (const optional<T>& other);

    /**
     * Оператор перемещающего присваивания.
     *
     * @param other Другой optional, из которого будет перемещено значение.
     * @return Ссылка на текущий объект.
     */
    optional<T>& operator= (optional<T>&& other);

    /**
     * Деструктор. Освобождает хранимое значение, если оно присутствует.
     */
    ~optional();

    /**
     * Проверяет, содержит ли optional значение.
     *
     * @return true, если значение присутствует, иначе false.
     */
    bool is_value() const;

    /**
     * Получает ссылку на хранимое значение.
     * 
     * @throws jstd::illegal_state_exception, если optional пуст.
     * @return Ссылка на хранимое значение.
     */
    T& get_value();

    /**
     * Получает константную ссылку на хранимое значение.
     * 
     * @throws jstd::illegal_state_exception, если optional пуст.
     * @return Константная ссылка на хранимое значение.
     */
    const T& get_value() const;

    /**
     * Оператор доступа к членам хранимого объекта.
     * 
     * @throws jstd::illegal_state_exception, если optional пуст.
     * @return Указатель на хранимый объект.
     */
    T* operator->();

    /**
     * Оператор доступа к членам хранимого объекта (константная версия).
     * 
     * @throws jstd::illegal_state_exception, если optional пуст.
     * @return Константный указатель на хранимый объект.
     */
    const T* operator->() const;

    /**
     * Оператор разыменования. Позволяет получить доступ к хранимому значению.
     * 
     * @throws jstd::illegal_state_exception, если optional пуст.
     * @return Ссылка на хранимый объект.
     */
    T& operator*();

    /**
     * Оператор разыменования (константная версия).
     * 
     * @throws jstd::illegal_state_exception, если optional пуст.
     * @return Константная ссылка на хранимый объект.
     */
    const T& operator*() const;

    /**
     * @return 
     *      Возвращает объект, хранящий нулевое значение.
     */
    static optional<T> null_opt() {
        return optional<T>();
    }
};

    template<typename T>
    optional<T>::optional() : _is_value(false) {}

    template<typename T>
    optional<T>::optional(const T& value) : _is_value(false) {
        new(_object) T(value);
        _is_value = true;
    }

    template<typename T>
    optional<T>::optional(T&& value) : _is_value(false) {
        new(_object) T(std::move(value));
        _is_value = true;
    }

    template<typename T>
    optional<T>::optional(const optional<T>& t) : _is_value(false) {
        if (t._is_value) {
            new (_object) T(*reinterpret_cast<const T*>(t._object));
            _is_value = true;
        }
    }
    
    template<typename T>
    optional<T>::optional(optional<T>&& t) : _is_value(false) {
        if (t._is_value) {
            new (_object) T(std::move(*reinterpret_cast<T*>(t._object)));
            _is_value = true;
        }
    }

    template<typename T>
    void optional<T>::dispose() {
        if (_is_value) {
            const T* object = reinterpret_cast<const T*>(_object);
            object->~T();
            _is_value = false;
        }
    }

    template<typename T>
    optional<T>::~optional() {
        dispose();
    }
    
    template<typename T>
    bool optional<T>::is_value() const {
        return _is_value;
    }

    template<typename T>
    T& optional<T>::get_value() {
        check_access();
        return *reinterpret_cast<T*>(_object);
    }
    
    template<typename T>
    const T& optional<T>::get_value() const {
        check_access();
        return *reinterpret_cast<const T*>(_object);
    }

    template<typename T>
    T* optional<T>::operator->() {
        check_access();
        return reinterpret_cast<T*>(_object);
    }
    
    template<typename T>
    const T* optional<T>::operator->() const {
        check_access();
        return reinterpret_cast<const T*>(_object);
    }
    
    template<typename T>
    T& optional<T>::operator*() {
        check_access();
        return get_value();
    }
    
    template<typename T>
    const T& optional<T>::operator*() const {
        check_access();
        return get_value();
    }

}
#endif//_JSTD_CPP_LANG_UTILS_OPTIONAL_H_