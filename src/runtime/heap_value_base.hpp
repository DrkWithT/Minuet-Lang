#ifndef MINUET_RUNTIME_HEAP_VALUE_BASE_HPP
#define MINUET_RUNTIME_HEAP_VALUE_BASE_HPP

#include <string>
#include <optional>

#include "runtime/fast_value.hpp"

namespace Minuet::Runtime {
    enum class SequenceOpPolicy : uint8_t {
        front,
        back,
    };

    class HeapValueBase {
    public:
        ~HeapValueBase() = default;

        virtual auto get_memory_score() const& noexcept -> std::size_t = 0;
        virtual auto get_ref_count() const& noexcept -> int = 0;
        virtual void dec_ref_count() & noexcept = 0;
        virtual auto is_primitive() const& noexcept -> bool = 0;
        virtual auto is_sequence() const& noexcept -> bool = 0;

        virtual auto push_value(FastValue arg) -> bool = 0;
        virtual auto pop_value() -> FastValue = 0;
        virtual auto set_value(FastValue arg, std::size_t pos) -> bool = 0;
        virtual auto get_value(std::size_t pos) -> std::optional<FastValue*> = 0;

        virtual auto as_fast_value() noexcept -> FastValue = 0;
        virtual auto to_string() const& noexcept -> std::string = 0;
    };
}

#endif