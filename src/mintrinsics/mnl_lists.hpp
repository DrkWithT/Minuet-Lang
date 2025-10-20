#ifndef MINUET_MINTRINSICS_LISTS_HPP
#define MINUET_MINTRINSICS_LISTS_HPP

#include "runtime/vm.hpp"

namespace Minuet::Intrinsics {
    [[nodiscard]] auto native_len_of(Runtime::VM::Engine& vm, int16_t argc) -> bool;
}

#endif
