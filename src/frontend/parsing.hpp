#ifndef MINUET_FRONTEND_PARSING_HPP
#define MINUET_FRONTEND_PARSING_HPP

#include <string_view>
#include <expected>
#include <stack>

#include "frontend/lexicals.hpp"
#include "frontend/lexing.hpp"
#include "syntax/ast.hpp"
#include "driver/utils.hpp"

namespace Minuet::Frontend::Parsing {
    void report_error(const Lexicals::Token& token, std::string_view src, std::string_view msg);

    class Parser {
    private:
        Lexicals::Token m_previous;
        Lexicals::Token m_current;
        int m_error_count;

        static auto match(const Lexicals::Token& token, std::same_as<Lexicals::TokenType> auto first, std::same_as<Lexicals::TokenType> auto ... rest) -> bool {
            const auto curr_tag = token.type;

            return ((curr_tag == first) || ... || (curr_tag == rest));
        }

        [[nodiscard]] auto advance(Lexing::Lexer& lexer, std::string_view src) -> Lexicals::Token;
        void consume(Lexing::Lexer& lexer, std::string_view src);

        void consume(Lexing::Lexer& lexer, std::string_view src, std::same_as<Lexicals::TokenType> auto first, std::same_as<Lexicals::TokenType> auto ... rest) {
            if (match(m_current, first, rest...)) {
                consume(lexer, src);
                return;
            }

            report_error(m_current, src, "Unexpected token.");
            recover(lexer, src);
        }

        void skip_terminators(Lexing::Lexer& lexer, std::string_view src);

        void recover(Lexing::Lexer& lexer, std::string_view src);

        [[nodiscard]] auto parse_literal(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_primary(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_lambda(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_lhs(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_call(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_unary(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_factor(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_term(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_equality(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_compare(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;
        [[nodiscard]] auto parse_assign(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr;

        [[nodiscard]] auto parse_expr_stmt(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_definition(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_if(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        // [[nodiscard]] auto parse_match_case(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        // [[nodiscard]] auto parse_match(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_return(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_while(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_break(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_block(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_function(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_native_stub(Lexing::Lexer& lexer, std::string_view source) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_import(Lexing::Lexer& lexer, std::string_view src, std::stack<Driver::Utils::PendingSource>& pending_srcs, uint32_t& src_counter) -> Syntax::Stmts::StmtPtr;
        [[nodiscard]] auto parse_program(Lexing::Lexer& lexer, std::string_view src, std::stack<Driver::Utils::PendingSource>& pending_srcs, uint32_t& src_counter) -> std::optional<Syntax::AST::UnitAST>;

    public:
        Parser();

        [[nodiscard]] auto operator()(Lexing::Lexer& lexer, std::string_view src, std::stack<Driver::Utils::PendingSource>& pending_srcs, uint32_t& src_counter) -> std::optional<Syntax::AST::UnitAST>;
    };
}

#endif
