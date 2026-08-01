#ifndef _ALLOCATORS_LINEAR_COMPACT_ALLOCATOR_ALLOCATOR_H
#define _ALLOCATORS_LINEAR_COMPACT_ALLOCATOR_ALLOCATOR_H

#include <allocators/base_allocator.hpp>
#include <allocators/Helpers.hpp>
#include <allocators/pool_allocator.hpp>
#include <cpp/lang/utils/shared_ptr.hpp>

#include <cstring>
#include <cstddef>
#include <utility>
#include <cstdio>
#include <cstdint>

#ifndef NDEBUG
    #include <cassert>
    #define ALLOC_LIB_CHECK_ALIGN(ptr, type) assert( ( ((std::intptr_t) ptr) % alignof(type) ) == 0 )
    #define ALLOC_LIB_CHECK_NULL_POINTER(ptr) assert(ptr)
    #define ALLOC_LIB_CHECK_INDEX(idx, len) assert(idx < len);
#else
    #define ALLOC_LIB_CHECK_ALIGN(ptr, type)
    #define ALLOC_LIB_CHECK_NULL_POINTER(ptr)
    #define ALLOC_LIB_CHECK_INDEX(idx, len)
#endif

namespace tca {

    struct Header {
        
        tc::internal::sptr::shared_control_block * _reference;
        
        /**
         * Указатель на функцию, которая занимается перемещением данных из src в dst
         * 
         * @param dst
         *          Указатель на начало данных, откуда будет совершаться перемещение.
         * 
         * @param src
         *          Указатель на начало данных, куда будет совершаться перемещение.
         * 
         * @param count
         *          Количество объектов, выделенных аллокатором.
         *          Используется, если блок памяти аллоцировался, как массив.
         */
        void (*_movfunc) (void* dst, void* src, std::size_t count);
    
        //Количество непрерывно выделенных объектов.
        std::size_t _count;
    
        /**
         * Создаёт объект заголовка по-умолчанию
         * и всем полям присваивает null-значение.
         */
        Header();
        
        /**
         * Перемещает данные заголовка в текущий объект
         * и, если _reference != nullptr обновляет указатель на tca::Header на this.
         */
        Header(Header&&);    
        
        /**
         * Если _reference != nullptr, то в tca::Ref::_data устанавливается значение null.
         */
        ~Header();
        
        /**
         * Возвращает указатель на блок памяти, которые следуют после заголовка.
         *
         * Структура такая:
         * [header... padding][data... padding]
         * padding - неиспользуемые байты, которые нужны для выравнивания стурктур.
         * 
         * 
         * Implementation defined:
         * Указатель на данные высчитывается как ((char*) this) + tca::Header::byteSize()
         */
        void* dataPointer();
    
        /**
         * Возвращает выровненный размер структуры так, 
         * чтобы размер был кратен максимальному выравниванию.
         * 
         */
        constexpr static std::size_t byteSize() {
            return align_up(sizeof(Header), alignof(std::max_align_t));
        }
    
    private:
        /**
         * Удалённые методы копирования.
         */
        Header(const Header&) = delete;
        Header& operator= (const Header&)  = delete;
        Header& operator= (Header&&) = delete;
    };
    
    /**
     * Класс линейного рапределителя, который дефрагментирует кучу.
     * При этом класс является динамически-расширяемым, то есть, куча умеет увеличиваться.
     * 
     * Принцип работы такой:
     * 
     * Есть блок памяти
     * _data        [..........................................]    Блок памяти.
     * _capacity    <------------------------------------------>    Емкость(В байтах).
     * 
     * _offset      [################..........................] 
     *                               ^                              Смещение указателя для выделения.      
     * 
     * При выделении выделяется память не только для фактических данных, но и для заголовка tca::Header
     * Который хранит общий размер выделенной области (Вместе с размером заголовка), 
     * функцию для перемещения (Так как аллокатор знать не знает какие типы он должен аллоцировать),
     * Количество выделенных объектов (Для массивов, например) для использование этой информации в функции перемещения.
     * 
     * А так-же заголовок хранит указатель на владеющий объект-ссылки, чтобы при перемещении блоков памяти обновить указатель на актуальный.
     * Объект-ссылка так же хранит указатель на заголовок, чтобы при вызове деструктора объекта-ссылки пометить блок памяти, как свободный.
     */
    class compact_linear_allocator {
        /**
         * Удалённые конструкторы копирования и присваивания с копированием
         */
        compact_linear_allocator(const compact_linear_allocator&) = delete;
        compact_linear_allocator& operator= (const compact_linear_allocator&) = delete;
        
        //Указатель на базовый аллокатор.
        base_allocator*  _allocator;
       
        //Аллокатор для выделения памяти под tc::internal::sptr::shared_control_block.
        pool_allocator m_ctrl_block_allocator;

        //Указатель на блок памяти
        void*           _data;
        
        //Размер блока памяти, на который указывает CompactAllocator::_data 
        std::size_t     _capacity;
        
        //Указатель на блок памяти в который аллоцируются временные объекты при компактизации.
        void*           _tmp;
        
        //Размер блока памяти, на который указывает CompactAllocator::_tmp.
        std::size_t     _tmpCapacity;
    
        //Смещение для выделения новых объектов. 
        std::size_t     _offset;
    
        //Перемещение данных из src в dst
        void moveHeader(Header* dst, Header* src);
        
        //Расширение буфера в два раза.
        void grow();

        //Функция для выделения сырых байтов под данные, а так же установки фукнции для перемещения.
        tc::internal::sptr::shared_control_block* allocate(std::size_t sz, std::size_t count, void (*move_func)(void*, void*, std::size_t));

