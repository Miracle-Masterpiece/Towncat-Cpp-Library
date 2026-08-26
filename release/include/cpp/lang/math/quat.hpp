#ifndef JSTD_CPP_LANG_MATH_QUAT_H
#define JSTD_CPP_LANG_MATH_QUAT_H

#include <internal/math_defs.hpp>
#include <cpp/lang/math/math.hpp>
#include <cpp/lang/math/vec3.hpp>
#include <cpp/lang/string.hpp

namespace tc
{

template<typename T>
struct base_quat;

typedef base_quat<float> quat;
typedef base_quat<double> quatd;
typedef base_quat<long double> quatld;

/**
 * Шаблонная структура для представления кватернионов.
 * 
 * Представляет кватернион в виде четырёх компонент: i, j, k (мнимые части) и a (действительная часть).
 * 
 * Также доступ к компонентам можно получить через альтернативные имена: x, y, z, w.
 * 
 * @tparam T 
 *      Тип компонент кватерниона (например, float, double).
 */
template<typename T>
struct base_quat {

    union {
        T i, x;
    };

    union {
        T j, y;
    };

    union {
        T k, z;
    };

    union {
        T a, w;
    };

    /**
     * Конструктор с параметрами по умолчанию.
     * 
     * @param i 
     *      Значение компоненты i (по умолчанию T()).
     * 
     * @param j 
     *      Значение компоненты j (по умолчанию T()).
     * 
     * @param k 
     *      Значение компоненты k (по умолчанию T()).
     * 
     * @param a 
     *      Значение компоненты a (по умолчанию T()).
     */
    base_quat(const T& i = T(), const T& j = T(), const T& k = T(), const T& a = T());

    /**
     * Конструктор копирования.
     * 
     * @param other 
     *      Кватернион, из которого копируются значения.
     */
    base_quat(const base_quat<T>& other);

    /**
     * Конструктор перемещения.
     * 
     * @param other 
     *      Кватернион, значения которого перемещаются.
     */
    base_quat(base_quat<T>&& other);

    /**
     * Оператор копирующего присваивания.
     * 
     * @param other 
     *      Кватернион, значения которого присваиваются.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_quat<T>& operator=(const base_quat<T>& other);

    /**
     * Оператор перемещающего присваивания.
     * 
     * @param other 
     *      Кватернион, значения которого перемещаются.
     * 
     * @return 
     *      Ссылка на текущий объект.
     */
    base_quat<T>& operator=(base_quat<T>&& other);

    /**
     * Выполняет поэлементное сложение двух кватернионов.
     * 
     * @param rhs 
     *      Второй операнд.
     * 
     * @return 
     *      Новый кватернион, являющийся результатом сложения.
     */
    base_quat<T> add(const base_quat<T>& rhs);

    /**
     * Оператор поэлементного сложения двух кватернионов.
     * 
     * @param rhs 
     *      Второй операнд.
     * 
     * @return 
     *      Новый кватернион, являющийся результатом сложения.
     */
    base_quat<T> operator+(const base_quat<T>& rhs);

    /**
     * Выполняет поэлементное вычитание двух кватернионов.
     * 
     * @param rhs 
     *      Второй операнд.
     * 
     * @return 
     *      Новый кватернион, являющийся результатом вычитания.
     */
    base_quat<T> sub(const base_quat<T>& rhs);

    /**
     * Оператор поэлементного вычитания двух кватернионов.
     * 
     * @param rhs 
     *      Второй операнд.
     * 
     * @return 
     *      Новый кватернион, являющийся результатом вычитания.
     */
    base_quat<T> operator-(const base_quat<T>& rhs);

    /**
     * Выполняет умножение кватернионов по правилам алгебры кватернионов.
     * 
     * @param rhs 
     *      Второй множитель.
     * 
     * @return 
     *      Новый кватернион, являющийся результатом умножения.
     */
    base_quat<T> mul(const base_quat<T>& rhs);

    /**
     * Оператор умножения кватернионов.
     * 
     * @param rhs 
     *      Второй множитель.
     * 
     * @return 
     *      Новый кватернион, являющийся результатом умножения.
     */
    base_quat<T> operator*(const base_quat<T>& rhs);

    /**
     * Вычисляет норму (сумму квадратов компонент) кватерниона.
     * 
     * @return 
     *      Норма кватерниона.
     */
    T norm() const;

    /**
     * Вычисляет длину (модуль) кватерниона.
     * 
     * @return 
     *      Длина кватерниона.
     */
    T magnitude() const;

