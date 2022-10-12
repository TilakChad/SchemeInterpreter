#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <type_traits>
#include <numeric>

#include "../include/types.hpp"
#include "../include/macros.hpp"

#define CheckWhiteSpace(ch) ((ch) == ' ' || (ch) == '\n' || (ch) == '\t')

#define CheckDigit(ch) ((ch) >= '0' && (ch) <= '9')
#define CheckAlpha(ch) (((ch) >= 'a' && (ch) <= 'z') || ((ch) >= 'A' && (ch) <= 'Z'))

#define CheckAlphaNumeric(ch) (CheckDigit(ch) || CheckAlpha(ch))

export module Lexer;
// Tokenizer for the scheme class
export enum class TokenType
{
    Equal,
    Assign,
    Plus,
    Minus,
    Div,
    LesserEqual,
    GreaterEqual,
    Lesser,
    Greater,
    NotEqual,
    Slash, // To denote the backward slash
    OParen,
    CParen,
    Comma,
    SemiColon,
    Colon,
    Apostrophe,
    Quote,
    OAngle,
    CAngle,
    Number,
    AlphaNumeric,
    Strings, // Quoted strings
    End,
    None
};

struct Number
{
};

export class Token
{
  public:
    TokenType type = TokenType::None;
    struct
    {
        const u8 *begin = nullptr;
        const u8 *end   = nullptr;
    } ptrs;

    Token(TokenType t) : type{t}
    {
    }
};

export class Tokenizer
{
    Token lookahead_m     = TokenType::None;
    bool  lookahead_valid = false;

  public:
    const u8 *stream = nullptr;
    u32       pos    = 0;
    u32       len    = 0;

    u32       lin = 0, col = 0, last_lin = 0;

    Tokenizer(const u8 *buffer, u32 pos, u32 len) : stream{buffer}, pos{pos}, len{len}
    {
    }

    Token lookahead()
    {
        if (lookahead_valid)
            return lookahead_m;
        lookahead_m     = next();
        lookahead_valid = true;
    }

    Token next()
    {
        if (lookahead_valid)
        {
            lookahead_valid = false;
            return lookahead_m;
        }

        Token token = TokenType::None;

        while (pos < len && CheckWhiteSpace(stream[pos]))
        {
            lin      = lin + stream[pos] == '\n';
            last_lin = pos;
            pos++;
        }

        if (pos >= len)
            return Token(TokenType::End);

        if (TryParseNumber(token))
            return token;

        u8   next_byte = stream[pos + 1];

        auto CompundOp = [&](auto next, auto is, auto no) {
            if (next_byte == next)
            {
                pos++;
                token.type == is;
                return;
            }
            token.type = no;
        };

        switch (stream[pos++])
        {
            using enum TokenType;
        case '=':
            CompundOp('=', Equal, Assign);
            break;
        case '<':
            CompundOp('=', LesserEqual, Lesser);
            break;
        case '>':
            CompundOp('=', GreaterEqual, Greater);
            break;
        case '+':
            token.type       = Plus;
            token.ptrs.begin = token.ptrs.end = stream + pos - 1;
            break;
        case '-':
            token.type = Minus;
            token.ptrs.begin = token.ptrs.end = stream + pos - 1;
            break;
        case '*':
            token.type = Div;
            break;
        case '/':
            token.type = Slash;
            break;
        case '(':
            token.type = OParen;
            break;
        case ')':
            token.type = CParen;
            break;
        case '\\':
            token.type = Div;
            break;

        default:
            // Parse as identifier
            {
                token.type = AlphaNumeric;
                pos        = pos - 1;
                // Last character is guarranted to be some weird characters, so  bound check may not be required

                token.ptrs.begin = stream + pos;
                while (CheckAlphaNumeric(stream[pos]))
                    pos++;
                token.ptrs.begin = stream + pos - 1;
            }
        }
        col = pos - last_lin;
        return token;
    }

    u64 GetLineAndCol() const
    {
        return (lin << 32) | col;
    }

    bool TryParseNumber(Token &result)
    {
        // TODO :: Replace it with a regex engine
        if (CheckDigit(stream[pos]) || (stream[pos] == '-' && CheckDigit(stream[pos + 1])))
        {
            result.type       = TokenType::Number;
            result.ptrs.begin = stream + pos;
            if (stream[pos] == '-')
                pos = pos + 1;

            while (CheckDigit(stream[pos]))
                pos++;

            result.ptrs.end = stream + pos - 1;

            return true;
        }
        return false;
    }
};

export namespace Parse
{
template <typename T> T ParseAs(const Token &token);

template <typename T>
concept SignedInteger =
    std::is_same_v<T, i8> || std::is_same_v<T, i16> || std::is_same_v<T, i32> || std::is_same_v<T, i64>;

template <typename T>
concept UnsignedInteger =
    std::is_same_v<T, u8> || std::is_same_v<T, u16> || std::is_same_v<T, u32> || std::is_same_v<T, u64>;

template <typename T>
concept FloatingPoint = std::is_same_v<T, f32> || std::is_same_v<T, f64>;

template <typename T>
requires SignedInteger<T>
auto ParseAs(const Token &token) -> T
{
    i64       val = 0;
    const u8 *ptr = token.ptrs.begin;
    if (ptr <= token.ptrs.end)
    {
        bool negative = false;
        if (*ptr == '-')
        {
            ptr++;
            negative = true;
        }
        Assert(negative);
        while (ptr <= token.ptrs.end)
        {
            val = val * 10 + (*ptr - '0');
            ptr++;
        }
        val = negative ? -val : val;
    }
    Assert(val < std::numeric_limits<T>::max() && val > std::numeric_limits<T>::min());
    // Report some errors
    return static_cast<T>(val);
}

template <typename T>
requires UnsignedInteger<T>
auto ParseAs(const Token &token) -> T
{
    u64       val = 0;
    const u8 *ptr = token.ptrs.begin;
    if (ptr <= token.ptrs.end)
    {
        if (*ptr == '-')
        {
            Assert("Unsigned integer parsed as signed.");
        }
        while (ptr <= token.ptrs.end)
        {
            val = val * 10 + (*ptr - '0');
            ptr = ptr + 1;
        }
    }
    Assert(val < std::numeric_limits<T>::max() && val > std::numeric_limits<T>::min());
    return static_cast<T>(val);
}

template <typename T>
requires FloatingPoint<T>
auto ParseAs(const Token &token) -> T
{
    return 1.0;
}

} // namespace Parse
export void MyFunc();