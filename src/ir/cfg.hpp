#ifndef CFG_HPP
#define CFG_HPP

#include <optional>
#include <vector>

#include "ir/steps.hpp"
#include "runtime/value.hpp"

namespace Minuet::IR::CFG {
    struct BasicBlock {
        std::vector<Steps::Step> steps;
        int truthy_id;
        int falsy_id;
    };

    class CFG {
    public:
        static constexpr auto dud_bb_id = -1;

    private:
        std::vector<BasicBlock> m_blocks;

    public:
        CFG();

        [[nodiscard]] auto bb_count(this auto&& self) noexcept -> int {
            return self.m_blocks.size();
        }

        [[nodiscard]] auto get_head() & noexcept -> std::optional<BasicBlock*>;
        [[nodiscard]] auto get_head() const& noexcept -> std::optional<const BasicBlock*>;
        [[nodiscard]] auto get_bb(int id) const& noexcept -> std::optional<const BasicBlock*>;
        [[nodiscard]] auto get_bb(int id) & noexcept -> std::optional<BasicBlock*>;
        [[nodiscard]] auto add_bb() & -> int;
        [[nodiscard]] bool link_bb(int from_id, int to_id) & noexcept;

        template <template<typename> typename Pass, typename Result>
        [[nodiscard]] auto take_pass(this auto&& self, Pass<Result>& cfg_pass) noexcept(noexcept(cfg_pass.apply(self))) -> Result {
            return cfg_pass.apply(self);
        }
    };

    struct FullIR {
        std::vector<CFG> cfg_list;
        std::vector<Runtime::Value> constants;
        int main_id;
    };
}

#endif
