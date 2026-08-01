#ifndef _JSTD_CPP_LANG_IO_FILEINPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_FILEINPUTSTREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/file.hpp>
#include <cstdio>

namespace tc {

/**
 * Класс предназначен для чтения из файла.
 * 
 * @remark
 *      Копирование и присваивание запрещены, чтобы избежать неожиданных побочных эффектов.  
 */
class ifstream : public istream {
    //Указатель на файл.
    FILE*   m_handle;
    
    //Любой ценой это значение должно указывать, сколько байт можно считать из файла.
    //Просто, чёрт возьми, убейте меня...
    std::uintmax_t m_available;     //сколько байт -доступно- для чтения.

public:
    /**
     * Создаёт пустой файловый поток.
     */
    ifstream();
    
    /**
     * Создаёт поток по указанному пути.
     * 
     * @param path
     *      Путь к файлу.
     * 
     * @throws file_not_found_exception
     *      Если файла не существует.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     * 
     * @throws sequrity_exception
     *      Если чтение из файла запрещено.
     */
    ifstream(const char* path);
    
    /**
     * Создаёт поток по объекту файла.
     * 
     * @param file  
     *      Объект, описыващий файл.
     * 
     * @throws file_not_found_exception 
     *      Если файла не существует.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     * 
     * @throws sequrity_exception 
     *      Если чтение из файла запрещено.
     */
    ifstream(const file& file);
    
    //Перемещение.
    ifstream(ifstream&&);
    
    /**
     * Перемещение.
     * 
     * @throws io_exception 
     *          Если произошла ошибка ввода/вывода.
     */
    ifstream& operator= (ifstream&&);
    
    /**
     * @note
     *      Для освобождения ресурсов должен явно вызываться this::close()
     */
    ~ifstream();
    
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
     * Читает sz байт из потока в передаваемый буфер.
     * 
     * @param buf
     *      Буфер в который будут записаны данные.
     * 
     * @param sz
     *      Сколько байт нужно считать из потока.
     * 
     * @return
     *      Фактическое количество считанных байт.
     * 
     * @throws io_exception 
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    std::size_t read(char buf[], std::size_t sz) override;
    
    /**
     * Пропускает n байт из потока.
     * 
     * @param n
     *      Сколько байт нужно пропустить.
     * 
     * @return
     *      Фактическое кол-во пропущенных байт.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */
    std::size_t skip(std::size_t n = 1) override;
    
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
     *      Если поток не был открыт.
     */
    std::uintmax_t available() const override;
    
    /**
     * Закрывает поток файла.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     *      Если поток не был открыт.
     */                     
    void close() override;
};

}
#endif//_JSTD_CPP_LANG_IO_FILEINPUTSTREAM_H_
