#include <cpp/lang/utils/audio/wav_data.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/io/bytebuffer.hpp>
#include <cpp/lang/utils/unique_ptr.hpp>
#include <cpp/lang/io/ifstream.hpp>
#include <cpp/lang/io/iostream.hpp>
#include <iostream>


#define TC_MAKE_INT(buf) ((static_cast<long>(buf[0]) & 0xFF) | ((static_cast<long>(buf[1]) & 0xFF) << CHAR_BIT) | ((static_cast<long>(buf[2]) & 0xFF) << (CHAR_BIT * 2)) | ((static_cast<long>(buf[3]) & 0xFF) << (CHAR_BIT * 3)));
#define TC_MAKE_SHORT(buf) ((static_cast<short>(buf[0]) & 0xFF) | ((static_cast<short>(buf[1]) & 0xff) << CHAR_BIT));

namespace tc 
{

    wav_data::wav_data() : m_allocator(nullptr), data(nullptr) {

    }

    wav_data::wav_data(const wav_data& wav) {
        m_allocator = wav.m_allocator;
        
        data = (char*) m_allocator->allocate( static_cast<std::size_t>(wav.subchunk2Size) );
        
        if (!data)
            throw_except<out_of_memory_error>("Out of memory");
        
        std::memcpy(data, wav.data, wav.subchunk2Size);
        
        chunk_size      = wav.chunk_size;
        subchunk1Size   = wav.subchunk1Size;
        sampleRate      = wav.sampleRate;
        byteRate        = wav.byteRate;
        subchunk2Size   = wav.subchunk2Size;
        block_align     = wav.block_align;
        audioFormat     = wav.audioFormat;
        numChannels     = wav.numChannels;
        bits_per_sample = wav.bits_per_sample;

        std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
        std::memcpy(format, wav.format, sizeof(format));
        std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
        std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
    }
    
    wav_data::wav_data(wav_data&& wav) : wav_data() {
        std::swap(m_allocator, wav.m_allocator);
        std::swap(data, wav.data);
        std::swap(chunk_size, wav.chunk_size);
        std::swap(subchunk1Size, wav.subchunk1Size);
        std::swap(sampleRate, wav.sampleRate);
        std::swap(byteRate, wav.byteRate);
        std::swap(subchunk2Size, wav.subchunk2Size);
        std::swap(block_align, wav.block_align);
        std::swap(audioFormat, wav.audioFormat);
        std::swap(numChannels, wav.numChannels);
        std::swap(bits_per_sample, wav.bits_per_sample);
        std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
        std::memcpy(format, wav.format, sizeof(format));
        std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
        std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
    }
    
    wav_data& wav_data::operator=(const wav_data& wav) {
        if (&wav != this)
        {
            char* new_data = (char*) wav.m_allocator->allocate( static_cast<std::size_t>(wav.subchunk2Size) );
            if (!new_data)
                throw_except<out_of_memory_error>("Out of memory");
                
            std::memcpy(new_data, wav.data, wav.subchunk2Size);
            
            if (data && m_allocator)
                m_allocator->deallocate(data);
            
            m_allocator     = wav.m_allocator;
            data            = new_data;
            chunk_size      = wav.chunk_size;
            subchunk1Size   = wav.subchunk1Size;
            sampleRate      = wav.sampleRate;
            byteRate        = wav.byteRate;
            subchunk2Size   = wav.subchunk2Size;
            block_align     = wav.block_align;
            audioFormat     = wav.audioFormat;
            numChannels     = wav.numChannels;
            bits_per_sample = wav.bits_per_sample;
    
            std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
            std::memcpy(format, wav.format, sizeof(format));
            std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
            std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
        }
        return *this;
    }
    
