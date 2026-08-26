#ifndef _JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <allocators/allocator.hpp>

namespace tc
{

/**
 * Output stream that writes to a dynamically growing memory buffer.
 * 
 * Provides an output stream interface for writing data to a memory buffer.
 * The buffer can be either dynamically allocated and automatically resized,
 * or provided externally by the user.
 * 
 * With allocator: Buffer grows automatically as needed using geometric growth
 * Without allocator (external buffer): Fixed-size buffer, throws on overflow
 */
class omstream : public ostream {
    static constexpr std::size_t INIT_BUF_SIZE = 16;
    
    tca::allocator* m_allocator;   
    char*       m_buffer;
    std::size_t m_capacity;
    std::size_t m_offset;

    /**
     * Resizes the buffer to accommodate more data.
     * 
     * Allocates a new buffer with sufficient capacity, copies existing data,
     * and frees the old buffer. Uses geometric growth (1.5x) for amortized
     * constant-time append operations.
     * 
     * @param sz
     *      Additional size needed beyond current offset.
     * 
     * @throws out_of_memory_error
     *      If allocation fails.
     */
    void resize(std::size_t sz);

public:
    
    /**
     * Constructor with automatic buffer allocation.
     * 
     * Creates a memory stream that allocates its own buffer using the
     * provided allocator. The buffer grows automatically as needed.
     * 
     * @param init_buf_size
     *      Initial buffer size in bytes (default: 16).
     * 
     * @param allocator
     *      Allocator to use for buffer allocation (default: default allocator).
     * 
     * @throws out_of_memory_error
     *      If initial buffer allocation fails.
     */
    omstream(std::size_t init_buf_size = INIT_BUF_SIZE, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Constructor with external buffer (fixed size).
     * 
     * Creates a memory stream that uses a user-provided buffer.
     * The buffer is fixed-size and will not grow.
     * 
     * Writing more than capacity bytes will throw overflow_exception.
     * 
     * @param buf
     *      Pointer to the external buffer.
     * 
     * @param capacity
     *      Size of the buffer in bytes.
     */
    omstream(char* buf, std::size_t capacity);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the buffer from another omstream object.
     * The source object is left in a valid but unspecified state.
     * 
     * @param stream
     *      The omstream object to move from.
     */
    omstream(omstream&&);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the buffer from another omstream object.
     * If this object currently holds a buffer, it is closed first.
     * Self-assignment is handled correctly.
     * 
     * @param stream
     *      The omstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    omstream& operator= (omstream&&);

    /**
     * Destructor.
     * 
     * Frees the internal buffer if it was allocated by this object.
     * Any errors during cleanup are ignored.
     */
    ~omstream();

    /**
     * Writes data to the memory buffer.
     * 
     * If the buffer is owned (allocator provided), it grows automatically
     * when more space is needed. If the buffer is external (fixed size),
     * throws overflow_exception when capacity is exceeded.
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param sz
     *      Number of bytes to write.
     * 
     * @throws overflow_exception
     *      If external buffer capacity is exceeded.
     * 
     * @throws out_of_memory_error
     *      If buffer allocation fails during resize.
     */
    void write(const char* data, std::size_t sz) override;

    /**
     * No-op
     */
    void flush() override;
    
    /**
     * Closes the memory stream without throwing exceptions.
     * 
     * Frees the internal buffer if it was allocated by this object.
     * All errors are reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err) override;
    
    /**
     * Returns a pointer to the internal buffer.
     * 
     * @return
     *      Pointer to the beginning of the buffer, or nullptr if not allocated.
     */
    const char* data() const;
    
    /**
     * Returns the current write position (number of bytes written).
     * 
     * @return
     *      Current offset in bytes from the beginning of the buffer.
     */
    std::size_t offset() const;
};

}

#endif//_JSTD_CPP_LANG_IO_ARRAY_OUTPUT_STREAM_H_