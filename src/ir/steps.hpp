#ifndef STEPS_HPP
#define STEPS_HPP

#include <cstdint>
#include <variant>

namespace Minuet::IR::Steps {
    enum class Op : uint8_t {
        nop,
        push,
        pop,
        inc,
        dec,
        mul,
        div,
        mod,
        add,
        sub,
        jump,
        jump_if,
        jump_else,
        call,
        native_call,
        ret,
        halt,
    };

    enum class AbsAddrTag : uint8_t {
        immediate,
        constant,
        temp,
        heap,
    };

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
