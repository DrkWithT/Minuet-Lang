#ifndef MINUET_IR_STEPS_HPP
#define MINUET_IR_STEPS_HPP

#include <cstdint>
#include <string_view>
#include <variant>

namespace Minuet::IR::Steps {
    enum class Op : uint8_t {
        nop,
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
        meta_begin_while,
        meta_end_while,
        meta_mark_break,
        meta_save_patch,
        meta_save_patch_back,
        meta_shuffle_patch,
        meta_patch_jmp_else,
        meta_patch_jmp,
        last,
    };

    [[nodiscard]] auto ir_op_name(Op op) noexcept -> std::string_view;

    enum class AbsAddrTag : uint8_t {
        immediate,
        constant,
        temp,
        stack,
        heap,
        last,
    };

    [[nodiscard]] auto ir_aa_tag_name(AbsAddrTag tag) noexcept -> std::string_view;

    struct AbsAddress {
        AbsAddrTag tag;
        int16_t id;
    };

    struct TACUnary {
        AbsAddress dest;
        AbsAddress arg_0;
        Op op;
    };

    struct TACBinary {
        AbsAddress dest;
        AbsAddress arg_0;
        AbsAddress arg_1;
        Op op;
    };

    struct OperNonary {
        Op op;
    };

    struct OperUnary {
        AbsAddress arg_0;
        Op op;
    };

    struct OperBinary {
        AbsAddress arg_0;
        AbsAddress arg_1;
        Op op;
    };

    using Step = std::variant<TACUnary, TACBinary, OperNonary, OperUnary, OperBinary>;
}

#endif
