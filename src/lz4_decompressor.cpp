#include <cpp/lang/compress/lz4_decompressor.hpp>
#include <cpp/lang/math.hpp>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <utility>

namespace tc
{
    lz4_decompressor::lz4_decompressor() : decompressor(),
    window_pos(0),
    rp(0),
    state(state::READ_TOKEN),
    lit_len(0),
    match_len(0),
    off(0),
    lit_rem(0),
    match_off(0) {

    }

    lz4_decompressor::lz4_decompressor(lz4_decompressor&& other) : decompressor(std::move(other)),
    window_pos(0),
    rp(0),
    state(state::READ_TOKEN),
    lit_len(0),
    match_len(0),
    off(0),
    lit_rem(0),
    match_off(0) {
        std::swap(window_pos, other.window_pos);
        std::swap(rp, other.rp);
        std::swap(state, other.state);
        std::swap(lit_len, other.lit_len);
        std::swap(match_len, other.match_len);
        std::swap(off, other.off);
        std::swap(lit_rem, other.lit_rem);
        std::swap(match_off, other.match_off);
    }

    lz4_decompressor& lz4_decompressor::operator=(lz4_decompressor&& other) {
        if (&other != this)
        {
            decompressor::operator=(std::move(other));
            std::swap(window_pos, other.window_pos);
            std::swap(rp, other.rp);
            std::swap(state, other.state);
            std::swap(lit_len, other.lit_len);
            std::swap(match_len, other.match_len);
            std::swap(off, other.off);
            std::swap(lit_rem, other.lit_rem);
            std::swap(match_off, other.match_off);
        }
        return *this;
    }

    lz4_decompressor::~lz4_decompressor() {

    }
    
    void lz4_decompressor::set_state(enum lz4_decompressor::state state) {
        this->state = state;
    }

    void lz4_decompressor::set_input(const char* in, std::size_t len) {
        input       = in;
        input_size  = len;
        window_pos  = 0;
        set_state(state::READ_TOKEN);
    }
 
    void lz4_decompressor::read_tok() {
        unsigned char token     = read_char();
        unsigned char lit       = (token >> 4) & 0xf;
        unsigned char match     = (token >> 0) & 0xf;
        
        lit_len     = lit;
        match_len   = match + MATCH_LENGTH;
        lit_rem     = lit_len;
        match_off   = 0;

        // Если длина равна 15, значит нужно прочитать дополнительные байты длины
        if (lit == 15)  set_state(state::READ_LIT_EXTRA);
        else            set_state(state::READ_LITERALS);
    }

    void lz4_decompressor::read_lit_extra() {
        unsigned char readed;
        do {
            readed   = read_char();
            lit_len += readed;
            lit_rem += readed;
        } while (readed == 0xff);
        set_state(state::READ_LITERALS);
    }

    void lz4_decompressor::read_match_extra() {
        unsigned char readed;
        do {
            readed     = read_char();
            match_len += readed;
        } while (readed == 0xff);
        set_state(state::DECOMPRESS);
    }

    std::size_t lz4_decompressor::read_literals(char buf[], std::size_t sz) {
        if (sz == 0) return 0; 
        
        if (lit_rem > 0)
        {
            std::size_t readed = math::min(lit_rem, sz);

            for (std::size_t i = 0; i < readed; ++i)
            {
                char b = input[rp];
                
                //Сохранить литералы в окно.
                window[window_pos & WINDOW_MASK]  = b;
                
                //Сохранить литералы непосредственно в выходной буфер
                buf[i]                              = b;
                
                assert(lit_rem > 0);
                --lit_rem;
                ++rp;
                ++window_pos;
            }

            return readed;
        }
        else
        {
            if (rp < input_size)
            {
                off = 0;
                set_state(state::READ_OFFSET_L);
            }
            else
            {
                set_state(state::FINISHED);
            }
        }

        return 0;
    }
    
    void lz4_decompressor::read_offset(offset_part part) {
        switch (part) {
            case offset_part::LOW_WORD: 
                off |=  (u16) (((u16) read_char()) << 0);
                set_state(state::READ_OFFSET_H);
                break;
            case offset_part::HIGH_WORD:
                off |=  (u16) (((u16) read_char()) << 8);
                if (match_len >= 15 + MATCH_LENGTH)
                    set_state(state::READ_MATCH_EXTRA);
                else
                    set_state(state::DECOMPRESS);
                break;
        }
    }

    std::size_t lz4_decompressor::decompress_literals(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        assert(off != 0);

        if (match_off < match_len)
        {
            std::size_t readed = math::min(sz, match_len - match_off);
            for (std::size_t i = 0; i < readed; ++i)
            {
                char b = window[(window_pos - off) & WINDOW_MASK];
                buf[i] = b;
                window[window_pos & WINDOW_MASK] = b;
                ++window_pos;
                ++match_off;
            }
            return readed;
        }
        else
        {
            set_state(state::READ_TOKEN);
        }

        return 0;
    }
    
    std::size_t lz4_decompressor::decompress(char buf[], std::size_t sz) {
        std::size_t rem     = sz;
        std::size_t readed  = 0;
        while (rem > 0 && state != state::FINISHED)
        {
            std::size_t current_readed = 0;
            switch (state) {
                case state::READ_TOKEN:
                    read_tok();
                    break;
                case state::READ_LIT_EXTRA:
                    read_lit_extra();
                    break;
                case state::READ_LITERALS:
                    current_readed = read_literals(buf + readed, rem);
                    break;
                case state::READ_OFFSET_L:
                    read_offset(offset_part::LOW_WORD);
                    break;
                case state::READ_OFFSET_H:
                    read_offset(offset_part::HIGH_WORD);
                    break;
                case state::READ_MATCH_EXTRA:
                    read_match_extra();
                    break;
                case state::DECOMPRESS:
                    current_readed = decompress_literals(buf + readed, rem);
                    break;
                case state::FINISHED:
                    break;
            }
            assert(rem >= current_readed);
            rem     -= current_readed;
            readed  += current_readed;

            // std::printf("rem %zu\n", rem);
        }

        return readed;
    }
}

#if 0

#include <lz4.h>
#include <cpp/lang/ustring.hpp>

int main() {
    // const char* str = "The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.";
    // const char* str = "abracadabraabracadabraabracadabraabracadabraabracadabra";
    const std::size_t BUF_SZ = 1 << 17;
    char compressed[BUF_SZ];
    std::size_t compressed_length;


    tc::string s;
    for (int i = 0; i < 100000; ++i)
        s.append("The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.");

    {//compress
        compressed_length = LZ4_compress_default(s.c_string(), compressed, (int) s.length(), (int) BUF_SZ);
        std::printf("compressed: %zu\n", compressed_length);
        for (std::size_t i = 0; i < compressed_length; ++i)
        {
            std::printf("%x ", compressed[i] & 0xff);
        }
        std::printf("\n");
    }
    
    {//decompress
        tc::lz4_decompressor decompressor;
        decompressor.set_input(compressed, compressed_length);
        
        char decompressed[BUF_SZ];
        std::size_t decompressed_length = decompressor.decompress(decompressed, BUF_SZ);
        std::printf("decompressed: %zu\n", decompressed_length);

        for (std::size_t i = 0; i < decompressed_length; ++i)
        {
            std::printf("%c", decompressed[i]);
        }
    }
    
}

#endif