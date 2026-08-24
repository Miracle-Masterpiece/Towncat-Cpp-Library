#ifndef _JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_
#define _JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <allocators/base_allocator.hpp>

namespace tc
{

/**
 * Input stream that reads from a fixed memory buffer.
 * 
 * Provides an input stream interface for reading data from a memory buffer
 * that is provided externally. The buffer is read-only and its lifetime must
 * be managed by the caller. This class is particularly useful for parsing
 * data that is already loaded in memory.
 * 
 * This class is move-only (non-copyable).
 * The buffer is not owned by this class and must remain valid for the lifetime of the stream.
 * The buffer is read-only (const char*).
 */
class imstream : public istream {
    const char* m_buffer;    
    std::size_t m_capacity;
    std::size_t m_offset;
public:
    using istream::read;
    using istream::close;

    /**
     * Default constructor.
     * 
     * Constructs an imstream object without an associated buffer.
     * Any read operations on this object will fail.
     */
    imstream();

    /**
     * Constructor from a memory buffer.
     * 
     * @param buffer
     *      Pointer to the memory buffer to read from.
     *      The buffer must remain valid for the lifetime of this object.
     *      The buffer is read-only; no modifications are made.
     * 
     * @param capacity
     *      Size of the buffer in bytes.
     * 
     */
    imstream(const char* buffer, std::size_t capacity);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the buffer reference from another imstream object.
     * The source object is left in a valid but unspecified state.
     * 
     * @param stream
     *      The imstream object to move from.
     */
    imstream(imstream&&);

    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the buffer reference from another imstream object.
     * If this object currently holds a buffer, it is closed first.
     * 
     * @param stream
     *      The imstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    imstream& operator= (imstream&&);

    /**
     * Destructor.
     * 
     * Does nothing since the buffer is not owned by this class.
     * The caller is responsible for managing the buffer's lifetime.
     */
    ~imstream();

    /**
     * Reads bytes from the memory buffer.
     * 
     * Copies up to sz bytes from the internal buffer to the output buffer.
     * The read position is advanced by the number of bytes copied.
     * 
     * @param buf
     *      Pointer to the output buffer.
     * 
     * @param sz
     *      Maximum number of bytes to read.
     * 
     * @return
     *      istream::eof_value() if end of buffer is reached,
     *      otherwise the actual number of bytes read (> 0).
     */
    std::size_t read(char buf[], std::size_t sz) override;

    /**
     * Closes the memory stream without throwing exceptions.
     * 
     * Releases the reference to the buffer by setting m_buffer to nullptr.
     * This does not free or deallocate the buffer itself.
     * 
     * @param err
     *      Reference to an error_code object (unused, always success).
     */
    void close(error_code& err) override;

    /**
     * Resets the read position to the beginning of the buffer.
     * 
     * Sets m_offset to 0, allowing the stream to be read from the start again.
     * Does not affect the buffer contents.
     */
    void reset();
};


}

#endif//_JSTD_CPP_LANG_IO_ARRAY_INPUT_STREAM_H_