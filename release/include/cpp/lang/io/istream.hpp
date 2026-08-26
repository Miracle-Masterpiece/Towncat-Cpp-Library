#ifndef _JSTD_CPP_LANG_IO_ISTREAM_H_
#define _JSTD_CPP_LANG_IO_ISTREAM_H_

#include <cpp/lang/errors.hpp>
#include <cstdint>
#include <cstddef>

namespace tc
{

class istream {
    /**
     * 
     */
    istream(const istream&)               = delete;
    
    /**
     * 
     */
    istream& operator= (const istream&)   = delete;
public:
    /**
     * Default constructor.
     */
    istream() {}

    /**
     * Reads a single byte from the stream.
     * 
     * @return
     *      The read byte in range 0..255, or -1 if end of stream (EOF) is reached.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    virtual int read();

    /**
     * Reads up to sz bytes into the supplied buffer.
     * 
     * This method reads at most sz bytes from the stream and stores them
     * into the buffer pointed by buf. The actual number of bytes read
     * may be less than sz if EOF is encountered before reading sz bytes.
     * 
     * @param buf
     *      Pointer to the buffer where data will be stored.
     * 
     * @param sz
     *      Maximum number of bytes to read.
     * 
     * @return 
     *      eof_value() if end of stream is reached (no bytes read),
     *      otherwise the actual number of bytes read (> 0).
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    virtual std::size_t read(char buf[], std::size_t sz) = 0;

    /**
     * Skips up to n bytes from the stream.
     * 
     * @param n
     *      Number of bytes to skip (default is 1).
     * 
     * @return
     *      The actual number of bytes skipped (may be less than n if EOF is reached).
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    virtual std::size_t skip(std::size_t n = 1);
    
    /**
     * Calls close(error_code&) internally, ignoring any errors that occur.
     */
    virtual ~istream();

    /**
     * Closes the stream without throwing exceptions.
     * 
     * This method is pure virtual and must be implemented by derived classes.
     * All errors are reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    virtual void close(error_code& err) = 0;
    
    /**
     * Closes the stream and throws an exception on error.
     * 
     * Default implementation calls close(error_code&). If the error_code
     * indicates an error, an io_exception is thrown.
     * 
     * @throws io_exception
     *      If an error occurs while closing the stream.
     */
    virtual void close();
    
    /**
     * @return
     *      The end-of-stream sentinel value
     */
    static std::size_t eof_value() {return (std::size_t) -1;} 
};

}
#endif//_JSTD_CPP_LANG_IO_ISTREAM_H_

