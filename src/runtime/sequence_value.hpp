#ifndef MINUET_RUNTIME_SEQUENCE_VALUE_HPP
#define MINUET_RUNTIME_SEQUENCE_VALUE_HPP

#include <unordered_map>

#include "runtime/fast_value.hpp"
#include "runtime/heap_value_base.hpp"

namespace Minuet::Runtime {
    struct FlexiblePolicy {};

    /**
     * @brief Contains a rope data structure of FastValue objects. This is to balance the tradeoffs of pushes and pops from either side. For a raw `std::vector` representation, all elements must shift on frontal pops. For a linked-list representation, the cache friendliness suffers by poor spatial locality.
     * @note There are custom copy, move, and assignment operations because destruction needs special action (to avoid a stack-overflow for larger lists).
     */
    class SequenceValue : public HeapValueBase {
    private:
        static constexpr auto cm_fast_val_memsize = 16UL;

        std::unordered_map<std::size_t, FastValue> m_items;
        std::size_t m_length;
        bool m_frozen;

    public:
        SequenceValue();
        SequenceValue(FlexiblePolicy flex_opt);

        [[nodiscard]] auto get_memory_score() const& noexcept -> std::size_t override;
        [[nodiscard]] auto get_tag() const& noexcept -> ObjectTag override;

        [[nodiscard]] auto push_value(FastValue arg) -> bool override;
        [[nodiscard]] auto pop_value() -> FastValue override;
        [[nodiscard]] auto set_value(FastValue arg, std::size_t pos) -> bool override;
        [[nodiscard]] auto get_value(std::size_t pos) -> std::optional<FastValue*> override;

        [[nodiscard]] auto as_fast_value() noexcept -> FastValue override;
        [[nodiscard]] auto to_string() const& noexcept -> std::string override;
    };
}

#endif
