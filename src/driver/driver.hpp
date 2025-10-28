#ifndef MINUET_DRIVER_HPP
#define MINUET_DRIVER_HPP

#include <optional>
#include <filesystem>
#include "frontend/lexing.hpp"
#include "frontend/parsing.hpp"
#include "ir/cfg.hpp"
#include "runtime/bytecode.hpp"
#include "runtime/natives.hpp"
#include "driver/plugins/printer.hpp"
#include "driver/plugins/ir_dumper.hpp"
#include "driver/plugins/disassembler.hpp"

namespace Minuet::Driver {
    class Driver {
    public:
        Driver();

        [[maybe_unused]] auto register_native_proc(const Runtime::NativeProcItem& item) -> bool;

        /// TODO: refactor to return full program AST from traversing includes.
        [[nodiscard]] auto parse_sources(const std::filesystem::path& main_path) -> std::optional<Syntax::AST::FullAST>;

        [[nodiscard]] auto check_semantics(const Syntax::AST::FullAST& ast) -> bool;

        [[maybe_unused]] auto generate_ir(const Syntax::AST::FullAST& ast) -> std::optional<IR::CFG::FullIR>;

        [[maybe_unused]] auto apply_ir_passes(IR::CFG::FullIR& ir) -> bool;

        [[maybe_unused]] auto generate_program(IR::CFG::FullIR& ir) -> std::optional<Runtime::Code::Program>;

        [[nodiscard]] auto operator()(const std::filesystem::path& entry_source_path) -> bool;

        void add_ir_dumper(Plugins::IRDumper ir_printer) noexcept;
        void add_disassembler(Plugins::Disassembler bc_printer) noexcept;

    private:
        Frontend::Lexing::Lexer m_lexer;
        std::unordered_map<uint32_t, std::string> m_src_map;
        Runtime::NativeProcTable m_native_procs;
        Runtime::NativeProcRegistry m_native_proc_ids;
        std::unique_ptr<Plugins::Printer> m_ir_printer;
        std::unique_ptr<Plugins::Printer> m_disassembler;
    };
}

#endif
