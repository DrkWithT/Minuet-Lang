#include <stdexcept>
#include <utility>
#include <memory>
#include <vector>
#include <iostream>
#include <print>

#include "frontend/lexicals.hpp"
#include "syntax/ast.hpp"
#include "syntax/exprs.hpp"
#include "frontend/parsing.hpp"

namespace Minuet::Frontend::Parsing {
    using Lexicals::TokenType;
    using Lexicals::Token;
    using Lexicals::token_span;
    using Lexicals::token_location;
    using Lexicals::token_length;
    using Lexicals::token_to_sv;
    using Syntax::Exprs::ExprPtr;
    using Syntax::Exprs::Expr;
    using Syntax::Stmts::StmtPtr;
    using Syntax::Stmts::Stmt;
    using Semantics::Enums::Operator;

    void report_error(const Lexicals::Token& token, std::string_view src, std::string_view msg) {
        const auto [culprit_ln, culprit_col] = token_location(token);
        const auto culprit_beg = token.start;
        const auto culprit_len = token_length(token);
        std::string_view culprit_txt = src.substr(culprit_beg, culprit_len);

        throw std::runtime_error {std::format("\033[1;31mParse Error\033[0m at \033[1;33msource[{}:{}]\033[0m:\n\nCulprit: '{}'\nNote: {}\n", culprit_ln, culprit_col, culprit_txt, msg)};
    }

    auto Parser::advance(Lexing::Lexer& lexer, std::string_view src) -> Token {
        Token temp {};

        do {
            temp = lexer(src);

            if (temp.type == TokenType::spaces || temp.type == TokenType::comment) {
                continue;
            }

            if (temp.type == TokenType::unknown) {
                const auto culprit_ln = temp.line;
                const auto culprit_col = temp.col;
                std::string_view culprit_txt = src.substr(temp.start, token_length(temp));

                throw std::runtime_error {std::format("\033[1;31mParse Error\033[0m at \033[1;33msource[{}:{}]\033[0m:\n\nCulprit: '{}'\nNote: {}\n", culprit_ln, culprit_col, culprit_txt, "Invalid token.")};
            }

            break;
        } while (true);

        return temp;
    }

    void Parser::consume(Lexing::Lexer& lexer, std::string_view src) {
        m_previous = m_current;
        m_current = advance(lexer, src);
    }

    void Parser::recover(Lexing::Lexer& lexer, std::string_view src) {
        ++m_error_count;

        while (!match(m_current, TokenType::eof)) {
            if (match(m_current, TokenType::keyword_fun)) {
                break;
            }

            consume(lexer, src);
        }
    }

    auto Parser::parse_literal(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Exprs::ExprPtr {
        const auto temp_token_copy = m_current;
        const auto [literal_beg, literal_end] = token_span(temp_token_copy);

        if (match(temp_token_copy, TokenType::literal_false, TokenType::literal_true, TokenType::literal_int, TokenType::literal_double, TokenType::literal_string)) {
            consume(lexer, src);

            return std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Literal {
                    .token = temp_token_copy,
                },
                .src_begin = literal_beg,
                .src_end = literal_end,
            });
        }

        report_error(temp_token_copy, src, "Invalid literal.");

