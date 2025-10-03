#include <print>
#include <string>
#include <set>
#include <stack>

#include "ir/steps.hpp"
#include "ir/printing.hpp"

namespace Minuet::IR::Printing {
    using IR::Steps::AbsAddress;
    using IR::Steps::Step;
    using IR::Steps::ir_op_name;
    using IR::Steps::ir_aa_tag_name;
    using IR::CFG::FullIR;

    [[nodiscard]] std::string fmt_step_arg(AbsAddress aa) {
        return std::format("{}:{}", ir_aa_tag_name(aa.tag), aa.id);
    }

    void print_step(const Step& step) {
        if (const auto tac_unary_p = std::get_if<Steps::TACUnary>(&step); tac_unary_p) {
            std::print("{} = {} {}", fmt_step_arg(tac_unary_p->dest), ir_op_name(tac_unary_p->op), fmt_step_arg(tac_unary_p->arg_0));
        } else if (const auto tac_binary_p = std::get_if<Steps::TACBinary>(&step); tac_binary_p) {
            std::print("{} = {} {} {}", fmt_step_arg(tac_binary_p->dest), fmt_step_arg(tac_binary_p->arg_0), ir_op_name(tac_binary_p->op), fmt_step_arg(tac_binary_p->arg_1));
        } else if (const auto oper_nonary_p = std::get_if<Steps::OperNonary>(&step); oper_nonary_p) {
            std::print("{}", ir_op_name(oper_nonary_p->op));
        } else if (const auto oper_unary_p = std::get_if<Steps::OperUnary>(&step); oper_unary_p) {
            std::print("{} {}", ir_op_name(oper_unary_p->op), fmt_step_arg(oper_unary_p->arg_0));
        } else if (const auto oper_binary_p = std::get_if<Steps::OperBinary>(&step); oper_binary_p) {
            std::print("{} {} {}", ir_op_name(oper_binary_p->op), fmt_step_arg(oper_binary_p->arg_0), fmt_step_arg(oper_binary_p->arg_1));
        }

        std::println();
    }

    void print_cfg(const CFG::CFG& cfg) {
        std::set<int> visited_ids;
        std::stack<int> frontier_ids;

        frontier_ids.push(0);

        while (!frontier_ids.empty()) {
            auto next_bb_id = frontier_ids.top();
            frontier_ids.pop();
            auto next_node = cfg.get_bb(next_bb_id).value();

            std::println("\n\033[1;33mBasic Block #{}:\033[0m\n", next_bb_id);

            for (const auto& step : next_node->steps) {
                print_step(step);
            }

            if (const auto falsy_child_id = next_node->falsy_id; falsy_child_id != -1 && !visited_ids.contains(falsy_child_id)) {
                frontier_ids.push(falsy_child_id);
            }

            if (const auto truthy_child_id = next_node->truthy_id; truthy_child_id != -1 && !visited_ids.contains(truthy_child_id)) {
                frontier_ids.push(truthy_child_id);
            }

            visited_ids.insert(next_bb_id);
        }
    }

    void print_ir(const FullIR& full_ir) {
        const auto& [ir_cfgs, ir_constants, entry_id] = full_ir;

        std::println("\n\033[1;33mComplete IR:\033[0m\n");

        for (const auto& cfg : ir_cfgs) {
            print_cfg(cfg);
        }
    }
}
