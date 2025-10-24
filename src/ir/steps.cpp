#include <array>
#include <string_view>
#include "ir/steps.hpp"

namespace Minuet::IR::Steps {
    static constexpr std::array<std::string_view, static_cast<std::size_t>(Op::last)> op_names = {
        "nop",
        "make_seq",
        "seq_obj_push",
        "seq_obj_pop",
        "seq_obj_get",
        "frz_seq_obj",
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
        "#begin_while",
        "#end_while",
        "#mark_break",
        "#mark_continue",
        "#save_patch",
        "#patch_jmp_else",
        "#patch_jmp",
    };

    static constexpr std::array<std::string_view, static_cast<std::size_t>(AbsAddrTag::last)> aa_names = {
        "immediate",
        "constant",
        "temp",
        // "stack",
    };

    auto ir_op_name(Op op) noexcept -> std::string_view {
        const auto op_ord = static_cast<std::size_t>(op);
        return op_names[op_ord];
    }

    auto ir_aa_tag_name(AbsAddrTag tag) noexcept -> std::string_view {
        const auto aa_tag_ord = static_cast<std::size_t>(tag);
        return aa_names[aa_tag_ord];
    }
}