    wav_data& wav_data::operator=(wav_data&& wav) {
        if (&wav != this)
        {
            std::swap(m_allocator, wav.m_allocator);
            std::swap(data, wav.data);
            std::swap(chunk_size, wav.chunk_size);
            std::swap(subchunk1Size, wav.subchunk1Size);
            std::swap(sampleRate, wav.sampleRate);
            std::swap(byteRate, wav.byteRate);
            std::swap(subchunk2Size, wav.subchunk2Size);
            std::swap(block_align, wav.block_align);
            std::swap(audioFormat, wav.audioFormat);
            std::swap(numChannels, wav.numChannels);
            std::swap(bits_per_sample, wav.bits_per_sample);
            std::memcpy(chunk_id, wav.chunk_id, sizeof(chunk_id));
            std::memcpy(format, wav.format, sizeof(format));
            std::memcpy(subchunk1Id, wav.subchunk1Id, sizeof(subchunk1Id));
            std::memcpy(subchunk2Id, wav.subchunk2Id, sizeof(subchunk2Id));
        }
        return *this;
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

    static void fill_buf_or_except(char buf[], std::size_t sz, /*non_null*/ istream* in) {
        std::size_t readed = in->read(buf, sz);
        if (readed < sz)
            throw_except<invalid_data_format_exception>("Invalid wav data!");
    }

    void wav_data::load_from(/*!non null!*/istream* in) {

        {//read "RIFF"
            fill_buf_or_except(chunk_id, sizeof(chunk_id), in);

            const char RIFF[] = "RIFF";
            for (std::size_t i = 0; i < sizeof(chunk_id); ++i)
                if (chunk_id[i] != RIFF[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'RIFF' wav");
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            chunk_size = TC_MAKE_INT(buf);
        }

        {//read "WAVE"
            fill_buf_or_except(format, sizeof(format), in);

            const char WAVE[] = "WAVE";
            for (std::size_t i = 0; i < sizeof(format); ++i)
                if (format[i] != WAVE[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'WAVE' wav");
        }
        
        {//read "fmt "
            
            fill_buf_or_except(subchunk1Id, sizeof(subchunk1Id), in);

            const char FMT[] = "fmt ";
            for (std::size_t i = 0; i < sizeof(subchunk1Id); ++i)
                if (subchunk1Id[i] != FMT[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'fmt ' wav");
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);        
            subchunk1Size = TC_MAKE_INT(buf);
        }
        
        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            audioFormat = TC_MAKE_SHORT(buf);
        }
        
        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            numChannels = TC_MAKE_SHORT(buf);
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            sampleRate = TC_MAKE_INT(buf);
        }
        
        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            byteRate = TC_MAKE_INT(buf);
        }

        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            block_align = TC_MAKE_SHORT(buf);
        }
        
        {
            char buf[2];
            fill_buf_or_except(buf, sizeof(buf), in);
            bits_per_sample = TC_MAKE_SHORT(buf);
        }

        {
            fill_buf_or_except(subchunk2Id, sizeof(subchunk2Id), in);
            const char data[] = "data";
            for (std::size_t i = 0; i < sizeof(subchunk2Id); ++i)
                if (subchunk2Id[i] != data[i])
                    throw_except<invalid_data_format_exception>("Invalid format 'subchunk2Id' wav");
        }

        {
            char buf[4];
            fill_buf_or_except(buf, sizeof(buf), in);
            subchunk2Size = TC_MAKE_INT(buf);
        }

        {      
            data = (char*) m_allocator->allocate(static_cast<std::size_t>(subchunk2Size));
            if (!data)
            {
                throw_except<out_of_memory_error>("Out of memory");
            }

            try {
                fill_buf_or_except(data, static_cast<std::size_t>(subchunk2Size), in);
            } catch (...) {
                m_allocator->deallocate(data);
                throw;
            }
        }
    }

    wav_data::~wav_data() {
        if (m_allocator != nullptr && data != nullptr)
        {
            m_allocator->deallocate(data, subchunk2Size);
            data = nullptr;
        }
    }

    const char* wav_data::get_data() const {
        return data;
    }
    
    long wav_data::get_sample_rate() const {
        return sampleRate;
    }
    
    long wav_data::get_byte_rate() const {
        return byteRate;
    }
    
    long wav_data::get_length() const {
        return subchunk2Size;
    }
    
    short wav_data::get_num_channels() const {
        return numChannels;
    }

    short wav_data::get_bits_per_sample() const {
        return bits_per_sample;
    }
}