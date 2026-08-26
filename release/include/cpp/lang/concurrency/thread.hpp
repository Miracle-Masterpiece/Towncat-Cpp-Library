#ifndef CPP_LANG_CONCURRENCY_THREAD_H
#define CPP_LANG_CONCURRENCY_THREAD_H

#include <cpp/lang/exceptions.hpp>
#include <thread>
#include <mutex>

#ifndef JSTD_THREAD_NAME_BUFFER
#define JSTD_THREAD_NAME_BUFFER 64
#endif

namespace tc {

struct runnable;

class thread {
public:
    struct state {
        /**
         * 
         */
        unsigned int m_state;
        
        /**
         * 
         */
        static const state NEW;
        
        /**
         * 
         */
        static const state RUNNABLE;
        
        /**
         * 
         */
        static const state TERMINATED;
        
        /**
         * 
         */
        bool operator==(const state& state) const;
        
        /**
         * 
         */
        bool operator!=(const state& state) const;
        
        /**
         * 
         */
        std::size_t hashcode() const;
        
        /**
         * 
         */
        bool equals(const state& other) const;
        
        /**
         * 
         */
        const char* to_string() const;
    };    
private:
    /**
     * Объект стандартной реализации потока.
     */
    std::thread m_thread;
    
    /**
     * Если указатель != nullptr,
     * то поток делегирует выполнение ему.
     */
    runnable*   m_runnable;

    /**
     * Идентификатор текущего потока.
     */
    std::size_t m_thread_id;

    /**
     * Имя потока.
     */
    char m_name[JSTD_THREAD_NAME_BUFFER];

    /**
     * Текущее состояние потока.
     */
    thread::state m_state;

    /**
     * Статическая переменная, увеличивающаяся каждый раз, когда вызывается любой конструктор этого класса.
     */
    static std::size_t total_threads;

    /**
     * Глобальный мьютекс для операций между потоков. 
     */
    static std::mutex global_lock;

    /**
     * Функция для выполнения в новом потоке.
     * Данная функция чисто утилитарная и должна вызывать функцию run у объекта jstd::thread.
     */
    friend void new_thread_start_func(thread*);
    
    /**
     * Конструктор копирования удалён.
     */
    thread(const thread&) = delete;
    
    /**
     * Оператор копирования удалён.
     */
    thread& operator= (const thread&) = delete;
public:    
    /**
     * Создаёт поток с именем по-умолчанию.
     * Данный конструктор по-большей части нужен для наследования.
     */
    thread();    
    
    /**
     * Создаёт поток с указанным объектом задачи, размером стека и именем.
     * 
     * @param task
     *      Объект задачи потока. Данный объект будет использоваться в качестве нового потока.
     * 
     * @param stack_size
     *      Размер стека потока.
     * 
     * @param thread_name
     *      Имя потока.
     */
    thread(runnable* task, const char* thread_name = nullptr, std::size_t stack_size = 0);

    /**
     * Конструктор перемещения.
     * 
     * Перемещает объект потока из другого экземпляра. Новый поток получает право
     * управления системным потоком из объекта other. 
     * Объект other после перемещения становится пустым не joinable.
     *
     * @param other
     *      Поток, из которого переносится состояние.
     */
    thread(thread&&);
    
    /**
     * Оператор перемещения.
     *
     * Присваивает текущему объекту поток из другого экземпляра, используя семантику перемещения.
     * Системный поток передаётся от other к *this, а other становится пустым.
     *
     * @param other
     *      Поток, из которого перемещается состояние.
     *
     * @return
     *      Ссылка на текущий объект после перемещения.
     *
     * @throws illegal_state_exception
     *      Вызывается, если текущий поток всё ещё представляет
     *      joinable-поток то есть joinable() == true в момент присваивания.
     * 
     * @warning
     *      Оператор не выполняет {@code join()} автоматически.
     *      Перед перемещением вызывающий код обязан гарантировать, что поток завершён или отсоединён.
     */
    thread& operator= (thread&&);

    /**
     * Запускает поток и вызывает функцию run().
     */
    void start();
    
    /**
     * Фукнция, которая запускается в отдельном потоке.
     */
    virtual void run();
    
    /**
     * 
     */
    virtual ~thread();

    /**
     * Возвращает имя потока.
     */
    const char* get_name() const;

    /**
     * Устанавливает имя потока.
     * 
     * @param new_name
     *      Новое имя потока.
     * 
     * @note
     *      Если входящее имя слишком большое, оно будет обрезано!
     */
    void set_name(const char* new_name);

    /**
     * @throws interrupted_exception
     *      Eсли какой-либо поток прервал текущий поток.
     */
    void join();

    /**
     * Проверяет, std::thread идентифицирует ли объект активный поток выполнения. 
     * Таким образом, если у объекта не был вызван метод start(), то поток не может быть присоединён.
     * 
     * Поток, который завершил выполнение кода, но еще не был присоединен, 
     * все еще считается активным потоком выполнения и, следовательно, может быть присоединен.
     */
    bool joinable() const;

    /**
     * Возвращает текущее состояние потока.
     */
    state get_state() const;
};


}

#endif//CPP_LANG_CONCURRENCY_THREAD_H