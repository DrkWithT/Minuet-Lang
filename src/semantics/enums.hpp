#ifndef SEMA_ENUMS_HPP
#define SEMA_ENUMS_HPP

#include <cstdint>

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
}

#endif
