#ifndef JSTD_CPP_LANG_MATH_VEC3_H
#define JSTD_CPP_LANG_MATH_VEC3_H

#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/math/math.hpp>
#include <cpp/lang/string.hpp>

namespace tc {

template<typename T>
struct base_vec3;

typedef base_vec3<float>  vec3;
typedef base_vec3<double> vec3d;
typedef base_vec3<long double> vec3ld;

typedef base_vec3<short> vec3s;
typedef base_vec3<signed int> vec3i;
typedef base_vec3<signed long> vec3l;
typedef base_vec3<signed long long> vec3ll;

typedef base_vec3<unsigned short> vec3us;
typedef base_vec3<unsigned int> vec3u;
typedef base_vec3<unsigned long> vec3ul;
typedef base_vec3<unsigned long long> vec3ull;

/**
 * Обобщённый трёхкомпонентный вектор.
 * 
 * Представляет собой структуру, содержащую три компоненты — x, y, z — и предоставляет
 * базовые арифметические, векторные и служебные операции. 
 * 
 * Поддерживает доступ к данным как по координатам (x, y, z), так и по цветовому представлению (r, g, b).
 * 
 * @tparam T 
 *      Тип компонентов вектора (например, float, double, int).
 */
template<typename T>
struct base_vec3 {

    union{T x, r, A;};
    union{T y, g, B;};
    union{T z, b, C;};

    /**
     * Конструктор по значениям компонентов.
     * 
     * @param x Значение X-компоненты.
     * @param y Значение Y-компоненты.
     * @param z Значение Z-компоненты.
     */
    base_vec3(const T& x = T(), const T& y = T(), const T& z = T());

    /**
     * Конструктор копирования.
     * 
     * @param other 
     *      Вектор, который будет скопирован.
     */
    base_vec3(const base_vec3<T>& other);

    /**
     * Конструктор перемещения.
     * 
     * @param other 
     *      Вектор, из которого будут перемещены данные.
     */
    base_vec3(base_vec3<T>&& other);

    /**
     * Оператор копирующего присваивания.
     * 
     * @param other 
     *      Вектор для копирования.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_vec3<T>& operator=(const base_vec3<T>& other);

    /**
     * Оператор перемещающего присваивания.
     * 
     * @param other 
     *      Вектор для перемещения.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_vec3<T>& operator=(base_vec3<T>&& other);

    /**
     *
     */
    ~base_vec3();

    /**
     * Сложение с другим вектором.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Сумма двух векторов.
     */
    base_vec3<T> add(const base_vec3<T>& other) const;

    /**
     * Оператор сложение с другим вектором.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Сумма двух векторов.
     */
    base_vec3<T> operator+(const base_vec3<T>& other) const;

    /**
     * Сложение с числом (покомпонентно).
     * 
     * @param scalar 
     *      Скалярное значение.
     * 
     * @return 
     *      Результат сложения.
     */
    base_vec3<T> add(const T& scalar) const;

    /**
     * Оператор сложение с числом (покомпонентно).
     * 
     * @param scalar 
     *      Скалярное значение.
     * 
     * @return 
     *      Результат сложения.
     */
    base_vec3<T> operator+(const T& scalar) const;

    /**
     * Вычитание другого вектора.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Разность двух векторов.
     */
    base_vec3<T> sub(const base_vec3<T>& other) const;

    /**
     * Оператор вычитания другого вектора.
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Разность двух векторов.
     */
    base_vec3<T> operator-(const base_vec3<T>& other) const;

    /**
     * Вычитание скаляра (покомпонентно).
     */
    base_vec3<T> sub(const T& scalar) const;

    /**
     * Оператор вычитания скаляра.
     */
    base_vec3<T> operator-(const T& scalar) const;

    /**
     * Умножение на вектор (покомпонентное).
     */
    base_vec3<T> mul(const base_vec3<T>& other) const;

