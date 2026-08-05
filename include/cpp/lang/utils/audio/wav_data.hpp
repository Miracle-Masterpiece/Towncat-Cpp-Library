#ifndef JSTD_LANG_UTILS_AUDIO_WAV_DATA_H
#define JSTD_LANG_UTILS_AUDIO_WAV_DATA_H

#include <allocators/allocator.hpp>
#include <cpp/lang/io/istream.hpp>
#include <cstdint>

namespace tc
{

class file;

class wav_data {
    
    // Allocator used for data storage.
    tca::allocator* m_allocator;
    
    // Raw audio data
    char* data;
    
    // Size of the entire file in bytes (minus 8)
    long chunk_size;
    
    // Size of the "fmt " chunk.
    long subchunk1Size;
    
    // Sample rate in Hz (e.g., 44100).
    long sampleRate;
    
    // 
    long byteRate;
    
    // Size of the audio data in bytes.
    long subchunk2Size;
    
    // 
    short block_align;
    
    // Audio format (1 = PCM, others = compressed).
    short audioFormat;
    
    // Number of channels (1 = mono, 2 = stereo).
    short numChannels;
    
    // Bits per sample (8, 16, 24, etc.)
    short bits_per_sample;
    
    // "RIFF"
    char chunk_id[4];
    
    // "WAVE"
    char format[4];

    // "fmt "
    char subchunk1Id[4];
    
    // "data"
    char subchunk2Id[4];

    /**
     * Loads WAV data from the specified input stream.
     * 
     * Reads and validates the WAV header, then reads the audio data
     * into memory. All header fields are populated from the stream.
     * 
     * @param in
     *      Pointer to the input stream containing WAV data.
     *      Must be valid and not nullptr.
     * 
     * @throws invalid_data_format_exception
     *      If the WAV format is invalid (wrong chunk IDs, incorrect sizes).
     * 
     * @throws out_of_memory_error
     *      If memory allocation for audio data fails.
     * 
     * @throws io_exception
     *      If reading from the stream fails.
     * 
     * @note
     *      The function validates "RIFF", "WAVE", "fmt ", and "data" chunks.
     * 
     * @see
     *      fill_buf_or_except
     */
    void load_from(/*!non null!*/istream* in);

public:
    /**
     * Constructs an empty wav_data object.
     * 
     */
    wav_data();

    /**
     * Loads WAV data from a file.
     * 
     * Opens and reads a WAV file, parsing the header and loading
     * the audio data into memory.
     * 
     * @param path
     *      Path to the .wav file to load.
     * 
     * @param allocator
     *      Custom memory allocator for storing audio data.
     *      If nullptr, the default allocator is used.
     * 
     * @throws file_not_found_exception
     *      If the file does not exist.
     * 
     * @throws io_exception
     *      If an I/O error occurs while reading the file.
     * 
     * @throws invalid_data_format_exception
     *      If the WAV format is invalid.
     * 
     * @throws out_of_memory_error
     *      If memory allocation for audio data fails.
     * 
     * @warning
     *      If an exception is thrown during loading, the file stream
     *      is properly closed and any exceptions during closing are
     *      suppressed to avoid masking the original error.
     * 
     * @example
     *      wav_data wav("audio.wav");
     *      const char* audio_data = wav.get_data();
     */
    explicit wav_data(const file& file, tca::allocator* allocator = tca::get_default_allocator());
    
    /**
     * Loads WAV data from an input stream.
     * 
     * Reads WAV data from a pre-opened input stream. This is useful
     * for loading from memory buffers or network streams.
     * 
     * @param in
     *      Pointer to the input stream containing WAV data.
     *      Must be valid and not nullptr.
     * 
     * @param allocator
     *      Custom memory allocator for storing audio data.
     *      If nullptr, the default allocator is used.
     * 
     * @throws io_exception
     *      If an I/O error occurs while reading from the stream.
     * 
     * @throws invalid_data_format_exception
     *      If the WAV format is invalid.
     * 
     * @throws out_of_memory_error
     *      If memory allocation for audio data fails.
     * 
     * @example
     *      ifstream file("audio.wav");
     *      wav_data wav(&file);
     */
    wav_data(istream* in, tca::allocator* allocator = tca::get_default_allocator());

    wav_data(const wav_data&);
    wav_data(wav_data&&);
    wav_data& operator=(const wav_data&);
    wav_data& operator=(wav_data&&);

    /**
     * 
     */
    ~wav_data();

    /**
     * Returns a pointer to the raw PCM audio data.
     * 
     * @return
     *      Pointer to the audio data buffer, or nullptr if no data is loaded.
     * 
     * @note
     *      The data format is determined by the WAV header fields:
     *      - numChannels: number of interleaved channels
     *      - bits_per_sample: sample size in bits
     *      - sampleRate: samples per second
     * 
     * @example
     *      const char* data = wav.get_data();
     *      // For 16-bit stereo: data[0..1] = left sample 1, data[2..3] = right sample 1
     */
    const char* get_data() const;

    /**
     * Returns the sample rate.
     * 
     * @return
     *      Sample rate in Hertz (e.g., 44100, 48000, 22050).
     * 
     * @example
     *      if (wav.get_sample_rate() == 44100) {
     *          // 44.1 kHz audio
     *      }
     */
    long get_sample_rate() const;

    /**
     * Returns the byte rate.
     * 
     * The byte rate is calculated as sampleRate * numChannels * bitsPerSample/8.
     * It indicates how many bytes are processed per second.
     * 
     * @return
     *      Byte rate in bytes per second.
     */
    long get_byte_rate() const;

    /**
     * Returns the length of the audio data in bytes.
     * 
     * @return
     *      Total number of bytes in the audio data buffer.
     * 
     * @note
     *      To get the number of samples, divide by (numChannels * bitsPerSample/8).
     */
    long get_length() const;

    /**
     * Returns the number of audio channels.
     * 
     * @return
     *      Number of channels (1 = mono, 2 = stereo).
     */
    short get_num_channels() const;

    /**
     * Returns the bits per sample.
     * 
     * @return
     *      Bits per sample (8, 16, 24, etc.).
     * 
     * @note
     *      Common values: 8 (unsigned), 16 (signed), 24 (signed), 32 (float).
     */
    short get_bits_per_sample() const;
};

}

#endif//JSTD_LANG_UTILS_AUDIO_WAV_DATA_H