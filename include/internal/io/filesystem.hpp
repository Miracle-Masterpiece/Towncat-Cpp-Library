#ifndef JSTD_CPP_LANG_IO_FILESYSTEM_H
#define JSTD_CPP_LANG_IO_FILESYSTEM_H

#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/io/constants.hpp>
#include <cpp/lang/io/file_filter.hpp>
#include <cpp/lang/types.hpp>
#include <allocators/allocator.hpp>
#include <cstdint>
#include <cstdio>

#if defined(__linux__) || defined(__APPLE__)
    #include <dirent.h>
#elif defined(_WIN32)
    #include <windows.h>
#endif

namespace tc
{

class filesystem {
public:
    /**
     * 
     */
    static std::size_t npos() {
        return ~((std::size_t) 0);
    }
#if defined(__linux__) || defined(__APPLE__) 
    static const char FILE_SEPARATOR = '/';
#elif defined(_WIN32)
    static const char FILE_SEPARATOR = '\\';
#endif

    /**
     * 
     */
    static const std::size_t MAX_LENGTH_PATH = io::constants::MAX_LENGTH_PATH;

    /**
     * 
     */
    static const unsigned int CAN_WRITE      = 1 << 0;
    
    /**
     * 
     */
    static const unsigned int CAN_READ       = 1 << 1;
    
    /**
     * 
     */
    static const unsigned int CAN_EXECUTE    = 1 << 2;

    /**
     * 
     */
    static std::size_t
        normalize_path(char path[], std::size_t path_length = npos());
    
    /**
     * 
     */
    static std::size_t
        get_parent(const char* path, char out_path[], std::size_t buf_size);

    /**
     * 
     */
    static bool
        is_separator(char ch);

    /**
     * 
     */
    static bool
        is_absolute(const char* path);

    /**
     * 
     */
    static bool
        exists(const char* path);
    
    /**
     * 
     */
    static std::size_t
        runtime_path(char buf[], std::size_t bufsize);

    /**
     * 
     */
    static bool
        is_file(const char* path);
    
    /**
     * 
     */
    static bool
        is_dir(const char* path);
    
    /**
     * 
     */
    static std::uintmax_t
        length(const char* path);
    
    /**
     * @return seconds
     */
    static timepoint
        last_modified(const char* path);
    
    /**
     * 
     */
    static bool
        set_last_modified(const char* path, timepoint seconds);
    
    /**
     * 
     */
    static bool
        create_new_file(const char* path);
    
    /**
     * 
     */
    static bool
        mkdir(const char* path);
    
    /**
     * 
     */
    static bool
        mkdirs(const char* path);

    /**
     * 
     */
    static bool
        remove(const char* path);
    
    /**
     * 
     */
    static bool
        rename_to(const char* old_path, const char* new_path);

    /**
     * 
     */
    static bool
        get_access(const char* path, unsigned int op);
    
    /**
     * 
     */
    static bool
        set_access(const char* path, unsigned int op, bool on_off);

    /**
     * 
     */
    static bool
        can_execute(const char* path);
    
    /**
     * 
     */
    static bool
        can_read(const char* path);
    
    /**
     * 
     */
    static bool
        can_write(const char* path);


    /**
     * 
     */
    static bool
        set_executable(const char* path, bool executable);
    
    /**
     * 
     */
    static bool
        set_readable(const char* path,   bool readable);
    
    /**
     * 
     */
    static bool
        set_writable(const char* path,   bool writable);

    /**
     * 
     */
    static std::size_t
        count_files_in_directory(const char* path, const file_filter& filter);

    /**
     * 
     */
    static FILE*
        open(const char* path, const char* mark);
    
    /**
     * 
     */
    static void
        close(FILE*);
    
    /**
     * 
     */
    static void
        flush(FILE*);

    /**
     * 
     */
    static int
        open_fd(const char* path, int oflags, int pmode);

    /**
     * 
     */
    static void
        close_fd(int fd);
};

// directory_entry — класс который хранит имя файла или папки.
class directory_entry {
    char _name[filesystem::MAX_LENGTH_PATH]; // UTF-8 строка, имя файла/папки.
public:
    // Создаёт directory_entry с указанным путём.
    // Если path_length -1, то считаем, что строка заканчивается на '\0'.
    directory_entry(const char* path, std::size_t path_length = filesystem::npos());

    // Копирует другой directory_entry. Просто копирует строку.
    directory_entry(const directory_entry& other);

    // Перемещает другой directory_entry. Но на самом деле просто копирует строку.
    directory_entry(directory_entry&& other);

    // Копирует данные из другого directory_entry.
    directory_entry& operator=(const directory_entry& other);

    // Перемещает другой directory_entry. Но по факту снова копирует строку.
    directory_entry& operator=(directory_entry&& other);

    // Уничтожает directory_entry.
    ~directory_entry();

    // Возвращает имя файла или папки (UTF-8).
    const char* get_name() const {
        return _name;
    }
};

// directory_iterator — класс, который перебирает файлы в папке.
class directory_iterator {
    
    // Копирование запрещено
    directory_iterator(const directory_iterator&)               = delete;
    directory_iterator& operator=(const directory_iterator&)    = delete;

#if defined(JSTD_OS_LINUX) || defined(JSTD_OS_MAC) || defined(JSTD_OS_UNIX)
    DIR* _dir;                  // Открытая папка.
    struct dirent* _entry;      // Текущий файл/папка.
#elif defined(JSTD_OS_WINDOWS)
    HANDLE _dir;                // Дескриптор поиска файлов.
    WIN32_FIND_DATAW _entry;    // Инфа о текущем файле.
    bool _end;                  // true, если файлы закончились.
#endif

    // Закрывает директорию, если открыта.
    void close();

public:
    // Открывает папку и начинает её перебирать.
    // Если path не указан, то создаётся итератор, как бы указывающий на конец.
    directory_iterator(const char* path = nullptr);

    // Перемещает другой directory_iterator.
    directory_iterator(directory_iterator&& other);

    // Перемещающее присваивание.
    directory_iterator& operator=(directory_iterator& other);

    // Закрывает директорию, если открыта.
    ~directory_iterator();

    // Проверяет, не один ли это итератор.
    bool operator!=(const directory_iterator& it) const;

    // Переход к следующему файлу.
    directory_iterator& operator++();

    // Возвращает текущий файл/папку.
    directory_entry operator*() const;
};

}
#endif//JSTD_CPP_LANG_IO_FILESYSTEM_H_
