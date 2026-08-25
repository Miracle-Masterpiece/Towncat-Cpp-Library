#include <allocators/inline_linear_allocator.hpp>
#include <internal/io/io_helpers.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/utils/coder.hpp>
#include <cpp/lang/string.hpp>
#include <cpp/lang/math.hpp>
#include <cerrno>
#include <cassert>

namespace tc
{
    using fs = filesystem;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                      C   R   O   S   S   P   L   A   T   F   O   R   M    
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace tc
{
    /**
     * 
     */
    static std::size_t normlen(const char* path, std::size_t len) {
        return len != fs::npos() ? len : std::strlen(path);
    }

    /**
     * 
     */
    bool fs::is_separator(char ch) {
        return ch == '\\' || ch == '/';
    }
    
    /**
     * Заменяет двойной разделитель, на одинарный
     * /test///aa//bb//ccc" преобразутеся в /test/aa/bb/cc
     * 
     * Но если путь сетевой
     * //test///aa//bb//ccc" преобразутеся в //test/aa/bb/cc
     * 
     */
    static std::size_t remove_double_separators(char path[], std::size_t path_length) {
        std::size_t wp = 0;
        std::size_t rp = 0;
        
        if (path_length > 2)
        {
            //проверка на то, что это сетевой путь. То есть, путь начинающийся на "\\" - "\\bla\bla\bla"
            if (fs::is_separator(path[rp]) && fs::is_separator(path[rp + 1]))
            {
                rp += 2, wp += 2;
            }
        }
        
        while (rp < path_length)
        {    
            if (fs::is_separator(path[rp]) && (rp + 1) >= path_length)
            {
                ++rp;
                continue;
            }

            if (rp + 1 < path_length)
            {
                if (fs::is_separator(path[rp]) && fs::is_separator(path[rp + 1]))
                {
                    ++rp;
                    continue;
                }
            }

            path[wp++] = path[rp++];
        }

        return wp;
    }
    
    /**
     * 
     */
    std::size_t fs::normalize_path(char path[], std::size_t path_length) {
        path_length = remove_double_separators(path, normlen(path, path_length));
        std::size_t wp = 0;
        std::size_t rp = 0;

        const std::size_t plen = path_length;

        while (rp < plen)
        {
            if (rp != 0 && (rp + 1 < plen) && path[rp] == '.' && is_separator(path[rp + 1]))
            {
                ++rp;
                if (wp > 0 && is_separator(path[wp - 1]))
                    --wp;
                continue;
            }
            //  C:/user/folder/../test
            else if ((rp + 2) < plen)
            {
                if (path[rp] == '.' && path[rp + 1] == '.' && is_separator(path[rp + 2]))
                {
                    rp += 3;
                    if (wp > 0)
                    {
                        --wp;
                        while (wp > 0 && !is_separator(path[wp - 1]))
                            --wp;
                    }
                    continue;
                }
            }
         
            path[wp++] = path[rp++];
        }
        path[wp] = '\0';
        return wp;
    }
    
    #if 0
    /**
     * Заменяет все разделители на константу зависящую от системы.
     * @see filesystem::FILE_SEPARATOR
     */
    static void replace_separators_to_system_separators(char path[], std::size_t path_length) {
        for (std::size_t i = 0; i < path_length; ++i) {
            if (fs::is_separator(path[i]))
                path[i] = filesystem::FILE_SEPARATOR;
        }
    }
    #endif

    /**
     * 
     */
    std::size_t get_parent_from_absolute(char path[], std::size_t path_length = filesystem::npos()) {
        path_length = normlen(path, path_length);
        for (std::size_t i = path_length; i > 0; )
        {
            if (fs::is_separator(path[--i]))
            {
                path[i] = '\0';
                return i;
            }
        }
        return 0;
    }
    
    /**
     * 
     */
    std::size_t fs::get_parent(const char* path, char out_path[], std::size_t buf_size) {
        if (buf_size == 0) return 0;
        
        std::size_t len = std::strlen(path);
        
        //Вычисляем максимальную длину выходной строки, с учётом 0-терминатора.
        std::size_t max_path_size    = math::min(buf_size - 1, len);
        
        if (max_path_size == 0)
        {
            out_path[0] = '\0';
            return 0;
        }

        //Путь в буфер
        std::memcpy(out_path, path, max_path_size);
        
        //Добавляем 0-териминатор
        out_path[max_path_size] = '\0';
        
        //Нормализация пути, для удаления мусора (/////, ./././)
        len = fs::normalize_path(out_path);
        
        return get_parent_from_absolute(out_path, len);
    }

    bool fs::mkdirs(const char* path)
    {
        std::size_t len = std::strlen(path);

        if (len == 0)
            return true;

        std::size_t start = 0;
        if (is_absolute(path))
        {
            if (is_separator(path[0]))
            {
                start = 1;
            }
            else if (len >= 3 && path[1] == ':' && is_separator(path[2]))
            {
                start = 3;
            }
        }


        char part_buffer[io::constants::MAX_LENGTH_PATH];
        for (std::size_t i = start; i < len; ++i)
        {
            if (is_separator(path[i]) || i + 1 == len)
            {
                std::size_t part_length = i + 1;

                if (part_length >= io::constants::MAX_LENGTH_PATH)
                    throw_except<io_exception>("Path very large '%s'", path);
                
                std::memcpy(part_buffer, path, part_length);
                part_buffer[part_length] = '\0';

                if (!exists(part_buffer))
                {
                    if (!mkdir(part_buffer))
                        return false;
                }
            }
        }

        return true;
    }

    std::size_t filesystem::count_files_in_directory(const char* path, const file_filter& filter) {
        directory_iterator begin(path);
        directory_iterator end;
        std::size_t count_files = 0;
        while (begin != end) {
            
            directory_entry entry = *begin;
            
            const char* name    = entry.get_name();
            std::size_t len     = std::strlen(name);
            
            if (filter.apply(name, len))
                ++count_files;            

            ++begin;
        }
        return count_files;
    }

    bool fs::can_execute(const char* path) {
        return get_access(path, CAN_EXECUTE);
    }
    
    bool filesystem::can_read(const char* path) {
        return get_access(path, CAN_READ);
    }
    
    bool filesystem::can_write(const char* path) {
        return get_access(path, CAN_WRITE);
    }

    bool filesystem::set_executable(const char* path, bool on_off) {
        return set_access(path, CAN_EXECUTE, on_off);
    }

    bool filesystem::set_readable(const char* path, bool on_off) {
        return set_access(path, CAN_READ, on_off);
    }
    
    bool filesystem::set_writable(const char* path, bool on_off) {
        return set_access(path, CAN_WRITE, on_off);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                          D   I   R   E   N   T   R   Y   
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace tc
{
    directory_entry::directory_entry(const char* path, std::size_t path_length) {
        path_length      = normlen(path, path_length);
        std::size_t size = math::min(sizeof(_name) - 1, path_length);
        std::memcpy(_name, path, size);
        _name[size] = 0;
    }
    
    directory_entry::directory_entry(const directory_entry& ent) {
        std::memcpy(_name, ent._name, sizeof(_name));
    }
    
    directory_entry::directory_entry(directory_entry&& ent) {
        std::memcpy(_name, ent._name, sizeof(_name));
    }
    
    directory_entry& directory_entry::operator= (const directory_entry& ent) {
        if (&ent != this)
            std::memcpy(_name, ent._name, sizeof(_name));
        return *this;
    }
    
    directory_entry& directory_entry::operator= (directory_entry&& ent) {
        if (&ent != this)
            std::memcpy(_name, ent._name, sizeof(_name));
        return *this;
    }
    
    directory_entry::~directory_entry() {

    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  P   O   S   I   X
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(JSTD_OS_MAC) || defined(JSTD_OS_LINUX) || defined(JSTD_OS_UNIX)
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include <utime.h>
#include <cerrno>

#define READ_OWNER 		        S_IRUSR 	//Чтение для владельца
#define WRITE_OWNER 		    S_IWUSR 	//Запись для владельца
#define EXECUTABLE_OWNER 	    S_IXUSR	    //Выполнение для владельца

#define READ_GROUP 		        S_IRGRP 	//Чтение для группы
#define WRITE_GROUP 		    S_IWGRP 	//Запись для группы
#define EXECUTABLE_GROUP 	    S_IXGRP	    //Выполнение для группы

#define READ_OTHER 		        S_IROTH 	//Чтение для остальных
#define WRITE_OTHER 		    S_IWOTH 	//Запись для остальных
#define EXECUTABLE_OTHER 	    S_IXOTH	    //Выполнение для остальных

#define NIX_READ              (READ_OWNER | READ_GROUP | READ_OTHER)
#define NIX_WRITE             (WRITE_OWNER | WRITE_GROUP | WRITE_OTHER)
#define NIX_EXECUTE           (EXECUTABLE_OWNER | EXECUTABLE_GROUP | EXECUTABLE_OTHER)
#define PERMISSION_ALL        (NIX_READ | NIX_WRITE | NIX_EXECUTE)

namespace tc
{

    static void throw_error() {
        assert(errno != 0);
        internal::io::throw_error_code(error_code(errno, system_category()));
    }

    static void init_stat(const char* path, struct stat* filestat) {
        if (stat(path, filestat) != 0)
            throw_error();
    }
    
    std::size_t fs::runtime_path(char rbuf[], std::size_t bufsize) {
        ssize_t len = readlink("/proc/self/exe", rbuf, bufsize);
        if (len == -1) throw_error();
        for (ssize_t i = len; i > 0; ) {
            --i;
            if (rbuf[i] == '/') {
                rbuf[i] = '0';
                return (std::size_t) i;
            }
        }
        return (std::size_t) len;  
    }

    bool fs::is_absolute(const char* path) {
        std::size_t len = std::strlen(path);
        if (len < 1)
            return false;
        if (path[0] == '/' || path[0] == '\\')
            return true;
        return false;
    }

    bool fs::exists(const char* path){
        struct stat filestat;
		return stat(path, &filestat) == 0;
	}

    bool fs::is_file(const char* path){
        if (!exists(path))
            return false;
        
        struct stat filestat;
        init_stat(path, &filestat);
        
        return S_ISREG(filestat.st_mode);
    }

    bool fs::is_dir(const char* path){
        if (!exists(path))
            return false;
        
        struct stat filestat;
        init_stat(path, &filestat);
        
        return S_ISDIR(filestat.st_mode);
    }

    std::uintmax_t fs::length(const char* path) {    
        struct stat fstat;
        if (stat(path, &fstat) == 0)
            return static_cast<std::uintmax_t>(fstat.st_size);
        return 0;
    }

    timepoint fs::last_modified(const char* path) {        
        if (!fs::exists(path))
            return 0;

        struct stat filestat;
        init_stat(path, &filestat);
		
        struct timespec& sTime = filestat.st_mtim;
        
        return (timepoint) ( (sTime.tv_nsec / 1000000) + (sTime.tv_sec * 1000) );
    }

    bool fs::set_last_modified(const char* path, timepoint ms_time) {
        if (!fs::exists(path))
            return 0;
        
        struct stat fstat;
        init_stat(path, &fstat);
        
        utimbuf timeBuf;
        timeBuf.actime  = fstat.st_atim.tv_sec;
        timeBuf.modtime = ms_time / 1000;  //ms to sec
        
        if (utime(path, &timeBuf) != 0)
            throw_error();
        
        return true;
    }

    bool fs::create_new_file(const char* path) {
        //       \/ это чёртово двойное двоеточие нужно, т.к иначе компилятор будет визжать, 
        //          из-за попытки вызвать filesystem::open(const char*, const char*, int, int).
        int fd = ::open(path, O_CREAT, PERMISSION_ALL);
        if (fd == -1)
        {
            if (errno == EEXIST)
                return false;
            throw_error();
        }
        
        return true;
    }

    bool fs::mkdir(const char* path) {
        if (::mkdir(path, PERMISSION_ALL) != 0)
            throw_error();
        return true;
    }

    bool fs::remove(const char* path) {
		if (!fs::exists(path))
            return false;

        if (::remove(path) != 0)
			throw_error();

		return true;
    }

    bool fs::rename_to(const char* old_path, const char* new_path) {
		
        if (!fs::exists(old_path))
            return false;
        
        if (rename(old_path, new_path) != 0)
            throw_error();
		
        return true;
    }

    bool fs::get_access(const char* path, unsigned int mode){

        int os_mode = 0;
        if (mode & fs::CAN_READ)    os_mode |= R_OK;
        if (mode & fs::CAN_WRITE)   os_mode |= W_OK;
        if (mode & fs::CAN_EXECUTE) os_mode |= X_OK;

        if (access(path, os_mode) != 0)
            return false;

		return true;
	}

    bool fs::set_access(const char* path, unsigned int mode, bool on_off) {
        if (!fs::exists(path))
            return false;
        
        struct stat filestat;
        init_stat(path, &filestat);

        if (mode == fs::CAN_READ)    mode = S_IRUSR;
        if (mode == fs::CAN_WRITE)   mode = S_IWUSR;
        if (mode == fs::CAN_EXECUTE) mode = S_IXUSR;

        unsigned int new_mode = on_off ?
                                        (filestat.st_mode | mode) : (~mode & filestat.st_mode);
        
        if (chmod(path, new_mode) != 0)
            throw_error();

        return true;
    }

    expected<FILE*, error_code> fs::open(const char* path, const char* mark) {
        FILE* handle = fopen(path, mark);   
        if (!handle)
        {
            return unexpected(error_code(errno, generic_category()));
        }
        return handle;
    }

    int fs::open_fd(const char* path, int oflags, int pmode) {
        
        int fd = ::open(path, oflags, pmode);
        if (fd == -1)
        {
            if (errno == EACCES)
                throw_except<security_exception>(std::strerror(errno));
            throw_except<io_exception>(std::strerror(errno));
        }
    
        return fd;
    }

    void fs::close_fd(int fd) {
        if (::close(fd) != 0)
        {
            throw_except<io_exception>(std::strerror(errno));
        }
    }
    
}


namespace tc
{
    directory_iterator::directory_iterator(const char* path) : _dir(nullptr), _entry(nullptr) {
        if (path != nullptr)
        {
            _dir = opendir(path);
            if (_dir == nullptr) {
                if (errno == EACCES) 
                    throw_error();
            }
            else
            {
                this->operator++();
            }
        }
    }
    
    directory_iterator::directory_iterator(directory_iterator&& it) : _dir(it._dir), _entry(it._entry) {
        it._dir     = nullptr;
        it._entry   = nullptr;
    }
    
    directory_iterator& directory_iterator::operator= (directory_iterator& it) {
        if (&it != this)
        {
            close();
            _dir        = it._dir;
            _entry      = it._entry;
            it._dir     = nullptr;
            it._entry   = nullptr;
        }
        return *this;
    }
    
    void directory_iterator::close() {
        if (_dir != nullptr)
        {
            closedir(_dir);
            _dir    = nullptr;
            _entry  = nullptr;
        }
    }

    directory_iterator::~directory_iterator() {
        close();
    }

    bool directory_iterator::operator!= (const directory_iterator& it) const {
        return _entry != it._entry;
    }

    static bool is_current_or_prev_folder(const char* path, std::size_t len) {
        return (len == 1 && path[0] == '.') || (len == 2 && path[0] == '.' && path[1] == '.');
    }
    
    directory_iterator& directory_iterator::operator++() {
        while (true) {
            _entry = readdir(_dir);
            if (_entry == nullptr)
                break;
            std::size_t len = std::strlen(_entry->d_name);
            if (is_current_or_prev_folder(_entry->d_name, len))
                continue;
            break;
        }
        return *this;
    }
    
    directory_entry directory_iterator::operator*() const {
        return directory_entry(_entry->d_name);
    }
}


#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                              W   I   N   D   O   W   S
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(JSTD_OS_WINDOWS)
#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#define WCHAR_BYTE_ORDER byte_order::LE
namespace tc
{
    
    static void throw_error() {
        DWORD lastErr = GetLastError();
        assert(lastErr != ERROR_SUCCESS);
        internal::io::throw_error_code(error_code(lastErr, system_category()));
    }
    
    static std::size_t wchar_to_char(const wchar_t* wstr, std::size_t wstrlen, char str[], std::size_t strlen) {
        return utf::u16_to_u8(wstr, wstrlen, str, strlen, WCHAR_BYTE_ORDER);
    }
    
    static std::size_t char_to_wchar(const char* str, std::size_t strlen, wchar_t wstr[], std::size_t wstrlen) {
        return utf::u8_to_u16(str, strlen, wstr, wstrlen, WCHAR_BYTE_ORDER);
    }

    static DWORD get_file_attrib(const char* path) {
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
        
        return GetFileAttributesW(wbuf);
    }

    std::size_t fs::runtime_path(char buf[], std::size_t bufsize) {
        
        // GetModuleFileNameW
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        std::size_t len = (std::size_t) GetModuleFileNameW(NULL, wbuf, (DWORD) io::constants::MAX_LENGTH_PATH);
        
        //Удаление имени исполняемого файла
        for (std::size_t i = len; i > 0; )
        {
            --i;
            if (wbuf[i] == '\\' || wbuf[i] == '/')
            {
                len = i;
                break;
            }
        }
        
        return wchar_to_char(wbuf, len, buf, bufsize);
    }
    
    bool fs::is_absolute(const char* path) {
        std::size_t len = std::strlen(path);
        
        if (len < 3)
            return false;
        
        char tom_name           = path[0];
        char double_dots        = path[1];
        char slash              = path[2];
        
        if ((tom_name < 'a' || tom_name > 'z')) return false;
        if ((tom_name < 'A' || tom_name > 'Z')) return false;
        if (double_dots != ':')                 return false;
        if (!fs::is_separator(slash))           return false;
        
        return true;
    }

    bool fs::exists(const char* path) {
        DWORD attrib = get_file_attrib(path);
        if (attrib != INVALID_FILE_ATTRIBUTES)
            return true;
		return false;
    }

    bool fs::is_file(const char* path) {
        if (!exists(path))
            return false;
        return !is_dir(path);
    }
    
    bool fs::is_dir(const char* path) {
		if (!exists(path))
            return false;
        DWORD attrib = get_file_attrib(path);
		if (attrib == INVALID_FILE_ATTRIBUTES)
			throw_error();
		return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }

    std::uintmax_t fs::length(const char* path) {
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        
        std::size_t len = std::strlen(path);
        char_to_wchar(path, len, wbuf, io::constants::MAX_LENGTH_PATH);
        
        struct _stat fstat;
        if (_wstat(wbuf, &fstat) == 0)
            return static_cast<std::uintmax_t>(fstat.st_size);

        return 0;
    }
    
    timepoint fs::last_modified(const char* path) {
        if (!fs::exists(path))
            return 0;
        
        wchar_t wpath[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wpath, io::constants::MAX_LENGTH_PATH);

		HANDLE hFile = CreateFileW(
                                    wpath,
                                    FILE_WRITE_ATTRIBUTES,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                    nullptr,
                                    OPEN_EXISTING,
                                    FILE_FLAG_BACKUP_SEMANTICS,
                                    nullptr
                                );               		

		if (hFile == INVALID_HANDLE_VALUE)
        {
		    if (GetLastError() != ERROR_FILE_NOT_FOUND)
                throw_error();
            return 0;
		}

		FILETIME lastModified;
		if (!GetFileTime(hFile, NULL, NULL, &lastModified)) {
		    CloseHandle(hFile);
            throw_error();
            return 0;
		}
		CloseHandle(hFile);

		ULARGE_INTEGER time;
		time.LowPart    = lastModified.dwLowDateTime;
		time.HighPart   = lastModified.dwHighDateTime;

		return (timepoint) ((time.QuadPart / 10000) - 11644473600000U);
    }

    bool fs::set_last_modified(const char* path, timepoint ms_time) {
        
        wchar_t wpath[io::constants::MAX_LENGTH_PATH];        
        char_to_wchar(path, std::strlen(path), wpath, io::constants::MAX_LENGTH_PATH);

        HANDLE hFile = CreateFileW(
            wpath,
            FILE_WRITE_ATTRIBUTES,              // Доступ только для изменения атрибутов
            FILE_SHARE_READ | FILE_SHARE_WRITE, // Общий доступ
            NULL,                               // Защита по умолчанию
            OPEN_EXISTING,                      // Открыть только существующий файл
            FILE_FLAG_BACKUP_SEMANTICS,         // Обычный файл
            NULL                                // Без шаблона
        );

        if (hFile == INVALID_HANDLE_VALUE)
        {
             if (GetLastError() != ERROR_FILE_NOT_FOUND)
                throw_error();
            return false;
        }

        ULONGLONG fileTime  = (ms_time * 10000ULL) + 116444736000000000ULL; 
        
        FILETIME ft;
        ft.dwLowDateTime    = (DWORD) (fileTime);
        ft.dwHighDateTime   = (DWORD) (fileTime >> 32);
        
        BOOL result = SetFileTime(
            hFile,
            NULL,          // Время создания
            NULL,          // Время последнего доступа
            &ft            // Время последней модификации
        );
        
        CloseHandle(hFile); 
        
        return result; 
    }

    bool fs::create_new_file(const char* path) {
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);

        HANDLE file = CreateFileW(
            wbuf,  
            GENERIC_WRITE,                                                      // Режим доступа
            0,                                                                  // Совместный доступ
            NULL,                                                               // Атрибуты безопасности
            CREATE_NEW,                                                         // Создание нового файла, если он не существует
            FILE_ATTRIBUTE_NORMAL,                                              // Атрибуты файла
            NULL                                                                // Шаблон файла
        );

        if (file == INVALID_HANDLE_VALUE) {
            DWORD error = GetLastError();
            if (error == ERROR_FILE_EXISTS)
                return false;
            throw_error();
        }
        
        CloseHandle(file);
        return true;
    }

    bool fs::mkdir(const char* path) {

        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);

        bool created        = CreateDirectoryW(wbuf, NULL) != 0;
        bool alreadyExists  = GetLastError() == ERROR_ALREADY_EXISTS;
        
        if (created || alreadyExists) return true;
        else                          throw_error();

        //Чтобы компилятор не истерил
        return false;
    }

    bool fs::remove(const char* path) {
        if (!fs::exists(path))
            return false;

        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);

        if (fs::is_dir(path))
        {
            if (!RemoveDirectoryW(wbuf))
                throw_error();
        }
        else
        {
            if (!DeleteFileW(wbuf))
                throw_error();
        }

		return true;
    }

    bool fs::rename_to(const char* old_path, const char* new_path) {
        
        if (!fs::exists(old_path))
        {
            return false;
        }
        
        wchar_t wbuf_old[io::constants::MAX_LENGTH_PATH];
        wchar_t wbuf_new[io::constants::MAX_LENGTH_PATH];
        
        char_to_wchar(old_path, std::strlen(old_path), wbuf_old, io::constants::MAX_LENGTH_PATH);
        char_to_wchar(new_path, std::strlen(new_path), wbuf_new, io::constants::MAX_LENGTH_PATH);

        if (_wrename(wbuf_old, wbuf_new) != 0)
            throw_error();

        return true;
    }
    
    bool fs::get_access(const char* path, unsigned int mode){
        if (!fs::exists(path))
            return false;
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        
        {
            char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
        }

        struct _stat filestat;
        if (_wstat(wbuf, &filestat) != 0)
            throw_error();
        
        if (mode == fs::CAN_READ)    mode = _S_IREAD;
        if (mode == fs::CAN_WRITE)   mode = _S_IWRITE;
        if (mode == fs::CAN_EXECUTE) mode = _S_IEXEC;
    
        return filestat.st_mode & mode;
	}
    
    bool fs::set_access(const char* path, unsigned int mode, bool on_off) {
        if (!fs::exists(path))
            return false;
        
        wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
        
        struct _stat filestat;
        if (_wstat(wbuf, &filestat) != 0)
            throw_error();

        if (mode == fs::CAN_READ)    mode = _S_IREAD;
        if (mode == fs::CAN_WRITE)   mode = _S_IWRITE;
        if (mode == fs::CAN_EXECUTE) mode = _S_IEXEC;

        unsigned int new_mode = on_off ? 
                                        (filestat.st_mode | mode) : (~mode & filestat.st_mode);
        
        if (_wchmod(wbuf, (int) new_mode) != 0)
            throw_error();
        
        return true;
    }

    expected<FILE*, error_code> fs::open(const char* path, const char* mark) {
        const std::size_t MARK_BUF_SIZE = 8;

        wchar_t wbuf_path[io::constants::MAX_LENGTH_PATH];
        wchar_t wbuf_mark[MARK_BUF_SIZE];
        
        char_to_wchar(path, std::strlen(path), wbuf_path, io::constants::MAX_LENGTH_PATH);
        char_to_wchar(mark, std::strlen(mark), wbuf_mark, MARK_BUF_SIZE);

        FILE* handle = _wfopen(wbuf_path, wbuf_mark);
        
        if (!handle)
        {
            return unexpected(error_code(errno, generic_category()));
        }
        
        return handle;
    }

    error_code fs::close(FILE* handle) {
        if (fclose(handle) != 0)
            return error_code(errno, generic_category());
        return error_code();
    }

    error_code fs::flush(FILE* handle) {
        if (fflush(handle) != 0)
            return error_code(errno, generic_category());
        return error_code();
    }

    int fs::open_fd(const char* path, int oflags, int pmode) {
    
        wchar_t wpath[io::constants::MAX_LENGTH_PATH];
        char_to_wchar(path, std::strlen(path), wpath, io::constants::MAX_LENGTH_PATH);

        int fd = ::_wopen(wpath, oflags, pmode);
        if (fd == -1)
        {
            if (errno == EACCES)
                throw_except<security_exception>(std::strerror(errno));
            throw_except<io_exception>(std::strerror(errno));
        }
    
        return fd;
    }

    void fs::close_fd(int fd) {
        if (::_close(fd) != 0)
        {
            throw_except<io_exception>(std::strerror(errno));
        }
    }
}

namespace tc
{

    static bool is_current_or_prev_folder(const wchar_t* path) {
        std::size_t len = wstring::str_len(path);
        return (len == 1 && path[0] == '.') || (len == 2 && path[0] == '.' && path[1] == '.');
    }

    directory_iterator::directory_iterator(const char* path) : _dir(), _entry(), _end(path == nullptr ? true : false) {
        if (path != nullptr) {

            wchar_t wbuf[io::constants::MAX_LENGTH_PATH];
            std::size_t len = char_to_wchar(path, std::strlen(path), wbuf, io::constants::MAX_LENGTH_PATH);
            
            if (len >= io::constants::MAX_LENGTH_PATH - 3)
                throw_except<io_exception>("Path very long");

            //TODO: этот кусок говна нужен только потому, что винде нужен шаблон. 
            //Если просто передать "C:\\" -- то эта чёртова система не поймёт, что нужны все папки в указанной директории.
            wbuf[len++] = (wchar_t) filesystem::FILE_SEPARATOR;
            wbuf[len++] = L'*';
            wbuf[len]   = 0;

            _dir = FindFirstFileW(wbuf, &_entry);
            if (_dir != INVALID_HANDLE_VALUE)
            {
                if (is_current_or_prev_folder(_entry.cFileName)) 
                    ++(*this);
            }
            else
            {
                _end = true;
            }
        }
    }   

    directory_iterator::directory_iterator(directory_iterator&& it) : _dir(std::move(it._dir)), _entry(std::move(it._entry)) {
        it._dir     = {0};
        //it._entry   = {0};
    }
    
    directory_iterator& directory_iterator::operator= (directory_iterator& it) {
        if (&it != this)
        {
            std::swap(_dir, it._dir);
            std::swap(_entry, it._entry);
        } 
        return *this;
    }
    
    void directory_iterator::close() {
        if (_dir != INVALID_HANDLE_VALUE)
        {
            FindClose(_dir);
            _dir    = INVALID_HANDLE_VALUE;
        }
    }

    directory_iterator::~directory_iterator() {
        close();
    }
    
    bool directory_iterator::operator!= (const directory_iterator& it) const {
        return _end != it._end;
    }
    
    directory_iterator& directory_iterator::operator++ () {   
        START:
        if (_end) 
            throw_except<illegal_state_exception>("End of directory stream!");
        
        if (FindNextFileW(_dir, &_entry) != 0)
        {
            if (is_current_or_prev_folder(_entry.cFileName))
                goto START;
        }
        else
        {
            _end = true;
        }
        
        return *this;
    }
    
    directory_entry directory_iterator::operator* () const {
        tca::inline_linear_allocator<filesystem::MAX_LENGTH_PATH> allocator;
        
        //Строка в кодировке виндоус (UTF-16)
        const wchar_t* entry_path   =_entry.cFileName;
        std::size_t path_length     = wstring::str_len(entry_path);

        //Перекодировка в UTF-8
        char buf[io::constants::MAX_LENGTH_PATH];
        std::size_t converted_length = wchar_to_char(entry_path, path_length, buf, io::constants::MAX_LENGTH_PATH);

        return directory_entry(buf, converted_length);
    }
}

#endif