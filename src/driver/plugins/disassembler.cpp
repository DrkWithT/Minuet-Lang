#include <print>

#include "driver/plugins/disassembler.hpp"

namespace Minuet::Driver::Plugins {
    Disassembler::Disassembler() noexcept
    : m_disabled {false} {}

    void Disassembler::set_disable_flag(bool b) noexcept {
        m_disabled = b;
    }

    auto Disassembler::is_disabled() const& noexcept -> bool {
        return m_disabled;
    }

    void Disassembler::operator()(std::any bytecode_ref_wrap) const {
        if (m_disabled || bytecode_ref_wrap.type() != typeid(Runtime::Code::Program*)) {
            return;
        }

        const Runtime::Code::Program* program_view = std::any_cast<Runtime::Code::Program*>(bytecode_ref_wrap);

        print_program(*program_view);
    }

    void Disassembler::print_instruction(const Runtime::Code::Instruction& inst) const {
        const auto inst_arity = Runtime::Code::instruct_arity(inst);

        if (inst_arity == 0) {
            std::println("{}", Runtime::Code::opcode_name(inst.op));
        } else if (inst_arity == 1) {
            std::println("{} {}:{}",
                Runtime::Code::opcode_name(inst.op),
                arg_mode_name_of<0>(inst),
                Runtime::Code::instruct_argval_at<0>(inst)
            );
        } else if (inst_arity == 2) {
            std::println("{} {}:{} {}:{}",
                Runtime::Code::opcode_name(inst.op),
                arg_mode_name_of<0>(inst),
                Runtime::Code::instruct_argval_at<0>(inst),
                arg_mode_name_of<1>(inst),
                Runtime::Code::instruct_argval_at<1>(inst)
            );
        } else if (inst_arity == 3) {
            std::println("{} {}:{} {}:{} {}:{}",
                Runtime::Code::opcode_name(inst.op),
                arg_mode_name_of<0>(inst),
                Runtime::Code::instruct_argval_at<0>(inst),
                arg_mode_name_of<1>(inst),
                Runtime::Code::instruct_argval_at<1>(inst),
                arg_mode_name_of<2>(inst),
                Runtime::Code::instruct_argval_at<2>(inst)
            );
        }
    }

    void Disassembler::print_chunk(const Runtime::Code::Chunk& chunk) const {
        for (const auto& inst : chunk) {
            print_instruction(inst);
        }
    }

    void Disassembler::print_program(const Runtime::Code::Program& program) const {
        std::println("\033[1;33mProgram Dump:\033[0m\n");
        std::println("\033[1;33mEntry Func-ID:\033[0m {}\n", program.entry_id.value_or(-1));

        auto chunk_id = 0;

        for (const auto& chunk : program.chunks) {
            std::println("\033[1;33mChunk #{}:\033[0m", chunk_id);

            print_chunk(chunk);
            ++chunk_id;
        }
    }
}