    /**
     * Оператор покомпонентного умножения.
     */
    base_vec3<T> operator*(const base_vec3<T>& other) const;

    /**
     * Умножение на скаляр.
     */
    base_vec3<T> mul(const T& scalar) const;

    /**
     * Оператор умножения на скаляр.
     */
    base_vec3<T> operator*(const T& scalar) const;

    /**
     * Деление на вектор (покомпонентное).
     */
    base_vec3<T> div(const base_vec3<T>& other) const;

    /**
     * Оператор покомпонентного деления.
     */
    base_vec3<T> operator/(const base_vec3<T>& other) const;

    /**
     * Деление на скаляр.
     */
    base_vec3<T> div(const T& scalar) const;

    /**
     * Оператор деления на скаляр.
     */
    base_vec3<T> operator/(const T& scalar) const;

    /**
     * Доступ к компоненте по индексу.
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    T& operator[](std::size_t index);

    /**
     * Доступ к компоненте по индексу. (Константный)
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    const T& operator[](std::size_t index) const;

    /**
     * Доступ к компоненте по индексу.
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    T& get(std::size_t index);

    /**
     * Доступ к компоненте по индексу. (Константный)
     * 
     * @param index 
     *      Индекс (0 - x, 1 - y, 2 - z).
     * 
     * @return 
     *      Ссылка на компоненту.
     */
    const T& get(std::size_t index) const;
    
    /**
     * Скалярное произведение (dot product).
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Скалярное произведение.
     */
    T dot(const base_vec3<T>& other) const;

    /**
     * Векторное произведение (cross product).
     * 
     * @param other 
     *      Второй вектор.
     * 
     * @return 
     *      Вектор, перпендикулярный двум исходным.
     */
    base_vec3<T> cross(const base_vec3<T>& other) const;

    /**
     * Нормализация вектора.
     * 
     * @return 
     *      Единичный вектор с тем же направлением.
     */
    base_vec3<T> normalized() const;

    /**
     * Длина (модуль) вектора.
     * 
     * @return 
     *      Длина вектора.
     */
    T length() const;

    /**
     * Проверка на равенство по компонентам.
     * 
     * @param other 
     *      Вектор для сравнения.
     * 
     * @return 
     *      true, если все компоненты равны.
     */
    bool equals(const base_vec3<T>& other) const;

    /**
     * Оператор сравнения на равенство.
     */
    bool operator==(const base_vec3<T>& other) const;

    /**
     * Оператор сравнения на неравенство.
     */
    bool operator!=(const base_vec3<T>& other) const;

    /**
     * Вычисление хеш-кода по компонентам.
     * 
     * @return 
     *      64-битное хеш-значение.
     */
    std::size_t hashcode() const;

    /**
     * Преобразует вектор в строковое представление.
     */
    tc::string to_string(tca::allocator* = tca::get_default_allocator()) const;

    /**
     * Оператор сложения с другим вектором (на месте).
     * 
     * Прибавляет компоненты другого вектора к текущему вектору.
     * 
     * @param other 
     *      Вектор, который прибавляется.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator +=(const base_vec3<T>& other);

    /**
     * Оператор вычитания другого вектора (на месте).
     * 
     * Вычитает компоненты другого вектора из текущего.
     * 
     * @param other 
     *      Вектор, который вычитается.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator -=(const base_vec3<T>& other);

    /**
     * Оператор поэлементного умножения с другим вектором (на месте).
     * 
     * Умножает соответствующие компоненты другого вектора на текущий.
     * 
     * @param other 
     *      Вектор, на который умножаются компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator *=(const base_vec3<T>& other);

    /**
     * Оператор поэлементного деления на другой вектор (на месте).
     * 
     * Делит компоненты текущего вектора на соответствующие компоненты другого.
     * 
     * @param other 
     *      Вектор, на компоненты которого происходит деление.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator /=(const base_vec3<T>& other);

    /**
     * Оператор прибавления скаляра ко всем компонентам вектора (на месте).
     * 
     * @param scalar 
     *      Значение, прибавляемое к каждой компоненте.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator +=(const T& scalar);

    /**
     * Оператор вычитания скаляра из всех компонентов вектора (на месте).
     * 
     * @param scalar 
     *      Значение, вычитаемое из каждой компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator -=(const T& scalar);

    /**
     * Оператор умножения всех компонентов вектора на скаляр (на месте).
     * 
     * @param scalar 
     *      Значение, на которое умножаются компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator *=(const T& scalar);

    /**
     * Оператор деления всех компонентов вектора на скаляр (на месте).
     * 
     * @param scalar 
     *      Значение, на которое делятся компоненты.
     * 
     * @return 
     *      Ссылка на текущий вектор после изменения.
     */
    base_vec3<T>& operator /=(const T& scalar);

