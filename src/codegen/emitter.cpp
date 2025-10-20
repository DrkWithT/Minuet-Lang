#include <iostream>
#include <print>
#include <utility>
#include <set>
#include <stack>

#include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "runtime/bytecode.hpp"
#include "codegen/emitter.hpp"

namespace Minuet::Codegen {
    using IR::Steps::Op;
    using IR::Steps::Step;
    using IR::Steps::AbsAddrTag;
    using IR::CFG::FullIR;
    using Runtime::Code::Opcode;
    using Runtime::Code::ArgMode;
    using Runtime::Code::Instruction;
    using Runtime::Code::Chunk;
    using Runtime::Code::Program;

    Emitter::Emitter()
    : m_result_chunks {}, m_patches {}, m_active_loops {}, m_next_fun_id {0} {}

    auto Emitter::operator()(FullIR& ir) -> std::optional<Program> {
        auto& [ir_cfgs, ir_constants, ir_main_fn_id] = ir;

        auto cfg_count = 0;
        for (const auto& cfg : ir_cfgs) {
            if (!emit_chunk(cfg)) {
                std::println(std::cerr, "Codegen Error: Failed to emit code chunk for CFG #{}", cfg_count);
                return {};
            }

            ++cfg_count;
        }

        return Program {
            .chunks = std::exchange(m_result_chunks, {}),
            .constants = std::exchange(ir.constants, {}),
            .entry_id = ir.main_id,
        };
    }


    auto Emitter::translate_value_aa(IR::Steps::AbsAddress aa) noexcept -> std::optional<Utils::PseudoArg> {
        const auto [aa_tag, aa_value] = aa;

        /// NOTE: remove AbsAddrTag::stack support?
        const auto pseudo_arg_tag = ([](AbsAddrTag aa_tag) noexcept -> std::optional<ArgMode> {
            switch (aa_tag) {
                case AbsAddrTag::immediate: return ArgMode::immediate;
                case AbsAddrTag::constant: return ArgMode::constant;
                case AbsAddrTag::temp: return ArgMode::reg;
                case AbsAddrTag::stack:
                case AbsAddrTag::heap:
                default: return {};
            }
        })(aa_tag);

        if (!pseudo_arg_tag) {
            return {};
        }

        return Utils::PseudoArg {
            .value = aa_value,
            .tag = pseudo_arg_tag.value(),
        };
    }

    auto Emitter::gen_function_id() noexcept -> std::optional<int16_t> {
        if (const auto next_func_id = m_next_fun_id; next_func_id < std::numeric_limits<int16_t>::max()) {
            ++m_next_fun_id;

            return next_func_id;
        }

        return {};
    }

    auto Emitter::emit_tac_unary(const IR::Steps::TACUnary& tac_unary) -> bool {
        const auto& [dest_aa, arg_0_aa, op] = tac_unary;

        auto dest_opt = translate_value_aa(dest_aa);
        auto arg_0_opt = translate_value_aa(arg_0_aa);

        if (!dest_opt || !arg_0_opt) {
            return false;
        }

        auto dest = dest_opt.value();
        auto arg_0 = arg_0_opt.value();

        if (op == Op::nop) {
            m_result_chunks.back().emplace_back(Instruction {
                .args = {dest.value, arg_0.value, 0},
                .metadata = Utils::encode_metadata(dest, arg_0),
                .op = Opcode::mov,
            });

            return true;
        } else if (op == Op::neg) {
            if (dest == arg_0) {
                m_result_chunks.back().emplace_back(Instruction {
                    .args = {dest.value, 0, 0},
                    .metadata = encode_metadata(dest),
                    .op = Opcode::neg,
                });
            } else {
                m_result_chunks.back().emplace_back(Instruction {
                    .args = {dest.value, arg_0.value, 0},
                    .metadata = encode_metadata(dest, arg_0),
                    .op = Opcode::mov,
                });
                m_result_chunks.back().emplace_back(Instruction {
                    .args = {dest.value, 0, 0},
                    .metadata = encode_metadata(dest),
                    .op = Opcode::neg,
                });
            }

            return true;
        }

        std::println("Invalid IR op at emit_tac_unary in current BB...");

        return false;
    }

