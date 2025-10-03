#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <cstdint>
#include <string_view>

namespace Minuet::Frontend::Lexicals {
    enum class TokenType {
        unknown,
        spaces,
        comment,
        keyword_import,
        keyword_fun,
        keyword_def,
        keyword_if,
        keyword_else,
        keyword_match,
        keyword_pat,
        keyword_discard,
        keyword_return,
        identifier,
        literal_false,
        literal_true,
        literal_int,
        literal_double,
        literal_string,
        oper_times,
        oper_slash,
        oper_modulo,
        oper_plus,
        oper_minus,
        oper_equality,
        oper_inequality,
        oper_lesser,
        oper_greater,
        oper_at_least,
        oper_at_most,
        oper_assign,
        open_bracket,
        close_bracket,
        open_brace,
        close_brace,
        open_paren,
        close_paren,
        comma,
        colon,
        dot,
        arrow,
        eof,
    };

    /**
     * @brief For lexer configuration- Stores a lexeme of a pre-determined keyword, operator, etc.
     */
    struct LexicalEntry {
        std::string_view text;
        TokenType tag;
    };

    /**
     * @brief Stores information for a piece of source text, including its lexical type, bounds, and source location.
     */
    struct Token {
        TokenType type;
        uint32_t start;
        uint32_t end;
        uint16_t line;
        uint16_t col;
    };

    /// @note Result type for a token's span over its source.
    struct TokenSpan {
        uint32_t start;
        uint32_t end;
    };

    /// @note Result type for a token's lexical location info.
    struct TokenLocation {
        uint16_t line;
        uint16_t col;
    };

    [[nodiscard]] constexpr auto token_length(const Token& token) -> uint32_t {
        return token.end - token.start + 1;
    }

    [[nodiscard]] constexpr auto token_location(const Token& token) -> TokenLocation {
        return {
            .line = token.line,
            .col = token.col,
        };
    }

    [[nodiscard]] constexpr auto token_span(const Token& token) -> TokenSpan {
        return {
            .start = token.start,
            .end = token.end,
        };
    }

    [[nodiscard]] constexpr auto token_to_sv(const Token& token, std::string_view src_view) -> std::string_view {
        return src_view.substr(token.start, token_length(token));
    }
}

#endif