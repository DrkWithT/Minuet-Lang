#ifndef LEXING_HPP
#define LEXING_HPP

#include <concepts>
#include <string_view>
#include <unordered_map>

#include "frontend/lexicals.hpp"

namespace Minuet::Frontend::Lexing {
    namespace Helpers {
        [[nodiscard]] constexpr auto match_discrete(std::same_as<char> auto target, std::same_as<char> auto first, std::same_as<char> auto ... rest) -> bool {
            return ((target == first) || ... || (target == rest));
        }

        [[nodiscard]] constexpr auto match_spaces(char c) -> bool {
            return match_discrete(c, ' ', '\t', '\r');
        }

        [[nodiscard]] constexpr auto match_alpha(char c) -> bool {
            return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
        }

        [[nodiscard]] constexpr auto match_digit(char c) -> bool {
            return c >= '0' && c <= '9';
        }

        [[nodiscard]] constexpr auto match_numeric(char c) -> bool {
            return match_digit(c) || c == '.';
        }

        [[nodiscard]] constexpr auto match_alphanum(char c) -> bool {
            return match_digit(c) || match_alpha(c);
        }

        [[nodiscard]] constexpr auto match_operator(char c) -> bool {
            return match_discrete(c, '*', '/', '%', '+', '-', '!', ':', '=', '<', '>');
        }
    }

    class Lexer {
    public:
        Lexer(std::string_view source) noexcept;

        void add_lexical_item(const Lexicals::LexicalEntry& entry);
        [[nodiscard]] auto operator()(std::string_view sv) -> Lexicals::Token;

    private:
        [[nodiscard]] auto at_eof() const -> bool;
        void update_src_location(char c);

        [[nodiscard]] auto lex_single(Lexicals::TokenType tag, std::string_view sv) noexcept -> Lexicals::Token;
        [[nodiscard]] auto lex_between(char delim, Lexicals::TokenType tag, std::string_view sv) noexcept -> Lexicals::Token;
        [[nodiscard]] auto lex_spaces(std::string_view sv) noexcept -> Lexicals::Token;
        [[nodiscard]] auto lex_numeric(std::string_view sv) noexcept -> Lexicals::Token;
        [[nodiscard]] auto lex_operator(std::string_view sv) noexcept -> Lexicals::Token;
        [[nodiscard]] auto lex_word(std::string_view sv) noexcept -> Lexicals::Token;

        std::unordered_map<std::string, Lexicals::TokenType> m_items;
        uint32_t m_pos;
        uint32_t m_end;
        uint16_t m_line;
        uint16_t m_col;
    };
}

#endif