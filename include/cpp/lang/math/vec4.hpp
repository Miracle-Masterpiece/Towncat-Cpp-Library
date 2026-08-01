#ifndef JSTD_CPP_LANG_MATH_VEC4_H
#define JSTD_CPP_LANG_MATH_VEC4_H

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/math/math.hpp>
#include <internal/math_defs.hpp>
#include <cpp/lang/string.hpp>

namespace tc
{

template<typename T>
struct base_vec4;

typedef base_vec4<float> vec4;
typedef base_vec4<double> vec4d;
typedef base_vec4<long double> vec4ld;

typedef base_vec4<short> vec4s;
typedef base_vec4<signed int> vec4i;
typedef base_vec4<signed long> vec4l;

typedef base_vec4<unsigned short> vec4us;
typedef base_vec4<unsigned int> vec4ui;
typedef base_vec4<unsigned long> vec4ul;
typedef base_vec4<unsigned long long> vec4ull;


/**
 * Обобщённый четырёхкомпонентный вектор.
 * 
 * Представляет собой структуру, содержащую четыре компоненты — x, y, z, w — и предоставляет
 * базовые арифметические, векторные и служебные операции. 
 * 
 * Поддерживает доступ к данным как по координатам (x, y, z, w), так и по цветовому представлению (r, g, b, a).
 * 
 * @tparam T 
 *      Тип компонентов вектора (например, float, double, int).
 */
template<typename T>
struct base_vec4 {

    union
    {
        struct
        {
            union {
                T x, r, A;
            };

            union {
                T y, g, B;
            };

            union {
                T z, b, C;
            };

            union {
                T w, a, D;
            };
        };
        
        T arr[4];
    };

    /**
     * Конструктор по значениям компонентов.
     * 
     * @param x 
     *      Значение X-компоненты.
     * 
     * @param y 
     *      Значение Y-компоненты.
     * 
     * @param z 
     *      Значение Z-компоненты.
     */
    base_vec4(const T& x = T(), const T& y = T(), const T& z = T(), const T& w = T());
    
    /**
     * Конструктор копирования.
     * 
     * @param other 
     *      Вектор, который будет скопирован.
     */
    base_vec4(const base_vec4<T>& other);
    
    /**
     * Конструктор перемещения.
     * 
     * @param other 
     *      Вектор, из которого будут перемещены данные.
     */
    base_vec4(base_vec4<T>&& other);
    
    /**
     * Оператор копирующего присваивания.
     * 
     * @param other 
     *      Вектор для копирования.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_vec4<T>& operator=(const base_vec4<T>& other);
    
    /**
     * Оператор перемещающего присваивания.
     * 
     * @param other 
     *      Вектор для перемещения.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_vec4<T>& operator=(base_vec4<T>&& other);

    /**
     *
     */
    ~base_vec4();

    /**
     * Сложение с другим вектором.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Сумма двух векторов.
     */
    base_vec4<T> add(const base_vec4<T>& other) const;
    
    /**
     * Оператор сложение с другим вектором.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Сумма двух векторов.
     */
    base_vec4<T> operator+(const base_vec4<T>& other) const;
    
    /**
     * Сложение с числом (покомпонентно).
     * 
     * @param scalar 
     *      Скалярное значение.
     * 
     * @return 
     *      Результат сложения.
     */
    base_vec4<T> add(const T& scalar) const;
    
    /**
     * Оператор сложение с числом (покомпонентно).
     * 
     * @param scalar 
     *      Скалярное значение.
     * 
     * @return 
     *      Результат сложения.
     */
    base_vec4<T> operator+(const T& scalar) const;

    /**
     * Вычитание другого вектора.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Разность двух векторов.
     */
    base_vec4<T> sub(const base_vec4<T>& other) const;
    
    /**
     * Оператор вычитания другого вектора.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Разность двух векторов.
     */
    base_vec4<T> operator-(const base_vec4<T>& other) const;
    
    /**
     * Вычитание скаляра из каждого компонента вектора.
     * 
     * @param scalar 
     *      Скаляр, который будет вычтен из каждого компонента.
     * 
     * @return 
     *      Новый вектор, содержащий результат вычитания скаляра.
     */
    base_vec4<T> sub(const T& scalar) const;
    
    /**
     * Операция вычитания скаляра из каждого компонента вектора.
     * 
     * @param scalar 
     *      Скаляр, который будет вычтен из каждого компонента.
     * 
     * @return 
     *      Новый вектор, содержащий результат вычитания скаляра.
     */
    base_vec4<T> operator-(const T& scalar) const;

