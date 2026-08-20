#ifndef D6E2F691_D7BE_47D6_87D8_510CCCFAB1ED
#define D6E2F691_D7BE_47D6_87D8_510CCCFAB1ED

#include <cstdint>
#include <cpp/lang/utils/utils.hpp>
#include <allocators/allocator.hpp>
#include <cpp/lang/string.hpp>
#include <cpp/lang/system.hpp>

namespace tc
{
namespace utf
{

#if __SIZEOF_INT__ == 4
    typedef unsigned int utfpoint;
#elif __SIZEOF_LONG__ == 4
    typedef unsigned long utfpoint;
#else
    #error Undefined utfpoint type!
#endif

namespace utf8
{
    template<typename U8>
    bool is_header(const U8& ch) {
        unsigned char chr = (unsigned char) (ch & 0xFF);
        if ( chr < 0x7f)          return true;
        if ((chr & 0xE0) == 0xC0) return true;
        if ((chr & 0xF0) == 0xE0) return true;
        if ((chr & 0xF8) == 0xF0) return true;
        return false;
    }

    template<typename U8>
    std::size_t symbol_size(const U8& header) {
        unsigned char chr = (unsigned char) (header & 0xFF);

        if ((chr & 0x80) == 0)       return 1;
        if ((chr & 0xE0) == 0xC0)    return 2;
        if ((chr & 0xF0) == 0xE0)    return 3;
        if ((chr & 0xF8) == 0xF0)    return 4;
        
        throw make_except<utf_format_exception>("Character %x is not header!", (unsigned int) chr);
    }

    template<typename U8>
    utfpoint chars_to_codepoint(const U8* str) {
        std::size_t char_size = symbol_size(*str);
        
        if (char_size == 1) 
        {
            return (unsigned char) *str;
        }
        
        if (char_size == 2)
        {
            utfpoint ch1 = (unsigned char) str[0] & 0x1F;
            utfpoint ch2 = (unsigned char) str[1] & 0x3F;
            return ch2 | (ch1 << 6);
        }
        
        if (char_size == 3)
        {
            utfpoint ch1 = (unsigned char) str[0] & 0xF;
            utfpoint ch2 = (unsigned char) str[1] & 0x3F;
            utfpoint ch3 = (unsigned char) str[2] & 0x3F;
            return ch3 | (ch2 << 6) | (ch1 << 12);
        }
        
        if (char_size == 4)
        {
            utfpoint ch1 = (unsigned char) str[0] & 0x7;
            utfpoint ch2 = (unsigned char) str[1] & 0x3F;
            utfpoint ch3 = (unsigned char) str[2] & 0x3F;
            utfpoint ch4 = (unsigned char) str[3] & 0x3F;
            return ch4 | (ch3 << 6) | (ch2 << 12) | (ch1 << 18);
        }
        
        throw make_except<utf_format_exception>("Character %x is not header!", (unsigned int) *str);
    }

    template<typename U8>
    std::size_t codepoint_to_chars(U8 buf[], utfpoint codepoint) {
        if (codepoint < 0x7f){
            buf[0] = (U8) (codepoint & 0xff);
            return 1;
        }
        if (codepoint >= 0x80      && codepoint < 0x7ff)
        {
            U8 b1 = (U8) (((codepoint >> 6) & 0x1F) | 0xC0);
            U8 b2 = (U8) (((codepoint >> 0) & 0x3F) | 0x80);
            buf[0] = b1;
            buf[1] = b2;
            return 2;
        }
        
        if (codepoint >= 0x800     && codepoint < 0xffff)
        {
            U8 b1 = (U8) (((codepoint >> 12) & 0xF)  | 0xE0);
            U8 b2 = (U8) (((codepoint >> 6)  & 0x3F) | 0x80);
            U8 b3 = (U8) (((codepoint >> 0)  & 0x3F) | 0x80);
            buf[0] = b1;
            buf[1] = b2;
            buf[2] = b3;
            return 3;
        }
        
        if (codepoint >= 0x10000   && codepoint < 0x10ffff)
        {
            U8 b1 = (U8) (((codepoint >> 18)  & 0x7)  | 0xF0);
            U8 b2 = (U8) (((codepoint >> 12)  & 0x3F) | 0x80);
            U8 b3 = (U8) (((codepoint >> 6)   & 0x3F) | 0x80);
            U8 b4 = (U8) (((codepoint >> 0)   & 0x3F) | 0x80);
            buf[0] = b1;
            buf[1] = b2;
            buf[2] = b3;
            buf[3] = b4;
            return 4;
        }
        
        throw make_except<utf_format_exception>("Invalid %lu codepoint", (unsigned long) codepoint);
    }

