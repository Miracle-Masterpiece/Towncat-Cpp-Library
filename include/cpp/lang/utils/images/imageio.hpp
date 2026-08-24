#ifndef JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H
#define JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H

#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cpp/lang/io/ostream.hpp>
#include <cpp/lang/io/file.hpp>

namespace tc
{
namespace imageio
{

    /**
     * Loads an image from an input stream.
     * 
     * Uses STB image library with custom I/O callbacks to read image data
     * from the provided stream. The image data is allocated using the custom
     * allocator and returned as an image object.
     * 
     * @param in
     *      Pointer to the input stream.
     * 
     * @param allocator
     *      Pointer to the allocator to use (may be unused as STB uses internal allocator).
     * 
     * @return image
     *      The loaded image object.
     * 
     * @throws illegal_state_exception
     *      If image loading fails.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    image load_image(istream* in, tca::allocator* allocator);
    
    /**
     * Loads an image from a file.
     * 
     * Opens the file and loads the image using the stream-based loader.
     * 
     * @param file
     *      Reference to the file object.
     * 
     * @param allocator
     *      Pointer to the allocator to use (defaults to system allocator).
     * 
     * @return image
     *      The loaded image object.
     * 
     * @throws illegal_state_exception
     *      If image loading fails.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    image load_image(const file& file , tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Writes an image to an output stream.
     * 
     * Encodes and writes the image data to the output stream in the specified format.
     * Supported formats: PNG, JPEG, TGA, BMP.
     * 
     * @param out 
     *      Pointer to the output stream.
     * 
     * @param img
     *      Reference to the image to write.
     * 
     * @param ext
     *      File extension determining the output format ("png", "jpeg", "jpg", "tga", "bmp").
     * 
     * @throws illegal_state_exception
     *      If image writing fails.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    void write_image(ostream* out, const image& img, const char* ext);
    
    /**
     * Writes an image to a file.
     * 
     * Opens the file and writes the image using the stream-based writer.
     * The format is determined by the file extension.
     * 
     * @param file
     *      Reference to the output file object.
     * 
     * @param img
     *      Reference to the image to write.
     * 
     * @param ext
     *      File extension determining the output format ("png", "jpeg", "jpg", "tga", "bmp").
     * 
     * @throws illegal_state_exception
     *      If image writing fails.
     * 
     * @throws io_exception
     *      If an I/O error occurs.
     */
    void write_image(const file& file, const image& img, const char* ext);

}//namespace imageio
}//namespace jstd 

#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H