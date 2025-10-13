#ifndef MINUET_RUNTIME_NATIVES_HPP
#define MINUET_RUNTIME_NATIVES_HPP

#include <string_view>
#include <unordered_map>
#include <vector>

namespace Minuet::Runtime::VM {
    class Engine;
}

namespace Minuet::Runtime {
    using native_proc_t = bool (*)(VM::Engine& vm, int16_t argc);

    /// NOTE: Only pass C-string literals to name_str, since they will be used to construct names of native procedure mappings as owning `std::string` objects.
    struct NativeProcItem {
        std::string_view name_str;
        native_proc_t proc_ptr;

        constexpr NativeProcItem(std::string_view name, native_proc_t fn_ptr) noexcept
        : name_str {name}, proc_ptr {fn_ptr} {}

        /**
         * @brief This overload is used for validation purposes only... Only a fully set name & function pointer pair is valid for the interpreter `Driver`.
         * @param self Deduced `this` of any `NativeProcItem` object.
         */
        [[nodiscard]] constexpr operator bool(this auto&& self) noexcept {
            return self.name_str.data() != nullptr && self.proc_ptr != nullptr;
        }
    };

    using NativeProcRegistry = std::unordered_map<std::string, int>;
    using NativeProcTable = std::vector<native_proc_t>;
}

#endif
