#include <set>
#include <stack>
#include <chrono>
#include <memory>
#include <iostream>

#include "semantics/analyzer.hpp"
#include "ir/convert_ast.hpp"
#include "bcgen/emitter.hpp"
#include "runtime/vm.hpp"
#include "driver/sources.hpp"
#include "driver/driver.hpp"

namespace Minuet::Driver {
    using Frontend::Lexicals::TokenType;
    using Frontend::Parsing::Parser;
    using Syntax::AST::SourcedAST;
    using Syntax::AST::FullAST;
    using IR::CFG::FullIR;
    using IR::Convert::ASTConversion;
    using Runtime::VM::Utils::EngineConfig;
    using Runtime::VM::Utils::ExecStatus;
    using Runtime::NativeProcTable;
    using Runtime::NativeProcRegistry;
    using Plugins::IRDumper;
    using Plugins::Disassembler;
    using Sources::read_source;

    static constexpr auto normal_vm_config = EngineConfig {
        .reg_buffer_limit = 8192,
        .call_frame_max = 512,
    };

    Driver::Driver()
    : m_lexer {}, m_src_map {}, m_native_procs {}, m_native_proc_ids {}, m_ir_printer {}, m_disassembler {} {
        m_lexer.add_lexical_item({.text = "true", .tag = TokenType::literal_true});
        m_lexer.add_lexical_item({.text = "false", .tag = TokenType::literal_false});
        m_lexer.add_lexical_item({.text = "fn", .tag = TokenType::keyword_fn});
        m_lexer.add_lexical_item({.text = "import", .tag = TokenType::keyword_import});
        m_lexer.add_lexical_item({.text = "fun", .tag = TokenType::keyword_fun});
        m_lexer.add_lexical_item({.text = "native", .tag = TokenType::keyword_native});
        m_lexer.add_lexical_item({.text = "def", .tag = TokenType::keyword_def});
        m_lexer.add_lexical_item({.text = "if", .tag = TokenType::keyword_if});
        m_lexer.add_lexical_item({.text = "else", .tag = TokenType::keyword_else});
        m_lexer.add_lexical_item({.text = "match", .tag = TokenType::keyword_match});
        m_lexer.add_lexical_item({.text = "pat", .tag = TokenType::keyword_pat});
        m_lexer.add_lexical_item({.text = "_", .tag = TokenType::keyword_discard});
        m_lexer.add_lexical_item({.text = "return", .tag = TokenType::keyword_return});
        m_lexer.add_lexical_item({.text = "while", .tag = TokenType::keyword_while});
        m_lexer.add_lexical_item({.text = "break", .tag = TokenType::keyword_break});
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

    auto Driver::register_native_proc(const Runtime::NativeProcItem& item) -> bool {
        const auto& [native_fn_name, native_fn_ptr] = item;
        const int next_native_fn_id = m_native_proc_ids.size();
        std::string key {native_fn_name.data()};

        if (m_native_proc_ids.contains(key)) {
            return false;
        }

        m_native_proc_ids[key] = next_native_fn_id;
        m_native_procs.emplace_back(native_fn_ptr);

        return true;
    }

    auto Driver::parse_sources(const std::filesystem::path& main_path) -> std::optional<FullAST> {
        std::set<std::string> visited_paths;
        std::stack<Utils::PendingSource> sources_frontier;
        FullAST full_ast;
        auto temp_src_id = 0U;

        sources_frontier.emplace(Utils::PendingSource {
            .file_path = main_path,
            .src_id = temp_src_id,
        });

        ++temp_src_id;

        while (!sources_frontier.empty()) {
            const auto [next_src_path, next_src_id] = sources_frontier.top();

            sources_frontier.pop();

            if (visited_paths.contains(next_src_path)) {
                continue;
            }

            std::string src_text {read_source(next_src_path)};

            m_src_map[next_src_id] = src_text;
            m_lexer.reset_with_src(src_text);

            {
                const auto source_unit_id = next_src_id;
                Parser parser;
                auto expected_parse_result = parser(m_lexer, src_text, sources_frontier, temp_src_id);

                if (!expected_parse_result) {
                    return {};
                }

                for (auto& temp_ast : expected_parse_result.value()) {
                    full_ast.emplace_back(SourcedAST {
                        .stmt_p = std::exchange(temp_ast, {}),
                        .src_id = source_unit_id,
                    });
                }
            }

            visited_paths.insert(next_src_path);
        }

        return full_ast;
    }

    auto Driver::check_semantics(const Syntax::AST::FullAST& ast) -> bool {
        Semantics::Analyzer analyzer;

        return analyzer(ast, m_src_map);
    }

    auto Driver::generate_ir(const FullAST& ast) -> std::optional<FullIR> {
        ASTConversion ir_generator {&m_native_proc_ids};

        auto ir_opt = ir_generator(ast, m_src_map);

        if (!ir_opt) {
            return {};
        }

        return ir_opt.value();
    }

    auto Driver::apply_ir_passes([[maybe_unused]] IR::CFG::FullIR& ir) -> bool {
        // IR::Pass::MovTrimmer opt_movs_pass;

        // for (const auto& cfg : ir.cfg_list) {
        //     if (!opt_movs_pass.apply(cfg)) {
        //         return false;
        //     }
        // }

        return true;
    }

    [[maybe_unused]] auto Driver::generate_program(IR::CFG::FullIR& ir) -> std::optional<Runtime::Code::Program> {
        Codegen::Emitter emitter;

        return emitter(ir);
    }

    void Driver::add_ir_dumper(IRDumper ir_printer) noexcept {
        m_ir_printer = std::make_unique<IRDumper>(ir_printer);
    }

    void Driver::add_disassembler(Disassembler bc_printer) noexcept {
        m_disassembler = std::make_unique<Disassembler>(bc_printer);
    }

    auto Driver::operator()(const std::filesystem::path& entry_source_path) -> bool {
        auto parsed_program = parse_sources(entry_source_path);

        if (!parsed_program) {
            return false;
        }

        if (!check_semantics(parsed_program.value())) {
            return false;
        }

        auto program_ir_opt = generate_ir(parsed_program.value());

        if (!program_ir_opt) {
            return false;
        }

        auto& program_ir = program_ir_opt.value();

        m_ir_printer->operator()(&program_ir);

        if (!apply_ir_passes(program_ir)) {
            return false;
        }

        auto program_opt = generate_program(program_ir);

        if (!program_opt) {
            return false;
        }

        auto& program = program_opt.value();

        m_disassembler->operator()(&program);

        if (!m_ir_printer->is_disabled() && !m_disassembler->is_disabled()) {
            return true;
        }

        Runtime::VM::Engine vm {normal_vm_config, program, &m_native_procs};

        auto run_start = std::chrono::steady_clock::now();
        const auto exec_status = vm();
        auto run_end = std::chrono::steady_clock::now();

        std::println("Finished in: {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(run_end - run_start));

        switch (exec_status) {
            case ExecStatus::ok:
                std::println("\033[1;32mStatus OK\033[0m\n");
                return true;
            default:
                std::println(std::cerr, "\033[1;31mRuntime Error: Exited with ExecStatus #{}, see vm.md for details.\033[0m\n", static_cast<int>(exec_status));
                return false;
        }
    }
}
