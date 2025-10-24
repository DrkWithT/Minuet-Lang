#ifndef MINUET_MINTRINSICS_LISTS_HPP
#define MINUET_MINTRINSICS_LISTS_HPP

#include "runtime/vm.hpp"

namespace Minuet::Intrinsics {
    /// @brief Gets the count of a list's items.
    [[nodiscard]] auto native_len_of(Runtime::VM::Engine& vm, int16_t argc) -> bool;

    /// @brief Takes a list reference and then any `Value` to append. If the sequence is frozen (aka tuple), this will fail.
    [[nodiscard]] auto native_list_push_back(Runtime::VM::Engine& vm, int16_t argc) -> bool;

    /// @brief Removes and returns the last item of a referenced list. If the sequence is frozen (aka tuple), this will fail.
    [[nodiscard]] auto native_list_pop_back(Runtime::VM::Engine& vm, int16_t argc) -> bool;

    /// @brief Removes and returns the first item of a referenced list. If the sequence is frozen, this will fail.
    [[nodiscard]] auto native_list_pop_front(Runtime::VM::Engine& vm, int16_t argc) -> bool;

    /// @brief Joins a list's items in sequence to a referenced list. If the target sequence is frozen, this will fail.
    [[nodiscard]] auto native_list_concat(Runtime::VM::Engine& vm, int16_t argc) -> bool;
}

#endif
