#include "ir/cfg.hpp"

namespace Minuet::IR::CFG {
    CFG::CFG()
    : m_blocks {} {}

    auto CFG::get_head() & noexcept -> std::optional<BasicBlock*> {
        if (m_blocks.empty()) {
            return {};
        }

        return {&m_blocks[0]};
    }

    auto CFG::get_head() const& noexcept -> std::optional<const BasicBlock*> {
        if (m_blocks.empty()) {
            return {};
        }

        return {&m_blocks[0]};
    }

    auto CFG::get_bb(int id) const& noexcept -> std::optional<const BasicBlock*> {
        if (const auto checked_id = static_cast<std::size_t>(id); checked_id < m_blocks.size()) {
            return {&m_blocks[checked_id]};
        }

        return {};
    }

    auto CFG::get_bb(int id) & noexcept -> std::optional<BasicBlock*> {
        if (const auto checked_id = static_cast<std::size_t>(id); checked_id < m_blocks.size()) {
            return {&m_blocks[checked_id]};
        }

        return {};
    }

    [[nodiscard]] auto CFG::get_newest_bb() & noexcept -> std::optional<BasicBlock*> {
        if (m_blocks.empty()) {
            return {};
        }

        return &m_blocks.back();
    }

    auto CFG::add_bb() & -> int {
        const int next_bb_id = m_blocks.size();

        m_blocks.emplace_back(BasicBlock {
            .steps = {},
            .truthy_id = dud_bb_id,
            .falsy_id = dud_bb_id,
        });

        return next_bb_id;
    }

    auto CFG::link_bb(int from_id, int to_id) & noexcept -> bool {
        auto target_opt = get_bb(from_id);

        if (const auto target_bb = target_opt.value_or(nullptr); target_opt != nullptr) {
            if (target_bb->truthy_id == dud_bb_id) {
                target_bb->truthy_id = to_id;
            } else if (target_bb->falsy_id == dud_bb_id) {
                target_bb->falsy_id = to_id;
            }

            return true;
        }

        return false;
    }
}
