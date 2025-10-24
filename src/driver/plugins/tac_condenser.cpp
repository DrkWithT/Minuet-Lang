// #include <utility>
#include <variant>
#include <vector>
#include <set>

#include "driver/plugins/tac_condenser.hpp"

namespace Minuet::Driver::Plugins {
    using IR::Steps::Op;
    using IR::Steps::AbsAddrTag;
    using IR::Steps::AbsAddress;

    TACCondenser::TACCondenser()
    : m_cached_temp_addrs {}, m_temp_addr_usages {}, m_omittable_ips {} {}

    [[maybe_unused]] auto TACCondenser::apply([[maybe_unused]] const IR::CFG::CFG& cfg) -> bool {
        return false;
    }

    [[nodiscard]] auto TACCondenser::apply(IR::CFG::CFG& cfg) -> bool {
        return process_cfg(cfg);
    }

    auto TACCondenser::process_cfg(IR::CFG::CFG& func_cfg) noexcept -> bool {
        if (!count_temp_aa_uses(func_cfg)) {
            return false;
        }

        std::set<int> bb_ids_visited;
        std::vector<int> bb_ids_frontier;

        bb_ids_frontier.push_back(0);

        while (!bb_ids_frontier.empty()) {
            const auto next_bb_id = bb_ids_frontier.back();
            bb_ids_frontier.pop_back();

            if (bb_ids_visited.contains(next_bb_id)) {
                continue;
            }

            auto next_bb_opt = func_cfg.get_bb(next_bb_id);

            if (!next_bb_opt) {
                return false;
            }

            auto next_bb = next_bb_opt.value();

            for (auto local_step_id = 0U; auto& step : next_bb->steps) {
                if (auto tac_unary_p = std::get_if<IR::Steps::TACUnary>(&step); tac_unary_p) {
                    // Base Case & General Case 1
                    if (condense_assignment(tac_unary_p->dest, tac_unary_p->op, tac_unary_p->arg_0)) {
                        m_omittable_ips.push_back(local_step_id);
                    }
                } else if (auto tac_binary_p = std::get_if<IR::Steps::TACBinary>(&step); tac_binary_p) {
                    // General Case 2
                    condense_assignment(tac_binary_p->dest, tac_binary_p->arg_0, tac_binary_p->arg_1, *tac_binary_p);
                } else {} // General Case 3 is a NO-OP.

                ++local_step_id;
            }

            cull_extra_assignments(*next_bb);

            bb_ids_visited.insert(next_bb_id);

            if (const auto falsy_bb_id = next_bb->falsy_id; falsy_bb_id != -1) {
                bb_ids_frontier.push_back(falsy_bb_id);
            }

            if (const auto truthy_bb_id = next_bb->truthy_id; truthy_bb_id != -1) {
                bb_ids_frontier.push_back(truthy_bb_id);
            }
        }

        m_temp_addr_usages.clear();

        return true;
    }

