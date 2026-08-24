#include <allocators/linear_allocator.hpp>
#include <cpp/lang/utils/images/image_packer.hpp>
#include <cpp/lang/utils/images/image_tree.hpp>
#include <cpp/lang/utils/images/image.hpp>
#include <algorithm>

namespace tc {
    using namespace texturing;

    image_packer::image_packer() : 
    m_allocator(nullptr), 
    m_root(), 
    m_images(),
    m_width(0),
    m_height(0) {

    }

    image_packer::image_packer(const image* img_array, std::size_t count_images, int w, int h, tca::allocator* allocator) :
    m_allocator(allocator),
    m_root(),
    m_images(img_array),
    m_cnt_imgs(count_images),
    m_width(w),
    m_height(h) {
        
    }

    image_packer::image_packer(image_packer&& packer) : 
    m_allocator(packer.m_allocator),
    m_root(std::move(packer.m_root)),
    m_images(packer.m_images),
    m_width(packer.m_width),
    m_height(packer.m_height) {
        packer.m_images         = nullptr;
        packer.m_cnt_imgs       = 0;
        packer.m_width          = 0;
        packer.m_height         = 0;
    }
    
    image_packer& image_packer::operator=(image_packer&& packer) {
        if (&packer != this) 
        {
            m_allocator         = packer.m_allocator;
            m_root              = std::move(packer.m_root);
            m_images            = packer.m_images;
            m_width             = packer.m_width;
            m_height            = packer.m_height;

            packer.m_images = nullptr;
            packer.m_width  = 0;
            packer.m_height = 0;
        }
        return *this;
    }

    image_packer::~image_packer() {

    }

    std::size_t get_max_mem_size_for_max_image(const image* images, std::size_t len) {
        std::size_t mem_size    = 0;
        for (std::size_t i = 0; i < len; ++i)
        {
            const image& img = images[i];
            mem_size = std::max<std::size_t>(mem_size, (std::size_t) (img.get_width() * img.get_height() * img.get_channels()));
        }
        return mem_size;
    }

    void image_packer::create_tree() {
        if (!m_root)   
        
        m_root = polymorph::allocate_unique<node>(m_allocator, node(m_width, m_height, m_allocator));

        BEGIN: 
        for (std::size_t i = 0; i < m_cnt_imgs; ++i)
        {
            const image& img = m_images[i];
            node* n = m_root->put_image(img.get_width(), img.get_height(), i);
            if (!n)
            {
                /**
                 * Если не получилось вставить изображение, начинаем заного, 
                 * но увеличиваем конечное изображение в два раза.
                 */
                m_width  <<= 1;
                m_height <<= 1;
                *m_root = node(m_width, m_height, m_allocator);
                goto BEGIN;
            }
        }
    }

