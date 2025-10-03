#ifndef STEPS_HPP
#define STEPS_HPP

#include <cstdint>
#include <string_view>
#include <variant>

namespace Minuet::IR::Steps {
    enum class Op : uint8_t {
        nop,
        push,
        pop,
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
        meta_load_aa,
        meta_begin_if,
        meta_end_if,
        meta_begin_else,
        meta_end_else,
        last,
    };

    [[nodiscard]] auto ir_op_name(Op op) noexcept -> std::string_view;

    enum class AbsAddrTag : uint8_t {
        immediate,
        constant,
        temp,
        heap,
        last,
    };

    [[nodiscard]] auto ir_aa_tag_name(AbsAddrTag tag) noexcept -> std::string_view;

    struct AbsAddress {
        AbsAddrTag tag;
        uint16_t id;
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
