#include <cpp/lang/utils/audio/wav_data.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/io/bytebuffer.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/iostream.hpp>
#include <iostream>

namespace tc 
{

    wav_data::wav_data() : m_allocator(nullptr), data(nullptr) {

    }

    wav_data::wav_data(const file& path, tca::allocator* allocator) {
        JSTD_DEBUG_CODE(
            if (allocator == nullptr)
                throw_except<illegal_argument_exception>("allocator must be != null");
        );
        
        if (!path.exists())
            throw_except<file_not_found_exception>("File not found!");

        m_allocator = allocator;
        
        ifstream in(path);
        try {
            load_from(&in);
        } catch(...) {
            close_stream_and_suppress_except(&in);
            throw;
        }
        in.close();

    }

    wav_data::wav_data(istream* in, tca::allocator* allocator) : wav_data() {
        JSTD_DEBUG_CODE(
            if (in == nullptr)
                throw_except<illegal_argument_exception>("in must be != null");
            if (allocator == nullptr)
                throw_except<illegal_argument_exception>("allocator must be != null");
        );
        m_allocator = allocator;
        load_from(in);
    }

    void wav_data::load_from(/*!non null!*/istream* in) {
        const std::size_t HEADER_SIZE = 44;
        char header[HEADER_SIZE];    
        
        std::size_t readed = in->read(header, HEADER_SIZE);
        if (readed != HEADER_SIZE)
            throw_except<eof_exception>("Invalid wav data!");
        
        byte_buffer header_buffer(header, HEADER_SIZE);
        header_buffer.order(byte_order::BE);
        
        //0…3 (4 байта)	chunkId	Содержит символы "RIFF" в ASCII кодировке 0x52494646. Является началом RIFF-цепочки.
        chunkId = header_buffer.get<int32_t>();
        if (chunkId != 0x52494646)
            throw_except<invalid_data_format_exception>("Invalid chunkId wav data!");

        //4…7 (4 байта)	chunkSize	Это оставшийся размер цепочки, начиная с этой позиции. Иначе говоря, это размер файла минус 8, то есть, исключены поля chunkId и chunkSize.
        chunkSize = header_buffer.get<uint32_t>();

        //8…11 (4 байта)	format	Содержит символы "WAVE" 0x57415645
        format = header_buffer.get<int32_t>();
        if (format != 0x57415645)
            throw_except<invalid_data_format_exception>("Invalid format wav data!");

        //12…15 (4 байта)	subchunk1Id	Содержит символы "fmt " 0x666d7420
        subchunk1Id = header_buffer.get<int32_t>();
        if (subchunk1Id != 0x666d7420)
            throw_except<invalid_data_format_exception>("Invalid subchunk1Id wav data!");

        header_buffer.order(byte_order::LE);
        subchunk1Size   = header_buffer.get<uint32_t>();
        audioFormat     = header_buffer.get<int16_t>();
        numChannels     = header_buffer.get<int16_t>();
        sampleRate      = header_buffer.get<int32_t>();
        byteRate        = header_buffer.get<int32_t>();
        blockAlign      = header_buffer.get<int16_t>();
        bitsPerSample   = header_buffer.get<int16_t>();
        header_buffer.order(byte_order::BE);

        subchunk2Id     = header_buffer.get<int32_t>();
        if (subchunk2Id != 0x64617461)
            throw_except<invalid_data_format_exception>("Invalid subchunk2Id wav data!");

        header_buffer.order(byte_order::LE);        
        subchunk2Size   = header_buffer.get<uint32_t>();

        data = reinterpret_cast<char*>(m_allocator->allocate(subchunk2Size));
        
        if (!data)
            throw_except<out_of_memory_error>("Out of memory");
        
        try {
            readed = in->read(data, subchunk2Size);
            if (readed != subchunk2Size)
                throw_except<eof_exception>("EOF wav data!");
        } catch (...) {
            m_allocator->deallocate(data, subchunk2Size);
            throw;
        }
    }

    void wav_data::cleanup() {
        if (m_allocator != nullptr && data != nullptr) {
            m_allocator->deallocate(data, subchunk2Size);
            data = nullptr;
        }
    }

    wav_data::~wav_data() {
        cleanup();
    }

    const char* wav_data::get_data() const {
        return data;
    }
    
    int32_t wav_data::get_format() const {
        return format;
    }
    
    int32_t wav_data::get_sample_rate() const {
        return sampleRate;
    }
    
    int32_t wav_data::get_byte_rate() const {
        return byteRate;
    }
    
    uint32_t wav_data::get_length() const {
        return subchunk2Size;
    }
    
    int16_t wav_data::get_num_channels() const {
        return numChannels;
    }

    int16_t wav_data::get_bits_per_sample() const {
        return bitsPerSample;
    }
}