    /**
     * Умножение вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для умножения.
     * 
     * @return 
     *      Новый вектор, результат умножения покомпонентно.
     */
    base_vec4<T> mul(const base_vec4<T>& other) const;

    /**
     * Операция умножения вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для умножения.
     * 
     * @return 
     *      Новый вектор, результат умножения покомпонентно.
     */
    base_vec4<T> operator*(const base_vec4<T>& other) const;

    /**
     * Умножение вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр, на который будет умножен вектор.
     * 
     * @return 
     *      Новый вектор, результат умножения на скаляр.
     */
    base_vec4<T> mul(const T& scalar) const;

    /**
     * Операция умножения вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр, на который будет умножен вектор.
     * 
     * @return 
     *      Новый вектор, результат умножения на скаляр.
     */
    base_vec4<T> operator*(const T& scalar) const;

    /**
     * Деление вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для деления.
     * 
     * @return 
     *      Новый вектор, результат деления покомпонентно.
     */
    base_vec4<T> div(const base_vec4<T>& other) const;

    /**
     * Операция деления вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для деления.
     * 
     * @return 
     *      Новый вектор, результат деления покомпонентно.
     */
    base_vec4<T> operator/(const base_vec4<T>& other) const;

    /**
     * Деление вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр, на который будет разделён вектор.
     * 
     * @return 
     *      Новый вектор, результат деления на скаляр.
     */
    base_vec4<T> div(const T& scalar) const;

    /**
     * Операция деления вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр, на который будет разделён вектор.
     * 
     * @return 
     *      Новый вектор, результат деления на скаляр.
     */
    base_vec4<T> operator/(const T& scalar) const;

    /**
     * Доступ к компоненту вектора по индексу.
     * 
     * @param index 
     *      Индекс компонента.
     * 
     * @return 
     *      Ссылка на компонент вектора.
     */
    T& operator[](std::size_t index);

    /**
     * Доступ к компоненту вектора по индексу (константный доступ).
     * 
     * @param index 
     *      Индекс компонента.
     * 
     * @return 
     *      Константная ссылка на компонент вектора.
     */
    const T& operator[](std::size_t index) const;

    /**
     * Получение компонента вектора по индексу.
     * 
     * @param idx 
     *      Индекс компонента.
     * 
     * @return 
     *      Ссылка на компонент вектора.
     */
    T& get(std::size_t idx);

    /**
     * Получение компонента вектора по индексу (константный доступ).
     * 
     * @param idx 
     *      Индекс компонента.
     * 
     * @return 
     *      Константная ссылка на компонент вектора.
     */
    const T& get(std::size_t idx) const;

    /**
     * Вычисление скалярного произведения двух векторов.
     * 
     * @param other 
     *      Второй вектор для скалярного произведения.
     * 
     * @return 
     *      Результат скалярного произведения.
     */
    T dot(const base_vec4<T>& other) const;

    /**
     * Нормализация вектора.
     * 
     * @return 
     *      Новый нормализованный вектор.
     */
    base_vec4<T> normalized() const;

    /**
     * Длина вектора.
     * 
     * @return 
     *      Длина вектора.
     */
    T length() const;

    /**
     * Проверка на равенство двух векторов.
     * 
     * @param other 
     *      Второй вектор для сравнения.
     * 
     * @return 
     *      true, если векторы равны, иначе false.
     */
    bool equals(const base_vec4<T>& other) const;

    /**
     * Операция сравнения двух векторов на равенство.
     * 
     * @param other 
     *      Второй вектор для сравнения.
     * 
     * @return 
     *      true, если векторы равны, иначе false.
     */
    bool operator==(const base_vec4<T>& other) const;

    /**
     * Операция сравнения двух векторов на неравенство.
     * 
     * @param other 
     *      Второй вектор для сравнения.
     * 
     * @return 
     *      true, если векторы не равны, иначе false.
     */
    bool operator!=(const base_vec4<T>& other) const;

    /**
     * Вычисление хеш-кода вектора.
     * 
     * @return 
     *      Хеш-код вектора.
     */
    std::size_t hashcode() const;

    /**
     * Преобразует вектор в строковое представление.
     */
    tc::string to_string(tca::allocator* = tca::get_default_allocator()) const;

