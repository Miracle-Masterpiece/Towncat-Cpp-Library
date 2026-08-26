#ifndef F862ADB9_4DF3_4ABB_8BB8_6402F7D25629
#define F862ADB9_4DF3_4ABB_8BB8_6402F7D25629


#include <cstddef>
#include <cpp/lang/string.hpp>
#include <cpp/lang/errors.hpp>
#include <cpp/lang/io/file.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/io/basebuf.hpp>

namespace tc
{

/**
 * Random access file stream with read/write capabilities.
 * 
 * Provides random access file I/O with both read and write operations.
 * Supports seeking, getting file pointer, truncating, and force-flushing
 * to disk. Includes typed read/write methods with endianness handling
 * and string serialization.
 */
class rafstream {
    void* m_handle;
public:
    
    /**
     * Returns the end-of-stream sentinel value.
     * 
     * @return
     *      Used to indicate EOF for read operations.
     */
    static std::size_t eof_value() {
        return ~std::size_t(0);
    }

    /**
     * Default constructor.
     * 
     * Constructs a rafstream object without opening a file.
     */
    rafstream();
    
    /**
     * Constructor from file path (C-style string).
     * 
     * @param file
     *      Path to the file to open.
     * 
     * @param mode
     *      File open mode:
     *          "r"  : Read mode (binary)
     *          "w"  : Write mode (binary, truncates)
     *          "rw" : Read/Write mode (binary, creates if not exists)
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    rafstream(const char* file, const char* mode);
    
    /**
     * Constructor from file object.
     * 
     * @param file
     *      File object containing the path to open.
     * 
     * @param mode
     *      File open mode (see constructor above).
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    rafstream(const file& file, const char* mode);
    
    /**
     * Constructor from file object with string mode.
     * 
     * @param file
     *      File object containing the path to open.
     * 
     * @param mode
     *      File open mode as string.
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    rafstream(const file& file, const string& mode);
    
    /**
     * Constructor from file path (C++ string).
     * 
     * @param file
     *      Path to the file to open.
     * 
     * @param mode
     *      File open mode as string.
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    rafstream(const string& file, const string& mode);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the file handle from another rafstream object.
     * 
     * @param s
     *      The rafstream object to move from.
     */
    rafstream(rafstream&&);

    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the file handle from another rafstream object.
     * If this object currently holds an open file, it is closed first.
     * 
     * @param s
     *      The rafstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    rafstream& operator=(rafstream&&);
    
    /**
     * Destructor.
     * 
     * Automatically closes the file handle if it is open.
     * Any errors during closing are ignored.
     */
    ~rafstream();
    
    /**
     * Closes the file and throws an exception on error.
     * 
     * @throws io_exception
     *      If an error occurs while closing the file.
     */
    void close();
    
    /**
     * Closes the file without throwing exceptions.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err);
    
     /**
     * Sets the file length (truncates or extends).
     * 
     * @param len
     *      New file size in bytes.
     * 
     * @throws io_exception
     *      If truncation fails.
     */
    void set_length(std::size_t len);
    
    /**
     * Returns the current file size.
     * 
     * @return
     *      File size in bytes.
     * 
     * @throws io_exception
     *         I/O fails.
     */
    std::size_t length();
    
    /**
     * Seeks to the specified position in the file.
     * 
     * @param pos
     *      Position to seek to (from beginning of file).
     * 
     * @throws io_exception
     *      If seek operation fails.
     */
    void seek(std::size_t pos);
    
    /**
     * Returns the current file pointer position.
     * 
     * @return
     *      Current position in bytes from the beginning of the file.
     * 
     * @throws io_exception
     *      If tell operation fails.
     */
    std::size_t get_file_pointer();
    
    /**
     * Skips n bytes by reading and discarding them.
     * 
     * @param n
     *      Number of bytes to skip.
     * 
     * @throws io_exception
     *      If read operation fails.
     */
    void skip_bytes(std::size_t n);
    
    /**
     * Writes a block of data to the file.
     * 
     * This method guarantees either all bytes are written or an exception is thrown.
     * 
     * @param arr
     *      Pointer to the data to write.
     * 
     * @param len
     *      Number of bytes to write.
     * 
     * @throws io_exception
     *      If the number of bytes written does not match len.
     * 
     */
    void write(const char* arr, std::size_t len);
    
    /**
     * Writes a single character to the file.
     * 
     * @param ch
     *      The character to write.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    void write(char ch);
    
    /**
     * Writes a typed value to the file.
     * 
     * The value is written in little-endian byte order.
     * 
     * @tparam T
     *      The type of value to write. Must be trivially copyable.
     * 
     * @param x
     *      The value to write.
     * 
     * @throws io_exception 
     *      If an I/O error occurs.
     */
    template<typename T>
    void write(T x);
    
    /**
     * Writes a string to the file with length prefix.
     * 
     * Writes the string length as a 32-bit unsigned integer followed
     * by the string data.
     * 
     * @param x
     *      The string to write.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    void write_string(const string& x);
    
    /**
     * Reads a block of data from the file.
     * 
     * @param buf
     *      Pointer to the output buffer.
     * 
     * @param len
     *      Maximum number of bytes to read.
     * 
     * @return eof_value() if end of file is reached,
     *         otherwise the actual number of bytes read (> 0).
     * 
     * @throws io_exception
     *      If an I/O error occurs during reading.
     */
    std::size_t read(char buf[], std::size_t len);
    
    /**
     * Reads a single byte from the file.
     * 
     * @return
     *      The read byte in range 0..255, or -1 if end of file is reached.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    int read();
    
    /**
     * Reads a typed value from the file.
     * 
     * The value is read in little-endian byte order.
     * 
     * @tparam T
     *      The type of value to read. Must be trivially copyable.
     * 
     * @return
     *      The read value of type T.
     * 
     * @throws eof_exception
     *      If exactly sizeof(T) bytes cannot be read.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    template<typename T>
    T read();
    
    /**
     * Reads a length-prefixed string from the file.
     * 
     * Reads the string length as a 32-bit unsigned integer followed
     * by the string data.
     * 
     * @param alloc
     *      Allocator to use for the string (default: default allocator).
     * 
     * @return
     *      The read string.
     * 
     * @throws eof_exception
     *      If the complete string cannot be read.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    string read_string(tca::allocator* alloc = tca::get_default_allocator());

    /**
     * Forces file data to be written to physical storage.
     * 
     * @param metadata
     *          If true, sync metadata as well (fsync);
     *          If false, sync only data (fdatasync on Unix).
     * 
     * @throws io_exception
     *      If flush or sync operation fails.
     */
    void force(bool metadata);
private:
    rafstream(const rafstream&) = delete;
    rafstream& operator= (const rafstream&) = delete;
};

    template<typename T>
    void rafstream::write(T x) {
        char buf[sizeof(T)];
        utils::write_le(buf, x);
        write(buf, sizeof(T));
    }
    
    template<typename T>
    T rafstream::read() {
        char buf[sizeof(T)];
        std::size_t readed = read(buf, sizeof(T));
        if (readed != sizeof(T))
        {
            throw_except<eof_exception>("cannot read type");
        }
        return utils::read_le<T>(buf);
    }

}

#endif /* F862ADB9_4DF3_4ABB_8BB8_6402F7D25629 */
