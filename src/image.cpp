#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/exceptions.hpp>
#include <utility>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <internal/img_utils.hpp>

namespace tc {

    image::image(tca::allocator* alloc) :
    m_allocator(alloc),
    m_pixels(nullptr),
    m_width(0),
    m_height(0),
    m_channels(0) {

    }
    
    image::image(unsigned char* data, tca::allocator* allocator, int w, int h, int channels) :
    m_allocator(allocator),
    m_pixels(data), 
    m_width(w), 
    m_height(h),
    m_channels(channels) {

    }

    image::image(int width, int height, int channels, tca::allocator* allocator) : image(allocator) {
        unsigned char* pixels = reinterpret_cast<unsigned char*>(allocator->allocate_align((width * height * channels), alignof(unsigned char)));
        if (!pixels)
            throw_except<out_of_memory_error>("out of memory");
        std::memset(pixels, 0, (width * height * channels));
        m_pixels    = pixels;
        m_width     = width;
        m_height    = height;
        m_channels  = channels;
    }
    
    image::~image() {
        if (m_pixels)
        {
            m_allocator->deallocate(m_pixels);
        }
    }

    image::image(const image& img) : image(img.m_allocator) {
        if (img.m_pixels)
        {
            std::size_t len = static_cast<std::size_t>(img.m_width * img.m_height * img.m_channels);
            
            unsigned char* pixels = (unsigned char*) m_allocator->allocate_align(len, alignof(char));
            if (!pixels)
                throw_except<out_of_memory_error>("out of memory");
            std::memcpy(pixels, img.m_pixels, len);
            
            m_pixels    = pixels;
            m_width     = img.m_width;
            m_height    = img.m_height;
            m_channels  = img.m_channels;
        }
    }

    image& image::operator= (const image& img) {
        if (&img != this) {
            unsigned char* pixels = nullptr;
            
            if (img.m_pixels)
            {
                std::size_t len = static_cast<std::size_t>(img.m_width * img.m_height * img.m_channels);
                pixels = (unsigned char*) m_allocator->allocate_align(len, alignof(char));
                if (!pixels)
                    throw_except<out_of_memory_error>("out of memory");
                std::memcpy(pixels, img.m_pixels, len);
            }

            if (m_pixels)
                m_allocator->deallocate(m_pixels);

            m_pixels    = pixels;
            m_width     = img.m_width;
            m_height    = img.m_height;
            m_channels  = img.m_channels;
        }
        return *this;
    }

    image::image(image&& img) : 
    m_allocator(img.m_allocator), 
    m_pixels(img.m_pixels), 
    m_width(img.m_width), 
    m_height(img.m_height), 
    m_channels(img.m_channels) {
        img.m_pixels    = nullptr;
        img.m_width     = 0;
        img.m_height    = 0;
        img.m_channels  = 0;
    }

    image& image::operator= (image&& img) {
        if (&img != this) {
            if (get_allocator() == img.get_allocator())
            {
                std::swap(m_pixels,     img.m_pixels);
                std::swap(m_width,      img.m_width);
                std::swap(m_height,     img.m_height);
                std::swap(m_channels,   img.m_channels);
            }
            else
            {
                *this = img;
            }
        }
        return *this;
    }
    
    int image::get_width() const {
        return m_width;
    }

    int image::get_height() const {
        return m_height;
    }

    unsigned char* image::pixels() {
        return m_pixels;
    }
    
    const unsigned char* image::pixels() const {
        return m_pixels;
    }

    image::pixel image::get_pixel(int x, int y) const {
        JSTD_DEBUG_CODE(
            if (x < 0 || x >= m_width) throw_except<index_out_of_bound_exception>("x %i >= width %i", x, m_width);
            if (y < 0 || y >= m_height) throw_except<index_out_of_bound_exception>("y %i >= width %i", y, m_height);
        );
        
        int idx = (x + y * m_width) * m_channels;

        if (m_channels == 4)
        {
            unsigned char r = m_pixels[idx + 0];
            unsigned char g = m_pixels[idx + 1];
            unsigned char b = m_pixels[idx + 2];
            unsigned char a = m_pixels[idx + 3];
            return {r, g, b, a};
        }
        else if (m_channels == 3)
        {
            unsigned char r = m_pixels[idx + 0];
            unsigned char g = m_pixels[idx + 1];
            unsigned char b = m_pixels[idx + 2];
            return {r, g, b, 255};
        }
        else if (m_channels == 1)
        {
            unsigned char g = m_pixels[idx];
            return {g, g, g, 255};
        }
        else
        {
            throw make_except<illegal_state_exception>("%i channels not support", m_channels);
        }
    }
    
    /**
     * 
     */
    void image::set_pixel(int x, int y, const pixel& p) {
        JSTD_DEBUG_CODE(
            if (x < 0 || x >= m_width) throw_except<index_out_of_bound_exception>("x %i >= width %i", x, m_width);
            if (y < 0 || y >= m_height) throw_except<index_out_of_bound_exception>("y %i >= width %i", y, m_height);
        );
        int idx = (x + y * m_width) * m_channels;

        if (m_channels == 4)
        {
            m_pixels[idx + 0] = p.red;
            m_pixels[idx + 1] = p.green;
            m_pixels[idx + 2] = p.blue;
            m_pixels[idx + 3] = p.alpha;
        }
        else if (m_channels == 3)
        {
            m_pixels[idx + 0] = p.red;
            m_pixels[idx + 1] = p.green;
            m_pixels[idx + 2] = p.blue;
        }
        else if (m_channels == 1)
        {
            
            float red   = static_cast<float>(p.red);
            float green = static_cast<float>(p.green);
            float blue  = static_cast<float>(p.blue);
            
            // 0.299 * R + 0.587 * G + 0.114 * B
            float grayscale = 0.299f * red + 0.587f * green + 0.114f * blue;

            m_pixels[idx] = static_cast<unsigned char>(grayscale);
        }
        else
        {
            throw make_except<illegal_state_exception>("%i channels not support", m_channels);
        }

    }

    int image::get_channels() const {
        return m_channels;
    }

    image image::resize(int neww, int newh, tca::allocator* alloc) const {
        if (neww <= 0 || newh <= 0)
            throw_except<illegal_argument_exception>("invalid width or height");

        if (!alloc)
            alloc = m_allocator;

        image resized_image(neww, newh, m_channels, alloc);
        
        int error;
        if (m_channels == 4)
        {
            error = internal::resize_image_alpha(
                m_pixels, m_width, m_height,
                resized_image.pixels(), neww, newh, 
                m_channels,
                3, /*alpha index*/
                m_allocator
            );
        }
        else
        {
            error = internal::resize_image(
                m_pixels, m_width, m_height,
                resized_image.pixels(), neww, newh, 
                m_channels, 
                m_allocator
            );
        }

        if (error != 0)
            throw_except<illegal_state_exception>("resize error: %i", error);

        return resized_image;
    }

    string image::to_string(tca::allocator* alloc) const {
        string result(alloc);
        
        result
        .append("[w:").append(tc::to_string(m_width, alloc)).append(',')
        .append("h:").append(tc::to_string(m_height, alloc)).append(',')
        .append("c:").append(tc::to_string(m_channels, alloc)).append(']');

        return result;
    }

    /**static */ image image::lock(unsigned char* data, tca::allocator* allocator, int width, int height, int channels) {
        JSTD_DEBUG_CODE(
            check_non_null(data);
            check_non_null(allocator);
        );
        return image(data, allocator, width, height, channels);
    }
}