    /**
     * Операция увеличения компонента вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для увеличения.
     * 
     * @return 
     *      Ссылку на текущий вектор после увеличения.
     */
    base_vec4<T>& operator +=(const base_vec4<T>& other);

    /**
     * Операция уменьшения компонента вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для уменьшения.
     * 
     * @return 
     *      Ссылку на текущий вектор после уменьшения.
     */
    base_vec4<T>& operator -=(const base_vec4<T>& other);

    /**
     * Операция умножения компонента вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для умножения.
     * 
     * @return 
     *      Ссылку на текущий вектор после умножения.
     */
    base_vec4<T>& operator *=(const base_vec4<T>& other);

    /**
     * Операция деления компонента вектора на другой вектор покомпонентно.
     * 
     * @param other 
     *      Второй вектор для деления.
     * 
     * @return 
     *      Ссылку на текущий вектор после деления.
     */
    base_vec4<T>& operator /=(const base_vec4<T>& other);

    /**
     * Операция увеличения компонента вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр для увеличения.
     * 
     * @return 
     *      Ссылку на текущий вектор после увеличения.
     */
    base_vec4<T>& operator +=(const T& scalar);

    /**
     * Операция уменьшения компонента вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр для уменьшения.
     * 
     * @return 
     *      Ссылку на текущий вектор после уменьшения.
     */
    base_vec4<T>& operator -=(const T& scalar);

    /**
     * Операция умножения компонента вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр для умножения.
     * 
     * @return 
     *      Ссылку на текущий вектор после умножения.
     */
    base_vec4<T>& operator *=(const T& scalar);

    /**
     * Операция деления компонента вектора на скаляр.
     * 
     * @param scalar 
     *      Скаляр для деления.
     * 
     * @return 
     *      Ссылку на текущий вектор после деления.
     */
    base_vec4<T>& operator /=(const T& scalar);

    /**
     * Префиксный инкремент (увеличение всех компонентов на 1).
     * 
     * @return 
     *      Ссылку на текущий вектор после инкремента.
     */
    base_vec4<T>& operator++();

    /**
     * Постфиксный инкремент (увеличение всех компонентов на 1).
     * 
     * @return 
     *      Копию вектора до инкремента.
     */
    base_vec4<T> operator++(int);

    /**
     * Префиксный декремент (уменьшение всех компонентов на 1).
     * 
     * @return 
     *      Ссылку на текущий вектор после декремента.
     */
    base_vec4<T>& operator--();

    /**
     * Постфиксный декремент (уменьшение всех компонентов на 1).
     * 
     * @return 
     *      Копию вектора до декремента.
     */
    base_vec4<T> operator--(int);

    /**
     * Унарный минус (инвертирует все компоненты вектора).
     *
     * @return
     *     Новый вектор с противоположными по знаку компонентами.
     */
    base_vec4<T> operator-() const;

    /**
     * Приводит этот вектор к другому типу.
     * 
     * @tparam
     *      Тип, в который будет преобразован этот вектор.
     * 
     * @return
     *      Новый, преобразованный, вектор.
     */
    template<typename E>
    explicit operator base_vec4<E>() const;
};


    template<typename T>
    base_vec4<T>::base_vec4(const T& x, const T& y, const T& z, const T& w) :
    x(x), y(y), z(z), w(w) {

    }
    
    template<typename T>
    base_vec4<T>::base_vec4(const base_vec4<T>& v) : 
    x(v.x), y(v.y), z(v.z), w(v.w) {

    }
    
