#include <utility>

#include "runtime/sequence_value.hpp"

namespace Minuet::Runtime {
    SequenceValue::SequenceValue()
    : m_items {}, m_length {0}, m_frozen {true} {}

    SequenceValue::SequenceValue([[maybe_unused]] FlexiblePolicy flex_opt)
    : m_items {}, m_length {0}, m_frozen {false} {}

    auto SequenceValue::get_memory_score() const& noexcept -> std::size_t {
        return m_length * cm_fast_val_memsize;
    }

    auto SequenceValue::get_tag() const& noexcept -> ObjectTag {
        return ObjectTag::sequence;
    }

    auto SequenceValue::push_value(FastValue arg) -> bool {
        const auto next_idx = m_items.size();

        m_items[next_idx] = std::move(arg);
        ++m_length;

        return true;
    }

    auto SequenceValue::pop_value() -> FastValue {
        if (m_items.empty() || m_frozen) {
            return {};
        }

        const auto target_idx = m_items.size() - 1;

        if (auto target_it = m_items.find(target_idx); target_it != m_items.end()) {
            auto [_, result_value] = *target_it;

            m_items.erase(target_it);
            --m_length;
        
            return result_value;
        }

        return {};
    }

    auto SequenceValue::set_value(FastValue arg, std::size_t pos) -> bool {
        m_items[pos] = std::move(arg);

        return true;
    }

    auto SequenceValue::get_value(std::size_t pos) -> std::optional<FastValue*> {
        if (m_items.contains(pos) || m_frozen) {
            return &m_items[pos];
        }

        return {};
    }

    auto SequenceValue::as_fast_value() noexcept -> FastValue {
        return {this};
    }

    auto SequenceValue::to_string() const& noexcept -> std::string {
        return "Sequence(...)\n\tNote: this is a dud, lacking support for item printing.";
    }
}
