#include <cpp/lang/compress/lz4_compressor.hpp>
#include <cpp/lang/math.hpp>
#include <utility>
#include <cassert>

namespace tc
{
    
    lz4_compressor::lz4_compressor() : compressor(),
    rp(0),
    lit_start(0),
    state(state::FIND_MATCH),
    lit_len(0),
    lit_writed(0),
    offset(0),
    lit_rem(0),
    match_rem(0),
    match_len(0) {
        
    }

    lz4_compressor::lz4_compressor(lz4_compressor&& other) : compressor(std::move(other)),
    rp(0),
    lit_start(0),
    state(state::FIND_MATCH),
    lit_len(0),
    lit_writed(0),
    offset(0),
    lit_rem(0),
    match_rem(0),
    match_len(0) {
        std::swap(rp,           other.rp);
        std::swap(lit_start,    other.lit_start);
        std::swap(state,        other.state);
        std::swap(lit_len,      other.lit_len);
        std::swap(lit_writed,   other.lit_writed);
        std::swap(offset,       other.offset);
        std::swap(lit_rem,      other.lit_rem);
        std::swap(match_rem,    other.match_rem);
        std::swap(match_len,    other.match_len);
    }

    lz4_compressor& lz4_compressor::operator=(lz4_compressor&& other) {
        if (&other != this)
        {
            std::swap(rp,           other.rp);
            std::swap(lit_start,    other.lit_start);
            std::swap(state,        other.state);
            std::swap(lit_len,      other.lit_len);
            std::swap(lit_writed,   other.lit_writed);
            std::swap(offset,       other.offset);
            std::swap(lit_rem,      other.lit_rem);
            std::swap(match_rem,    other.match_rem);
            std::swap(match_len,    other.match_len);
        }
        return *this;
    }

    lz4_compressor::~lz4_compressor() {

    }
    
    lz4_compressor::u32 lz4_compressor::read_u32(const char* in) {
        return  ((u32) (unsigned char) in[0] << 0)   | 
                ((u32) (unsigned char) in[1] << 8)   | 
                ((u32) (unsigned char) in[2] << 16)  | 
                ((u32) (unsigned char) in[3] << 24);
    }

    /*static*/ std::size_t lz4_compressor::hash_idx(u32 val) {
        return (val * 0x85EBCA77u) & (TABLE_SIZE - 1);
    }

    void lz4_compressor::put_hash(u32 val, std::size_t pos) {
        hash_table[hash_idx(val)] = (u16) (pos & 0xffff);
    }

    std::size_t lz4_compressor::from_hash(u32 val) {
        std::size_t idx = hash_idx(val);
        return hash_table[idx] != 0xffffu ? hash_table[idx] : no_val;
    }

    void lz4_compressor::set_input(const char* in, std::size_t length) {
        compressor::set_input(in, length);
        state       = state::FIND_MATCH;
        for (std::size_t i = 0; i < TABLE_SIZE; ++i)
            hash_table[i] = 0xffffu;
    }

    void lz4_compressor::set_state(enum lz4_compressor::state state) {
        this->state = state;
    }

    static std::size_t extend_match(const char* x, std::size_t current, std::size_t prev, std::size_t len) 
    {
        std::size_t matched = 0;
        std::size_t apos = current;
        std::size_t bpos = prev;
        while ((apos < len))
        {
            if (x[apos] != x[bpos])
                break;
            ++apos;
            ++bpos;
            ++matched;
        }        
        return matched;
    }

    void lz4_compressor::find_match() {
        assert(state == state::FIND_MATCH);
        const char* in = input;

        lit_start = rp;

        const std::size_t NO_COMPRESS_SIZE  = 5;
        const std::size_t TAIL              = NO_COMPRESS_SIZE + MATCH_LENGTH;

        while ((input_size > TAIL) && (rp < (input_size - TAIL)))
        {
            assert(rp < input_size);
            u32 seq             = read_u32(in + rp);
            std::size_t prev    = from_hash(seq);
            put_hash(seq, rp);

            //Эта последовательность встретилась впервые.
            if (prev == lz4_compressor::no_val)
            {
                ++rp;
                continue;
            }
            
            assert(rp >= prev);
            u16 dst = (u16)((u16)rp - (u16)prev);

            if (dst == 0)
            {
                ++rp;
                continue;   
            }
            
            //Проверяем, чтобы текущая последовательность совпадала с той, которая хранится в хеш-карте.
            if (read_u32(in + rp - dst) != seq)
            {
                ++rp;
                continue;
            }
            
            std::size_t prev_match_pos  = rp - dst;
            std::size_t match_len       = MATCH_LENGTH + extend_match(in, rp + MATCH_LENGTH, prev_match_pos + MATCH_LENGTH, input_size - NO_COMPRESS_SIZE);

            this->lit_len   = rp - lit_start;
            this->match_len = match_len;
            this->offset    = dst;

            rp += match_len;
            
            //Установить следующее состояние, как запись токена.
            set_state(state::WRITE_TOKEN);

            return;
        }

        //Дописать оставшиеся, не сжатые, данные
        if (rp < input_size)
        {
            this->lit_len   = input_size - lit_start;
            this->match_len = 0;
            this->offset    = 0;
            rp              = input_size;
            set_state(state::WRITE_TOKEN);
        }

    }
    
