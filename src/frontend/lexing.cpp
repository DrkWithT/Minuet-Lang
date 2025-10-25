#include <format>
#include <string>

#include "frontend/lexicals.hpp"
#include "frontend/lexing.hpp"

namespace Minuet::Frontend::Lexing {
    Lexer::Lexer() noexcept
    : m_items {}, m_pos {0U}, m_end {0U}, m_line {1}, m_col {1} {}

    void Lexer::add_lexical_item(const Lexicals::LexicalEntry& entry) {
        m_items[std::format("{}", entry.text)] = entry.tag;
    }

    void Lexer::reset_with_src(std::string_view next_src_v) {
        m_pos = 0U;
        m_end = next_src_v.length();
    }

    [[nodiscard]] auto Lexer::operator()(std::string_view sv) -> Lexicals::Token {
        if (at_eof()) {
            return Lexicals::Token {
                .type = Lexicals::TokenType::eof,
                .start = m_end,
                .end = m_end,
                .line = m_line,
                .col = m_col,
            };
        }

        const auto peek_0 = sv[m_pos];

        if (Helpers::match_spaces(peek_0)) {
            return lex_spaces(sv);
        }

        switch (peek_0) {
            case '\0': return lex_single(Lexicals::TokenType::eof, sv);
            case '#': return lex_between(peek_0, Lexicals::TokenType::comment, sv);
            case '\"': return lex_between(peek_0, Lexicals::TokenType::literal_string, sv);
            case '[': return lex_single(Lexicals::TokenType::open_bracket, sv);
            case ']': return lex_single(Lexicals::TokenType::close_bracket, sv);
            case '{': return lex_single(Lexicals::TokenType::open_brace, sv);
            case '}': return lex_single(Lexicals::TokenType::close_brace, sv);
            case '(': return lex_single(Lexicals::TokenType::open_paren, sv);
            case ')': return lex_single(Lexicals::TokenType::close_paren, sv);
            case ',': return lex_single(Lexicals::TokenType::comma, sv);
            case ':': return lex_single(Lexicals::TokenType::colon, sv);
            case '.': return lex_single(Lexicals::TokenType::dot, sv);
            default:
                break;
        }

        if (Helpers::match_alpha(peek_0)) {
            return lex_word(sv);
        } else if (Helpers::match_digit(peek_0)) {
            return lex_numeric(sv);
        } else if (Helpers::match_operator(peek_0)) {
            return lex_operator(sv);
        }

        return lex_single(Lexicals::TokenType::unknown, sv);
    }

    [[nodiscard]] auto Lexer::at_eof() const -> bool {
        return m_pos >= m_end;
    }

    void Lexer::update_src_location(char c) {
        if (c == '\n') {
            ++m_line;
            m_col = 1;
        } else {
            ++m_col;
        }
    }

    [[nodiscard]] auto Lexer::lex_single(Lexicals::TokenType tag, std::string_view sv) noexcept -> Lexicals::Token {
        const auto temp_begin = m_pos;
        const auto temp_ln = m_line;
        const auto temp_col = m_col;

        update_src_location(sv[temp_begin]);
        ++m_pos;

        return Lexicals::Token {
            .type = tag,
            .start = temp_begin,
            .end = temp_begin,
            .line = temp_ln,
            .col = temp_col
        };
    }

    [[nodiscard]] auto Lexer::lex_between(char delim, Lexicals::TokenType tag, std::string_view sv) noexcept -> Lexicals::Token {
        update_src_location(sv[m_pos]);
        ++m_pos;

        const auto temp_begin = m_pos;
        auto temp_end = m_pos;
        const auto temp_ln = m_line;
        const auto temp_col = m_col;
        auto closed = false;

        while (!at_eof()) {
            const auto c = sv[m_pos];

            update_src_location(c);

            if (c == delim) {
                --temp_end;
                ++m_pos;
                closed = true;
                break;
            }

            ++m_pos;
            ++temp_end;
        }

        const auto checked_tag = (closed) ? tag : Lexicals::TokenType::unknown ;

        return Lexicals::Token {
            .type = checked_tag,
            .start = temp_begin,
            .end = temp_end,
            .line = temp_ln,
            .col = temp_col,
        };
    }

