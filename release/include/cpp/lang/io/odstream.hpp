#ifndef _JSTD_CPP_LANG_IO_DATA_OUTPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_DATA_OUTPUT_STREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/types.hpp>

namespace tc
{

class odstream;

namespace internal
{

template<typename T, std::size_t SIZE = sizeof(T)>
struct ostream_string_write {
    static void write(const tstring<T>& s, odstream*);
};

template<typename T>
struct ostream_string_write<T, sizeof(char)> {
    static void write(const tstring<T>& s, odstream*);
};

} //namespace internal

/**
 * Output decorator stream for writing typed data.
 * 
 * Provides typed write operations on top of a raw ostream. Writes integers,
 * structures, and arrays with proper endianness handling (little-endian).
 * Acts as a decorator/wrapper around any ostream-derived object.
 */
class odstream : public ostream {
    ostream* m_out;
public:
    using ostream::write;
    using ostream::close;
    
    /**
     * Default constructor.
     * 
     * Constructs an odstream object without an associated stream.
     * Any write operations on this object will fail.
     */
    odstream();

    /**
     * Constructor from a raw ostream pointer.
     * 
     * @param out
     *      Pointer to the underlying output stream.
     * 
     * @throws io_exception
     *      In debug builds if out is nullptr.
     * 
     * @note The wrapped stream is not owned; it must remain valid for the lifetime of this object.
     */
    odstream(ostream* out);

    /**
     * Move constructor.
     * 
     * Transfers ownership of the wrapped stream pointer from another odstream object.
     * The source object is left in a valid but unspecified state (m_out set to nullptr).
     * 
     * @param stream
     *      The odstream object to move from.
     */
    odstream(odstream&&);

    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the wrapped stream pointer from another odstream object.
     * If this object currently holds a stream, it is closed first.
     * 
     * @param out
     *      The odstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    odstream& operator= (odstream&&);
    
    /**
     * Destructor.
     * 
     * Closes the wrapped stream if it is associated. Any errors during closing are ignored.
     */
    ~odstream();

    /**
     * Writes raw bytes to the underlying stream.
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param sz
     *      Number of bytes to write.
     * 
     * @throws io_exception
     *      If the underlying stream throws an exception.
     */
    void write(const char* data, std::size_t sz) override;

     /**
     * Flushes the underlying stream.
     * 
     * Forces any buffered data to be written to the underlying output device.
     * 
     * @throws io_exception 
     *      If the underlying stream throws an exception.
     */
    void flush() override;

    /**
     * Closes the wrapped stream without throwing exceptions.
     * 
     * If the wrapped stream is nullptr, this function does nothing.
     * Closes the underlying stream and sets m_out to nullptr.
     * All errors are reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err) override;

    /**
     * Writes a typed value to the stream.
     * 
     * Writes a value of type T to the underlying stream in little-endian byte order.
     * 
     * @tparam T
     *      The type of value to write. Must be trivially copyable.
     * 
     * @param v
     *      The value to write.
     * 
     * @throws io_exception
     *      If the underlying stream throws an error.
     */
    template<typename T>
    void write(T v) {
        char buf[sizeof(T)];
        utils::write_le(buf, v);
        write(buf, sizeof(T));
    }

    /**
     * Writes a string to the stream with length prefix.
     * 
     * Writes the string length as a 32-bit unsigned integer followed
     * by the string data. The string data is written in little-endian
     * byte order for each character.
     * 
     * @tparam TCHAR
     *      The character type of the string (char, wchar_t, etc.).
     * 
     * @param s
     *      The string to write.
     * 
     * @throws io_exception
     *      If the underlying stream throws an error.
     * 
     * @note The string format is: [length (4 bytes)] [character data]
     * 
     * @note Characters are written in little-endian byte order.
     */
    template<typename TCHAR>
    void write_string(const tstring<TCHAR>& s);

    /**
     * Writes an array of typed values to the stream.
     * 
     * Writes sz elements of type T to the underlying stream.
     * Each element is written using the single-element write<T>() method.
     * 
     * @tparam T
     *      The type of values to write. Must be trivially copyable.
     * 
     * @param arr
     *      Pointer to the array of values to write.
     * 
     * @param sz
     *      Number of elements to write.
     * 
     * @throws io_exception
     *      If the underlying stream throws an error.
     */
    template<typename T>
    void write(const T* v, std::size_t sz);
};

    template<typename T>
    void odstream::write(const T* arr, std::size_t sz) {
        for (std::size_t i = 0; i < sz; ++i)
        {
            write<T>(arr[i]);
        }
    }

    template<typename TCHAR>
    void odstream::write_string(const tstring<TCHAR>& s) {
        using internal::ostream_string_write;
        ostream_string_write<TCHAR, sizeof(TCHAR)>::write(s, this);
    }
}


namespace tc
{
namespace internal
{
    template<typename T, std::size_t SIZE>
    /*static*/ void ostream_string_write<T, SIZE>::write(const tstring<T>& s, odstream* out) {
        len_type len = static_cast<len_type>(s.length());
        out->write<len_type>(len);
        
        for (std::size_t i = 0; i < s.length(); ++i)
            out->write<T>(s[i]);
    }
    

    template<typename T>
    /*static*/ void ostream_string_write<T, sizeof(char)>::write(const tstring<T>& s, odstream* out) {    
        len_type len     = static_cast<len_type>(s.length());
        const char* data = static_cast<const char*>(s.c_str());
    
        // Write string length
        out->write<len_type>(len);
        
        // Write string data
        out->write(data, len);
    }

} //namespace internal
} //namespace tc

#endif//_JSTD_CPP_LANG_IO_DATA_OUTPUT_STREAM_H_