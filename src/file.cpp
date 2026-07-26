#include <cpp/lang/io/file.hpp>
#include <internal/io/filesystem.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/objects.hpp>
#include <cpp/lang/math.hpp>
#include <cstring>
#include <algorithm>
#include <cstdarg>
#include <iostream>
#include <utility>
#include <cassert>

namespace jstd
{

    file::file() {
        _path[0] = '\0';
    }

    file::file(const char* path, std::size_t path_length) {
        path_length = path_length == npos() ? std::strlen(path) : path_length;
        
        const std::size_t BUF_SIZE  = sizeof(_path) - 1;
        std::size_t result_size     = math::min(BUF_SIZE, path_length);
        
        std::memcpy(_path, path, result_size);
        _path[result_size] = '\0';
        
        filesystem::normalize_path(_path, result_size);
    }
    
    file::file(const file& f) {
        std::memcpy(_path, f._path, sizeof(_path));
    }
    
    file::file(file&& f) {
        std::memcpy(_path, f._path, sizeof(_path));
      	f._path[0] = '\0';
    }
    
    file& file::operator= (const file& f) {
        if (&f != this)
            std::memcpy(_path, f._path, sizeof(_path));
        return *this;
    }
    
    file& file::operator= (file&& f) {
        if (&f != this)
	{
            std::memcpy(_path, f._path, sizeof(_path));
	    f._path[0] = '\0';
	}
        return *this;
    }
    
    file::~file() {
        
    }
    
    bool file::is_file() const {
        if (_path[0] == '\0')
            return false;
        return filesystem::is_file(_path);
    }

    bool file::exists() const {
        if (_path[0] == '\0')
            return filesystem::exists(".");
        return filesystem::exists(_path);
    }

    bool file::is_dir() const {
        if (_path[0] == '\0')
            return filesystem::is_dir(".");
        return filesystem::is_dir(_path);
    }

    bool file::mkdir() {
        if (_path[0] == '\0')
            return true;
        return filesystem::mkdir(_path);
    }

    bool file::mkdirs() {
        if (_path[0] == '\0')
            return true;
        return filesystem::mkdirs(_path);
    }

    std::uintmax_t file::length() const {
        return filesystem::length(_path);
    }

    timepoint file::last_modified() const {
        return filesystem::last_modified(_path);
    }

    bool file::set_last_modified(uint64_t ms) {
        return filesystem::set_last_modified(_path, ms);   
    }

    bool file::is_absolute_path() const {
        return filesystem::is_absolute(_path);
    }

    bool file::create_new_file() {
        return filesystem::create_new_file(_path);
    }

    bool file::remove() {
        return filesystem::remove(_path);
    }

    bool file::rename_to(const char* new_name) {
        return filesystem::rename_to(_path, new_name);
    }

    bool file::can_execute() const {
        return filesystem::can_execute(_path);
    }

    bool file::can_read() const {
        return filesystem::can_read(_path);
    }

    bool file::can_write() const {
        return filesystem::can_write(_path);
    }

    bool file::set_executable(bool on_off) {
        return filesystem::set_executable(_path, on_off);
    }

    bool file::set_readable(bool on_off) {
        return filesystem::set_readable(_path, on_off);
    }

    bool file::set_writable(bool on_off) {
        return filesystem::set_writable(_path, on_off);
    }

    const char* file::cstr() const {
        return _path;
    }

    tc::string file::get_name(tca::allocator* alloc) const {
        tc::string str_path(alloc);
     
        // Временный буфер для нормализованного пути.
        char path[sizeof(_path)];
        std::memcpy(path, _path, sizeof(path));
        filesystem::normalize_path(path);

        // Поиск индекса, откуда начинается название файла
        const std::size_t len = std::strlen(path);
        std::size_t idx = 0;
        for (std::size_t i = len; i > 0; ) {
            --i;
            if (filesystem::is_separator(path[i]))
            {
                idx = i + 1;
                break;    
            }
        }

        assert(len >= idx);
        str_path.append(path + idx, len - idx);

        return tc::string(std::move(str_path));
    }

    array<file> file::list_files(const file_filter& filter, tca::allocator* allocator) const {

        const char* path = _path[0] != '\0' ? _path : ".";

        std::size_t count_files = filesystem::count_files_in_directory(path, filter);
        
        array<file> files(count_files, allocator);

        directory_iterator begin(path);
        directory_iterator end;

        std::size_t i = 0;
        char path_buf[filesystem::MAX_LENGTH_PATH];
        while (begin != end) {
            
            directory_entry entry   = *begin;
            std::size_t path_length = std::strlen(entry.get_name());
            
            if (filter.apply(entry.get_name(), path_length))
            {
                if (_path[0] != '\0')
                    std::snprintf(path_buf, sizeof(path_buf), "%s/%s", path, entry.get_name());
                else
                    std::snprintf(path_buf, sizeof(path_buf), "%s", entry.get_name());
                files[i++]  = file(path_buf);
            }
            
            ++begin;
        }

        return array<file>(std::move(files));
    }

    /*static*/ file file::runtime_path() {
        char buf[filesystem::MAX_LENGTH_PATH];
        std::size_t sz = filesystem::runtime_path(buf, sizeof(buf));
        return file(buf, sz);
    }

    /*static*/ file file::make(const char* format, ...) {
        va_list args;
        char buf[filesystem::MAX_LENGTH_PATH];
        
        va_start(args, format);
        std::vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        
        return file(buf);
    }

    /*static*/ file file::make_absolute(const char* format, ...) {
        char rt_path[filesystem::MAX_LENGTH_PATH];
        std::size_t len = filesystem::runtime_path(rt_path, sizeof(rt_path));
        
        if (len + 1 > (int) sizeof(rt_path))
            throw_except<illegal_state_exception>("buffer is small!");
        
        rt_path[len++] = filesystem::FILE_SEPARATOR;
        
        assert(sizeof(rt_path) >= len);
        std::size_t free_space = sizeof(rt_path) - len;
        
        {
            va_list args;
            va_start(args, format);
            std::vsnprintf(rt_path + len, free_space, format, args);
            va_end(args);
        }
        
        return file(rt_path);
    }

    file file::get_parent() const {
        char parent_path_buf[filesystem::MAX_LENGTH_PATH];
        filesystem::get_parent(_path, parent_path_buf, sizeof(parent_path_buf));
        return file(parent_path_buf);
    }

    std::size_t file::hashcode() const {
        return objects::hashcode(_path, std::strlen(_path));
    }

    file file::plus(const file& f) const {
        char buf[io::constants::MAX_LENGTH_PATH];
        std::snprintf(buf, sizeof(buf), "%s/%s", _path, f._path);
        return file(buf);
    }

    bool file::equals(const file& f) const {
        const char* p1 = _path;
        const char* p2 = f._path;
        while (*p1 && *p2)
            if (*(p1++) != *(p2++))
                return false;
        return true;
    }
}