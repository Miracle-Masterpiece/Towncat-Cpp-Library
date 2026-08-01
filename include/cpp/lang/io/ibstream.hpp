#ifndef _JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <allocators/allocator.hpp>

namespace tc
{

/**
 * Класс предназначен для буферезированного чтения данных.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию ibstream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class ibstream : public istream {
    static const int DEFAULT_BUFFER_SIZE = 4096;
    
    // 
    tca::allocator* _allocator;
    
    // 
    char* _buffer;
    
    // 
    std::size_t _capacity;
    
    // 
    std::size_t _offset;
    
    // 
    std::size_t _limit;
    
    // 
    istream* _in;

    // Особождает память, выделенную под буфер.
    // Если аллокатор != nullptr.
    // Если аллокатор == nullptr - значит использовался внешний буфер.
    void free();

    // 
    // 
    // 
    void fill_buffer();

public:
    /**
     * Создаёт пустой, не открытый, поток.
     */
    ibstream();
    
    /**
     * @param stream
     *          Поток из которого будут читаться данные.
     * 
     * @param allocator
     *          Распределитель памяти, который владеет внутренним буфером.
     * 
     * @param buf_size
     *          Необязательный параметр, задающий размер внутреннего буфера.
     */
    ibstream(istream* stream, std::size_t buf_size = DEFAULT_BUFFER_SIZE, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * @param stream
     *          Поток из которого будут читаться данные.
     * 
     * @param buf
     *          Буфер, который будет использоваться в качестве внутреннего буфеара.
     * 
     * @param buf_size
     *          Размер буфера.
     */
    ibstream(istream* stream, char* buf, std::size_t buf_size);

    /**
     * Перемещение
     */
    ibstream(ibstream&& stream);

    /**
     * Перемещение
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    ibstream& operator= (ibstream&& stream);

    /**
     * Читает один байт из потока.
     * 
     * @return 
     *      Значение прочитанного байта (0–255) или -1, если достигнут конец потока.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    int read() override;

    /**
     * Читает несколько байт из потока.
     * 
     * Заполняет переданный буфер данными из потока, но не более чем sz байт.
     * 
     * @param buf
     *      Указатель на буфер, в который будут записаны данные.
     * 
     * @param sz
     *      Максимальное количество байт для чтения.
     * 
     * @return 
     *      Количество реально прочитанных байт или -1, если достигнут конец потока.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    std::size_t read(char buf[], std::size_t sz) override;

    /**
     * Пропускает указанное количество байт во входном потоке.
     * 
     * @param n
     *      Количество байт для пропуска.
     * 
     * @return
     *      Количество фактически пропущенных байт.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    std::size_t skip(std::size_t n = 1) override;

    /**
     * Возвращает количество доступных для чтения байт.
     * 
     * Позволяет узнать, сколько данных можно прочитать без блокировки.
     * 
     * @return 
     *      Количество доступных байт в потоке.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    std::uintmax_t available() const override;

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~ibstream();

    /**
     * Закрывает поток файла.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     *      Если поток уже закрыт.
     */
    void close() override;

};

}
#endif//_JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_