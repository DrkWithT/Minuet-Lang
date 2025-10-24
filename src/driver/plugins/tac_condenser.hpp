#ifndef MINUET_PLUGINS_TAC_CONDENSER_HPP
#define MINUET_PLUGINS_TAC_CONDENSER_HPP

#include <unordered_map>

#include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "ir/pass.hpp"

namespace Minuet::Driver::Plugins {
    class TACCondenser : public IR::Pass::PassBase<bool> {
    public:
        TACCondenser();

        /// NOTE: This is a dud override only for interface purposes.
        [[maybe_unused]] auto apply(const IR::CFG::CFG& cfg) -> bool override;

        /// NOTE: This will mutate the CFG steps to use less useless or redundant MOVs.
        [[nodiscard]] auto apply(IR::CFG::CFG& cfg) -> bool override;

    private:
        struct CachedTempItem {
            IR::Steps::AbsAddress aa;
            bool is_original;
        };

        std::unordered_map<int16_t, CachedTempItem> m_cached_temp_addrs;
        std::unordered_map<int16_t, int> m_temp_addr_usages;
        std::vector<uint32_t> m_omittable_ips;

        /// @brief This is a helper method to try pruning useless TAC assignments per CFG.
        [[maybe_unused]] auto process_cfg(IR::CFG::CFG& func_cfg) noexcept -> bool;

        [[nodiscard]] auto count_temp_aa_uses(IR::CFG::CFG& func_cfg) noexcept -> bool;

        /**
         * @note This method does most of the work for this process, using a simple caching approach described below. This pass assumes that the naive IR is valid beforehand.
         * * BASE case: Assignments for initial local definitions will source a final constant or final `Value` location (example: function argument in a parameter).
         *   1. Let `A` be some IR address and `F` be a final location... Assume that every basic unary assignment will be like `A = nop F`, having an initial usage count of 0.
         * * GENERAL cases: Instead of chasing data dependencies for every assignment, just condense transitive dependencies when needed, omitting some work needed for later assignments.
         *   1. UNARY case: If `C` is a non-final location and `A <- B` and `B <- C` and `B` has a use count of 0, then `A <- C` for `A = nop B`. Non-NOP's will just collapse their RHS operands.
         *   1. BINARY case: If `A <- B op C`, just condense `B` and `C`.
         *   2. OTHER case: Do nothing.
         * """
         * @param old_dest_aa 
         * @param old_src_aa 
         * @return `std::optional<IR::Steps::AbsAddress>` Empty on failure.
         */
        [[nodiscard]] auto condense_assignment(IR::Steps::AbsAddress old_dest_aa, IR::Steps::Op op, IR::Steps::AbsAddress old_src_aa) noexcept -> bool;

        /// NOTE: This is an overload for the `TACBinary` case for the `condense_assignment` algorithm.
        void condense_assignment(IR::Steps::AbsAddress old_dest_aa, IR::Steps::AbsAddress old_arg_aa_0, IR::Steps::AbsAddress old_arg_aa_1, IR::Steps::TACBinary& step) noexcept;

        void cull_extra_assignments(IR::CFG::BasicBlock& bb) noexcept;
    };
}

#endif