    [[nodiscard]] auto Lexer::lex_spaces(std::string_view sv) noexcept -> Lexicals::Token {
        const auto temp_begin = m_pos;
        auto temp_end = m_pos;
        const auto temp_ln = m_line;
        const auto temp_col = m_col;

        while (!at_eof()) {
            const auto c = sv[m_pos];

            if (!Helpers::match_spaces(c)) {
                --temp_end;
                break;
            }
            
            update_src_location(c);
            ++m_pos;
            ++temp_end;
        }

        return Lexicals::Token {
            .type = Lexicals::TokenType::spaces,
            .start = temp_begin,
            .end = temp_end,
            .line = temp_ln,
            .col = temp_col,
        };
    }

    [[nodiscard]] auto Lexer::lex_numeric(std::string_view sv) noexcept -> Lexicals::Token {
        const auto temp_begin = m_pos;
        auto temp_end = m_pos;
        const auto temp_ln = m_line;
        const auto temp_col = m_col;
        auto dots = 0;
        auto minuses = 0;

        while (!at_eof()) {
            const auto c = sv[m_pos];

            if (!Helpers::match_numeric(c) && c != '-') {
                --temp_end;
                break;
            }

            if (c == '.') {
                ++dots;
            } else if (c == '-') {
                ++minuses;
            }

            update_src_location(c);
            ++m_pos;
            ++temp_end;
        }

        const auto checked_tag = ([](int dot_count, int minus_count) {
            if (minus_count > 1) {
                return Lexicals::TokenType::unknown;
            }

            switch (dot_count) {
                case 0: return Lexicals::TokenType::literal_int;
                case 1: return Lexicals::TokenType::literal_double;
                default: return Lexicals::TokenType::unknown;
            }
        })(dots, minuses);

        return Lexicals::Token {
            .type = checked_tag,
            .start = temp_begin,
            .end = temp_end,
            .line = temp_ln,
            .col = temp_col,
        };
    }

    [[nodiscard]] auto Lexer::lex_operator(std::string_view sv) noexcept -> Lexicals::Token {
        const auto temp_begin = m_pos;
        auto temp_end = m_pos;
        const auto temp_ln = m_line;
        const auto temp_col = m_col;

        while (!at_eof()) {
            const auto c = sv[m_pos];

            if (!Helpers::match_operator(c)) {
                --temp_end;
                break;
            }

            update_src_location(c);
            ++m_pos;
            ++temp_end;
        }

        std::string lexeme = std::format("{}", sv.substr(temp_begin, temp_end - temp_begin + 1));
        const auto checked_tag = (m_items.contains(lexeme)) ? m_items[lexeme] : Lexicals::TokenType::unknown ;

        return Lexicals::Token {
            .type = checked_tag,
            .start = temp_begin,
            .end = temp_end,
            .line = temp_ln,
            .col = temp_col,
        };
    }

    [[nodiscard]] auto Lexer::lex_word(std::string_view sv) noexcept -> Lexicals::Token {
        const auto temp_begin = m_pos;
        auto temp_end = m_pos;
        const auto temp_ln = m_line;
        const auto temp_col = m_col;

        while (!at_eof()) {
            const auto c = sv[m_pos];

            if (!Helpers::match_alphanum(c)) {
                --temp_end;
                break;
            }

            update_src_location(c);
            ++m_pos;
            ++temp_end;
        }

        std::string lexeme = std::format("{}", sv.substr(temp_begin, temp_end - temp_begin + 1));
        const auto checked_tag = (m_items.contains(lexeme)) ? m_items[lexeme] : Lexicals::TokenType::identifier ;

        return Lexicals::Token {
            .type = checked_tag,
            .start = temp_begin,
            .end = temp_end,
            .line = temp_ln,
            .col = temp_col,
        };
    }
}