        return {};
    }

    auto Parser::parse_primary(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto temp_token_copy = m_current;

        if (match(temp_token_copy, Lexicals::TokenType::identifier)) {
            consume(lexer, src);

            return std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Literal {
                    .token = temp_token_copy,
                },
                .src_begin = m_current.start,
                .src_end = m_current.end,
            });
        } else if (match(temp_token_copy, TokenType::open_bracket)) {
            return parse_lambda(lexer, src);
        } else if (match(temp_token_copy, TokenType::open_paren)) {
            consume(lexer, src);
            auto wrapped_expr = parse_compare(lexer, src);
            consume(lexer, src, TokenType::close_paren);

            return wrapped_expr;
        } else {
            return parse_literal(lexer, src);
        }
    }

    auto Parser::parse_lambda(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        std::vector<Token> params;
        const auto lambda_beg = m_current.start;

        consume(lexer, src, TokenType::open_bracket);

        if (!match(m_current, TokenType::close_bracket)) {
            consume(lexer, src, TokenType::identifier);
            params.emplace_back(m_previous);
        }

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::comma)) {
                break;
            }

            consume(lexer, src);
            consume(lexer, src, TokenType::identifier);

            params.emplace_back(m_previous);
        }

        consume(lexer, src, TokenType::close_bracket);
        consume(lexer, src, TokenType::arrow);

        auto lambda_body = parse_block(lexer, src);
        const auto lambda_end = m_current.start;

        return std::make_unique<Expr>(Expr {
            .data = Syntax::Exprs::Lambda {
                .params = std::move(params),
                .body = std::move(lambda_body),
            },
            .src_begin = lambda_beg,
            .src_end = lambda_end,
        });
    }

    auto Parser::parse_lhs(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto lhs_expr = parse_primary(lexer, src);

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::dot)) {
                break;
            }

            consume(lexer, src);

            auto rhs_expr = parse_call(lexer, src);
            const auto expr_end = m_current.start;

            lhs_expr = std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Binary {
                    .left = std::move(lhs_expr),
                    .right = std::move(rhs_expr),
                    .op = Operator::access,
                },
                .src_begin = expr_begin,
                .src_end = expr_end,
            });
        }

        return lhs_expr;
    }

    auto Parser::parse_call(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto lhs_expr = parse_lhs(lexer, src);
        
        if (!match(m_current, TokenType::open_paren)) {
            return lhs_expr;
        }

        consume(lexer, src);

        std::vector<ExprPtr> args;

        if (!match(m_current, TokenType::close_paren)) {
            args.emplace_back(parse_compare(lexer, src));
        }

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::comma)) {
                break;
            }

            consume(lexer, src);
            args.emplace_back(parse_compare(lexer, src));
        }

        const auto expr_end = m_current.start;
        consume(lexer, src, TokenType::close_paren);

        return std::make_unique<Expr>(Expr {
            .data = Syntax::Exprs::Call {
                .callee = std::move(lhs_expr),
                .args = std::move(args),
            },
            .src_begin = expr_begin,
            .src_end = expr_end,
        });
    }

    auto Parser::parse_unary(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto is_negated = false;

        if (match(m_current, TokenType::oper_minus)) {
            consume(lexer, src);
            is_negated = true;
        }

        auto inner_expr = parse_call(lexer, src);
        const auto expr_end = m_current.start;

        if (!is_negated) {
            return inner_expr;
        }

        return std::make_unique<Expr>(Expr {
            .data = Syntax::Exprs::Unary {
                .inner = std::move(inner_expr),
                .op = Operator::negate,
            },
            .src_begin = expr_begin,
            .src_end = expr_end,
        });
    }

    auto Parser::parse_factor(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto factor_expr = parse_unary(lexer, src);

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::oper_times, TokenType::oper_slash, TokenType::oper_modulo)) {
                break;
            }

            const auto checked_op = ([](TokenType tag) {
                switch (tag) {
                case TokenType::oper_times:
                    return Operator::mul;
                case TokenType::oper_slash:
                    return Operator::div;
                case TokenType::oper_modulo:
                default:
                    return Operator::modulo;
                }
            })(m_current.type);

            consume(lexer, src);

            auto other_operand_expr = parse_unary(lexer, src);
            const auto expr_end = m_current.start;

            factor_expr = std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Binary {
                    .left = std::move(factor_expr),
                    .right = std::move(other_operand_expr),
                    .op = checked_op,
                },
                .src_begin = expr_begin,
                .src_end = expr_end,
            });
        }

        return factor_expr;
    }

    auto Parser::parse_term(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto term_expr = parse_factor(lexer, src);

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::oper_plus, TokenType::oper_minus)) {
                break;
            }

            const auto checked_op = (m_current.type == TokenType::oper_plus) ? Operator::add : Operator::sub ;

            consume(lexer, src);

            auto other_operand_expr = parse_factor(lexer, src);
            const auto expr_end = m_current.start;

            term_expr = std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Binary {
                    .left = std::move(term_expr),
                    .right = std::move(other_operand_expr),
                    .op = checked_op,
                },
                .src_begin = expr_begin,
                .src_end = expr_end,
            });
        }

        return term_expr;
    }

    auto Parser::parse_equality(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto equality_expr = parse_term(lexer, src);

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::oper_equality, TokenType::oper_inequality)) {
                break;
            }

            const auto checked_op = (m_current.type == TokenType::oper_equality) ? Operator::equality : Operator::inequality ;

            consume(lexer, src);

            auto other_operand_expr = parse_term(lexer, src);
            const auto expr_end = m_current.start;

            equality_expr = std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Binary {
                    .left = std::move(equality_expr),
                    .right = std::move(other_operand_expr),
                    .op = checked_op,
                },
                .src_begin = expr_begin,
                .src_end = expr_end,
            });
        }

        return equality_expr;
    }

    auto Parser::parse_compare(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto compare_expr = parse_equality(lexer, src);

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::oper_lesser, TokenType::oper_greater, TokenType::oper_at_most, TokenType::oper_at_least)) {
                break;
            }

            const auto checked_op = ([](TokenType tag) {
                switch (tag) {
                case TokenType::oper_lesser:
                    return Operator::lesser;
                case TokenType::oper_greater:
                    return Operator::greater;
                case TokenType::oper_at_most:
                    return Operator::at_most;
                default:
                    return Operator::at_least;
                }
            })(m_current.type);

            consume(lexer, src);

            auto other_operand_expr = parse_equality(lexer, src);
            const auto expr_end = m_current.start;

            compare_expr = std::make_unique<Expr>(Expr {
                .data = Syntax::Exprs::Binary {
                    .left = std::move(compare_expr),
                    .right = std::move(other_operand_expr),
                    .op = checked_op,
                },
                .src_begin = expr_begin,
                .src_end = expr_end,
            });
        }

        return compare_expr;
    }

    auto Parser::parse_assign(Lexing::Lexer& lexer, std::string_view src) -> ExprPtr {
        const auto expr_begin = m_current.start;
        auto lhs_expr = parse_unary(lexer, src);

        if (!match(m_current, TokenType::oper_assign)) {
            return lhs_expr;
        }

        consume(lexer, src);

        auto setting_expr = parse_compare(lexer, src);
        const auto expr_end = m_current.start;

        return std::make_unique<Expr>(Expr {
            .data = Syntax::Exprs::Assign {
                .left = std::move(lhs_expr),
                .value = std::move(setting_expr),
            },
            .src_begin = expr_begin,
            .src_end = expr_end,
        });
    }

    auto Parser::parse_expr_stmt(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;
        auto inner_expr = parse_assign(lexer, src);
        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::ExprStmt {
                .expr = std::move(inner_expr),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    auto Parser::parse_definition(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;
        consume(lexer, src, TokenType::keyword_def);

        auto name_token = m_current;

        consume(lexer, src, TokenType::identifier);
        consume(lexer, src, TokenType::oper_assign);

        auto init_expr = parse_compare(lexer, src);
        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::LocalDef {
                .name = name_token,
                .init_expr = std::move(init_expr),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    auto Parser::parse_if(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;
        consume(lexer, src, TokenType::keyword_if);

        auto cond_expr = parse_compare(lexer, src);
        auto if_body = parse_block(lexer, src);

        if (!match(m_current, TokenType::keyword_else)) {
            const auto stmt_end_no_else = m_current.start;

            return std::make_unique<Stmt>(Stmt {
                .data = Syntax::Stmts::If {
                    .cond_expr = std::move(cond_expr),
                    .if_body = std::move(if_body),
                    .else_body = {nullptr},
                },
                .src_begin = stmt_begin,
                .src_end = stmt_end_no_else,
            });
        }

        consume(lexer, src, TokenType::keyword_else);

        auto else_body = parse_block(lexer, src);
        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::If {
                .cond_expr = std::move(cond_expr),
                .if_body = std::move(if_body),
                .else_body = std::move(else_body),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    auto Parser::parse_return(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;
        consume(lexer, src, TokenType::keyword_return);

        auto result_expr = parse_compare(lexer, src);
        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::Return {
                .result = std::move(result_expr),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    [[nodiscard]] auto Parser::parse_while(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;
        consume(lexer, src, TokenType::keyword_while);

        auto check_expr = parse_compare(lexer, src);
        auto body_block = parse_block(lexer, src);
        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::While {
                .check = std::move(check_expr),
                .body = std::move(body_block),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    auto Parser::parse_break(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        consume(lexer, src, TokenType::keyword_break);

        return std::make_unique<Stmt>(Syntax::Stmts::Break {});
    }

    // auto Parser::parse_match_case(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;
    // auto Parser::parse_match(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr;

    auto Parser::parse_block(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;

        consume(lexer, src, TokenType::open_brace);

        std::vector<StmtPtr> stmts;

        while (!match(m_current, TokenType::eof)) {
            if (match(m_current, Lexicals::TokenType::close_brace)) {
                consume(lexer, src);
                break;
            }

            auto next_stmt = ([&lexer, &src, this](TokenType tag) {
                switch (tag) {
                case TokenType::keyword_def:
                    return parse_definition(lexer, src);
                case TokenType::keyword_if:
                    return parse_if(lexer, src);
                case TokenType::keyword_return:
                    return parse_return(lexer, src);
                case TokenType::keyword_while:
                    return parse_while(lexer, src);
                case TokenType::keyword_break:
                    return parse_break(lexer, src);
                default:
                    return parse_expr_stmt(lexer, src);
                }
            })(m_current.type);

            stmts.emplace_back(std::move(next_stmt));
        }

        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::Block {
                .items = std::move(stmts),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    auto Parser::parse_function(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stmt_begin = m_current.start;
        consume(lexer, src, TokenType::keyword_fun);
        consume(lexer, src, TokenType::identifier);

        auto name_token = m_previous;

        consume(lexer, src, TokenType::colon);
        consume(lexer, src, TokenType::open_bracket);

        std::vector<Token> params;

        if (!match(m_current, TokenType::close_bracket)) {
            consume(lexer, src, TokenType::identifier);
            params.emplace_back(m_previous);
        }

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::comma)) {
                break;
            }

            consume(lexer, src);
            consume(lexer, src, TokenType::identifier);

            params.emplace_back(m_previous);
        }

        consume(lexer, src, TokenType::close_bracket);
        consume(lexer, src, TokenType::arrow);

        auto body = parse_block(lexer, src);
        const auto stmt_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::Function {
                .params = std::move(params),
                .name = name_token,
                .body = std::move(body),
            },
            .src_begin = stmt_begin,
            .src_end = stmt_end,
        });
    }

    auto Parser::parse_native_stub(Lexing::Lexer& lexer, std::string_view src) -> Syntax::Stmts::StmtPtr {
        const auto stub_src_begin = m_current.start;

        consume(lexer, src, TokenType::keyword_native);
        consume(lexer, src, TokenType::keyword_fun);

        auto name_token = m_current;

        consume(lexer, src, TokenType::identifier);
        consume(lexer, src, TokenType::colon);
        consume(lexer, src, TokenType::open_bracket);

        std::vector<Token> params;

        if (!match(m_current, TokenType::close_bracket)) {
            consume(lexer, src, TokenType::identifier);
            params.emplace_back(m_previous);
        }

        while (!match(m_current, TokenType::eof)) {
            if (!match(m_current, TokenType::comma)) {
                break;
            }

            consume(lexer, src);
            consume(lexer, src, TokenType::identifier);

            params.emplace_back(m_previous);
        }

        consume(lexer, src, TokenType::close_bracket);
        const auto stub_src_end = m_current.start;

        return std::make_unique<Stmt>(Stmt {
            .data = Syntax::Stmts::NativeStub {
                .params = std::move(params),
                .name = name_token,
            },
            .src_begin = stub_src_begin,
            .src_end = stub_src_end,
        });
    }

    auto Parser::parse_import(Lexing::Lexer& lexer, std::string_view src, std::stack<Driver::Utils::PendingSource>& pending_srcs, uint32_t& src_counter) -> Syntax::Stmts::StmtPtr {
        consume(lexer, src, TokenType::keyword_import);
        consume(lexer, src, TokenType::literal_string);

        auto import_target_token = m_previous;
        std::string raw_target_str = std::format("{}", token_to_sv(import_target_token, src));

        ++src_counter;

        const auto target_src_id = src_counter;

        pending_srcs.push({
            .file_path = {raw_target_str},
            .src_id = target_src_id,
        });

        return std::make_unique<Stmt>(Syntax::Stmts::Import {
            .target = import_target_token,
        });
    }

    auto Parser::parse_program(Lexing::Lexer& lexer, std::string_view src, std::stack<Driver::Utils::PendingSource>& pending_srcs, uint32_t& src_counter) -> std::optional<Syntax::AST::UnitAST> {
        consume(lexer, src);

        Syntax::AST::UnitAST decls;

        while (!match(m_current, TokenType::eof)) {
            try {
                if (match(m_current, TokenType::keyword_import)) {
                    decls.emplace_back(parse_import(lexer, src, pending_srcs, src_counter));
                } else if (match(m_current, TokenType::keyword_native)) {
                    decls.emplace_back(parse_native_stub(lexer, src));
                } else if (match(m_current, TokenType::keyword_fun)) {
                    decls.emplace_back(parse_function(lexer, src));
                } else {
                    const auto culprit_ln = m_current.line;
                    const auto culprit_col = m_current.col;
                    std::string_view culprit_txt = src.substr(m_current.start, token_length(m_current));

                    throw std::runtime_error {std::format("\033[1;31mParse Error\033[0m at \033[1;33msource[{}:{}]\033[0m:\n\nCulprit: '{}'\nNote: {}\n", culprit_ln, culprit_col, culprit_txt, "Invalid token starting a top-level statement.")};
                }
            } catch (const std::runtime_error& parse_err) {
                std::println(std::cerr, "{}: {}", m_error_count, parse_err.what());
                recover(lexer, src);
            }
        }

        if (m_error_count > 0) {
            return {};
        }

        return decls;
    }

    Parser::Parser()
    : m_previous {}, m_current {}, m_error_count {0} {}

    [[nodiscard]] auto Parser::operator()(Lexing::Lexer& lexer, std::string_view src, std::stack<Driver::Utils::PendingSource>& pending_srcs, uint32_t& src_counter) -> std::optional<Syntax::AST::UnitAST> {
        return parse_program(lexer, src, pending_srcs, src_counter);
    }
}
