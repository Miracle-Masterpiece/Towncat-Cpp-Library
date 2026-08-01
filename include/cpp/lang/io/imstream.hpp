#ifndef _JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <allocators/base_allocator.hpp>

namespace tc {

/**
 * Класс предназначен для представления массива char, как входного потока.
 */
class imstream : public istream {
    const char* _buffer;    
    std::size_t _capacity;
    std::size_t _offset;
public:
    /**
     * Пустой поток ввода.
     */
    imstream();
    
    /**
     * @param buffer
     *      Буфер, откуда будут читаться данные.
     * 
     * @param capacity
     *      Размер буфера, означающий сколько байт доступно для чтения.
     */
    imstream(const char* buffer, std::size_t capacity);
    
    //перемещение
    imstream(imstream&&);
    
    //перемещение
    imstream& operator= (imstream&&);
    
    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~imstream();
    
    /**
     * Читает один байт из потока.
     * 
     * @return 
     *      Значение прочитанного байта (0–255) или -1, если достигнут конец потока.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода
     *      Если поток не был открыт.
     *      Если поток уже закрыт.
     */
    int read() override;
    
    /**
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
     *      Если поток уже закрыт.
     */
    std::size_t read(char buf[], std::size_t sz) override;
    
     /**
     * Возвращает количество доступных для чтения байт.
     * 
     * @return Количество доступных байт в потоке.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     *      Если поток уже закрыт.
     */
    std::uintmax_t available() const override;
    
    /**
     * Закрывает поток.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    void close() override;
    
    /**
     * Сбрасывает текущее смещение и начинает читать данные сначала.
     * Можно сказать, что переоткрывает поток.
     */
    void reset();
};


}

#endif//_JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_