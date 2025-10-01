#include <iostream>
#include <print>
#include "driver/sources.hpp"
#include "frontend/lexicals.hpp"
#include "driver/driver.hpp"

namespace Minuet::Driver::Compilation {
    CompileDriver::CompileDriver()
    : m_lexer {""} {
        using Frontend::Lexicals::TokenType;

        m_lexer.add_lexical_item({.text = "fun", .tag = TokenType::keyword_fun});
        m_lexer.add_lexical_item({.text = "def", .tag = TokenType::keyword_def});
        m_lexer.add_lexical_item({.text = "match", .tag = TokenType::keyword_match});
        m_lexer.add_lexical_item({.text = "pat", .tag = TokenType::keyword_pat});
        m_lexer.add_lexical_item({.text = "_", .tag = TokenType::keyword_discard});
        m_lexer.add_lexical_item({.text = "*", .tag = TokenType::oper_times});
        m_lexer.add_lexical_item({.text = "/", .tag = TokenType::oper_slash});
        m_lexer.add_lexical_item({.text = "%", .tag = TokenType::oper_modulo});
        m_lexer.add_lexical_item({.text = "+", .tag = TokenType::oper_plus});
        m_lexer.add_lexical_item({.text = "-", .tag = TokenType::oper_minus});
        m_lexer.add_lexical_item({.text = "==", .tag = TokenType::oper_equality});
        m_lexer.add_lexical_item({.text = "!=", .tag = TokenType::oper_inequality});
        m_lexer.add_lexical_item({.text = "<", .tag = TokenType::oper_lesser});
        m_lexer.add_lexical_item({.text = ">", .tag = TokenType::oper_greater});
        m_lexer.add_lexical_item({.text = "<=", .tag = TokenType::oper_at_most});
        m_lexer.add_lexical_item({.text = ">=", .tag = TokenType::oper_at_least});
        m_lexer.add_lexical_item({.text = ":=", .tag = TokenType::oper_assign});
    }

    [[nodiscard]] bool CompileDriver::parse_sources(const std::filesystem::path& main_path) {
        using Frontend::Lexicals::Token;
        using Frontend::Lexicals::TokenType;
        using Frontend::Lexicals::token_to_sv;

        std::string main_src {Driver::Sources::read_source(main_path)};

        if (main_src.empty()) {
            return false;
        }

        Frontend::Lexicals::Token temp;

        do {
            temp = m_lexer(main_src);

            if (temp.type == Frontend::Lexicals::TokenType::unknown) {
                std::println(std::cerr, "Error: unknown token '{}' at source {}:{}", token_to_sv(temp, main_src), temp.line, temp.col);

                return false;
            }
        } while (temp.type != Frontend::Lexicals::TokenType::eof);

        return true;
    }
}
