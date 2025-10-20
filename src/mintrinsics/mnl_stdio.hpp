#ifndef MINUET_MINTRINSICS_STDIO_HPP
#define MINUET_MINTRINSICS_STDIO_HPP

#include "runtime/vm.hpp"

namespace Minuet::Intrinsics {
    [[nodiscard]] auto native_print_value(Runtime::VM::Engine& vm, int16_t argc) -> bool;

    [[nodiscard]] auto native_prompt_int(Runtime::VM::Engine& vm, int16_t argc) -> bool;

    [[nodiscard]] auto native_prompt_float(Runtime::VM::Engine& vm, int16_t argc) -> bool;
}

#endif
