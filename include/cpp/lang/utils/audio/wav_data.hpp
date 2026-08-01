#ifndef JSTD_LANG_UTILS_AUDIO_WAV_DATA_H
#define JSTD_LANG_UTILS_AUDIO_WAV_DATA_H

#include <allocators/allocator.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cstdint>

namespace tc
{

class file;

/**
 * Представляет декодированные заголовки и данные WAV-файла.
 * 
 * Этот класс предоставляет низкоуровневый доступ к структурам WAV-формата,
 * включая заголовочные поля RIFF, формат аудио, частоту дискретизации, 
 * количество каналов, битность, а также сами аудиоданные. 
 * 
 * Загрузка производится вручную из входного потока или по пути к файлу.
 * Все выделения памяти производятся через пользовательский аллокатор.
 */
class wav_data {
    tca::allocator* m_allocator;
    int32_t     chunkId;
    uint32_t    chunkSize;
    int32_t     format;
    int32_t     subchunk1Id;
    uint32_t    subchunk1Size;
    int16_t     audioFormat;
    int16_t     numChannels;
    int32_t     sampleRate;
    int32_t     byteRate;
    int16_t     blockAlign;
    int16_t     bitsPerSample;
    int32_t     subchunk2Id;
    uint32_t    subchunk2Size;
    char*       data;

    /**
     * Загружает WAV-данные из указанного входного потока.
     *
     * @param in
     *      Указатель на входной поток (например, файл или буфер), содержащий WAV-данные.
     *      Должен быть валидным и не равным nullptr.
     * 
     * @throws std::runtime_error
     *      Если формат WAV недопустим или при чтении произошла ошибка.
     */
    void load_from(/*!non null!*/istream* in);

    /**
     * Освобождает ресурсы, выделенные при загрузке WAV-данных.
     */
    void cleanup();

public:
    /**
     * Создаёт пустой объект wav_data.
     * 
     * После вызова необходимо вручную вызвать метод load_from()
     * для загрузки данных WAV из потока.
     */
    wav_data();

    /**
     * Загружает WAV-данные из файла.
     *
     * @param path
     *      Путь к .wav-файлу.
     * 
     * @param allocator
     *      Пользовательский аллокатор памяти, который будет использоваться для хранения аудиоданных.
     *
     * @warning
     *      Если при загрузки аудио возникло исключение, а после при закрытии потока, то исключение закрытия потока будет проигнорировано!
     * 
     * @throws file_not_found_exception
     *      Если файла не существует.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     * 
     * @throws eof_exception
     *      Если данные неожиданно закончились.
     * 
     * @throws invalid_data_format_exception
     *      Если формат WAV недопустим
     * 
     * @throws out_of_memory_error
     *      Если памяти не хватило
     */
    explicit wav_data(const file& file, tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     * Загружает WAV-данные из указанного входного потока.
     * 
     * @param in
     *      Указатель на поток ввода, содержащий WAV-данные.
     *      Должен быть валидным и не равным nullptr.
     * 
     * @param allocator
     *      Пользовательский аллокатор памяти, используемый для хранения аудиоданных.
     * 
     * @throws io_exception
     *      Если произошла ошибка ввода/вывода.
     * 
     * @throws eof_exception
     *      Если данные неожиданно закончились.
     * 
     * @throws invalid_data_format_exception
     *      Если формат WAV недопустим
     * 
     * @throws out_of_memory_error
     *      Если памяти не хватило
     */
    wav_data(istream* in, tca::allocator* allocator = tca::get_scoped_or_default());

    /**
     * Освобождает все ресурсы, связанные с WAV-данными.
     */
    ~wav_data();

    /**
     * @return
     *      Указатель на необработанные PCM-данные WAV-файла.
     */
    const char* get_data() const;

    /**
     * @return
     *      Формат WAV-аудио.
     */
    int32_t get_format() const;

    /**
     * @return
     *      Частота дискретизации (в герцах).
     */
    int32_t get_sample_rate() const;

    /**
     * @return
     *      Количество байт, передаваемых в секунду.
     */
    int32_t get_byte_rate() const;

    /**
     * @return
     *      Длина аудиоданных в байтах.
     */
    uint32_t get_length() const;

    /**
     * @return
     *      Количество звуковых каналов (1 — моно, 2 — стерео и т.д.).
     */
    int16_t get_num_channels() const;

    /**
     * @return
     *      Количество бит на один сэмпл.
     */
    int16_t get_bits_per_sample() const;
};


}

#endif//JSTD_LANG_UTILS_AUDIO_WAV_DATA_H