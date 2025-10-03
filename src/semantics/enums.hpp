#ifndef SEMA_ENUMS_HPP
#define SEMA_ENUMS_HPP

#include <cstdint>
#include <optional>
#include "ir/steps.hpp"

namespace Minuet::Semantics::Enums {
    enum class Operator : uint8_t {
        access,
        negate,
        mul,
        div,
        modulo,
        add,
        sub,
        equality,
        inequality,
        lesser,
        greater,
        at_most,
        at_least,
        assign,
    };

    [[nodiscard]] constexpr auto operator_to_ir_op(Operator oper) -> std::optional<IR::Steps::Op> {
        using IR::Steps::Op;
        switch (oper) {
        case Operator::negate:
            return Op::neg;
        case Operator::mul:
            return Op::mul;
        case Operator::div:
            return Op::div;
        case Operator::modulo:
            return Op::mod;
        case Operator::add:
            return Op::add;
        case Operator::sub:
            return Op::sub;
        case Operator::equality:
            return Op::equ;
        case Operator::inequality:
            return Op::neq;
        case Operator::lesser:
            return Op::lt;
        case Operator::greater:
            return Op::gt;
        case Operator::at_most:
            return Op::lte;
        case Operator::at_least:
            return Op::gte;
        case Operator::access:
        case Operator::assign:
        default:
            return {};
        }
    }
}

#endif
