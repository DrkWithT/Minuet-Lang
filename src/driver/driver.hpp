#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <vector>
#include <optional>
#include <filesystem>
#include "frontend/lexing.hpp"
#include "frontend/parsing.hpp"
#include "ir/cfg.hpp"
#include "runtime/bytecode.hpp"

namespace Minuet::Driver::Compilation {
    class CompileDriver {
    public:
        CompileDriver();

        /// TODO: refactor to return full program AST from traversing includes.
        [[nodiscard]] auto parse_sources(const std::filesystem::path& main_path) -> std::optional<Syntax::AST::FullAST>;

        [[maybe_unused]] auto generate_ir(const Syntax::AST::FullAST& ast) -> std::optional<IR::CFG::FullIR>;

        [[maybe_unused]] auto generate_program(IR::CFG::FullIR& ir) -> std::optional<Runtime::Code::Program>;

        [[nodiscard]] auto operator()(const std::filesystem::path& entry_source_path) -> std::optional<Runtime::Code::Program>;

    private:
        Frontend::Lexing::Lexer m_lexer;
        std::vector<std::string> m_src_map;
    };
}

#endif
