#ifndef MINUET_RUNTIME_SEQUENCE_VALUE_HPP
#define MINUET_RUNTIME_SEQUENCE_VALUE_HPP

#include <vector>

#include "runtime/fast_value.hpp"
#include "runtime/heap_value_base.hpp"

namespace Minuet::Runtime {
    struct FlexiblePolicy {};

    /**
     * @brief Contains an index to FastValue map to simulate an array.
     */
    class SequenceValue : public HeapValueBase {
    private:
        static constexpr auto cm_fast_val_memsize = 16UL;

        std::vector<FastValue> m_items;
        int m_length;
        bool m_frozen;

    public:
        SequenceValue();
        SequenceValue(FlexiblePolicy flex_opt);

        [[nodiscard]] auto items() noexcept -> std::vector<FastValue>&;

        [[nodiscard]] auto get_memory_score() const& noexcept -> std::size_t override;
        [[nodiscard]] auto get_tag() const& noexcept -> ObjectTag override;

        [[nodiscard]] auto push_value(FastValue arg) -> bool override;
        [[nodiscard]] auto pop_value(SequenceOpPolicy mode) -> FastValue override;
        [[nodiscard]] auto set_value(FastValue arg, std::size_t pos) -> bool override;
        [[nodiscard]] auto get_value(std::size_t pos) -> std::optional<FastValue*> override;

        void freeze() noexcept override;

        [[nodiscard]] auto as_fast_value() noexcept -> FastValue override;
        [[nodiscard]] auto to_string() const& noexcept -> std::string override;
    };
}

#endif
