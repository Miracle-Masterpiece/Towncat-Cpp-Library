#ifndef A6CDFE96_6B88_495C_92FC_56F238D141CA
#define A6CDFE96_6B88_495C_92FC_56F238D141CA

#include <cstddef>

namespace jstd
{

/**
 * Абстрактный базовый класс для декомпрессии данных.
 * 
 * Класс предоставляет интерфейс для последовательной или блочной распаковки данных.
 * Объекты этого класса не могут быть скопированы, но поддерживают семантику перемещения (move).
 */
class decompressor {
private:
    //Копирование запрещено
    decompressor(const decompressor&) = delete;
    decompressor& operator=(const decompressor&) = delete;
protected:
    /**
     * Указатель на буфер с закодированными (входными) данными. 
     */
    const char* input;

    /**
     * Размер закодированных входных данных в байтах. 
     */
    std::size_t input_size;

public:
    /**
     * 
     */
    decompressor();

    /**
     * 
     */
    decompressor(decompressor&&);

    /**
     * 
     */
    decompressor& operator=(decompressor&&);

    /**
     * Устанавливает буфер с входными (закодированными) данными.
     * 
     * @param in
     *      Указатель на начало буфера сжатых данных.
     * 
     * @param length
     *      Размер буфера в байтах.
     */
    virtual void set_input(const char* in, std::size_t length);

    /**
     * Распаковывает данные в предоставленный буфер.
     * Виртуальная функция, которая должна быть переопределен в классах-наследниках
     * для реализации конкретного алгоритма декомпрессии (например, LZMA, GZIP, Deflate).
     * 
     * @param buf
     *      Целевой буфер, куда будут записаны распакованные данные.
     * 
     * @param sz
     *      Максимальный размер целевого буфера (в байтах).
     * 
     * @return
     *      Количество байт, фактически записанных в буфер. 
     *      Возвращает 0, если данные закончились.
     */
    virtual std::size_t decompress(char buf[], std::size_t sz) = 0;

    /**
     * Виртуальный деструктор.
     * Гарантирует корректное удаление объектов производных классов через указатель на базовый класс.
     */
    virtual ~decompressor();
};

} //namespace jstd

namespace tc = jstd;

#endif /* A6CDFE96_6B88_495C_92FC_56F238D141CA */
