#ifndef CPP_LANG_CONCURRENCY_RUNNABLE_H
#define CPP_LANG_CONCURRENCY_RUNNABLE_H

namespace tc {
    
/**
 * Абстрактная структура, представляющая исполнимый объект, который может быть выполнен.
 * Структура предоставляет интерфейс для классов, которые реализуют поведение при запуске.
 * Этот интерфейс требует реализации метода `run()`, который должен содержать логику исполнения.
 */
struct runnable {
    /**
     * Фукнция, которая будет запущена другим потоком.
     */
    virtual void run() = 0;
    
    virtual ~runnable() {}
};

}

#endif//CPP_LANG_CONCURRENCY_RUNNABLE_H
