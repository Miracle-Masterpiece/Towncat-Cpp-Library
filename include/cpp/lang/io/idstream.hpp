#ifndef _JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_
#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/utils.hpp>

namespace tc
{

/**
 * Input decorator stream for reading typed data.
 * 
 * Provides typed read operations on top of a raw istream. Reads integers,
 * structures, and arrays with proper endianness handling (little-endian).
 * Acts as a decorator/wrapper around any istream-derived object.
 * 
 * This class is move-only (non-copyable).
 * The wrapped stream must outlive this object unless ownership is transferred.
 * Does not own the wrapped stream by default (decorator pattern).
 */
class idstream : public istream {
    istream* m_in;
public:
    using istream::read;
    using istream::close;

    /**
     * Default constructor.
     * 
     * Constructs an idstream object without an associated stream.
     * Any read operations on this object will fail.
     */
    idstream();
    
    /**
     * Constructor from a raw istream pointer.
     * The wrapped stream is not owned.
     * it must remain valid for the lifetime of this object.
     * 
     * @param in
     *  Pointer to the underlying input stream.
     * 
     * @throws null_pointer_exception
     *      if 'in' is nullptr.
     */
    idstream(istream* in);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the wrapped stream pointer from another idstream object.
     * The source object is left in a valid but unspecified state (m_in set to nullptr).
     */
    idstream(idstream&&);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the wrapped stream pointer from another idstream object.
     * If this object currently holds a stream, it is closed first.
     * 
     * @param stream
     *      The idstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    idstream& operator= (idstream&&);
    
    /**
     * Destructor.
     * 
     * Closes the wrapped stream if it is associated. Any errors during closing are ignored.
     */
    ~idstream();
    
    /**
     * Reads raw bytes from the underlying stream.
     * 
     * @param buf
     *      Pointer to the buffer where data will be stored.
     * 
     * @param sz
     *      Maximum number of bytes to read.
     * 
     * @return istream::eof_value() if end of stream is reached,
     *         otherwise the actual number of bytes read (> 0).
     * 
     * @throws io_exception
     *      The underlying stream throws an exception.
     */
    std::size_t read(char buf[], std::size_t sz) override;
    
    /**
     * Closes the wrapped stream without throwing exceptions.
     * 
     * If the wrapped stream is nullptr, this function does nothing.
     * Closes the underlying stream and sets m_in to nullptr.
     * All errors are reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err) override;
    
    /**
     * Reads a typed value from the stream.
     * 
     * Reads exactly sizeof(T) bytes from the underlying stream and
     * interprets them as a little-endian value of type T.
     * 
     * @tparam T
     *      The type of value to read. Must be trivially copyable.
     * 
     * @return
     *      The read value of type T.
     * 
     * @throws eof_exception
     *      if fewer than sizeof(T) bytes could be read.
     * 
     * @throws io_exception
     *      if the underlying stream throws an error.
     */
    template<typename T>
    T read() {
        char buf[sizeof(T)];
        std::size_t readed = read(buf, sizeof(T));
        if (readed != sizeof(T))
        {
            throw_except<eof_exception>("cannot read type");
        }
        utils::read_le<T>(buf);
    }

    template<typename T>
    void read(T buf[], std::size_t sz);
};

    /**
     * Reads an array of typed values from the stream.
     * 
     * Reads sz elements of type T from the underlying stream.
     * Each element is read using the single-element read<T>() method.
     * 
     * @tparam T
     *      The type of values to read. Must be trivially copyable.
     * 
     * @param buf
     *      Pointer to the array where values will be stored.
     * 
     * @param sz
     *      Number of elements to read.
     * 
     * @throws eof_exception
     *      if any read<T>() fails to read sizeof(T) bytes.
     * 
     * @throws io_exception
     *      if the underlying stream throws an error.
     */
    template<typename T>
    void idstream::read(T buf[], std::size_t sz) {
        for (std::size_t i = 0; i < sz; ++i)
            buf[i] = read<T>();
    }
}

#endif//_JSTD_CPP_LANG_IO_DATA_INPUT_STREAM_H_