    auto Emitter::emit_tac_binary(const IR::Steps::TACBinary& tac_binary) -> bool {
        const auto& [dest_aa, arg_0, arg_1, op] = tac_binary;

        const auto opcode_opt = ([](Op ir_op) noexcept -> std::optional<Opcode> {
            switch (ir_op) {
                case Op::mul: return Opcode::mul;
                case Op::div: return Opcode::div;
                case Op::mod: return Opcode::mod;
                case Op::add: return Opcode::add;
                case Op::sub: return Opcode::sub;
                case Op::equ: return Opcode::equ;
                case Op::neq: return Opcode::neq;
                case Op::lt: return Opcode::lt;
                case Op::gt: return Opcode::gt;
                case Op::lte: return Opcode::lte;
                case Op::gte: return Opcode::gte;
                default: return {};
            }
        })(op);

        if (!opcode_opt) {
            std::println("Invalid opcode at emit_tac_binary in current BB...");

            return false;
        }

        const auto opcode = opcode_opt.value();
        auto dest_opt = translate_value_aa(dest_aa);
        auto arg_0_opt = translate_value_aa(arg_0);
        auto arg_1_opt = translate_value_aa(arg_1);

        if (!dest_opt || !arg_0_opt || !arg_1_opt) {
            std::println("Invalid locatios at emit_tac_binary in current BB...");

            return false;
        }

        const auto dest = dest_opt.value();
        const auto a0 = arg_0_opt.value();
        const auto a1 = arg_1_opt.value();

        m_result_chunks.back().emplace_back(Instruction {
            .args = {dest.value, a0.value, a1.value},
            .metadata = Utils::encode_metadata(dest, a0, a1),
            .op = opcode,
        });

        return true;
    }

    auto Emitter::emit_oper_nonary(const IR::Steps::OperNonary& oper_nonary) -> bool {
        const auto& [op] = oper_nonary;

        if (op == Op::nop) {
            m_result_chunks.back().emplace_back(Instruction {
                .args = {0, 0, 0},
                .metadata = Utils::encode_metadata(),
                .op = Opcode::nop,
            });
        } else if (op == Op::meta_begin_while) {
            const auto starting_nop_ip = m_result_chunks.back().size();

            m_active_loops.emplace_back(Utils::ActiveLoop {
                .brk_ips = {},
                .cont_ips = {},
                .start_ip = starting_nop_ip,
                .exit_ip = 0,
            });
        } else if (op == Op::meta_end_while) {
            const auto ending_nop_ip = m_result_chunks.back().size() - 1;

            m_active_loops.back().exit_ip = ending_nop_ip;

            const auto& [break_ips, continuing_ips, loop_begin, loop_end] = m_active_loops.back();

            for (const auto& brk_jump_ip : break_ips) {
                m_result_chunks.back()[brk_jump_ip].args[0] = loop_end;
            }

            for (const auto& cnt_jump_ip : continuing_ips) {
                m_result_chunks.back()[cnt_jump_ip].args[0] = loop_begin;
            }

            m_active_loops.pop_back(); // NOTE: the most recent active loop ref dangles, but it's OK because its usage has certainly finished here.
        } else if (op == Op::meta_mark_break) {
            const auto brk_jump_ip = m_result_chunks.back().size() - 1;

            m_active_loops.back().brk_ips.push_back(brk_jump_ip);
        } else if (op == Op::meta_mark_continue) {
            const auto cnt_jump_ip = m_result_chunks.back().size() - 1;

            m_active_loops.back().cont_ips.push_back(cnt_jump_ip);
        } else if (op == Op::meta_save_patch) {
            /// NOTE: Patches a jumping instruction forwards.
            const auto patchable_ip = m_result_chunks.back().size() - 1;

            m_patches.emplace_back(Utils::Patch {
                .instruction_pos = patchable_ip,
                .target_ip = 0,
                .cf_forward = true,
            });
        } else if (op == Op::meta_patch_jmp_else) {
            const auto patched_jmp_else_ip = m_result_chunks.back().size() - 1;

            const auto patch = m_patches.back();
            m_patches.pop_back();

            if (patch.cf_forward) {
                m_result_chunks.back()[patch.instruction_pos].args[0] = patched_jmp_else_ip;
            } else {
                /// NOTE: No support for backwards jump_else is currently planned.
                std::println("Invalid case (jump_else backwards) at emit_oper_nonary in current BB...");
                return false;
            }
        } else if (op == Op::meta_patch_jmp) {
            /// NOTE: Add support for backwards JUMP patching later for constructs, including while loops!
            const auto patched_jmp_ip = m_result_chunks.back().size() - 1;

            const auto patch = m_patches.back();
            m_patches.pop_back();

            if (patch.cf_forward) {
                m_result_chunks.back()[patch.instruction_pos].args[0] = patched_jmp_ip;
            } else {
                m_result_chunks.back()[patched_jmp_ip].args[0] = patch.target_ip;
            }
        } else {
            std::println("Invalid IR op at emit_oper_nonary in current BB...");
            return false;
        }

        return true;
    }

