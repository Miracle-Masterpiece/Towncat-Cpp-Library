#ifndef _JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_
#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>

namespace tc {

/**
 * Класс предназначен для буферезированного чтения бинарных данных.
 * Бинарные данные будут прочтены из порядка байт Little-Endian, даже на Big-Endian платформе, в системный порядок байт. 
 * 
 * @note
 *      Для закрытия ресурсов необходимо вызывать функцию idstream::close().
 *      Деструктор не освобождает данные! В силу требований об явной обработке ошибок.
 */
class idstream : public istream {
    istream* _in;
public:
    using istream::read;

    /**
     * Создаёт пустой поток.
     */
    idstream();
    
    /**
     * Оборачивает поток ввода, для чтения из него бинарных данных.
     * 
     * @throws null_pointer_exception
     *      Если in равен nullptr.
     */
    idstream(istream* in);
    
    //Перемещение
    idstream(idstream&&);
    
    /**
     * Перемещение.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     */
    idstream& operator= (idstream&&);

    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~idstream();
    
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
     *      Количество реально прочитанных байт или istream::eof_value(), если достигнут конец потока.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    std::size_t read(char buf[], std::size_t sz) override;

    /**
     * Возвращает количество доступных для чтения байт.
     * 
     * Позволяет узнать, сколько данных можно прочитать из этого потока.
     * 
     * @return
     *      Количество доступных байт в потоке.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    std::uintmax_t available() const override;

    /**
     * Закрывает поток файла.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     */
    void close() override;

    /**
     * @see this->read<T>(T buf[], std::size_t sz);
     */
    template<typename T>
    T read() {
        T tmp;
        read<T>(&tmp, 1);
        return tmp;
    }

    /**
     * Читает из потока массив типа T в порядке байт Little-Endian.
     * @tparam 
     *      Любой тип, который тривиально-копируемый
     * 
     * @remark 
     *      Данные читаются из порядка Little-Endian.
     * 
     * @param buf
     *      Указатель на массив данных типа T, куда будут записаны данные.
     * 
     * @param sz
     *      Размер массива v
     * 
     * @throws io_exception
     *          Если произошла ошибка ввода/вывода.
     *      
     * @throws eof_exception
     *          Если попытка прочитать больше данных, чем есть в потоке.
     */
    template<typename T>
    std::size_t read(T buf[], std::size_t sz);
};

    template<typename T>
    std::size_t idstream::read(T buf[], std::size_t sz) {
        std::size_t readed = read(reinterpret_cast<char*>(buf), sizeof(T) * sz);
        if (readed != sizeof(T) * sz)
            throw_except<eof_exception>("Cannot read type");   
        if (system::native_byte_order() != byte_order::LE) {
            for (std::size_t i = 0; i < sz; ++i)
                buf[i] = utils::bswap<T>(buf[i]);
        }
        return sz;
    }

}

#endif//_JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_