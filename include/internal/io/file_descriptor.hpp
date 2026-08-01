/*Занятно, похоже это самый чистый внутренний класс*/

#ifndef JSTD_INTERNAL_IO_FILE_DESCRIPTOR_H
#define JSTD_INTERNAL_IO_FILE_DESCRIPTOR_H

namespace tc
{

namespace internal 
{

/**
 * @internal
 *      Внутренний класс обёртка над системным файловым дескриптором.
 * 
 * @note
 *      Для освобождения ресурсов вызывайте close() явно, деструктор не освобождает дескриптор.
 * 
 */
class file_descriptor {
    
    /**
     * Внутренний дескриптор.
     */
    int fd;
    
    /**
     * @internal
     */
    file_descriptor(int fd);

    /**
     * 
     */
    file_descriptor(const file_descriptor&)             = delete;
    
    /**
     * 
     */
    file_descriptor& operator=(const file_descriptor&)  = delete;

public:
    /**
     * Нулевой дескриптор.
     */
    static const file_descriptor NULL_FD;
    
    /**
     * Создаёт дескриптор не привязанный к файлу.
     */
    file_descriptor();
    
    /**
     * Открывает файл привязывает к текущему объекту.
     * 
     * @param u8papth
     *      Путь к файлу в формате UTF-8.
     * 
     * @param oflags
     *      Флаги открытия.
     * 
     * @param pmode
     *      Файлы защиты файла. (Дают эффект только про создании нового файла).
     */
    file_descriptor(const char* u8path, int oflags, int pmode);
    
    /**
     * Перемещет дескриптор из другого объекта.
     */
    file_descriptor(file_descriptor&&);
    
    /**
     * Перемещет дескриптор из другого объекта.
     * При этом закрывая текущий.
     * 
     * @throws io_exception
     *      Если при закрытии текущего файла произошла ошибка.
     */
    file_descriptor& operator=(file_descriptor&&);
    
    /**
     * @note
     *      Деструктор не освобождает файловый дескриптор.
     *      Для освобождения должна явно вызываться функция this::close()
     */
    ~file_descriptor();
    
    /**
     * Возвращает числовой идентификатор дескриптора.
     */
    int get_fd() const;
    
    /**
     * Проверяет, равен ли этот дескриптор другому.
     * 
     * @return 
     *      true, если внутренний идентификатор совпадает, иначе false.
     */
    bool operator== (const file_descriptor&) const;
    
    /**
     * Проверяет, не равен ли этот дескриптор другому.
     * 
    * @return 
     *      true, если внутренний идентификатор не совпадает, иначе false.
     */
    bool operator!= (const file_descriptor&) const;

    /**
     * Оператор приведения типа.
     * 
     * @return
     *      true - тогда и только тогда, когда дескриптор не нулевой. В остальных случаях всегда - false.
     */
    operator bool() const;

    /**
     * Закрывает этот файловый дескриптор.
     * 
     * @throws io_exception
     *      Если при закрытии произошла ошибка.
     */
    void close();
};

}// namespace internal
}// namespace jstd

#endif//JSTD_INTERNAL_IO_FILE_DESCRIPTOR_H