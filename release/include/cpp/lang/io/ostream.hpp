#ifndef _JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_

#include <cpp/lang/errors.hpp>
#include <cstdint>
#include <cstddef>

namespace tc
{

/**
 * Abstract base class for output streams.
 * 
 * Defines the interface for writing data, flushing buffers, and managing
 * stream resources. This class is non-copyable.
 */
class ostream {
    ostream(const ostream&)               = delete; 
    ostream& operator= (const ostream&)   = delete;
public:
    /**
     * Default constructor.
     */
    ostream() {}
    
    /**
     * Writes a single character to the stream.
     * 
     * @param c
     *      The character to write.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     * 
     * @note Default implementation calls write(&c, 1).
     */
    virtual void write(char c);
    
    /**
     * Writes a sequence of characters to the stream.
     * 
     * This method writes exactly sz bytes from the data buffer to the stream.
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param sz
     *      Number of bytes to write.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    virtual void write(const char* data, std::size_t sz) = 0;
    
    /**
     * Flushes the stream buffers.
     * 
     * Forces any buffered data to be written to the underlying output device.
     * 
     * @throws io_exception
     *      If an I/O error occurs during flushing.
     */
    virtual void flush() = 0;
    
    /**
     * Closes the stream and throws an exception on error.
     * 
     * @throws io_exception
     *      If an error occurs while closing the stream.
     */
    virtual void close();
    
    /**
     * Closes the stream without throwing exceptions.
     * 
     * All errors are reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    virtual void close(error_code& err) = 0;

    /**
     * 
     */
    virtual ~ostream();
};

}
#endif//_JSTD_CPP_LANG_IO_OUTPUTSTREAM_H_

