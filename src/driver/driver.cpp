#include "driver/sources.hpp"
#include "frontend/parsing.hpp"
#include <__ostream/print.h>
#include <iostream>
#include "driver/driver.hpp"

namespace Minuet::Driver::Compilation {
    CompileDriver::CompileDriver()
    : m_lexer {} {
        using Frontend::Lexicals::TokenType;

        m_lexer.add_lexical_item({.text = "fun", .tag = TokenType::keyword_fun});
        m_lexer.add_lexical_item({.text = "def", .tag = TokenType::keyword_def});
        m_lexer.add_lexical_item({.text = "if", .tag = TokenType::keyword_if});
        m_lexer.add_lexical_item({.text = "else", .tag = TokenType::keyword_else});
        m_lexer.add_lexical_item({.text = "match", .tag = TokenType::keyword_match});
        m_lexer.add_lexical_item({.text = "pat", .tag = TokenType::keyword_pat});
        m_lexer.add_lexical_item({.text = "_", .tag = TokenType::keyword_discard});
        m_lexer.add_lexical_item({.text = "return", .tag = TokenType::keyword_return});
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
        m_lexer.add_lexical_item({.text = "=", .tag = TokenType::oper_assign});
        m_lexer.add_lexical_item({.text = "=>", .tag = TokenType::arrow});
    }

    [[nodiscard]] bool CompileDriver::parse_sources(const std::filesystem::path& main_path) {
        using Frontend::Lexicals::Token;
        using Frontend::Lexicals::TokenType;
        using Frontend::Lexicals::token_to_sv;
        using Frontend::Parsing::Parser;

        std::string main_src {Driver::Sources::read_source(main_path)};

        if (main_src.empty()) {
            return false;
        }

        // reset lexer state with given source
        m_lexer.reset_with_src(main_src);

        Parser parser;
        auto parsed_unit = parser(m_lexer, main_src);

        if (const auto err_count = parsed_unit.error_or(0); err_count > 0) {
            std::println(std::cerr, "Parsing failed with {} errors.", err_count);
        }

        return parsed_unit.has_value();
    }
}
