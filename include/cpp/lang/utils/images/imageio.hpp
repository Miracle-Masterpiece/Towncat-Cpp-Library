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
     * Загружает изображение из входящего потока.
     * 
     * @param in
     *      Входящий поток ввода.
     * 
     * @param allocator
     *      Распределитель памяти для выделения памяти под изображения и его загрузку.
     * 
     * @throws out_of_memory_error
     *      Если памяти для временного буфера не хватило.
     *      Если прямяти для конечного изображения не хватило.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     * 
     * @throws illegal_state_exception
     *      Если произошла ошибка во внутренней библитеки.
     */
    image load_image(istream* in, tca::allocator* allocator);

    /**
     * Читает изображение из переданного файла.
     * 
     * @param file
     *      Файл, являющийся изображением.
     * 
     * @param allocator
     *      Распределитель памяти для выделения памяти под изображения и его загрузку.
     * 
     * @throws file_not_found_exception
     *      Если файла не существует.
     * 
     * Остальные выбрасываемые исключения, такие-же, как и у функции {@code load_image(istream*, tca::allocator*)}.
     */
    image load_image(const file& file , tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     * Сохраняет изображение в поток вывода.
     * 
     * @param out
     *      Поток вывода поток ввода.
     * 
     * @param img
     *      Указатель на изображение, которое нужно сохранить.
     * 
     * @param ext
     *      Указатель на строку, отвечаюшую за тип сохранённого изображения.
     *      Например:
     *      "png", "jpeg" or "jpg", "bmp", "tga"
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода
     * 
     * @throws illegal_state_exception
     *      Если произошла ошибка во внутренней библитеки.
     */
    void write_image(ostream* out, const image* img, const char* ext);
    
    /**
     * Тот же самый, метод, что и {
     * @code void save_image(ostream*, tca::allocator*)
     * }
     * 
     * Только вместо потока, файл в который сохранить изображение.
     * Выбрасываемые исключения, такие-же, как и у функции {@code void write_image(ostream*, const image*, const char*)}.
     */
    void write_image(const file& file, const image* img, const char* ext);

    

}//namespace imageio
}//namespace jstd 
namespace tc = tc;

#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGEIO_H