    std::size_t lz4_compressor::write_tok(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
    
        char lit_tok    = 0;
        char match_tok  = 0;

        /*
            Если размер литерала или матча будет 15, то всё равно нужно будет прочитать нулевой байт, даже если rem будет равен нулю.
            При этом, если если длина литерала меньше 15, то rem даже не читается при записи

        */
        //длина литералов
        if (lit_len >= 15)
        {
            lit_tok = 15;
            lit_rem = lit_len - 15;
        }
        else
        {
            lit_tok |= (char) lit_len;
            lit_rem  = 0;
        }

        assert(match_len == 0 || match_len >= MATCH_LENGTH);
        std::size_t token_match = match_len == 0 ? 0 : match_len - MATCH_LENGTH;

        //длина мэтча
        if (token_match >= 15)
        {
            match_tok = 15;
            match_rem = token_match - 15;
        }
        else
        {
            match_tok |= (char) token_match;
            match_rem = 0;
        }
        
        //Если размер равен или больше 15, то нужно записать дополнительные байты, для кодирования длины.
        if (lit_len >= 15) set_state(state::WRITE_LIT_LEN_EXTRA);
        
        // Иначе просто записываем литералы.
        else                set_state(state::WRITE_LITERALS);

        *buf = (char) ((lit_tok << 4) | (match_tok << 0));

        return 1;
    }
    
    std::size_t lz4_compressor::write_lit_len_extra(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        std::size_t writed = 0;

        while ((writed < sz) && lit_rem >= 255)
        {
            buf[writed++] = (char) 255;
            lit_rem      -= 255;            
        }

        // Нужно записать остаток. Даже если 0, его всё равно нужно записать.
        if (writed < sz)
        {
            buf[writed++]   = (char) lit_rem;
            set_state(state::WRITE_LITERALS);
            lit_rem         = 0;
        }

        return writed;
    }
    
    std::size_t lz4_compressor::write_lit(char buf[], std::size_t sz) {
        if (sz == 0) return 0;

        assert(lit_len >= lit_writed);
        
        std::size_t write_rem   = lit_len - lit_writed;
        std::size_t writed      = math::min(sz, write_rem);
        std::memcpy(buf, input + lit_start + lit_writed, writed);
        lit_writed += writed;

        // Если записали все данные
        if (lit_writed == lit_len)
        {
            //Обнуляем для последующих записей.
            lit_writed = 0;
            
            // Если длина матча не нулевая, значит следом нужно записать смещение в little-endian порядке.
            if (match_len > 0)
            {
                //Начинаем с младшего слова.
                set_state(state::WRITE_OFFSET_L);
            }
            else
            {
                set_state(state::FIND_MATCH);
            }
        }

        return writed;
    }
    
    std::size_t lz4_compressor::write_offset(char buf[], std::size_t sz, offset_part part) {
        if (sz < 1) return 0;
        // std::printf("offset %zu\n", offset);
        switch (part) {
        case offset_part::LOW_WORD:
            *buf = (char) ((offset >> 0) & 0xFF);
            set_state(state::WRITE_OFFSET_H);
            break;
        case offset_part::HIGH_WORD:
            *buf = (char) ((offset >> 8) & 0xFF);
            //Если длина матча равна 15, то нужно записать дополнительные байты, иначе просто искать новый матч.
            assert(match_len >= MATCH_LENGTH);
            std::size_t token_match = match_len - MATCH_LENGTH;
            if (token_match >= 15)
                set_state(state::WRITE_MATCH_LEN_EXTRA);
            else
                set_state(state::FIND_MATCH);
            break;
        }
        return 1;
    }
    
    std::size_t lz4_compressor::write_match_len_extra(char buf[], std::size_t sz) {
        if (sz == 0) return 0;
        std::size_t writed = 0;

        while ((writed < sz) && match_rem >= 255)
        {
            buf[writed++] = (char) 255;
            match_rem    -= 255;
        }

        // Нужно записать остаток. Даже если 0, его всё равно нужно записать.
        if (writed < sz)
        {
            buf[writed++]   = (char) match_rem;
            set_state(state::FIND_MATCH);
            match_rem         = 0;
        }

        return writed;
    }

    bool lz4_compressor::finished() const {
        return rp >= input_size;
    }

    std::size_t lz4_compressor::compress(char out[], std::size_t outlen) {

        std::size_t writed  = 0;
        std::size_t rem     = outlen;

        while ((rem > 0) && (state != state::FIHISHED))
        {            
            std::size_t current_writed = 0;
            switch (state) {
                case state::FIND_MATCH:
                    if (finished())
                    {
                        set_state(state::FIHISHED);
                        finish();
                    }
                    else
                    {
                        find_match();
                    }
                    break;
                case state::WRITE_TOKEN:
                    current_writed = write_tok(out, rem);
                    break;
                case state::WRITE_LIT_LEN_EXTRA:
                    current_writed = write_lit_len_extra(out, rem);
                    break;
                case state::WRITE_LITERALS:
                    current_writed = write_lit(out, rem);
                    break;
                case state::WRITE_OFFSET_L:
                    current_writed = write_offset(out, rem, offset_part::LOW_WORD);
                    break;
                case state::WRITE_OFFSET_H:
                    current_writed = write_offset(out, rem, offset_part::HIGH_WORD);
                    break;
                case state::WRITE_MATCH_LEN_EXTRA:
                    current_writed = write_match_len_extra(out, rem);
                    break;
                default:
                    set_state(state::FIHISHED);
                    break;
            }
            //Увеличиваем коливество всех записей
            writed  += current_writed;
            //Двигаем указатель записи
            out     += current_writed;
            //Уменьшаем количество свободного места в выходном буфере
            assert(rem >= current_writed);
            rem     -= current_writed;
        }

        return writed;
    }
}