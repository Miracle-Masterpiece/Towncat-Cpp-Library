#ifndef B72D3EC0_A785_4B9E_A22A_414586432947
#define B72D3EC0_A785_4B9E_A22A_414586432947

#include <cstdint>
#include <cpp/lang/compress/decompressor.hpp>

namespace tc
{

class lz4_decompressor : public decompressor {
    // 
    using u16   = uint16_t;
    
    // 
    using u32   = uint32_t;

    enum struct state {
        READ_TOKEN,
        READ_LIT_EXTRA,
        READ_LITERALS,
        READ_OFFSET_L,
        READ_OFFSET_H,
        READ_MATCH_EXTRA,
        DECOMPRESS,
        FINISHED
    };

    enum struct offset_part {
        LOW_WORD,
        HIGH_WORD
    };

    // must be x^2
    static const std::size_t WINDOW_SIZE = 1 << 16;

    // 
    std::size_t WINDOW_MASK = WINDOW_SIZE - 1;

    // 
    std::size_t MATCH_LENGTH = sizeof(u32);

    // Окно, куда сохраняются последние N-байт
    char window[WINDOW_SIZE];

    // Текущая позиция в окне
    std::size_t window_pos;

    // Текущий указатель чтения сжатых байт
    std::size_t rp;

    // Состояние потока
    state state;

    // Длина литералов, которую нужно прочитать
    std::size_t lit_len;
    
    // Длина повторения
    std::size_t match_len;

    // Смещение, от текущего положения в окне
    std::size_t off;

    // Сколько литералов осталось прочитать
    std::size_t lit_rem;

    //
    std::size_t match_off;

    // 
    void set_state(enum lz4_decompressor::state state);

    // 
    unsigned char read_char() {
        return (unsigned char) input[rp++];
    }

    // 
    void read_tok();
    
    // 
    void read_lit_extra();
    
    // 
    std::size_t read_literals(char buf[], std::size_t sz);
    
    // 
    void read_offset(offset_part part);
    
    // 
    void read_match_extra();
    
    // 
    std::size_t decompress_literals(char buf[], std::size_t sz);

public:
    /**
     * 
     */
    lz4_decompressor();

    /**
     * 
     */
    lz4_decompressor(lz4_decompressor&&);
    
    /**
     * 
     */
    lz4_decompressor& operator=(lz4_decompressor&&);
    
    /**
     * 
     */
    ~lz4_decompressor();
    
    /**
     * 
     */
    void set_input(const char* in, std::size_t len) override;
    
    /**
     * 
     */
    std::size_t decompress(char buf[], std::size_t sz) override;
};

} //namespace tc



#endif /* B72D3EC0_A785_4B9E_A22A_414586432947 */
