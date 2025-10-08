#include <print>
#include "codegen/disassemble.hpp"

namespace Minuet::Codegen {
    void print_instruction(const Runtime::Code::Instruction& inst) {
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

    void print_chunk(const Runtime::Code::Chunk& chunk) {
        for (const auto& inst : chunk) {
            print_instruction(inst);
        }
    }

    void print_program(const Runtime::Code::Program& program) {
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
