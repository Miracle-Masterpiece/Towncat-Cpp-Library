#ifndef JSTD_CPP_LANG_IO_IOUTILITY_H
#define JSTD_CPP_LANG_IO_IOUTILITY_H

#include <cpp/lang/io/constants.hpp>
#include <cstddef>

namespace tc
{

/**
 * file_filter — базовый класс для фильтрации файлов.
 */
class file_filter {
public:
    /**
     * Проверяет, подходит ли файл под фильтр.
     * 
     * @param path 
     *      Путь к файлу (UTF-8).
     * 
     * @param path_length 
     *      Длина пути.
     * 
     * @return 
     *      true, если файл подходит.
     */
    virtual bool apply(const char* path, std::size_t path_length) const = 0;

    /**
     * 
     */
    virtual ~file_filter() {};
};
    
//Фильтр, который принимает всё.  
class accept_all_filter : public file_filter {
public:
    // Всегда возвращает true.     
    bool apply(const char*, std::size_t) const {
        return true;
    } 
};

}


#endif//JSTD_CPP_LANG_IO_IOUTILITY_H