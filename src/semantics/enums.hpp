#ifndef MINUET_SEMA_ENUMS_HPP
#define MINUET_SEMA_ENUMS_HPP

#include <cstdint>
#include <optional>
#include <string_view>
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

    [[nodiscard]] constexpr auto operator_name(Operator oper) -> std::string_view {
        switch (oper) {
        case Operator::negate:
            return "-(negate)";
        case Operator::mul:
            return "*";
        case Operator::div:
            return "/";
        case Operator::modulo:
            return "%";
        case Operator::add:
            return "+";
        case Operator::sub:
            return "-";
        case Operator::equality:
            return "==";
        case Operator::inequality:
            return "!=";
        case Operator::lesser:
            return "<";
        case Operator::greater:
            return ">";
        case Operator::at_most:
            return "<=";
        case Operator::at_least:
            return ">=";
        case Operator::access:
            return ".";
        case Operator::assign:
            return "=";
        default:
            return {};
        }
    }

    enum class EntityKinds : uint8_t {
        anything,
        primitive,
        sequence_fixed,
        sequence_flexible,
        callable,
    };

    [[nodiscard]] constexpr auto entity_kinds_to_sv(EntityKinds kinds) noexcept -> std::string_view {
        switch (kinds) {
            case EntityKinds::anything: return "(any)";
            case EntityKinds::primitive: return "primitive";
            case EntityKinds::sequence_fixed: return "tuple";
            case EntityKinds::sequence_flexible: return "list";
            case EntityKinds::callable: default: return "callable";
        }
    }

    enum class ValueGroup : uint8_t {
        temporary,
        locator,
    };

    [[nodiscard]] constexpr auto value_group_name(ValueGroup val_group) noexcept -> std::string_view {
        switch (val_group) {
            case ValueGroup::temporary: return "temporary";
            case ValueGroup::locator: return "locator";
            default: return "(unknown)";
        }
    }
}

#endif
