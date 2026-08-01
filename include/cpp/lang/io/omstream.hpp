#ifndef _JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <allocators/allocator.hpp>

namespace tc {

/**
 * Класс предназначен для представления байтового массива, как выходной поток в который сохраняются данные.
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию omstream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class omstream : public ostream {
    static const std::size_t INIT_BUF_SIZE = 16;
    
    // Может быть nullptr, тогда это означает, что this->_buffer является внешним массивом.
    tca::allocator* _allocator;   
    
    // 
    char* _buffer;
    
    // 
    std::size_t _capacity;
    
    // 
    std::size_t _offset;
    
    // 
    void resize(std::size_t sz);
public:

    /**
     * Инициализирует класс с помощью аллокатора и начальным размером буфера.
     * 
     * @param allocator
     *      Аллокатор, который управляет памятью.
     * 
     * @param init_buf_size
     *      Начальный размер буфера.
     * 
     * @remark
     *      Буфер можно быть выделен лениво, при первом использовании.
     */
    omstream(std::size_t init_buf_size = INIT_BUF_SIZE, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Инициализирует класс уже существующим массивом.
     * 
     * @param buf
     *      Массив, куда будут записываться данные.
     * 
     * @param capacity
     *      Размер массива, в который будут записываться данные.
     */
    omstream(char* buf, std::size_t capacity);

    //перемещение
    omstream(omstream&&);

    //перемещение
    omstream& operator= (omstream&&);

    /**
     * Записывает один символ в поток.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     * 
     * @throws buffer_owerflow
     *          Если для класса используется внешний массив и места в нём не хватает.
     */
    void write(char c) override;

    /**
     * Записывает массив байтов в поток.
     * 
     * @param data Указатель на массив байтов.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода
     * 
     * @throws buffer_owerflow
     *          Если для класса используется внешний массив и места в нём не хватает.
     */
    void write(const char* data, std::size_t sz) override;

    /**
     * Ничего не делает.
     */
    void flush() override;

    /**
     * Если this->_buffer не является внешним массивом, то освобождает память.
     * 
     * @throws io_exception
     *          Если поток не открыт.
     */
    void close() override;

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~omstream();

    /**
     * Возвращает указатель на внутренний буфер.
     */
    const char* data() const;

    /**
     * Возвращает размер полезных данных в потоке.
     */
    std::size_t offset() const;
};

}

#endif//_JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_