#ifndef JSTD_CPP_LANG_IO_CHANNEL_H
#define JSTD_CPP_LANG_IO_CHANNEL_H

namespace tc
{

/**
 * @version 1.1
 */
class channel {
    channel(const channel&)             = delete;
    channel& operator= (const channel&) = delete;
public:
    
    /**
     * 
     * */    
    channel();
    
    /**
     * 
     * */    
    channel(channel&&);
    
    /**
     * 
     * */    
    channel& operator= (channel&&);
    
    /**
     * Закрывает этот канал, но игнорирует любые ошибки.
     * */    
    virtual ~channel() = 0;

    /**
     * Является ли этот абстрактный канал открытым.
     * 
     * @return
     *      true - если этот абстрактный канал открыт, иначе false.
     */
    virtual bool is_open() const = 0;
    
    /**
     * Закрывает канал.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     */
    virtual void close()    = 0;
};

}

#endif//JSTD_CPP_LANG_IO_CHANNEL_H