    auto Emitter::emit_oper_unary(const IR::Steps::OperUnary& oper_unary) -> bool {
        const auto& [aa_0, op] = oper_unary;
        const auto opcode_opt = ([](Op ir_op) noexcept -> std::optional<Opcode> {
            switch (ir_op) {
                case Op::make_seq: return Opcode::make_seq;
                case Op::frz_seq_obj: return Opcode::frz_seq_obj;
                case Op::jump: return Opcode::jump;
                case Op::jump_if: return Opcode::jump_if;
                case Op::jump_else: return Opcode::jump_else;
                case Op::ret: return Opcode::ret;
                case Op::halt: return Opcode::halt;
                default: return {};
            }
        })(op);

        if (!opcode_opt) {
            std::println("Invalid opcode at emit_oper_unary in current BB!");

            return false;
        }

        const auto opcode = opcode_opt.value();
        auto arg_0_opt = translate_value_aa(aa_0);

        if (!arg_0_opt) {
            std::println("Invalid opcode 'arg_0' at emit_oper_unary in current BB!");

            return false;
        }

        const auto arg_0 = arg_0_opt.value();

        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_0.value, 0, 0},
            .metadata = Utils::encode_metadata(arg_0),
            .op = opcode,
        });

        return true;
    }

    auto Emitter::emit_oper_binary(const IR::Steps::OperBinary& oper_binary) -> bool {
        const auto [aa_0, aa_1, op] = oper_binary;
        const auto opcode_opt = ([](Op ir_op) noexcept -> std::optional<Opcode> {
            switch (ir_op) {
                case Op::call: return Opcode::call;
                case Op::native_call: return Opcode::native_call;
                default: return {};
            }
        })(op);

        if (!opcode_opt) {
            std::println("Invalid opcode at emit_oper_unary in current BB!");

            return false;
        }

        const auto opcode = opcode_opt.value();
        auto arg_0_opt = translate_value_aa(aa_0);
        auto arg_1_opt = translate_value_aa(aa_1);

        if (!arg_0_opt || !arg_1_opt) {
            std::println("Invalid opcode 'arg_0' or `arg_1` at emit_oper_binary in current BB!");

            return false;
        }

        const auto arg_0 = arg_0_opt.value();
        const auto arg_1 = arg_1_opt.value();

        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_0.value, arg_1.value},
            .metadata = Utils::encode_metadata(arg_0, arg_1),
            .op = opcode,
        });

        return true;
    }

    auto Emitter::emit_oper_ternary(const IR::Steps::OperTernary& oper_ternary) -> bool {
        const auto& [aa_0, aa_1, aa_2, op] = oper_ternary;
        const auto opcode_opt = ([](Op op) noexcept -> std::optional<Opcode> {
            switch (op) {
            case Op::seq_obj_push: return Opcode::seq_obj_push;
            // case Op::seq_obj_pop: return Opcode::seq_obj_push;
            case Op::seq_obj_get: return Opcode::seq_obj_get;
            default: return {};
            }
        })(op);

        if (!opcode_opt) {
            std::println("Invalid opcode at emit_oper_ternary in current BB!");

            return false;
        }

        auto arg_0_opt = translate_value_aa(aa_0);
        auto arg_1_opt = translate_value_aa(aa_1);
        auto arg_2_opt = translate_value_aa(aa_2);

        if (!arg_0_opt || !arg_1_opt || !arg_2_opt) {
            std::println("Invalid opcode arg(s) at emit_oper_ternary in current BB!");

            return false;
        }

        const auto opcode = opcode_opt.value();
        const auto arg_0 = arg_0_opt.value();
        const auto arg_1 = arg_1_opt.value();
        const auto arg_2 = arg_2_opt.value();

        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_0.value, arg_1.value, arg_2.value},
            .metadata = encode_metadata(arg_0, arg_1, arg_2),
            .op = opcode,
        });

        return true;
    }

    auto Emitter::emit_step(const IR::Steps::Step& step) -> bool {
        if (auto tac_unary_p = std::get_if<IR::Steps::TACUnary>(&step); tac_unary_p) {
            /// NOTE: handle TAC SSA (1 operand)
            return emit_tac_unary(*tac_unary_p);
        } else if (auto tac_binary_p = std::get_if<IR::Steps::TACBinary>(&step); tac_binary_p) {
            /// NOTE: handle TAC SSA (2 operands)
            return emit_tac_binary(*tac_binary_p);
        } else if (auto oper_nonary_p = std::get_if<IR::Steps::OperNonary>(&step); oper_nonary_p) {
            /// NOTE: handle opcode-style IR (no operands)
            return emit_oper_nonary(*oper_nonary_p);
        } else if (auto oper_unary_p = std::get_if<IR::Steps::OperUnary>(&step); oper_unary_p) {
            /// NOTE: handle opcode-style IR (1 operands)
            return emit_oper_unary(*oper_unary_p);
        } else if (auto oper_binary_p = std::get_if<IR::Steps::OperBinary>(&step); oper_binary_p) {
            /// NOTE: handle opcode-style IR (2 operands)
            return emit_oper_binary(*oper_binary_p);
        } else if (auto oper_ternary_p = std::get_if<IR::Steps::OperTernary>(&step); oper_ternary_p) {
            /// NOTE: handle opcode-style IR (3 operands)
            return emit_oper_ternary(*oper_ternary_p);
        } else {
            return false;
        }
    }

    auto Emitter::emit_bb(const IR::CFG::BasicBlock& bb) -> bool {
        for (const auto& step : bb.steps) {
            if (!emit_step(step)) {
                return false;
            }
        }

        return true;
    }

    auto Emitter::emit_chunk(const IR::CFG::CFG& cfg) -> bool {
        std::set<int> visited_ids;
        std::stack<int> frontier;

        frontier.push(0);
        m_result_chunks.emplace_back();

        while (!frontier.empty()) {
            auto next_bb_id = frontier.top();
            frontier.pop();

            if (visited_ids.contains(next_bb_id)) {
                continue;
            }

            auto next_bb_opt = cfg.get_bb(next_bb_id);

            if (!next_bb_opt) {
                std::println(std::cerr, "Failed to get basic block #{} of CFG...", next_bb_id);

                return false;
            }

            auto next_bb_p = next_bb_opt.value();

            if (!emit_bb(*next_bb_p)) {
                std::println(std::cerr, "Failed to emit basic block #{} of CFG...", next_bb_id);

                return false;
            }

            visited_ids.insert(next_bb_id);

            if (const auto falsy_child_id = next_bb_p->falsy_id; falsy_child_id != -1) {
                frontier.push(falsy_child_id);
            }

            if (const auto truthy_child_id = next_bb_p->truthy_id; truthy_child_id != -1) {
                frontier.push(truthy_child_id);
            }
        }

        return true;
    }
}
