#ifndef BA02FACF_D516_4679_8D5D_BDBF01904A8B
#define BA02FACF_D516_4679_8D5D_BDBF01904A8B

#include <cpp/lang/utils/hash_map.hpp>
#include <cpp/lang/utils/array_list.hpp>
#include <cstdint>
#include <cpp/lang/compress/compressor.hpp>

namespace tc
{

class lz4_compressor : public compressor {
    // 
    using u16   = uint16_t;
    
    // 
    using u32   = uint32_t;
    
    // 
    static const std::size_t MATCH_LENGTH = sizeof(u32);

    // 
    enum struct state {
        FIND_MATCH,
        WRITE_TOKEN,
        WRITE_LIT_LEN_EXTRA,
        WRITE_LITERALS,
        WRITE_OFFSET_L,
        WRITE_OFFSET_H,
        WRITE_MATCH_LEN_EXTRA,
        FIHISHED,
    };

    // 
    enum struct offset_part {
        LOW_WORD,
        HIGH_WORD
    };

    // 
    // 
    // 
    void find_match();
    
    // 
    // 
    // 
    std::size_t write_tok(char buf[], std::size_t sz);
    
    // 
    // 
    // 
    std::size_t write_lit_len_extra(char buf[], std::size_t sz);
    
    // 
    // 
    // 
    std::size_t write_lit(char buf[], std::size_t sz);
    
    // 
    // 
    // 
    std::size_t write_offset(char buf[], std::size_t sz, offset_part part);
    
    // 
    // 
    // 
    std::size_t write_match_len_extra(char buf[], std::size_t sz);
    
    // 
    // 
    // 
    bool finished() const;

    // 
    static const std::size_t no_val = ~ (std::size_t) 0;

    // 
    static const std::size_t TABLE_SIZE = 0xffff / 4;

    // 
    u16 hash_table[TABLE_SIZE];

    /**
     * Текущий указатель чтения
     */
    std::size_t rp;

    /**
     * Смещение, от которого начинаются литералы.
     */
    std::size_t lit_start;

    /**
     * Текущее состояние комперссии
     */
    state state;

    /**
     * Длина текущих литералов
     */
    std::size_t lit_len;
    
    /**
     * Количество записанных литералов.
     */
    std::size_t lit_writed;

    /**
     * Смещение от текущего чтения до начала литерала текущего матча
     */
    std::size_t offset;

    /**
     * Хранит, значение, которое нужно записать в выходной буфер.
     */
    std::size_t lit_rem;

    /**
     * Хранит, значение, которое нужно записать в выходной буфер.
     */
    std::size_t match_rem;

    /**
     * Длина текущего матча.
     */
    std::size_t match_len;

    // 
    static u32 read_u32(const char* in);

    // 
    void put_hash(u32 val, std::size_t pos);

    // 
    std::size_t from_hash(u32 val);

    // 
    static std::size_t hash_idx(u32 val);

    // 
    void set_state(enum lz4_compressor::state state);

public:
    /**
     * 
     */
    lz4_compressor();
    
    /**
     * 
     */
    lz4_compressor(lz4_compressor&&);
    
    /**
     * 
     */
    lz4_compressor& operator=(lz4_compressor&&);

    /**
     * 
     */
    ~lz4_compressor();

    /**
     * 
     */
    void set_input(const char* in, std::size_t len) override;

    /**
     * 
     */
    std::size_t compress(char out[], std::size_t outlen) override;
};

}


#endif /* BA02FACF_D516_4679_8D5D_BDBF01904A8B */
