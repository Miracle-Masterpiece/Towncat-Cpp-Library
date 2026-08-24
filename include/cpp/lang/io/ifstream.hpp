#ifndef _JSTD_CPP_LANG_IO_FILEINPUTSTREAM_H_
#define _JSTD_CPP_LANG_IO_FILEINPUTSTREAM_H_

#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/file.hpp>
#include <cstdio>

namespace tc
{

/**
 * File input stream class.
 * 
 * Provides file-based input stream functionality using C standard library FILE* handles.
 * Inherits from istream and implements file reading and closing operations.
 */
class ifstream : public istream {
    FILE*   m_handle;
public:
    using istream::close;

    /**
     * Default constructor.
     * 
     * Constructs an ifstream object without opening a file.
     * The stream is not associated with any file.
     */
    ifstream();
    
    /**
     * Constructor from file path (C-style string).
     * 
     * @param path
     *      Path to the file to open.
     * 
     * @throws io_exception
     *      if the file cannot be opened.
     * 
     * @throws security_exception
     *      if permission is denied.
     */
    ifstream(const char* path);
    
    /**
     * Constructor from file path (C++ string).
     * 
     * @param path
     *      Path to the file to open.
     * 
     * @throws io_exception
     *      if the file cannot be opened.
     * 
     * @throws security_exception
     *      if permission is denied.
     */
    ifstream(const string& path);
    
    /**
     * Constructor from file object.
     * 
     * @param file
     *      File object containing the path to open.
     * 
     * @throws io_exception
     *      if the file cannot be opened.
     * 
     * @throws security_exception
     *      if permission is denied.
     */
    ifstream(const file& file);
    
    /**
     * Move constructor.
     * 
     * Transfers ownership of the file handle from another ifstream object.
     * The source object is left in a valid but unspecified state (handle set to nullptr).
     * 
     * @param stream
     *      The ifstream object to move from.
     */
    ifstream(ifstream&&);
    
    /**
     * Move assignment operator.
     * 
     * Transfers ownership of the file handle from another ifstream object.
     * If this object currently holds an open file, it is closed first.
     * Self-assignment is handled correctly.
     * 
     * @param stream
     *      The ifstream object to move from.
     * 
     * @return
     *      Reference to this object.
     */
    ifstream& operator= (ifstream&&);
    
    /**
     * Destructor.
     * 
     * Automatically closes the file handle if it is open.
     * Any errors during closing are ignored.
     */
    ~ifstream();
    
    /**
     * Reads up to sz bytes from the file into the buffer.
     * 
     * @param buf
     *      Pointer to the buffer where data will be stored.
     * @param sz Maximum number of bytes to read.
     * 
     * @return istream::eof_value() if end of file is reached,
     *         otherwise the actual number of bytes read (> 0).
     * 
     * @throws io_exception
     *      An I/O error occurs during reading.
     */
    std::size_t read(char buf[], std::size_t sz) override;
    
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
#endif//_JSTD_CPP_LANG_IO_FILEINPUTSTREAM_H_