    template<typename T>
    base_vec4<T>::base_vec4(base_vec4<T>&& v) : 
    x(v.x), y(v.y), z(v.z), w(v.w) {

    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator= (const base_vec4<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator= (base_vec4<T>&& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        w = v.w;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>::~base_vec4() {

    }

    template<typename T>
    base_vec4<T> base_vec4<T>::add(const base_vec4<T>& v) const {
        return base_vec4<T>(x + v.x, y + v.y, z + v.z, w + v.w);
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::operator+(const base_vec4<T>& v) const {
        return add(v);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::add(const T& s) const {
        return base_vec4<T>(x + s, y + s, z + s, w + s);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator+(const T& s) const {
        return add(s);
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::sub(const base_vec4<T>& v) const {
        return base_vec4<T>(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator-(const base_vec4<T>& v) const {
        return sub(v);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::sub(const T& s) const {
        return base_vec4<T>(x - s, y - s, z - s, w - s);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator-(const T& s) const {
        return sub(s);
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::mul(const base_vec4<T>& v) const {
        return base_vec4<T>(x * v.x, y * v.y, z * v.z, w * v.w);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator*(const base_vec4<T>& v) const {
        return mul(v);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::mul(const T& s) const {
        return base_vec4<T>(x * s, y * s, z * s, w * s);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator*(const T& s) const {
        return mul(s);
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::div(const base_vec4<T>& v) const {
        return base_vec4<T>(x / v.x, y / v.y, z / v.z, w / v.w);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator/(const base_vec4<T>& v) const {
        return div(v);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::div(const T& s) const {
        return base_vec4<T>(x / s, y / s, z / s, w / s);
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::operator/(const T& s) const {
        return div(s);
    }

    template<typename T>
    T& base_vec4<T>::operator[] (std::size_t idx) {
        return get(idx);
    }
    
    template<typename T>
    const T& base_vec4<T>::operator[] (std::size_t idx) const {
        return get(idx);
    }

    template<typename T>
    T& base_vec4<T>::get(std::size_t idx) {
        JSTD_DEBUG_CODE(check_index(idx, 4));
        return arr[idx];
    }
    
    template<typename T>
    const T& base_vec4<T>::get(std::size_t idx) const {
        JSTD_DEBUG_CODE(check_index(idx, 4));
        return arr[idx];
    }

    template<typename T>
    T base_vec4<T>::dot(const base_vec4<T>& v) const {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }
    
    template<typename T>
    base_vec4<T> base_vec4<T>::normalized() const {
        const T len = length();
        return base_vec4<T>(x / len, y / len, z / len, w / len);
    }

    template<typename T>
    T base_vec4<T>::length() const {
        return (T) math::sqrt(x * x + y * y + z * z + w * w);
    }

    template<typename T>
    bool base_vec4<T>::equals(const base_vec4<T>& v) const {
        return x == v.x && v.y == v.y && z == v.z && w == v.w;
    }
    
    template<typename T>
    bool base_vec4<T>::operator== (const base_vec4<T>& v) const {
        return equals(v);
    }
    
    template<typename T>
    bool base_vec4<T>::operator!= (const base_vec4<T>& v) const {
        return !equals(v);
    }
    
    template<typename T>
    std::size_t base_vec4<T>::hashcode() const {
        return objects::hashcode(arr, sizeof(arr) / sizeof(*arr));
    }

    template<typename T>
    base_vec4<T>& base_vec4<T>::operator +=(const base_vec4<T>& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator -=(const base_vec4<T>& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator *=(const base_vec4<T>& other) {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator /=(const base_vec4<T>& other) {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator +=(const T& scalar) {
        x += scalar;
        y += scalar;
        z += scalar;
        w += scalar;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator -=(const T& scalar) {
        x -= scalar;
        y -= scalar;
        z -= scalar;
        w -= scalar;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator *=(const T& scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        w *= scalar;
        return *this;
    }
    
    template<typename T>
    base_vec4<T>& base_vec4<T>::operator /=(const T& scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        w /= scalar;
        return *this;
    }

    template<typename T>
    base_vec4<T>& base_vec4<T>::operator++() {
        x += 1;
        y += 1;
        z += 1;
        w += 1;
        return *this;
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::operator++(int) {
        base_vec4<T> old_value = *this;
        ++(*this);
        return old_value;
    }

    template<typename T>
    base_vec4<T>& base_vec4<T>::operator--() {
        x -= 1;
        y -= 1;
        z -= 1;
        w -= 1;
        return *this;
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::operator--(int) {
        base_vec4<T> old_value = *this;
        --(*this);
        return old_value;
    }

    template<typename T>
    base_vec4<T> base_vec4<T>::operator-() const {
        return base_vec4<T>(-x, -y, -z, -w);
    }

    template<typename T>
    tc::string base_vec4<T>::to_string(tca::allocator* alloc) const {
        tc::string result(alloc);
        
        result
        .append("[x=").append(tc::to_string(x)).append(',')
         .append("y=").append(tc::to_string(y)).append(',')
         .append("z=").append(tc::to_string(z)).append(',')
         .append("w=").append(tc::to_string(w)).append(']');
        
        return alloc;
    }

    template<typename T>
    template<typename E>
    base_vec4<T>::operator base_vec4<E>() const {
        return base_vec4<E>((E) x, (E) y, (E) z, (E) w);
    }
}

#endif//JSTD_CPP_LANG_MATH_VEC4_H