    /**
     * Вычисляет длину (модуль) кватерниона.
     * 
     * @return 
     *      Длина кватерниона.
     */    
    T length() const;

    /**
     * Вычисляет сопряжённый кватернион.
     * 
     * Сопряжённый кватернион инвертирует знак мнимых компонент.
     * 
     * @return 
     *      Сопряжённый кватернион.
     */
    base_quat<T> conjugate() const;

    /**
     * Вычисляет обратный кватернион.
     * 
     * Обратный кватернион равен сопряжённому, делённому на квадрат длины.
     * 
     * @return 
     *      Обратный кватернион.
     */
    base_quat<T> inverse() const;

    /**
     * Возвращает результат покомпонентного умножения данного кватерниона на скаляр.
     *
     * @param scalar 
     *      Скаляр, на который нужно умножить кватернион.
     * 
     * @return 
     *      Новый кватернион, полученный покомпонентным умножением на scalar.
     */
    base_quat<T> mul(const T& scalar) const;

    /**
     * Перегрузка оператора умножения: выполняет покомпонентное умножение кватерниона на скаляр.
     *
     * @param scalar 
     *      Скаляр, на который производится умножение.
     * 
     * @return 
     *      Новый кватернион, результат умножения.
     */
    base_quat<T> operator*(const T& scalar) const;

    /**
     * Возвращает результат покомпонентного деления данного кватерниона на скаляр.
     *
     * @param scalar 
     *      Скаляр, на который нужно разделить компоненты кватерниона.
     * 
     * @return 
     *      Новый кватернион, полученный делением на scalar.
     * 
     * @warning 
     *      При делении на 0 возвращается единичный кватернион.
     */
    base_quat<T> div(const T& scalar) const;

    /**
     * Перегрузка оператора деления: выполняет покомпонентное деление кватерниона на скаляр.
     *
     * @param scalar 
     *      Скаляр, на который производится деление.
     * 
     * @return 
     *      Новый кватернион, результат деления.
     * 
     * @warning 
     *      При делении на 0 возвращается единичный кватернион.
     */
    base_quat<T> operator/(const T& scalar) const;

    /**
     * Возвращает нормализованную копию данного кватерниона.
     *
     * @return Новый кватернион с единичной длиной (модуль равен 1).
     *         Если исходный модуль слишком мал, возвращается кватернион без вращения (0, 0, 0, 1).
     */
    base_quat<T> normalized() const;

    /**
     * Проверяет равенство данного кватерниона с другим покомпонентно.
     *
     * @param other 
     *      Кватернион, с которым производится сравнение.
     * 
     * @return 
     *      true, если все соответствующие компоненты равны; false в противном случае.
     */
    bool equals(const base_quat<T>& other) const;

    /**
     * Перегрузка оператора сравнения на равенство.
     *
     * @param other 
     *      Кватернион, с которым производится сравнение.
     * 
     * @return 
     *      true, если все компоненты равны; false в противном случае.
     */
    bool operator==(const base_quat<T>& other) const;

    /**
     * Перегрузка оператора сравнения на неравенство.
     *
     * @param other 
     *      Кватернион, с которым производится сравнение.
     * 
     * @return 
     *      true, если хотя бы одна из компонент не равна; false, если все компоненты равны.
     */
    bool operator!=(const base_quat<T>& other) const;

    /**
     * Преобразует кватернион в строковое представление.
     */
    tc::string to_string(tca::allocator* = tca::get_default_allocator()) const;

    /**
     * Возвращает хеш-код кватерниона.
     */
    std::size_t hashcode() const;
};


    template<typename T>
    base_quat<T>::base_quat(const T& i, const T& j, const T& k, const T& a) : 
    i(i), j(j), k(k), a(a){
        
    }

    template<typename T>
    base_quat<T>::base_quat(const base_quat<T>& q) : 
    i(q.i), j(q.j), k(q.k), a(q.a) {
        
    }
    
    template<typename T>
    base_quat<T>::base_quat(base_quat<T>&& q) : 
    i(q.i), j(q.j), k(q.k), a(q.a) {
        
    }
    
    template<typename T>
    base_quat<T>& base_quat<T>::operator= (const base_quat<T>& q) {
        i = q.i;
        j = q.j;
        k = q.k;
        a = q.a;
        return *this;
    }
    
    template<typename T>
    base_quat<T>& base_quat<T>::operator= (base_quat<T>&& q) {
        i = q.i;
        j = q.j;
        k = q.k;
        a = q.a;
        return *this;
    }

    template<typename T>
    base_quat<T> base_quat<T>::add(const base_quat<T>& q) {
        return base_quat<T>(i + q.i, j + q.j, k + q.k, a + q.a);
    }
    
