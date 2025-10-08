#ifndef DISASSEMBLE_HPP
#define DISASSEMBLE_HPP

#include <string_view>

#include "runtime/bytecode.hpp"

namespace Minuet::Codegen {
    template <std::size_t Idx>
    [[nodiscard]] auto arg_mode_name_of(const Runtime::Code::Instruction& inst) noexcept -> std::string_view {
        if constexpr (Idx >= 0 && Idx < 3) {
            constexpr uint16_t meta_tag_offset = static_cast<uint16_t>(2 + Idx * 4);
            const uint16_t meta_arg_tag_n = (inst.metadata & static_cast<uint16_t>(0b1111) << meta_tag_offset) >> meta_tag_offset;

            switch (meta_arg_tag_n) {
                case 0: return "imm";
                case 1: return "const";
                case 2: return "reg";
                case 3: return "stack";
                case 4: default: return "heap";
            }
        }

        return "??";
    }

    void print_instruction(const Runtime::Code::Instruction& inst);
    void print_chunk(const Runtime::Code::Chunk& chunk);
    void print_program(const Runtime::Code::Program& program);
}

#endif