    template<typename U8>
    class iterator {
        const U8*   seq;
        std::size_t len;
        std::size_t off;
    public:
        iterator(const U8* seq, std::size_t len, std::size_t off) : seq(seq), len(len), off(off) {}
        iterator(const U8* seq, std::size_t len) : iterator(seq, len, 0) {}
    
        bool operator==(const iterator<U8>& it) const {
            return seq == it.seq && len == it.len && off == it.off;
        }
        
        bool operator!=(const iterator<U8>& it) const {
            return !(*this == it);
        }

        utfpoint operator* () const {
            check_index(off, len);
            return utf8::chars_to_codepoint(seq + off);
        }

        iterator& operator++() {
            std::size_t char_sz = utf8::symbol_size(*(seq + off));
            off += char_sz;
            return *this;
        }
        
        iterator operator++(int) {
            iterator it(seq, len, off);
            std::size_t char_sz = utf8::symbol_size(*(seq + off));
            off += char_sz;
            return it;
        }
    };

}

namespace utf16
{
    template<typename U16>
    bool is_high_surrogate(const U16& ch) {
        return ch >= 0xd800 && ch <= 0xdbff;
    }

    template<typename U16>
    bool is_low_surrogate(const U16& ch) {
        return ch >= 0xdc00 && ch <= 0xdfff;
    }

    template<typename U16>
    std::size_t symbol_size(const U16& ch) {
        return is_high_surrogate(ch) ? 2 : 1;
    }

    template<typename U16>
    utfpoint chars_to_codepoint(const U16* str, byte_order str_byteorder) {
        U16 ch1 = utils::read_with_order<U16>(str, str_byteorder);
        
        if (is_high_surrogate(ch1))
        {
            U16 ch2 = utils::read_with_order<U16>(str + 1, str_byteorder);
            if (is_low_surrogate(ch2))
            {
                utfpoint c1 = (utfpoint) (ch1 - 0xD800);
                utfpoint c2 = (utfpoint) (ch2 - 0xDC00);
                return ((c1 << 10) | (c2 << 0)) + 0x10000;
            }   
        }

        return (utfpoint) ch1;
    }

    template<typename U16>
    std::size_t codepoint_to_chars(U16 buf[], utfpoint cp, byte_order out_str_order) {
        if (cp < 0x10000) {
            utils::write_with_order<U16>(buf, (U16) cp, out_str_order);
            return 1;
        } else {
            cp -= 0x10000;
            U16 c1 = (U16) (((cp >> 10) & 0x3ff) + 0xD800);
            U16 c2 = (U16) (((cp >>  0) & 0x3ff) + 0xDC00);
            utils::write_with_order<U16>(buf + 0, c1, out_str_order);
            utils::write_with_order<U16>(buf + 1, c2, out_str_order);
            return 2;
        }
    }

    template<typename U16>
    class iterator {
        const U16*  seq;
        std::size_t len;
        std::size_t off;
        byte_order  order;
    public:
        iterator(const U16* seq, std::size_t len, std::size_t off, byte_order order) : seq(seq), len(len), off(off), order(order) {}
        iterator(const U16* seq, std::size_t len, byte_order order) : iterator(seq, len, 0, order) {}
    
        bool operator==(const iterator<U16>& it) const {
            return seq == it.seq && len == it.len && off == it.off;
        }
        
        bool operator!=(const iterator<U16>& it) const {
            return !(*this == it);
        }

        utfpoint operator* () const {
            check_index(off, len);
            return utf16::chars_to_codepoint(seq + off, order);
        }

        iterator& operator++() {
            U16 ch = utils::read_with_order<U16>(seq + off, order);
            std::size_t char_sz = utf16::symbol_size(ch);
            off += char_sz;
            return *this;
        }
        