    template<typename T>
    base_quat<T> base_quat<T>::operator+(const base_quat<T>& q) {
        return add(q);
    }

    template<typename T>
    base_quat<T> base_quat<T>::sub(const base_quat<T>& q) {
        return base_quat<T>(i - q.i, j - q.j, k - q.k, a - q.a);
    }
    
    template<typename T>
    base_quat<T> base_quat<T>::operator-(const base_quat<T>& q) {
        return sub(q);
    }

    template<typename T>
    T base_quat<T>::norm() const {
        return i*i + j*j + k*k;
    }

    template<typename T>
    T base_quat<T>::magnitude() const {
        return math::sqrt(norm());
    }

    template<typename T>
    T base_quat<T>::length() const {
        return magnitude();
    }

    template<typename T>
    base_quat<T> base_quat<T>::conjugate() const {
        return base_quat<T>(-i, -j, -k, a);
    }

    template<typename T>
    base_quat<T> base_quat<T>::mul(const base_quat<T>& q) {

        /**
         * Умножение двух кватернионов можно записать в виде:
         * qq' = [ vv' + wv' + w'v, ww' – v•v' ] где vv' — векторное произведение, v•v' — скалярное произведение векторов.
         */

        const base_vec3<T> v(x,y,z);
        const base_vec3<T> v2(q.x, q.y, q.z);
        const base_vec3<T> r = v.cross(v2) + w * v2 + q.w * v;
        const T w2 = w * q.w - v.dot(v2);

        return base_quat<T>(r.x, r.y, r.z, w2);
    }
    
    template<typename T>
    base_quat<T> base_quat<T>::inverse() const {
        const T EPSILON = (const T) 0.000001;
        const T n = norm();
        if (n < EPSILON) {
            return base_quat<T>(0, 0, 0, 1);
        } else {
            return base_quat<T>(-i / n, -j / n, -k / n, a / n);
        }
    }

    template<typename T>
    base_quat<T> base_quat<T>::operator*(const base_quat<T>& q) {
        return mul(q);
    }
    
    template<typename T>
    bool base_quat<T>::equals(const base_quat<T>& other) const {
        return i == other.i && j == other.j && k == other.k && a == other.a;
    }
    
    template<typename T>
    bool base_quat<T>::operator==(const base_quat<T>& other) const {
        return equals(other);
    }
    
    template<typename T>
    bool base_quat<T>::operator!=(const base_quat<T>& other) const {
        return !equals(other);
    }
    
    template<typename T>
    base_quat<T> base_quat<T>::mul(const T& scalar) const {
        return base_quat<T>(i * scalar, j * scalar, k * scalar, a * scalar);
    }
    
    template<typename T>
    base_quat<T> base_quat<T>::operator*(const T& scalar) const {
        return mul(scalar);
    }

    template<typename T>
    base_quat<T> operator*(const T& scalar, const base_quat<T>& q) {
        return q * scalar;
    }

    template<typename T>
    base_quat<T> base_quat<T>::div(const T& scalar) const {
        const T EPSILON = (const T) 0.000001;
        if (math::abs(scalar) > EPSILON)
        {
            return base_quat<T>(i / scalar, j / scalar, k / scalar, a / scalar);
        }
        else
        {
            return base_quat<T>(0, 0, 0, 1);
        }
    }
    
    template<typename T>
    base_quat<T> base_quat<T>::operator/(const T& scalar) const {
        return div(scalar);
    }

    template<typename T>
    base_quat<T> base_quat<T>::normalized() const {
        const T mag = magnitude();
        const T EPSILON = (const T) 0.000001;
        if (mag < EPSILON)
        {
            return base_quat<T>(0, 0, 0, 1);
        }
        else
        {
            return base_quat<T>(i / mag, j / mag, k / mag, a / mag);
        }
    }

    template<typename T>
    tc::string base_quat<T>::to_string(tca::allocator* alloc) const {
        tc::string result(alloc);
        
        result
        .append("[i=").append(tc::to_string(x)).append(',')
         .append("j=").append(tc::to_string(y)).append(',')
         .append("k=").append(tc::to_string(z)).append(',')
         .append("a=").append(tc::to_string(w)).append(']');
        
        return alloc;
    }

    template<typename T>
    std::size_t base_quat<T>::hashcode() const {
        const T arr[] = {i, j, k, a};
        return objects::hashcode(arr, sizeof(arr) / sizeof(*arr));
    }    
}

#endif//JSTD_CPP_LANG_MATH_QUAT_H
