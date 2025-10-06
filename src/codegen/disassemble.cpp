#include <print>

#include "runtime/bytecode.hpp"
#include "codegen/disassemble.hpp"

namespace Minuet::Codegen {
    using Runtime::Code::ArgMode;
    using Runtime::Code::instruct_arity;
    using Runtime::Code::opcode_name;
    using Runtime::Code::arg_mode_name;
    using Runtime::Code::instruct_argval_at;

    void print_instruction(const Runtime::Code::Instruction& instr) {
        const auto& [args, metadata, op] = instr;
        const auto inst_arity = instruct_arity(instr);

        if (inst_arity == 0) {
            std::println("{}", opcode_name(op));
        } else if (inst_arity == 1) {
            std::println("{} {}:{}",
                opcode_name(op),
                arg_mode_name(static_cast<ArgMode>((metadata & 0b00110000) >> 4)),
                instruct_argval_at<0>(instr)
            );
        } else if (inst_arity == 2) {
            std::println("{} {}:{} {}:{}",
                opcode_name(op),
                arg_mode_name(static_cast<ArgMode>((metadata & 0b00110000) >> 4)),
                instruct_argval_at<0>(instr),
                arg_mode_name(static_cast<ArgMode>((metadata & 0b00001100) >> 2)),
                instruct_argval_at<1>(instr)
            );
        } else if (inst_arity == 3) {
            std::println("{} {}:{} {}:{} {}:{}",
                opcode_name(op),
                arg_mode_name(static_cast<ArgMode>((metadata & 0b00110000) >> 4)),
                instruct_argval_at<0>(instr),
                arg_mode_name(static_cast<ArgMode>((metadata & 0b00001100) >> 2)),
                instruct_argval_at<1>(instr),
                arg_mode_name(static_cast<ArgMode>(metadata & 0b00000011)),
                instruct_argval_at<2>(instr)
            );
        }
    }

    void print_chunk(const Runtime::Code::Chunk& chunk) {
        for (const auto& inst : chunk) {
            print_instruction(inst);
        }
    }

    void print_program(const Runtime::Code::Program& prgm) {
        std::println("\033[1;33mProgram Dump:\033[0m\n");
        std::println("\033[1;33mEntry Function ID:\033[0m {}\n", prgm.entry_id.value_or(-1));

        auto chunk_id = 0U;
        for (const auto& temp_chunk : prgm.chunks) {
            std::println("\033[1;33mChunk #{}:\033[0m\n", chunk_id);
            print_chunk(temp_chunk);
            ++chunk_id;
        }
    }
}