    image image_packer::pack(int scale_factor, int out_image_channels) {
        JSTD_DEBUG_CODE (
            if (scale_factor <= 0)           throw_except<illegal_argument_exception>("scale_factor %li is illegal", (long int) scale_factor);
            if (m_allocator == nullptr)      throw_except<illegal_state_exception>("m_allocator must be != null");
            if (out_image_channels <= 0 || out_image_channels > 4)
                throw_except<illegal_argument_exception>("out_image_channels %li is illegal", (long int) scale_factor);
        );
            
        if (!m_root) create_tree();
        image result(m_root->get_rect().w / scale_factor, m_root->get_rect().h / scale_factor, out_image_channels, m_allocator);

        /**
         * Размер временного буфера, куда будет сохранено масштабированное изображение.
         * Буфер делится на rescale, так как если изначальное изображение весит килобайт, то масштабированный на 2, размер будет в два раза меньше и так далее.
         */
        std::size_t buffer_size_for_resized_image = get_max_mem_size_for_max_image(m_images, m_cnt_imgs) / scale_factor;

        /**
         * Линейный аллокатор, который нужен, чтобы выделять память под хранения временного масштабированного изображения
         */
        tca::linear_allocator allocator_for_rescaled_image; 
        
        //Если делитель больше 1, то масштабирование изображения требуется, а значит требуется и временный буфер.
        if (scale_factor > 1)
            allocator_for_rescaled_image = tca::linear_allocator(buffer_size_for_resized_image, m_allocator);

        struct callback_image {
            /**
             * Массив оригинальных изображений.
             */
            const image* m_array_of_image;
            
            /**
             * Указатель на результирующее изображение (атлас).
             * В который будет записано изображение.
             */
            image* m_atlas;
            
            /**
             * Делитель для уменьшения изображения.
             * Для корректных результатов должен быть кратен степени двойки. 2 4 8 16 32....1024
             */
            int m_rescale;
            
            /**
             * Аллокатор для выделения памяти под уменьшенное изображение.
             * Поскольку void operator() (node*) вызывается каждый раз для каждого изображения, то внутри него предвыделить буфер невозможно.
             * 
             */
            tca::linear_allocator*  m_allocator_for_tmp_resized_image;

            //Я даже описывать не хочу, какого чёрта тут происходит. Stupid Fuck.
            void operator()(const node* n) {
                std::size_t image_index = n->get_id();

                const image& img_original = m_array_of_image[image_index];
                int w_image = img_original.get_width();
                int h_image = img_original.get_height();

                image rescaled_image(m_allocator_for_tmp_resized_image); 
                if (m_rescale > 1 && m_allocator_for_tmp_resized_image != nullptr)
                    rescaled_image = img_original.resize(w_image / m_rescale, h_image / m_rescale, m_allocator_for_tmp_resized_image);

                const image& img = (m_rescale > 1) ? rescaled_image : img_original;

                /**
                 * Нода имеет координату, ширину и величину изображения.
                 * Причём ширина и высота всегда идентична изображению!
                 */
                const int x = n->get_rect().x     / m_rescale;
                const int y = n->get_rect().y     / m_rescale;
                const int w = n->get_rect().w     / m_rescale;
                const int h = n->get_rect().h     / m_rescale;
                
                // xo & yo - это позиции resized изображении.
                for (int xo = 0; xo < w; ++xo)
                {
                    for (int yo = 0; yo < h; ++yo)
                    {
                        // xx & yy - это позиция в атласе
                        int xx = x + xo;
                        int yy = y + yo;
                        image::pixel from_resized = img.get_pixel(xo, yo);
                        m_atlas->set_pixel(xx, yy, from_resized);
                    }
                }

                if (m_allocator_for_tmp_resized_image != nullptr)
                    m_allocator_for_tmp_resized_image->reset();
            }
        };

        callback_image callback;
        callback.m_array_of_image   = m_images;
        callback.m_atlas            = &result;
        callback.m_rescale          = scale_factor;
        callback.m_allocator_for_tmp_resized_image = scale_factor > 1 ? &allocator_for_rescaled_image : nullptr;

        m_root->depth_search(callback);

        return image(std::move(result));
    }

    array<image_packer::uv> image_packer::get_uv() const {
        array<image_packer::uv> uvs(m_cnt_imgs, m_allocator);
        
        struct node_visitor {
            array<image_packer::uv>* m_array;
            void operator() (const node* n) {
                const rect& pos = n->get_rect();
                int u0 = pos.x;
                int v0 = pos.y;
                int u1 = pos.x + pos.w;
                int v1 = pos.y + pos.h;
                image_packer::uv texcoord;
                texcoord.u0 = u0;
                texcoord.v0 = v0;
                texcoord.u1 = u1;
                texcoord.v1 = v1;
                (*m_array)[n->get_id()] = texcoord;
            }
        };

        node_visitor visitor;
        visitor.m_array = &uvs;
        
        m_root->depth_search(visitor);

        return array<image_packer::uv>(std::move(uvs));
    }
}

#if 0

#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/file.hpp>
#include <cpp/lang/string.hpp>
#include <cpp/lang/io/utility.hpp>
#include <cpp/lang/utils/images/imageio.hpp>
#include <cpp/lang/utils/arrays.hpp>
#include <cpp/lang/utils/utils.hpp>

class png_filter : public tc::file_filter {
public:
    bool apply(const char* path, std::size_t path_length) const override {
        return tc::string(path).ends_with(".png");   
    }
};


tc::array<tc::image> read_all_images(const tc::file& path) {
    tc::array<tc::file> files;
    {
        png_filter filter;
        files = path.list_files(filter);
    }

    tc::array<tc::image> imgs(files.length);
    for (std::size_t i = 0; i < imgs.length; ++i)
    {
        imgs[i] = tc::imageio::load_image(files[i]);
    }

    return imgs;
}

namespace jstd
{
    template<>
    struct compare_to<image> {
        int operator() (const image& a, const image& b) const {
            return (b.get_width() * b.get_height()) - (a.get_width() * a.get_height());
        }
    };
}

int main(int argc, const char** args) {
    try {
        tc::file root = "./images/";    
        tc::array<tc::image> images = read_all_images(root);
        printf("len: %zu\n", images.length);
        {
            tc::utils::quick_sort(images.data(), images.length);
        }
        {
            tc::image_packer packer(images.data(), images.length, 1024, 1024);
            tc::image atlas = packer.pack(argc > 1 ? (*args[1]) - '0' : 1);
            tc::imageio::write_image(tc::file("./img.png"), &atlas, "png");
        }
    } catch( const tc::throwable& t) {
        std::printf("except: %s\n", t.cause());
    }
    
}

#endif