    /**
     * Префиксный инкремент (увеличение всех компонентов на 1).
     * 
     * @return 
     *      Ссылку на текущий вектор после инкремента.
     */
    base_vec3<T>& operator++();

    /**
     * Постфиксный инкремент (увеличение всех компонентов на 1).
     * 
     * @return 
     *      Копию вектора до инкремента.
     */
    base_vec3<T> operator++(int);

    /**
     * Префиксный декремент (уменьшение всех компонентов на 1).
     * 
     * @return 
     *      Ссылку на текущий вектор после декремента.
     */
    base_vec3<T>& operator--();

    /**
     * Постфиксный декремент (уменьшение всех компонентов на 1).
     * 
     * @return 
     *      Копию вектора до декремента.
     */
    base_vec3<T> operator--(int);

    /**
     * Унарный минус (инвертирует все компоненты вектора).
     *
     * @return
     *     Новый вектор с противоположными по знаку компонентами.
     */
    base_vec3<T> operator-() const;

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
    explicit operator base_vec3<E>() const;
};


    template<typename T>
    base_vec3<T>::base_vec3(const T& x, const T& y, const T& z) :
    x(x), y(y), z(z) {

    }
    
    template<typename T>
    base_vec3<T>::base_vec3(const base_vec3<T>& v) : 
    x(v.x), y(v.y), z(v.z) {

    }
    
