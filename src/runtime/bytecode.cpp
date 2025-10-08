#include <array>
#include <string_view>
#include "runtime/bytecode.hpp"

namespace Minuet::Runtime::Code {
    static constexpr std::array<std::string_view, static_cast<std::size_t>(Opcode::last)> opcode_names = {
        "nop",
        "load_const",
        "mov",
        "neg",
        "inc",
        "dec",
        "mul",
        "div",
        "mod",
        "add",
        "sub",
        "equ",
        "neq",
        "lt",
        "gt",
        "lte",
        "gte",
        "jump",
        "jump_if",
        "jump_else",
        "call",
        "native_call",
        "ret",
        "halt",
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(ArgMode::last)> arg_mode_names = {
        "immediate",
        "constant",
        "reg",
        "stack",
        "heap",
    };
    
    auto opcode_name(Opcode op) -> std::string_view {
        return opcode_names[static_cast<std::size_t>(op)];
    }

    auto arg_mode_name(ArgMode mode) -> std::string_view {
        return arg_mode_names[static_cast<std::size_t>(mode)];
    }
}
