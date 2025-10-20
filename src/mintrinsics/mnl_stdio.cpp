#include <iostream>
#include <print>
#include <utility>

#include "mintrinsics/mnl_stdio.hpp"

namespace Minuet::Intrinsics {
    [[nodiscard]] auto native_print_value(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        const auto& argument_value = vm.handle_native_fn_access(argc, 0);

        std::println("{}", argument_value.to_string());

        return true;
    }

    [[nodiscard]] auto native_prompt_int(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        int temp_i32 = 0;

        std::cin >> temp_i32;

        Runtime::FastValue temp_value {temp_i32};

        vm.handle_native_fn_return(std::move(temp_value), argc);

        return true;
    }

    [[nodiscard]] auto native_prompt_float(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        double temp_f64 = 0;

        std::cin >> temp_f64;

        Runtime::FastValue temp_value {temp_f64};

        vm.handle_native_fn_return(std::move(temp_value), argc);

        return true;
    }
}