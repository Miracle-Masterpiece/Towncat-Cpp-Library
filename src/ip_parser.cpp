#include <internal/inet/ip_parser.h>
#include <cstdio>
#include <cctype>
#include <cassert>
#include <cstdlib>

namespace tc
{
namespace net
{
namespace internal
{
    ip_parser::ip_parser(const char* ip_string) : 
    str(ip_string), 
    cur(0),
    off(0),
    len(std::strlen(ip_string)),
    end(false) {
        
    }

    void ip_parser::next_char() {
        if (off >= len)
        {
            end = true;
            cur = 0;
            return;
        }
        cur = str[off++];
    }

    char ip_parser::get_char() const {
        return cur;
    }

    bool ip_parser::eof() const {
        return end;
    }

    token ip_parser::next_tok() {
        if (eof())  return token(token_kind::END);
        if (cur == '\0') next_char();
        
        const std::size_t BUF_SIZE = 64;
        char digits[BUF_SIZE];

        if (get_char() == '.')
        {
            next_char();
            return token(token_kind::DOT);
        }
        else if (get_char() == ':')
        {
            next_char();
            if (get_char() == ':')
            {
                next_char();
                return token(token_kind::COLON2);
            }
            return token(token_kind::COLON);
        }
        else if (std::isxdigit( get_char() ) || std::isdigit( get_char() ))
        {
            bool is_hex = false;
            std::size_t digits_pos = 0;
            while(std::isxdigit( get_char() ) || std::isdigit( get_char() ))
            {
                if (digits_pos + 1 >= BUF_SIZE)
                    throw_except<illegal_argument_exception>("'%s' is not ip", str);
                if ((get_char() >= 'a' && get_char() <= 'f') || (get_char() >= 'A' && get_char() <= 'F'))
                    is_hex = true;
                digits[digits_pos++] = get_char();
                next_char();
            }

            assert(digits_pos < BUF_SIZE);
            digits[digits_pos] = '\0';

            token number (token_kind::NUMBER);
            number.hex = -1;
            number.dec = -1;
            if (!is_hex)
                number.dec = strtol(digits, NULL, 10);
            number.hex = strtol(digits, NULL, 16);
            
            return number;
        }
        
        next_char();
        
        return token(token_kind::UNDEFINED);
    }

    inet_address ip_parser::parse_v4() {

        uint8_t parts[4];
        std::size_t part = 0;

        for (std::size_t i = 0; i < 8; ++i)
        {
            const token& tok = next_tok();
            if (i == 7)
            {
                if (tok.kind != token_kind::END)
                    goto ERR;
                break;
            }
            
            if (i % 2 != 0)
            {
                if (tok.kind != token_kind::DOT)
                    goto ERR;
            }
            else
            {
                if ((tok.kind != token_kind::NUMBER) || (tok.dec < 0 || tok.dec > 255))
                    goto ERR;
                parts[part++] = (uint8_t) (tok.dec & 0xFF);
            }
            
            continue;
            ERR:
                throw_except<illegal_argument_exception>("Invalid ipv4 address '%s'", str);
        }
        
        return inet_address::as_ip4(parts, 4);
    }
    
    inet_address ip_parser::parse_v6() {
        uint16_t parts[8] = {0};
        
        int double_colon_pos = -1;
        int pos = 0;
        
        while (!eof())
        {
            token tok = next_tok();
            if (tok.kind == token_kind::COLON2)
            {
                if (double_colon_pos != -1)
                    throw_except<illegal_argument_exception>("Invalid ipv6 address '%s'", str);
                double_colon_pos = pos;
            }
            else if (tok.kind == token_kind::NUMBER)
            {
                parts[pos++] = (uint16_t) (tok.hex & 0xFFFF);
            }
        }

        if (double_colon_pos != -1)
        {
            int shift = 8 - pos;
            for (int i = pos - 1; i >= double_colon_pos; --i)
                parts[i + shift] = parts[i];
            for (int i = 0; i < shift; ++i)
                parts[double_colon_pos + i] = 0;
        }
        else
        {
            if (pos != 8)
                throw_except<illegal_argument_exception>("Invalid ipv6 address '%s'", str);
        }

        return inet_address::as_ip6(parts, 8);
    }

    inet_address ip_parser::parse(inet_family family) {
        if (family == inet_family::IPV4) return parse_v4();
        if (family == inet_family::IPV6) return parse_v6();
        throw make_except<illegal_argument_exception>("%d is not inet family", (int) family);
    }
    

} //namespace internal
} //namespace net
} //namespace jstd