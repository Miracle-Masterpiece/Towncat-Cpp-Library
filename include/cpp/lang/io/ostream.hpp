#ifndef _JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_

#include <cstdint>
#include <cstddef>

namespace tc
{

/**
 * Абстрактный класс для работы с выходными потоками.
 *
 * Обеспечивает интерфейс для записи данных в различные выходные потоки, 
 * такие как файлы, буферы памяти или сетевые соединения.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию ostream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class ostream {
    ostream(const ostream&)               = delete; 
    ostream& operator= (const ostream&)   = delete;
public:
    /**
     * Конструктор по умолчанию.
     */
    ostream() {}
    
    /**
     * Записывает один символ в поток.
     * 
     * @param c 
     *      Символ для записи.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода
     */
    virtual void write(char c);

    /**
     * Записывает массив байтов в поток.
     * 
     * @param data 
     *      Указатель на массив байтов.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода
     */
    virtual void write(const char* data, std::size_t sz) = 0;

    /**
     * Сбрасывает буферизированные данные.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода
     */
    virtual void flush() = 0;

    /**
     * Закрывает поток.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода
     */
    virtual void close() = 0;

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    virtual ~ostream();
};

}
#endif//_JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_

