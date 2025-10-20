#include <utility>
#include <sstream>

#include "runtime/sequence_value.hpp"

namespace Minuet::Runtime {
    SequenceValue::SequenceValue()
    : m_items {}, m_length {0}, m_frozen {false} {}

    auto SequenceValue::items() noexcept -> std::vector<FastValue>& {
        return m_items;
    }


    auto SequenceValue::get_memory_score() const& noexcept -> std::size_t {
        return m_length * cm_fast_val_memsize;
    }

    auto SequenceValue::get_tag() const& noexcept -> ObjectTag {
        return ObjectTag::sequence;
    }

    auto SequenceValue::get_size() const& noexcept -> int {
        return m_length;
    }

    auto SequenceValue::push_value(FastValue arg) -> bool {
        m_items.emplace_back(std::move(arg));
        ++m_length;

        return true;
    }

    auto SequenceValue::pop_value(SequenceOpPolicy mode) -> FastValue {
        if (m_items.empty() || m_frozen) {
            return {};
        }

        auto target_value = (mode == SequenceOpPolicy::back)
            ? m_items.back()
            : m_items.front();

        if (mode == SequenceOpPolicy::back) {
            m_items.pop_back();
        } else {
            m_items.erase(m_items.begin());
        }

        --m_length;

        return target_value;
    }

    auto SequenceValue::set_value(FastValue arg, std::size_t pos) -> bool {
        m_items[pos] = std::move(arg);

        return true;
    }

    auto SequenceValue::get_value(std::size_t pos) -> std::optional<FastValue*> {
        if (pos >= 0 && pos < m_items.size()) {
            return &m_items[pos];
        }

        return {};
    }

    void SequenceValue::freeze() noexcept {
        m_frozen = true;
    }

    auto SequenceValue::as_fast_value() noexcept -> FastValue {
        return {this};
    }

    auto SequenceValue::to_string() const& noexcept -> std::string {
        struct Delims {
            char d_open;
            char d_close;
        };

        std::ostringstream sout;
        const auto [delim_open, delim_close] = (m_frozen)
            ? Delims { .d_open = '[', .d_close = ']' }
            : Delims { .d_open = '{', .d_close = '}' };

        sout << delim_open;

        for (const auto& item_v : m_items) {
            sout << item_v.to_string() << ' ';
        }

        sout << delim_close;

        return sout.str();
    }
}
