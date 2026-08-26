#ifndef _JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <allocators/allocator.hpp>

namespace tc
{

/**
 * Buffered input stream decorator.
 * 
 * Provides buffered reading capabilities on top of any istream implementation.
 * Reads data from the underlying stream in blocks and serves it from an
 * internal buffer, significantly improving performance for small reads.
 * 
 * The buffer can be allocated either by the class (using an allocator) or
 * provided externally by the user.
 * 
 * This class is move-only (non-copyable).
 * The wrapped stream must outlive this object unless ownership is transferred.
 * Supports both owned and external buffer management.
 */
class ibstream : public istream {
    static const int DEFAULT_BUFFER_SIZE = 4096;
    
    tca::allocator* m_allocator;
    char*       m_buffer;
    std::size_t m_capacity;
    std::size_t m_offset;
    std::size_t m_limit;
    istream*    m_in;

    /**
     * Frees the internal buffer if it was allocated by this object.
     * 
     * If m_allocator is not nullptr, deallocates the buffer using the allocator.
     * Sets m_allocator to nullptr after deallocation.
     * Does nothing if the buffer was provided externally.
     */
    void free();

    /**
     * Refills the buffer from the underlying stream.
     * 
     * Reads up to m_capacity bytes from m_in into m_buffer.
     * Sets m_limit to the number of bytes actually read (0 on EOF).
     * Resets m_offset to 0.
     * 
     * @throws io_exception
     *      If the underlying stream throws an error.
     */
    void fill_buffer();

public:
    /**
     * 
     */
    using istream::close;
    using istream::read;
    
    /**
     * Default constructor.
     * 
     * Constructs an ibstream object without an associated stream or buffer.
     * Any read operations on this object will fail.
     */
    ibstream();

    /**
     * Constructor with automatic buffer allocation.
     * 
     * Creates a buffered stream that allocates its own buffer using the
     * provided allocator. The buffer is automatically freed in the destructor.
     * 
     * @param stream
     *      Pointer to the underlying input stream.
     * 
     * @param buf_size
     *      Size of the buffer in bytes (default: 4096).
     * 
     * @param allocator
     *      Allocator to use for buffer allocation (default: default allocator).
     * 
     * @throws io_exception
     *      If the underlying stream is nullptr.
     */
    ibstream(istream* stream, std::size_t buf_size = DEFAULT_BUFFER_SIZE, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Constructor with external buffer.
     * 
     * Creates a buffered stream that uses a user-provided buffer.
     * The caller is responsible for managing the buffer's lifetime.
     * 
     * The buffer must remain valid for the lifetime of this object.
     * 
     * @param stream
     *      Pointer to the underlying input stream.
     * 
     * @param buf
     *      Pointer to the external buffer.
     * 
     * @param buf_size
     *      Size of the buffer in bytes.
     * 
     * @throws io_exception
     *      If the underlying stream is nullptr.
     * 
     */
    ibstream(istream* stream, char* buf, std::size_t buf_size);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the underlying stream and buffer from another
     * ibstream object. The source object is left in a valid but unspecified state.
     * 
     * @param stream
     *      The ibstream object to move from.
     */
    ibstream(ibstream&& stream);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the underlying stream and buffer from another
     * ibstream object. If this object currently holds resources, they are
     * properly released first.
     * 
     * @param stream
     *      The ibstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    ibstream& operator= (ibstream&& stream);
    
    /**
     * Reads bytes from the buffered stream.
     * 
     * Reads up to sz bytes from the underlying stream using the internal buffer.
     * Data is served from the buffer when available; the buffer is refilled
     * automatically when exhausted.
     * 
     * @param buf
     *      Pointer to the output buffer.
     * 
     * @param sz
     *      Maximum number of bytes to read.
     * 
     * @return istream::eof_value() if end of stream is reached,
     *         otherwise the actual number of bytes read (> 0).
     * 
     * @throws io_exception if
     *      The underlying stream's read() throws an exception.
     */
    std::size_t read(char buf[], std::size_t sz) override;
    
    /**
     * Destructor.
     * 
     * Closes the underlying stream and frees the internal buffer if it was
     * allocated by this object. Any errors during closing are ignored.
     */
    ~ibstream();
    
    /**
     * Closes the buffered stream without throwing exceptions.
     * 
     * Closes the underlying stream and frees the internal buffer if it was
     * allocated by this object. All errors from the underlying stream are
     * reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err) override;

};

}
#endif//_JSTD_CPP_LANG_IO_BUFFEREDINPUTSTREAM_H_