#ifndef _JSTD_CPP_LANG_IO_BUFFEREDOUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_BUFFEREDOUTPUTSTREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <allocators/base_allocator.hpp>
#include <allocators/allocator.hpp>

namespace tc
{

/**
 * Buffered output stream decorator.
 * 
 * Provides buffered writing capabilities on top of any ostream implementation.
 * Data is written to an internal buffer first and flushed to the underlying
 * stream when the buffer becomes full or when flush() is called explicitly.
 * 
 * The buffer can be allocated either by the class (using an allocator) or
 * provided externally by the user.
 */
class obstream : public ostream {

    static constexpr std::size_t DEFAULT_BUFFER_SIZE = 0x1000;
    
    tca::allocator* m_allocator;
    char*       m_buffer;
    std::size_t m_capacity;
    std::size_t m_offset;
    ostream*    m_out;

    /**
     * Frees the internal buffer if it was allocated by this object.
     * 
     * If m_allocator is not nullptr, deallocates the buffer using the allocator.
     * Sets m_allocator to nullptr after deallocation.
     * Does nothing if the buffer was provided externally.
     */
    void free();

public:

    using ostream::close;
    using ostream::write;

    /**
     * Default constructor.
     * 
     * Constructs an obstream object without an associated stream or buffer.
     * Any write operations on this object will fail.
     */
    obstream();

    /**
     * Constructor with automatic buffer allocation.
     * 
     * Creates a buffered stream that allocates its own buffer using the
     * provided allocator. The buffer is automatically freed in the destructor.
     * 
     * @param stream
     *      Pointer to the underlying output stream.
     * 
     * @param allocator
     *      Allocator to use for buffer allocation (default: default allocator).
     * 
     * @param buf_size
     *      Size of the buffer in bytes (default: 4096).
     * 
     * @throws out_of_memory_error
     *      If buffer allocation fails.
     */
    obstream(ostream* stream, tca::allocator* allocator = tca::get_default_allocator(), std::size_t buf_size = DEFAULT_BUFFER_SIZE);
    
    /**
     * Constructor with external buffer.
     * 
     * Creates a buffered stream that uses a user-provided buffer.
     * The caller is responsible for managing the buffer's lifetime.
     * 
     * @param stream
     *      Pointer to the underlying output stream.
     * 
     * @param buffer
     *      Pointer to the external buffer.
     * 
     * @param buf_size
     *      Size of the buffer in bytes.
     */
    obstream(ostream* stream, char* buffer, std::size_t buf_size);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the underlying stream and buffer from another
     * obstream object. The source object is left in a valid but unspecified state.
     * 
     * @param stream
     *      The obstream object to move from.
     */
    obstream(obstream&& stream);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the underlying stream and buffer from another
     * obstream object. If this object currently holds resources, they are
     * properly released first. Self-assignment is handled correctly.
     * 
     * @param stream
     *      The obstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    obstream& operator= (obstream&& stream);

    /**
     * Destructor.
     * 
     * Flushes any buffered data and closes the underlying stream.
     * Any errors during flushing or closing are ignored.
     */
    ~obstream();
    
    /**
     * Writes data to the buffered stream.
     * 
     * Data is written to the internal buffer first. If the buffer does not
     * have enough space, it is flushed automatically. If the data size exceeds
     * the buffer capacity, it is written directly to the underlying stream
     * without buffering.
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param sz
     *      Number of bytes to write.
     * 
     * @throws io_exception
     *      If the underlying stream throws an exception during flush or direct write.
     */
    void write(const char* data, std::size_t sz) override;

    /**
     * Flushes the buffered stream.
     * 
     * Writes any buffered data to the underlying stream and resets the buffer
     * write position to 0.
     * 
     * @throws io_exception
     *      If the underlying stream throws an exception.
     */
    void flush() override;

    /**
     * Closes the buffered stream without throwing exceptions.
     * 
     * Flushes any buffered data, closes the underlying stream, and frees the
     * internal buffer if it was allocated by this object. All errors are reported
     * through the err parameter.
     * 
     * If flush() fails, the error is captured and combined with any close error.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err) override;
};

}


#endif//_JSTD_CPP_LANG_IO_BUFFEREDOUTPUTSTREAM_H_