    template<typename T>
    base_vec3<T>::base_vec3(base_vec3<T>&& v) : 
    x(v.x), y(v.y), z(v.z) {

    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator= (const base_vec3<T>& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator= (base_vec3<T>&& v) {
        x = v.x;
        y = v.y;
        z = v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>::~base_vec3() {

    }

    template<typename T>
    base_vec3<T> base_vec3<T>::add(const base_vec3<T>& v) const {
        return base_vec3<T>(x + v.x, y + v.y, z + v.z);
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::operator+(const base_vec3<T>& v) const {
        return add(v);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::add(const T& s) const {
        return base_vec3<T>(x + s, y + s, z + s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator+(const T& s) const {
        return add(s);
    }

    template<typename T>
    base_vec3<T> operator+(const T& s, const base_vec3<T>& v) {
        return v + s;
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::sub(const base_vec3<T>& v) const {
        return base_vec3<T>(x - v.x, y - v.y, z - v.z);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator-(const base_vec3<T>& v) const {
        return sub(v);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::sub(const T& s) const {
        return base_vec3<T>(x - s, y - s, z - s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator-(const T& s) const {
        return sub(s);
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::mul(const base_vec3<T>& v) const {
        return base_vec3<T>(x * v.x, y * v.y, z * v.z);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator*(const base_vec3<T>& v) const {
        return mul(v);
    }
    
    template<typename T>
    base_vec3<T> operator*(const T& s, const base_vec3<T>& v) {
        return v * s;
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::mul(const T& s) const {
        return base_vec3<T>(x * s, y * s, z * s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator*(const T& s) const {
        return mul(s);
    }

    template<typename T>
    base_vec3<T> base_vec3<T>::div(const base_vec3<T>& v) const {
        return base_vec3<T>(x / v.x, y / v.y, z / v.z);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator/(const base_vec3<T>& v) const {
        return div(v);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::div(const T& s) const {
        return base_vec3<T>(x / s, y / s, z / s);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator/(const T& s) const {
        return div(s);
    }

    template<typename T>
    T& base_vec3<T>::operator[] (std::size_t index) {
        return get(index);
    }
    
    template<typename T>
    const T& base_vec3<T>::operator[] (std::size_t index) const {
        return get(index);
    }

    template<typename T>
    T& base_vec3<T>::get(std::size_t idx) {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default:
                throw make_except<index_out_of_bound_exception>("%zu out of bound vec3", idx);
        }
    }

    template<typename T>
    const T& base_vec3<T>::get(std::size_t idx) const {
        switch (idx) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default:
                throw make_except<index_out_of_bound_exception>("%zu out of bound vec3", idx);
        }
    }

    template<typename T>
    T base_vec3<T>::dot(const base_vec3<T>& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::cross(const base_vec3<T>& v) const {
        const T nx = (y * v.z) - (z * v.y);
        const T ny = (z * v.x) - (x * v.z);
        const T nz = (x * v.y) - (y * v.x);
        return base_vec3<T>(nx, ny, nz);
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::normalized() const {
        const T len = length();
        return base_vec3<T>(x / len, y / len, z / len);
    }

    template<typename T>
    T base_vec3<T>::length() const {
        return (T) math::sqrt(x * x + y * y + z * z);
    }

    template<typename T>
    bool base_vec3<T>::equals(const base_vec3<T>& v) const {
        return x == v.x && v.y == v.y && z == v.z;
    }
    
    template<typename T>
    bool base_vec3<T>::operator== (const base_vec3<T>& v) const {
        return equals(v);
    }
    
    template<typename T>
    bool base_vec3<T>::operator!= (const base_vec3<T>& v) const {
        return !equals(v);
    }
    
    template<typename T>
    std::size_t base_vec3<T>::hashcode() const {
        const T tmp[] = {x, y, z};
        return objects::hashcode(tmp, sizeof(tmp) / sizeof(T));
    }

    template<typename T>
    base_vec3<T>& base_vec3<T>::operator +=(const base_vec3<T>& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator -=(const base_vec3<T>& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator *=(const base_vec3<T>& v) {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator /=(const base_vec3<T>& v) {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }


    template<typename T>
    base_vec3<T>& base_vec3<T>::operator +=(const T& s) {
        x += s;
        y += s;
        z += s;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator -=(const T& s) {
        x -= s;
        y -= s;
        z -= s;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator *=(const T& s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator /=(const T& s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    template<typename T>
    tc::string base_vec3<T>::to_string(tca::allocator* alloc) const {
        tc::string result(alloc);
        result
        .append("[x=").append(tc::to_string(x)).append(',')
         .append("y=").append(tc::to_string(y)).append(',')
         .append("z=").append(tc::to_string(z)).append(']');
        return result;
    }

    template<typename T>
    base_vec3<T>& base_vec3<T>::operator++() {
        ++x; ++y; ++z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator++(int) {
        base_vec3<T> old_value = *this;
        ++(*this);
        return old_value;
    }
    
    template<typename T>
    base_vec3<T>& base_vec3<T>::operator--() {
        --x; --y; --z;
        return *this;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator--(int) {
        base_vec3<T> old_value = *this;
        --(*this);
        return old_value;
    }
    
    template<typename T>
    base_vec3<T> base_vec3<T>::operator-() const {
        return base_vec3<T>(-x, -y, -z);
    }

    template<typename T>
    template<typename E>
    base_vec3<T>::operator base_vec3<E>() const {
        return base_vec3<E>((E) x, (E) y, (E) z);
    }
}
#endif//JSTD_CPP_LANG_MATH_VEC3_H