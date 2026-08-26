#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <allocators/malloc_free_allocator.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/ofstream.hpp>
#include <cpp/lang/io/iostream.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG

/**
 * Macro overriding STB image's malloc with custom allocator.
 */
#define STBI_MALLOC(size) tc_malloc(size)

/**
 * Macro overriding STB image's realloc with custom allocator.
 */
#define STBI_REALLOC(pointer, new_size) tc_realloc(pointer, new_size)

/**
 * Macro overriding STB image's free with custom allocator.
 */
#define STBI_FREE(pointer) tc_free(pointer)

/**
 * Enables detailed error messages from STB image write library.
 */
#define STBIW_FAILURE_USERMSG

/**
 * Instructs STB image write library to include implementation.
 */
#define STB_IMAGE_WRITE_IMPLEMENTATION

/**
 * Macro overriding STB image write's malloc with custom allocator.
 */
#define STBIW_MALLOC(size) tc_malloc(size)

/**
 * Macro overriding STB image write's realloc with custom allocator.
 */
#define STBIW_REALLOC(pointer, new_size) tc_realloc(pointer, new_size)

/**
 * Macro overriding STB image write's free with custom allocator.
 */
#define STBIW_FREE(pointer) tc_free(pointer)

static void* tc_malloc(std::size_t sz);
static void* tc_realloc(void* p, std::size_t sz);
static void  tc_free(void* ptr);

#include <cpp/lang/utils/images/stb_image.h>
#include <cpp/lang/utils/images/stb_image_write.h>

/**
 * Returns the singleton allocator instance used by STB libraries.
 * 
 * @return
 *      Pointer to the static allocator.
 */
static tca::malloc_free_allocator* get_allocator_for_stb() {
    static tca::malloc_free_allocator alloc;
    return &alloc;
}

/**
 * Allocates memory using the STB allocator.
 * 
 * @param sz
 *      Size in bytes to allocate.
 * 
 * @return void*
 *      Pointer to allocated memory.
 */
static void* tc_malloc(std::size_t sz) {
    return get_allocator_for_stb()->allocate(sz);
}

/**
 * Reallocates memory using the STB allocator.
 * 
 * @param p
 *      Pointer to previously allocated memory.
 * 
 * @param sz
 *      New size in bytes.
 * 
 * @return
 *      void* pointer to reallocated memory.
 */
static void* tc_realloc(void* p, std::size_t sz) {
    return get_allocator_for_stb()->reallocate(p, sz);
}

/**
 * Deallocates memory using the STB allocator.
 * 
 * @param ptr
 *      Pointer to memory to free.
 */
static void tc_free(void* ptr) {
    return get_allocator_for_stb()->deallocate(ptr);
}

namespace tc 
{

namespace imageio 
{

namespace read_callbacks
{

/**
 * Wrapper structure for input stream with EOF tracking.
 */
struct stream_wrapper {
    istream* m_in;  //input stream for load image
    bool     m_eof; //end of file flag  
};

    /**
     * Reads data from the stream.
     * 
     * STB image callback that reads a specified number of bytes from the stream.
     * 
     * @param user
     *      Pointer to stream_wrapper instance.
     * 
     * @param data
     *      Output buffer for read data.
     * 
     * @param size
     *      Number of bytes to read.
     * 
     * @return
     *      Number of bytes actually read.
     */
    int read_callback(void* user, char* data, int size) {
        stream_wrapper* in = static_cast<stream_wrapper*>(user);
        int readed = static_cast<int>(
                                        in->m_in->read(data, static_cast<std::size_t>(size) 
                                    )
        );
        if (readed < size)
        {
            in->m_eof = true;
        }    
        return readed;
    }

    /**
     * Skips data in the stream.
     * 
     * STB image callback that skips a specified number of bytes in the stream.
     * 
     * @param user
     *      Pointer to stream_wrapper instance.
     * 
     * @param n
     *      Number of bytes to skip.
     */
    void skip_callback(void *user, int n) {
        stream_wrapper* in = static_cast<stream_wrapper*>(user);
        int skipped = static_cast<int>(in->m_in->skip(static_cast<std::size_t>(n)));
        if (skipped < n)
        {
            in->m_eof = true;
        }
    }

    /**
     * Checks if the stream has reached end-of-file.
     * 
     * STB image callback that returns EOF status.
     * 
     * @param user
     *      Pointer to stream_wrapper instance.
     * 
     * @return
     *      1 if EOF reached, 0 otherwise.
     */
    int eof_callback(void *user) {
        stream_wrapper* in = static_cast<stream_wrapper*>(user);
        return in->m_eof ? 1 : 0;
    }
}

    image load_image(istream* in, tca::allocator* allocator) {        
        
        int width;
        int heigth;
        int channels;
        
        read_callbacks::stream_wrapper wrapper = {in, false};

        stbi_io_callbacks callbacks = {
            read_callbacks::read_callback,
            read_callbacks::skip_callback,
            read_callbacks::eof_callback,
        };

        stbi_uc* pixels = stbi_load_from_callbacks(&callbacks, static_cast<void*>(&wrapper), &width, &heigth, &channels, 0);
        if (!pixels)
        {
            throw_except<illegal_state_exception>("%s", stbi_failure_reason());
        }
            
        return image::lock(pixels, get_allocator_for_stb(), width, heigth, channels);

    }

    image load_image(const file& file, tca::allocator* allocator) {
        ifstream in(file);
        image img;
        try {
            img = load_image(&in, allocator);
        } catch (...) {
            throw;
        }
        return image(std::move(img));
    }
    
    /**
     * Write callback for STB image library.
     * 
     * Callback function that writes image data to an output stream.
     * 
     * @param context
     *      Pointer to the output stream (cast from void*).
     * 
     * @param data
     *      Pointer to the data to write.
     * 
     * @param size
     *      Number of bytes to write.
     */
    static void save_context(void* context, void* data, int size){
		assert(size >= 0);
        ostream* out = reinterpret_cast<ostream*>(context);
        out->write(reinterpret_cast<const char*>(data), (std::size_t) size);
	}

    void write_image(const file& file, const image& img, const char* ext) {
        ofstream out(file);
        try {
            write_image(&out, img, ext);
        } catch (...) {
            throw;
        }
    }

    void write_image(ostream* out, const image& img, const char* ext) {
        int width    = img.get_width();
        int height   = img.get_height();
        int channels = img.get_channels();
        
        int error = 0;

        if (std::strcmp(ext, "png") == 0)
        {
            error = stbi_write_png_to_func(save_context, out, width, height, channels, img.pixels(), 0);
        } 
        else if (std::strcmp(ext, "jpeg") == 0 || std::strcmp(ext, "jpg") == 0)
        {
            error = stbi_write_jpg_to_func(save_context, out, width, height, channels, img.pixels(), 100);
        } 
        else if (std::strcmp(ext, "tga") == 0)
        {
            error = stbi_write_tga_to_func(save_context, out, width, height, channels, img.pixels());
        }
        else if (std::strcmp(ext, "bmp") == 0)
        {
            error = stbi_write_bmp_to_func(save_context, out, width, height, channels, img.pixels());
        } 

        if (error == 0)
        {
            throw_except<illegal_state_exception>("stbi_write error: %i", error);
        }
    }

}

}