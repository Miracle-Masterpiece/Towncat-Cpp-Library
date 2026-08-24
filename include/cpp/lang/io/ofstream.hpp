#ifndef _JSTD_CPP_LANG_IO_FILEOUTPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_FILEOUTPUTSTREAM_H_

#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/file.hpp>
#include <cstdio>

namespace tc
{

/**
 * File output stream class.
 * 
 * Provides file-based output stream functionality using C standard library FILE* handles.
 * Inherits from ostream and implements file writing, flushing, and closing operations.
 */
class ofstream : public ostream {
    FILE* m_handle;
public:
    using ostream::close;
    using ostream::write;

    /**
     * Default constructor.
     * 
     * Constructs an ofstream object without opening a file.
     * The stream is not associated with any file.
     */
    ofstream();

    /**
     * Constructor from file path (C++ string).
     * 
     * Opens the file in binary mode. If append is true, opens in append mode ("ab");
     * otherwise, opens in write mode ("wb").
     * 
     * @param path
     *      Path to the file to open.
     * 
     * @param append
     *      If true, append to existing file; if false, overwrite existing file.
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    ofstream(const string& path, bool append = false);

    /**
     * Constructor from file path (C-style string).
     * 
     * Opens the file in binary mode. If append is true, opens in append mode ("ab");
     * otherwise, opens in write mode ("wb").
     * 
     * @param path
     *      Path to the file to open.
     * 
     * @param append
     *      If true, append to existing file; if false, overwrite existing file.
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    ofstream(const char* path, bool append = false);

    /**
     * Constructor from file object.
     * 
     * Opens the file specified by the file object in binary mode.
     * 
     * @param file
     *      File object containing the path to open.
     * 
     * @param append
     *      If true, append to existing file; if false, overwrite existing file.
     * 
     * @throws io_exception
     *      If the file cannot be opened.
     * 
     * @throws security_exception
     *      If permission is denied.
     */
    ofstream(const file& file, bool append = false);

    /**
     * Move constructor.
     * 
     * Transfers ownership of the file handle from another ofstream object.
     * The source object is left in a valid but unspecified state (handle set to nullptr).
     */
    ofstream(ofstream&&);

    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the file handle from another ofstream object.
     * If this object currently holds an open file, it is closed first.
     * 
     * @param out
     *      The ofstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    ofstream& operator= (ofstream&&);

    /**
     * Destructor.
     * 
     * Automatically closes the file handle if it is open.
     * Any errors during closing are ignored.
     */
    ~ofstream();

    /**
     * Writes a block of data to the file.
     * 
     * This method guarantees either all bytes are written or an exception is thrown.
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param sz
     *      Number of bytes to write.
     * 
     * @throws io_exception
     *      If the number of bytes written does not match sz.
     */
    void write(const char* data, std::size_t sz) override;
    
    /**
     * Flushes the file buffers.
     * 
     * Forces any buffered data to be written to the file.
     * 
     * @throws io_exception
     *      If I/O error occurs during flushing.
     */
    void flush() override;

    /**
     * Closes the file stream without throwing exceptions.
     * 
     * If the file handle is already nullptr, this function does nothing.
     * Closes the file and sets the handle to nullptr.
     * All errors are reported through the err parameter.
     * 
     * @param err
     *      Reference to an error_code object that will receive the error status.
     */
    void close(error_code& err) override;
};

} 
#endif//_JSTD_CPP_LANG_IO_FILEOUTPUTSTREAM_H_