        iterator operator++(int) {
            iterator it(seq, len, off, order);
            U16 ch = utils::read_with_order<U16>(seq + off, order);
            std::size_t char_sz = utf16::symbol_size(ch);
            off += char_sz;
            return it;
        }
    };

}    

namespace utf32
{
    template<typename U32>
    class iterator {
        const U32*  seq;
        std::size_t len;
        std::size_t off;
        byte_order  order;
    public:
        iterator(const U32* seq, std::size_t len, std::size_t off, byte_order order) : seq(seq), len(len), off(off), order(order) {}
        iterator(const U32* seq, std::size_t len, byte_order order) : iterator(seq, len, 0, order) {}
    
        bool operator==(const iterator<U32>& it) const {
            return seq == it.seq && len == it.len && off == it.off;
        }
        
        bool operator!=(const iterator<U32>& it) const {
            return !(*this == it);
        }

        utfpoint operator* () const {
            check_index(off, len);
            const U32 MASK = ~((U32) 0);
            return ((utfpoint) utils::read_with_order<U32>(seq + off, order)) & MASK;
        }

        iterator& operator++() {
            ++off;
            return *this;
        }
        
        iterator operator++(int) {
            iterator it(seq, len, off, order);
            ++off;
            return it;
        }
    };
}

    template<typename U8>
    class utf8::iterator<U8> u8_begin(const U8* str, std::size_t len) {
        return utf8::iterator<U8>(str, len);
    }
    
    template<typename U8>
    class utf8::iterator<U8> u8_end(const U8* str, std::size_t len) {
        return utf8::iterator<U8>(str, len, len);
    }
    
    template<typename U8>
    class utf8::iterator<U8> u8_begin(const tstring<U8>& str) {
        return utf8::iterator<U8>(str.c_str(), str.length());
    }
    
    template<typename U8>
    class utf8::iterator<U8> u8_end(const tstring<U8>& str) {
        return utf8::iterator<U8>(str.c_str(), str.length(), str.length());
    }

    template<typename U16>
    class utf16::iterator<U16> u16_begin(const U16* str, std::size_t len,  byte_order order = system::native_byte_order()) {
        return utf16::iterator<U16>(str, len, order);
    }
    
    template<typename U16>
    class utf16::iterator<U16> u16_end(const U16* str, std::size_t len,  byte_order order = system::native_byte_order()) {
        return utf16::iterator<U16>(str, len, len, order);
    }
    
    template<typename U16>
    class utf16::iterator<U16> u16_begin(const tstring<U16>& str, byte_order order = system::native_byte_order()) {
        return utf16::iterator<U16>(str.c_str(), str.length(), order);
    }
    
    template<typename U16>
    class utf16::iterator<U16> u16_end(const tstring<U16>& str, byte_order order = system::native_byte_order()) {
        return utf16::iterator<U16>(str.c_str(), str.length(), str.length(), order);
    }

    template<typename U32>
    class utf32::iterator<U32> u32_begin(const U32* str, std::size_t len,  byte_order order = system::native_byte_order()) {
        return utf32::iterator<U32>(str, len, order);
    }
    
    template<typename U32>
    class utf32::iterator<U32> u32_end(const U32* str, std::size_t len,  byte_order order = system::native_byte_order()) {
        return utf32::iterator<U32>(str, len, len, order);
    }
    
    template<typename U32>
    class utf32::iterator<U32> u32_begin(const tstring<U32>& str, byte_order order = system::native_byte_order()) {
        return utf32::iterator<U32>(str.c_str(), str.length(), order);
    }
    
    template<typename U32>
    class utf32::iterator<U32> u32_end(const tstring<U32>& str, byte_order order = system::native_byte_order()) {
        return utf32::iterator<U32>(str.c_str(), str.length(), str.length(), order);
    }

    template<typename U8, typename U16>
    tstring<U16>& u8_to_u16(const tstring<U8>& u8, tstring<U16>& u16, byte_order u16_byteorder = system::native_byte_order()) {
        
        utf8::iterator<U8> begin = u8_begin(u8);
        utf8::iterator<U8> end   = u8_end(u8);

        U16 u16_buf[2];
        while (begin != end)
        {
            utfpoint codepoint   = *begin;
            std::size_t u16_size = utf16::codepoint_to_chars(u16_buf, codepoint, u16_byteorder);
            u16.append(u16_buf, u16_size);
            ++begin;
        }
        
        return u16;
    }
    