    auto TACCondenser::count_temp_aa_uses(IR::CFG::CFG& func_cfg) noexcept -> bool {
        if (func_cfg.bb_count() < 1) {
            return false;
        }

        std::set<int> bb_ids_visited;
        std::vector<int> bb_ids_frontier;

        while (!bb_ids_frontier.empty()) {
            const auto next_bb_id = bb_ids_frontier.back();
            bb_ids_frontier.pop_back();

            if (bb_ids_visited.contains(next_bb_id)) {
                continue;
            }

            auto next_bb_opt = func_cfg.get_bb(next_bb_id);

            if (!next_bb_opt) {
                return false;
            }

            auto next_bb = next_bb_opt.value();

            for (auto& step : next_bb->steps) {
                if (auto tac_unary_p = std::get_if<IR::Steps::TACUnary>(&step); tac_unary_p) {
                    if (tac_unary_p->op == Op::nop) {
                        m_temp_addr_usages[tac_unary_p->dest.id] = 0;
                    }

                    if (tac_unary_p->arg_0.tag == AbsAddrTag::temp) {        
                        ++m_temp_addr_usages[tac_unary_p->arg_0.id];
                    }
                } else if (auto tac_binary_p = std::get_if<IR::Steps::TACBinary>(&step); tac_binary_p) {
                    if (const auto bin_dest_id = tac_binary_p->dest.id, bin_a0_id = tac_binary_p->arg_0.id, bin_a1_id = tac_binary_p->arg_1.id; !m_temp_addr_usages.contains(bin_dest_id)) {
                        m_temp_addr_usages[bin_dest_id] = 0;
                    } else {
                        ++m_temp_addr_usages[bin_dest_id];
                        ++m_temp_addr_usages[bin_a0_id];
                        ++m_temp_addr_usages[bin_a1_id];
                    }
                } else {}
            }

            bb_ids_visited.insert(next_bb_id);

            if (const auto falsy_bb_id = next_bb->falsy_id; falsy_bb_id != -1) {
                bb_ids_frontier.push_back(falsy_bb_id);
            }

            if (const auto truthy_bb_id = next_bb->truthy_id; truthy_bb_id != -1) {
                bb_ids_frontier.push_back(truthy_bb_id);
            }
        }

        return true;
    }

    auto TACCondenser::condense_assignment(AbsAddress old_dest_aa, IR::Steps::Op op, AbsAddress old_src_aa) noexcept -> bool {
        const auto [dest_tag, dest_id] = old_dest_aa; // abstract address `A` (see decl. comments)
        const auto [src_tag, src_id] = old_src_aa; // abstract address `B` (see decl. comments)

        if (dest_tag != AbsAddrTag::temp || src_tag == AbsAddrTag::immediate || op != Op::nop) {
            return false;
        }

        if (src_tag == AbsAddrTag::constant) {
            m_cached_temp_addrs[dest_id] = {
                .aa = old_src_aa,
                .is_original = true,
            };
        } else if (src_tag == AbsAddrTag::temp) {
            if (!m_cached_temp_addrs.contains(src_id)) {
                m_cached_temp_addrs[src_id] = {
                    .aa = old_src_aa,
                    .is_original = true,
                };
            } else if (auto post_src_item = m_cached_temp_addrs[src_id]; post_src_item.is_original && m_temp_addr_usages.at(src_id) == 0) {
                m_cached_temp_addrs[dest_id] = post_src_item;
            } else {
                return true;
            }
        }

        return false;
    }

    void TACCondenser::condense_assignment(AbsAddress old_dest_aa, AbsAddress old_arg_aa_0, AbsAddress old_arg_aa_1, IR::Steps::TACBinary& step) noexcept {
        const auto [dest_tag, dest_id] = old_dest_aa;
        const auto [arg_0_tag, arg_0_id] = old_arg_aa_0;
        const auto [arg_1_tag, arg_1_id] = old_arg_aa_1;

        if (dest_tag != AbsAddrTag::temp || (arg_0_tag == AbsAddrTag::immediate && arg_1_tag == AbsAddrTag::immediate)) {
            return;
        }

        if (auto original_arg_0 = m_cached_temp_addrs[arg_0_id]; arg_0_tag == AbsAddrTag::temp && !original_arg_0.is_original) {
            step.arg_0 = original_arg_0.aa;
        }

        if (auto original_arg_1 = m_cached_temp_addrs[arg_1_id]; arg_1_tag == AbsAddrTag::temp && !original_arg_1.is_original) {
            step.arg_1 = original_arg_1.aa;
        }
    }

    void TACCondenser::cull_extra_assignments(IR::CFG::BasicBlock& bb) noexcept {
        for (const auto& extra_assignment_pos : m_omittable_ips) {
            bb.steps[extra_assignment_pos] = IR::Steps::OperNonary {
                .op = Op::nop,
            };
        }

        m_omittable_ips.clear();
    }
}
