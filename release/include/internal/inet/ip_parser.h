#ifndef F92FAA17_8D3D_4F5C_82C0_33C9DABD1156
#define F92FAA17_8D3D_4F5C_82C0_33C9DABD1156

#include <cpp/lang/net/inetaddr.hpp>

namespace tc
{
namespace net
{
namespace internal
{

// 
// 
// 
enum class token_kind {
    END, UNDEFINED, NUMBER, IDENTIFICATOR, COLON, COLON2, DOT
};

// 
// 
// 
struct token {
    token_kind kind;
    long dec;
    long hex;
    token() : kind(token_kind::END) {}
    token(token_kind kind) : kind(kind) {}
};

// 
// 
// 
class ip_parser {
    const char* str;    // string
    char        cur;    // current
    std::size_t off;    // offset
    std::size_t len;    // length
    bool        end;    //
    void next_char();
    char get_char() const;
    bool eof() const;
    token next_tok();
    inet_address parse_v4();
    inet_address parse_v6();
public:
    ip_parser(const char* ip_string);
    inet_address parse(inet_family family);
};

} //namespace internal
} //namespace net
} //namespace jstd

#endif /* F92FAA17_8D3D_4F5C_82C0_33C9DABD1156 */