        /**
         * Функция перемещения по-умолчанию.
         * 
         * Так как аллокатор расчитан на то, что может выделять не только примитивные типы, но и сложные объекты,
         * то для сложных объектов необходимо вызывать конструктор перемещения! НЕ ОПЕРАТОР ПЕРЕМЕЩЕНИЯ!
         * 
         * Все объекты перемещаются только с помощью конструктора и конструкции вида:
         * {
         *      for (std::size_t i = 0; i < count; ++i)
         *          new(dst + i) T(std::move( *(src + i) );
         * }
         * 
         * Хотя функция и принимает аргменты, как void*, но так-как функция является шаблонной, в теле функции тип сохраняется.
         * 
         * Почему это нужно?
         * Чтобы сделать tca::Header максимально абстрактным.
         * tca::Header хранит указатель на сырые данные, размер и сколько фактически было выделено объектов и ничего больше.
         * 
         * Но при перемещении блоков нельзя просто вызываться std::memmove, то нужна так же и функция, которая так же не знает тип в параметре, 
         * но знает тип уже внутри своего тела.
         * 
         * Поэтой причине tca::Header добавляет +1 поле, которым является указатель на функцию перемещения. void (*move_func)(void*, void*);
         */
        template<typename T>
        static void mov(void* dst, void* src, std::size_t count);
    
    public:
        /**
         * Создаёт линейный аллокатор используя объект BaseAllocator и начальную емкость.
         * 
         * @param allocator
         *          Указатель на аллокатор из которого будет аллоцироваться память для текущего аллокатора.
         * 
         * @param capacity
         *          Начальная емкость аллокатора.
         */
        compact_linear_allocator(std::size_t capacity, base_allocator* allocator = get_scoped_or_default());
        
        /**
         * Перемещает данные из ранее выделенного аллокатора в этот аллокатор.
         */
        compact_linear_allocator(compact_linear_allocator&&);
        
        /**
         * Перемещает данные из ранее выделенного аллокатора в этот аллокатор.
         * При этом вызывая функцию compact_linear_allocator::free()
         */
        compact_linear_allocator& operator= (compact_linear_allocator&&);
    
        /**
         * Очищает выделенную аллокатором память.
         * 
         * @remark
         *      Все нетривиальные типы данных, которые были аллоцированы с помощью этого аллокатора или аллокаторов наследников,
         *      обязаны быть освобождены до вызова этого декстуктора, иначе поведение не определено!
         * 
         */
        ~compact_linear_allocator();
    
        /**
         * Выделяет память под объект типа T
         * 
         * @return 
         *      shared_ptr на объект.
         */
        template<typename T, typename... ARGS>
        tc::shared_ptr<T> allocate(ARGS&&... args);
        
        /**
         * Выделяет память под массив объект типа T
         * 
         * @return 
         *      shared_ptr на массив объектов.
         */
        template<typename T>
        tc::shared_ptr<T[]> allocate_array(uint32_t length);

        /**
         * Устанавливает новую ёмкость для внутреннего буфера аллокатора. (В байтах)
         * Новый размер обязан быть больше, чем текущая ёмкость.
         * 
         * @param newCapacity - новая ёмкость аллокатора.
         */
        void grow(std::size_t newCapacity);
        
        /**
         * Производит уплотнение выделенных блоков памяти, 
         * чтобы убрать фрагментацию или освободить место для новых аллокаций.
         */
        void compact();
        
        /**
         * Производит полное очищение аллокатора.
         *  @remark
         *      Все нетривиальные типы данных, которые были аллоцированы с помощью этого аллокатора или аллокаторов наследников,
         *      обязаны быть освобождены до вызова этой функции, иначе поведение не определено!
         */
        void free();
    
        /**
         * Выводит на консоль отладочную информацию аллокатора.
         * Функция использует std::printf
         */
        void print() const;
    };  

    template<typename T>
    /*static*/ void compact_linear_allocator::mov(void* dst, void* src, std::size_t count) {
        if (std::is_trivially_copyable<T>::value) {
            memcpy(dst, src, sizeof(T) * count);
        } else {
            using non_const_type = typename tc::remove_cv<T>::type;
            non_const_type* d = const_cast<non_const_type*>(reinterpret_cast<T*>(dst));
            non_const_type* s = const_cast<non_const_type*>(reinterpret_cast<T*>(src));
            for (std::size_t i = 0; i < count; ++i) {
                new(&d[i]) T(std::move(s[i]));
                s[i].~T();
            }
        }
    }

    template<typename T, typename... ARGS>
    tc::shared_ptr<T> compact_linear_allocator::allocate(ARGS&&... args) {
        tc::internal::sptr::shared_control_block* block = allocate(sizeof(T), 1, mov<T>);
        if (!block) return tc::shared_ptr<T>();
        new (block->m_object) T(args...);
        return tc::shared_ptr<T>(block);
    }

    template<typename T>
    tc::shared_ptr<T[]> compact_linear_allocator::allocate_array(uint32_t length) {
        tc::internal::sptr::shared_control_block* block = allocate(sizeof(T), length, mov<T>);
        if (!block) 
            return tc::shared_ptr<T[]>();
        using non_const_T = typename tc::remove_cv<T>::type;
        tc::placement_new(reinterpret_cast<non_const_T*>(block->m_object), length);
        return tc::shared_ptr<T[]>(block, length);
    }
}

#endif//_ALLOCATORS_LINEAR_COMPACT_ALLOCATOR_ALLOCATOR_H