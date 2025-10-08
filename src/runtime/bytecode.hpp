#ifndef BYTECODE_HPP
#define BYTECODE_HPP

#include <cstdint>
#include <vector>
#include <string_view>

#include "runtime/value.hpp"

namespace Minuet::Runtime::Code {
    enum class Opcode : uint8_t {
        nop,
        load_const,
        mov,
        neg,
        inc,
        dec,
        mul,
        div,
        mod,
        add,
        sub,
        equ,
        neq,
        lt,
        gt,
        lte,
        gte,
        jump,
        jump_if,
        jump_else,
        call,
        native_call,
        ret,
        halt,
        last,
    };

    [[nodiscard]] auto opcode_name(Opcode op) -> std::string_view;

    enum class ArgMode : uint8_t {
        immediate,
        constant,
        reg,
        stack,
        heap,
        last,
    };

    [[nodiscard]] auto arg_mode_name(ArgMode mode) -> std::string_view;

    struct Instruction {
        int16_t args[3];
        uint16_t metadata; // NOTE: `2` bits for arity between `0` to `3`, `3` max arg-values.
        Opcode op;
    };

    [[nodiscard]] constexpr auto instruct_arity(Instruction inst) -> uint8_t {
        return static_cast<uint8_t>(inst.metadata & 0b11);
    }
    
    template <std::size_t ArgPos>
    [[nodiscard]] constexpr auto instruct_argval_at(Instruction inst) -> uint16_t {
        if constexpr (ArgPos >= 0 && ArgPos < 3) {
            return inst.args[ArgPos];
        }

        return 0;
    }

    using Chunk = std::vector<Instruction>;

    struct Program {
        std::vector<Runtime::Value> constants;
        std::vector<Chunk> chunks;
        std::optional<int> entry_id;
    };
}

#endif
