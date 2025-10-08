#include <set>
#include <stack>

#include "ir/convert_ast.hpp"
#include "ir/printing.hpp"
#include "codegen/emitter.hpp"
#include "driver/sources.hpp"
#include "driver/driver.hpp"

namespace Minuet::Driver::Compilation {
    using Frontend::Lexicals::TokenType;
    using Frontend::Parsing::Parser;
    using Syntax::AST::SourcedAST;
    using Syntax::AST::FullAST;
    using IR::CFG::FullIR;
    using IR::Convert::ASTConversion;
    using IR::Printing::print_ir;
    using Driver::Sources::read_source;

    CompileDriver::CompileDriver()
    : m_lexer {}, m_src_map {} {
        m_lexer.add_lexical_item({.text = "import", .tag = TokenType::keyword_import});
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

    auto CompileDriver::parse_sources(const std::filesystem::path& main_path) -> std::optional<FullAST> {
        std::set<int> visited_src_ids;
        std::stack<Utils::PendingSource> sources_frontier;
        FullAST full_ast;
        auto temp_src_id = 0U;

        sources_frontier.emplace(Utils::PendingSource {
            .file_path = main_path,
            .src_id = temp_src_id,
        });

        while (!sources_frontier.empty()) {
            const auto [next_src_path, next_src_id] = sources_frontier.top();

            sources_frontier.pop();

            if (visited_src_ids.contains(next_src_id)) {
                continue;
            }

            std::string src_text {read_source(next_src_path)};

            m_src_map.push_back(src_text);
            m_lexer.reset_with_src(src_text);

            {
                const auto source_unit_id = next_src_id;
                Parser parser;
                auto expected_parse_result = parser(m_lexer, src_text, sources_frontier, temp_src_id);

                if (!expected_parse_result.has_value()) {
                    return {};
                }

                for (auto& temp_ast : expected_parse_result.value()) {
                    full_ast.emplace_back(SourcedAST {
                        .stmt_p = std::exchange(temp_ast, {}),
                        .src_id = source_unit_id,
                    });
                }
            }

            visited_src_ids.insert(next_src_id);
        }

        return full_ast;
    }

    auto CompileDriver::generate_ir(const FullAST& ast) -> std::optional<FullIR> {
        ASTConversion ir_generator;

        auto ir_opt = ir_generator(ast, m_src_map);

        if (!ir_opt) {
            return {};
        }

        return ir_opt.value();
    }

    [[maybe_unused]] auto CompileDriver::generate_program(IR::CFG::FullIR& ir) -> std::optional<Runtime::Code::Program> {
        Codegen::Emitter emitter;

        return emitter(ir);
    }

    auto CompileDriver::operator()(const std::filesystem::path& entry_source_path) -> std::optional<Runtime::Code::Program> {
        auto parsed_program = parse_sources(entry_source_path);

        if (!parsed_program) {
            return {};
        }

        auto program_ir_opt = generate_ir(parsed_program.value());

        if (!program_ir_opt) {
            return {};
        }

        /// NOTE: print the IR to check its correctness!
        print_ir(program_ir_opt.value());

        return generate_program(program_ir_opt.value());
    }
}
