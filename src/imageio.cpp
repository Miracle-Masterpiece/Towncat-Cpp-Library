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
 * Определение макросов, чтобы stbimage использовал фукнции распреления памяти из jstd
 */
#define STBI_MALLOC(size) jstd_malloc(size)
#define STBI_REALLOC(pointer, new_size) jstd_realloc(pointer, new_size)
#define STBI_FREE(pointer) jstd_free(pointer)

#define STBIW_FAILURE_USERMSG
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBIW_MALLOC(size) jstd_malloc(size)
#define STBIW_REALLOC(pointer, new_size) jstd_realloc(pointer, new_size)
#define STBIW_FREE(pointer) jstd_free(pointer)

static void* jstd_malloc(std::size_t sz);
static void* jstd_realloc(void* p, std::size_t sz);
static void jstd_free(void* ptr);

#include <cpp/lang/utils/images/stb_image.h>
#include <cpp/lang/utils/images/stb_image_write.h>

static tca::malloc_free_allocator jstd_allocator_for_stb_image;

static void* jstd_malloc(std::size_t sz) {
    return jstd_allocator_for_stb_image.allocate(sz);
}

static void* jstd_realloc(void* p, std::size_t sz) {
    return jstd_allocator_for_stb_image.reallocate(p, sz);
}

static void jstd_free(void* ptr) {
    return jstd_allocator_for_stb_image.deallocate(ptr);
}

namespace tc 
{

namespace imageio 
{

    image load_image(istream* in, tca::allocator* allocator) {        
        std::uintmax_t size = (std::size_t) in->available();

        if (size > INT_MAX)
        {
            throw_except<illegal_state_exception>("Image size to large");
        }

        unique_ptr<stbi_uc[]> raw_image = make_unique_array<stbi_uc>((std::size_t) size, allocator);
        
        in->read((char*) raw_image.get(), size);
    
        int width;
        int heigth;
        int channels;
        
        stbi_uc* pixels = stbi_load_from_memory(raw_image.get(), (int) size, &width, &heigth, &channels, 0);
        if (pixels == nullptr)
        {
            throw_except<illegal_state_exception>("%s", stbi_failure_reason());
        }
            
        return image::lock(pixels, &jstd_allocator_for_stb_image, width, heigth, channels);
    }

    image load_image(const file& file, tca::allocator* allocator) {
        ifstream in(file);
        image img;
        try {
            img = load_image(&in, allocator);
        } catch (...) {
            close_stream_and_suppress_except(&in);    
            throw;
        }
        in.close();
        return image(std::move(img));
    }
    
    static void save_contex(void* context, void* data, int size){
		assert(size >= 0);
        ostream* out = reinterpret_cast<ostream*>(context);
        out->write(reinterpret_cast<const char*>(data), (std::size_t) size);
	}

    void write_image(const file& file, const image* img, const char* ext) {
        ofstream out(file);
        try {
            write_image(&out, img, ext);
        } catch (...) {
            close_stream_and_suppress_except(&out);
            throw;
        }
        out.close();
    }

    void write_image(ostream* out, const image* img, const char* ext) {
        int width    = img->get_width();
        int height   = img->get_height();
        int channels = img->get_channels();
        
        int error = 0;

        if (std::strcmp(ext, "png") == 0)
        {
            error = stbi_write_png_to_func(save_contex, out, width, height, channels, img->pixels(), 0);
        } 
        else if (std::strcmp(ext, "jpeg") == 0 || std::strcmp(ext, "jpg") == 0)
        {
            error = stbi_write_jpg_to_func(save_contex, out, width, height, channels, img->pixels(), 100);
        } 
        else if (std::strcmp(ext, "tga") == 0)
        {
            error = stbi_write_tga_to_func(save_contex, out, width, height, channels, img->pixels());
        }
        else if (std::strcmp(ext, "bmp") == 0)
        {
            error = stbi_write_bmp_to_func(save_contex, out, width, height, channels, img->pixels());
        } 

        if (error == 0)
        {
            throw_except<illegal_state_exception>("stbi_write error: %i", error);
        }
    }

}

}