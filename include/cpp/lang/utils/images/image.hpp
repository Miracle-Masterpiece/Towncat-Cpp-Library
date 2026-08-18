#ifndef JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H
#define JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H

#include <cstdint>
#include <allocators/allocator.hpp>
#include <cpp/lang/string.hpp>

namespace tc
{

/**
 * Класс image представляет собой двумерное изображение в формате массива байт (пикселей),
 * с указанием ширины, высоты и количества цветовых каналов.
 * Поддерживает клонирование, ресайз и управление памятью через пользовательский аллокатор.
 */
class image {
public:
    struct pixel {
        unsigned char red, green, blue, alpha;
    };
private:
    /**
     * Аллокатор, используемый для управления памятью.
     */
    tca::allocator* const m_allocator;

    /**
     * Указатель на данные пикселей.
     */
    unsigned char* m_pixels;
    
    /**
     * Ширина изображения в пикселях.
     */
    int m_width;
    
    /**
     * Высота изображения в пикселях.
     */
    int m_height;
    
    /**
     * Количество цветовых каналов (например, 3 = RGB, 4 = RGBA).
     */
    int m_channels;

    /**
     * Для создания view на массив байтов.
     */
    image(unsigned char* data, int w, int h, int channels);
    
    /**
     * Для захвата владения над массивом байтов.
     */
    image(unsigned char* data, tca::allocator* allocator, int w, int h, int channels);

public:
    /**
     * Конструктор по умолчанию.
     * Создаёт пустое изображение без выделения памяти.
     */
    image(tca::allocator* alloc = tca::get_default_allocator());

    /**
     * Создаёт изображение заданного размера и количества каналов с использованием указанного аллокатора.
     *
     * @param allocator 
     *      Аллокатор для выделения памяти.
     * 
     * @param width 
     *      Ширина изображения.
     * 
     * @param height 
     *      Высота изображения.
     * 
     * @param channels 
     *      Количество каналов (например, 3 = RGB, 4 = RGBA).
     */
    image(int width, int height, int channels, tca::allocator* allocator = tca::get_default_allocator());

    /**
     * Конструктор копирования.
     *
     * @param other 
     *      Другое изображение для копирования.
     */
    image(const image& other);

    /**
     * Конструктор перемещения.
     *
     * @param other 
     *      Другое изображение, ресурсы которого будут переняты.
     */
    image(image&& other);

    /**
     * Оператор копирующего присваивания.
     *
     * @param other 
     *      Другое изображение.
     * 
     * @return 
     *  Ссылка на это изображение.
     */
    image& operator=(const image& other);

    /**
     * Оператор перемещающего присваивания.
     *
     * @param other 
     *      Другое изображение.
     * 
     * @return 
     *      Ссылка на это изображение.
     */
    image& operator=(image&& other);

    /**
     * Деструктор. 
     * Освобождает выделенную память.
     */
    ~image();

    /**
     * Возвращает указатель на пиксели изображения.
     *
     * @return 
     *      Указатель на первый байт массива пикселей.
     */
    unsigned char* pixels();

    /**
     * 
     */
    pixel get_pixel(int x, int y) const;
    
    /**
     * 
     */
    void set_pixel(int x, int y, const pixel& p);

    /**
     * Возвращает указатель на пиксели изображения (константная версия).
     *
     * @return 
     *      Указатель на первый байт массива пикселей (только для чтения).
     */
    const unsigned char* pixels() const;

    /**
     * Возвращает аллокатор, управляющий памятью этого изображения.
     */
    tca::allocator* get_allocator() const {
        return m_allocator;
    }

    /**
     * Возвращает ширину изображения.
     *
     * @return 
     *      Ширина в пикселях.
     */
    int get_width() const;

    /**
     * Возвращает высоту изображения.
     *
     * @return 
     *      Высота в пикселях.
     */
    int get_height() const;

    /**
     * Возвращает количество цветовых каналов.
     *
     * @return 
     *      Количество каналов (например, 3 или 4).
     */
    int get_channels() const;

    /**
     * Возвращает новое изображение, масштабированное до заданных размеров.
     *
     * @note
     *      Если передаваемый аллокатор равен nullptr и текущее изображение является view, то функция вернёт пустое изображение.
     * 
     * @param neww 
     *      Новая ширина.
     * 
     * @param newh 
     *      Новая высота.
     * 
     * @param alloc
     *      Аллокатор, используемый для выделения памяти под уменьшенное изображение.
     * 
     * @return 
     *      Новое масштабированное изображение.
     */
    image resize(int neww, int newh, tca::allocator* alloc = nullptr) const;

    /**
     * Возвращает краткое строковое представление изображения.
     */
    string to_string(tca::allocator* = tca::get_default_allocator()) const;
    
    /**
     * Захватывает указатель на массив изображения в своё владение. 
     * 
     * @note
     *      После вызова этой функции, изображение становится владельцем указателя.
     *      Указатель будет освобождён в момент вызова декструктора изображения.
     * 
     * @param data
     *      Указатель на массив байтов изображения.
     * 
     * @param allocator
     *      Указатель на аллокатор, которым был выделен блок памяти {@param data}
     * 
     * @param width
     *      Ширина изображения.
     * 
     * @param height
     *      Высота изображения.
     * 
     * @param channles
     *      Количество каналов изображения.
     * 
     * @throw null_pointer_exception
     *      Если data равна nullptr.
     *      Если allocator равен nullptr.
     */
    static image lock(unsigned char* data, tca::allocator* allocator, int width, int height, int channels);
};
    

}

#endif//JSTD_CPP_LANG_UTILS_IMAGES_IMAGE_H