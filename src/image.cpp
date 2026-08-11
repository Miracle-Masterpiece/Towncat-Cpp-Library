#include <cpp/lang/utils/images/image.hpp>
#include <cpp/lang/exceptions.hpp>
#include <utility>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <internal/img_utils.hpp>

namespace tc {

    image::image() :
    m_allocator(nullptr),
    m_pixels(nullptr),
    m_width(0),
    m_height(0),
    m_channels(0) {

    }
    
    image::image(unsigned char* data, int w, int h, int channels) : 
    m_allocator(nullptr),
    m_pixels(data),
    m_width(w),
    m_height(h),
    m_channels(channels) {

    }

    image::image(unsigned char* data, tca::allocator* allocator, int w, int h, int channels) :
    m_allocator(allocator),
    m_pixels(data), 
    m_width(w), 
    m_height(h),
    m_channels(channels) {

    }

    image::image(int width, int height, int channels, tca::allocator* allocator) {
        unsigned char* pixels = reinterpret_cast<unsigned char*>(allocator->allocate_align((width * height * channels), alignof(unsigned char)));
        if (pixels == nullptr)
            throw_except<out_of_memory_error>("Out of memory!");
        std::memset(pixels, 0, (width * height * channels));
        m_allocator = allocator;
        m_pixels    = pixels;
        m_width     = width;
        m_height    = height;
        m_channels  = channels;
    }
    
    void image::cleanup() {
        if (m_allocator != nullptr && m_pixels != nullptr) {
            m_allocator->deallocate(m_pixels);
            m_pixels    = nullptr;
            m_allocator = nullptr;
        }
    }

    image::~image() {
        cleanup();
    }

    image::image(const image& img) : image() {
        image tmp = img.clone();
        *this = std::move(tmp);
    }

    image& image::operator= (const image& img) {
        if (&img != this) {
            image tmp = img.clone();
            *this = std::move(tmp);
        }
        return *this;
    }

    image::image(image&& img) : 
    m_allocator(img.m_allocator), 
    m_pixels(img.m_pixels), 
    m_width(img.m_width), 
    m_height(img.m_height), 
    m_channels(img.m_channels) {
        img.m_allocator = nullptr;
        img.m_pixels    = nullptr;
        img.m_width     = 0;
        img.m_height    = 0;
        img.m_channels  = 0;
    }

    image& image::operator= (image&& img) {
        if (&img != this) {
            cleanup();
            m_allocator = img.m_allocator;
            m_pixels    = img.m_pixels;
            m_width     = img.m_width;
            m_height    = img.m_height;
            m_channels  = img.m_channels;
            
            img.m_allocator = nullptr;
            img.m_pixels    = nullptr;
            img.m_width     = 0;
            img.m_height    = 0;
            img.m_channels  = 0;
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

    int image::get_channels() const {
        return m_channels;
    }

    image::rgb& image::get_rgb(int x, int y) {
        JSTD_DEBUG_CODE(
            if (m_channels != 3) throw_except<illegal_state_exception>("image is not rgb");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgb[idx];
    }

    const image::rgb& image::get_rgb(int x, int y) const {
        JSTD_DEBUG_CODE(
            if (m_channels != 3) throw_except<illegal_state_exception>("image is not rgb");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgb[idx];
    }

    image::rgba& image::get_rgba(int x, int y) {
        JSTD_DEBUG_CODE(
            if (m_channels != 4) throw_except<illegal_state_exception>("image is not rgba");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgba[idx];
    }

    const image::rgba& image::get_rgba(int x, int y) const {
        JSTD_DEBUG_CODE(
            if (m_channels != 4) throw_except<illegal_state_exception>("image is not rgba");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_rgba[idx];
    }

    image::gray& image::get_gray(int x, int y) {
        JSTD_DEBUG_CODE(
            if (m_channels != 1) throw_except<illegal_state_exception>("image is not gray");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_gray[idx];
    }

    const image::gray& image::get_gray(int x, int y) const {
        JSTD_DEBUG_CODE(
            if (m_channels != 1) throw_except<illegal_state_exception>("image is not gray");
            check_index(x, m_width);
            check_index(y, m_height);
        );
        int idx = x + y * m_width;
        return m_gray[idx];
    }

    int image::rgba::to_string(char buf[], std::size_t bufsize) const {
        using ui = int;
        return snprintf(buf, bufsize, "[R: %u, G: %u, B: %u, A: %u]", (ui) r, (ui) g, (ui) b, (ui) a);
    }

    int image::rgb::to_string(char buf[], std::size_t bufsize) const {
        using ui = int;
        return snprintf(buf, bufsize, "[R: %u, G: %u, B: %u]", (ui) r, (ui) g, (ui) b);
    }

    int image::gray::to_string(char buf[], std::size_t bufsize) const {
        using ui = int;
        return snprintf(buf, bufsize, "[GRAY: %u]", (ui) brightness);
    }

    image image::resize(int neww, int newh, tca::allocator* allocator) const {
        if (neww <= 0 || newh <= 0)
            throw_except<illegal_argument_exception>("Invalid width or height");
        if (allocator == nullptr)
        {
            if (m_allocator == nullptr)
                return image();
            allocator = m_allocator;
        }
        
        image resized_image(neww, newh, m_channels, allocator);
        
        int error;
        if (m_channels == 4)
        {
            error = internal::resize_image_alpha(
                m_pixels, m_width, m_height,
                resized_image.pixels(), neww, newh, 
                m_channels,
                3, /*alpha index*/
                allocator
            );
        }
        else
        {
            error = internal::resize_image(
                m_pixels, m_width, m_height,
                resized_image.pixels(), neww, newh, 
                m_channels, 
                allocator
            );
        }

        if (error != 0)
            throw_except<illegal_state_exception>("resize error: %i", error);

        return image(std::move(resized_image));
    }

    image image::clone(tca::allocator* allocator) const {
        if (allocator == nullptr) {
            if (m_allocator == nullptr)
                return image();
            allocator = m_allocator;
        }
        image img(m_width, m_height, m_channels, allocator);
        std::memcpy(img.m_pixels, m_pixels, 
                                            (m_width * m_height * m_channels));
        return image(std::move(img));
    }

    string image::to_string(tca::allocator* alloc) const {
        string result(alloc);
        
        result
        .append("[w:").append(tc::to_string(m_width, alloc)).append(',')
        .append("h:").append(tc::to_string(m_height, alloc)).append(',')
        .append("c:").append(tc::to_string(m_channels, alloc)).append(']');

        return result;
    }

    /*static*/ image image::make_view(unsigned char* data, int width, int height, int channels) {
        JSTD_DEBUG_CODE(
            check_non_null(data);
        );
        return image(data, width, height, channels);
    }

    /**static */ image image::lock(unsigned char* data, tca::allocator* allocator, int width, int height, int channels) {
        JSTD_DEBUG_CODE(
            check_non_null(data);
            check_non_null(allocator);
        );
        return image(data, allocator, width, height, channels);
    }

}