    template<typename U16, typename U8>
    std::size_t u16_to_u8(const U16* u16, std::size_t u16_len, U8 u8[], std::size_t u8_len, byte_order u16_byteorder = system::native_byte_order()) {

        utf16::iterator<U16> begin = u16_begin   (u16, u16_len, u16_byteorder);
        utf16::iterator<U16> end   = u16_end     (u16, u16_len, u16_byteorder);
        
        std::size_t off = 0;
        
        while (begin != end)
        {
            utfpoint codepoint      = *begin;
            
            U8 u8_buf[4];
            std::size_t u8_chr_size = utf8::codepoint_to_chars(u8_buf, codepoint);
            
            //                    + 1 для нуль-терминатора
            if (u8_len - off < u8_chr_size + 1)
                break;
            
            std::memcpy(u8 + off, u8_buf, u8_chr_size * sizeof(U8));
            off += u8_chr_size;

            ++begin;
        }

        u8[off] = 0;
        
        return off;
    }

    template<typename U16, typename U8>
    tstring<U8>& u16_to_u8(const tstring<U16>& u16, tstring<U8>& u8, byte_order u32_byteorder = system::native_byte_order()) {
        
        utf16::iterator<U16> begin = u16_begin  (u16, u32_byteorder);
        utf16::iterator<U16> end   = u16_end    (u16, u32_byteorder);

        U8 u8_buf[4];
        while (begin != end)
        {
            utfpoint codepoint  = *begin;
            std::size_t u8_size = utf8::codepoint_to_chars(u8_buf, codepoint);
            u8.append(u8_buf, u8_size);
            ++begin;
        }
        
        return u8;
    }

    template<typename U8, typename U16> 
    std::size_t u8_to_u16(const U8* u8, std::size_t u8_len, U16 u16[], std::size_t u16_len, byte_order u16_byteorder = system::native_byte_order()) {

        utf8::iterator<U8> begin = u8_begin   (u8, u8_len);
        utf8::iterator<U8> end   = u8_end     (u8, u8_len);
        
        std::size_t off = 0;
        
        while (begin != end)
        {
            utfpoint codepoint      = *begin;
            U16 u16_buf[2];
            std::size_t u16_chr_size = utf16::codepoint_to_chars(u16_buf, codepoint, u16_byteorder);
            
            //                     + 1 для нуль-терминатора
            if (u16_len - off < u16_chr_size + 1)
                break;
            
            std::memcpy(u16 + off, u16_buf, u16_chr_size * sizeof(U16));
            off += u16_chr_size;

            ++begin;
        }

        u16[off] = 0;
        
        return off;
    }

    template<typename U8, typename U32>
    tstring<U32>& u8_to_u32(const tstring<U8>& u8, tstring<U32>& u32, byte_order u32_byteorder = system::native_byte_order()) {
        
        utf8::iterator<U8> begin = u8_begin(u8);
        utf8::iterator<U8> end   = u8_end(u8);

        const U32 MASK = ~((U32) 0);

        while (begin != end)
        {
            utfpoint codepoint = *begin;
            U32 chr;
            utils::write_with_order<U32>(&chr, codepoint & MASK, u32_byteorder);
            u32.append(&chr, 1);
            ++begin;
        }
        
        return u32;
    }

    template<typename U8, typename U32>
    tstring<U8>& u32_to_u8(const tstring<U32>& u32, tstring<U32>& u8, byte_order u32_byteorder = system::native_byte_order()) {
        
        utf32::iterator<U32> begin = u32_begin  (u32, u32_byteorder);
        utf32::iterator<U32> end   = u32_end    (u32, u32_byteorder);

        const U32 MASK = ~((U32) 0);

        U8 u8_buf[4];
        while (begin != end)
        {
            utfpoint codepoint = *begin;
            std::size_t u8_sz  = utf8::codepoint_to_chars(u8_buf, codepoint);
            u8.append(u8_buf, u8_sz);
            ++begin;
        }
        
        return u8;
    }
    
}//namespace utf
}//namespace jstd




#endif /* D6E2F691_D7BE_47D6_87D8_510CCCFAB1ED */
