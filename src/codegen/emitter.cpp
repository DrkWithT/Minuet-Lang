#include <cstdlib>
#include <set>
#include <stack>
#include <optional>

#include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "runtime/bytecode.hpp"
#include "codegen/emitter.hpp"

namespace Minuet::Codegen {
    using namespace Codegen::Utils;
    using IR::Steps::AbsAddrTag;
    using IR::Steps::AbsAddress;
    using IR::Steps::Op;
    using IR::Steps::Step;
    using IR::CFG::FullIR;
    using Runtime::Code::Opcode;
    using Runtime::Code::ArgMode;
    using Runtime::Code::Instruction;
    using Runtime::Code::Chunk;
    using Runtime::Code::Program;


    Emitter::Emitter()
    : m_local_patches {}, m_tmp_stack_slots {}, m_result_chunks {}, m_local_stack_id {} {}

    auto Emitter::apply(const IR::CFG::CFG &cfg) -> bool {
        return emit_cfg(cfg);
    }

    auto Emitter::apply([[maybe_unused]] IR::CFG::CFG& cfg) -> bool {
        return true;
    }

    auto Emitter::operator()(FullIR& ir) -> std::optional<Program> {
        for (const auto& temp_cfg : ir.cfg_list) {
            if (!apply(temp_cfg)) {
                return {};
            }
        }

        return Program {
            .chunks = std::exchange(m_result_chunks, {}),
            .constants = std::exchange(ir.constants, {}),
            .entry_id = ir.main_id,
        };
    }


    auto Emitter::next_stack_slot_id() noexcept -> std::optional<uint16_t> {
        if (const auto next_stack_id = m_local_stack_id; next_stack_id < std::numeric_limits<uint16_t>::max()) {
            ++m_local_stack_id;

            return next_stack_id;
        }

        return {};
    }

    void Emitter::update_stack_slot_id(Utils::StackDelta normal_delta, std::optional<Utils::StackDelta> opt_delta) noexcept {
        const auto [delta_n, delta_pops] = ([&normal_delta, &opt_delta]() {
            if (opt_delta) {
                return opt_delta.value();
            }

            return normal_delta;
        })();
        
        if (delta_pops) {
            m_local_stack_id -= delta_n;
        } else {
            m_local_stack_id += delta_n;
        }
    }
    
    [[nodiscard]] auto Emitter::find_stack_slot_of(IR::Steps::AbsAddress aa) noexcept -> std::optional<Utils::PseudoArg> {
        const auto aa_loc_id = aa.id;

        if (!m_tmp_stack_slots.contains(aa_loc_id)) {
            return {};
        }

        const auto stack_slot_n = m_tmp_stack_slots[aa_loc_id];

        return PseudoArg {
            .val = stack_slot_n,
            .tag = ArgMode::stack,
        };
    }

    auto Emitter::resolve_abs_addr(IR::Steps::AbsAddress aa) noexcept -> std::optional<Utils::PseudoArg> {
        const auto [tag, id] = aa;

        switch (tag) {
            case AbsAddrTag::immediate: return PseudoArg {
                .tag = ArgMode::immediate,
                .val = id,
            };
            case AbsAddrTag::constant: return PseudoArg {
                .tag = ArgMode::constant,
                .val = id,
            };
            case AbsAddrTag::temp: {
                if (!m_tmp_stack_slots.contains(id)) {
                    if (const auto next_slot_opt = next_stack_slot_id(); next_slot_opt) {
                        const auto next_stack_n = next_slot_opt.value();

                        m_tmp_stack_slots[id] = next_stack_n;

                        const auto temp_arg = PseudoArg {
                            .tag = ArgMode::stack,
                            .val = next_stack_n,
                        };

                        return temp_arg;
                    }

                    return {};
                }

                return find_stack_slot_of(aa);
            };
            case AbsAddrTag::heap: return PseudoArg {
                .tag = ArgMode::heap,
                .val = id,
            };
            default: return {};
        }
    }

    auto Emitter::emit_step(AbsAddress dest, Op op, AbsAddress a0) -> bool {
        auto a0_arg_opt = resolve_abs_addr(a0);
        auto dest_arg_opt = resolve_abs_addr(dest);

        if (!dest_arg_opt || !a0_arg_opt) {
            return false;
        }

        auto dest_arg = dest_arg_opt.value();
        auto arg_0 = a0_arg_opt.value();

        if (op == Op::nop) {
            m_result_chunks.back().emplace_back(Instruction {
                .args = {dest_arg.val, arg_0.val, 0},
                .metadata = Utils::encode_inst_metadata(dest_arg, arg_0),
                .op = Opcode::mov,
            });

            /// NOTE: Update pre-calculated stack offset by custom offset if passed in the optional- Otherwise update by the opcode's stack delta.
            update_stack_slot_id(sm_op_stack_deltas[static_cast<std::size_t>(Opcode::mov)], {});

            return true;
        } else if (op == Op::neg) {
            m_result_chunks.back().emplace_back(Instruction {
                .args = {arg_0.val, 0, 0},
                .metadata = Utils::encode_inst_metadata(arg_0),
                .op = Opcode::neg,
            });

            if (arg_0 != dest_arg) {
                m_result_chunks.back().emplace_back(Instruction {
                    .args = {dest_arg.val, arg_0.val, 0},
                    .metadata = Utils::encode_inst_metadata(dest_arg, arg_0),
                    .op = Opcode::mov,
                });
            }

            update_stack_slot_id(sm_op_stack_deltas[static_cast<std::size_t>(Opcode::mov)], {});

            return true;
        }

        return false;
    }

    auto Emitter::emit_step(AbsAddress dest, Op op, AbsAddress a0, AbsAddress a1) -> bool {
        const auto bin_math_or_logical_opcode = ([](Op ir_op) noexcept {
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
                default: return Opcode::nop;
            }
        })(op);

        if (bin_math_or_logical_opcode == Opcode::nop) {
            return false;
        }

        auto arg_0_opt = resolve_abs_addr(a0);
        auto arg_1_opt = resolve_abs_addr(a1);
        auto dest_opt = resolve_abs_addr(dest);

        if (!dest_opt || !arg_0_opt || !arg_1_opt) {
            return false;
        }

        auto dest_arg = dest_opt.value();
        auto arg_0 = arg_0_opt.value();
        auto arg_1 = arg_1_opt.value();

        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_0.val, 0, 0},
            .metadata = encode_inst_metadata(arg_0),
            .op = Opcode::push,
        });
        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_1.val, 0, 0},
            .metadata = encode_inst_metadata(arg_1),
            .op = Opcode::push,
        });
        update_stack_slot_id(Utils::StackDelta {
            .val = 2,
            .shrinks = false,
        }, {});

        m_result_chunks.back().emplace_back(Instruction {
            .args = {0, 0, 0},
            .metadata = encode_inst_metadata(),
            .op = bin_math_or_logical_opcode,
        });
        update_stack_slot_id(sm_op_stack_deltas[static_cast<std::size_t>(bin_math_or_logical_opcode)], {});

        /// NOTE: the MOV instruction simply takes the top temporary and replaces the destination iff all other arg-slots are zeroed.
        m_result_chunks.back().emplace_back(Instruction {
            .args = {dest_arg.val, 0, 0},
            .metadata = encode_inst_metadata(dest_arg),
            .op = Opcode::mov,
        });
        update_stack_slot_id(Utils::StackDelta {
            .val = 1,
            .shrinks = true,
        }, {});

        return true;
    }

    auto Emitter::emit_step(Op op) -> bool {
        if (op == Op::nop) {
            m_result_chunks.back().emplace_back(Instruction {
                .args = {0, 0, 0},
                .metadata = encode_inst_metadata(),
                .op = Opcode::nop,
            });
        } else if (op == Op::pop) {
            m_result_chunks.back().emplace_back(Instruction {
                .args = {0, 0, 0},
                .metadata = encode_inst_metadata(),
                .op = Opcode::pop,
            });

            update_stack_slot_id(StackDelta {
                .val = 1,
                .shrinks = true,
            }, {});
        } else if (op == Op::meta_save_patch) {
            /// NOTE: The target jump to patch requires an offset of `N - 1` where N is the current length of emitted chunk instructions. This cannot be `<= N - 2` because `meta_begin_xxxx, meta_end_xxxx` IR opcodes are markers for jump patching.
            const auto jmp_pos = m_result_chunks.back().size() - 1;

            m_local_patches.push({
                .instruction_pos = jmp_pos,
                .dest_ip = 0,
                .cf_forward = true,
            });
        } else if (op == Op::meta_patch_jmp_else) {
            const auto jmp_else_patch_n = m_result_chunks.back().size() - 1;
            const auto [inst_pos, _, cf_forward] = m_local_patches.top();
            m_local_patches.pop();

            m_result_chunks.back().at(inst_pos).args[1] = static_cast<uint16_t>(jmp_else_patch_n);
        } else if (op == Op::meta_patch_jmp) {
            const auto jmp_else_patch_n = m_result_chunks.back().size() - 1;
            const auto [inst_pos, _, cf_forward] = m_local_patches.top();
            m_local_patches.pop();

            m_result_chunks.back().at(inst_pos).args[0] = static_cast<uint16_t>(jmp_else_patch_n);
        } else {
            return false;
        }

        return true;
    }

    auto Emitter::emit_step(Op op, AbsAddress a0) -> bool {
        auto unary_opcode_opt = ([](Op ir_op) noexcept -> std::optional<Opcode> {
            switch (ir_op) {
                case Op::meta_load_aa:
                case Op::push: return Opcode::push;
                case Op::jump: return Opcode::jump;
                case Op::ret: return Opcode::ret;
                case Op::halt: return Opcode::halt;
                default: return {};
            }
        })(op);

        if (!unary_opcode_opt) {
            return false;
        }

        const auto arg_0_opt = resolve_abs_addr(a0);

        if (!arg_0_opt) {
            return false;
        }

        const auto unary_opcode = unary_opcode_opt.value();
        const auto arg_0 = arg_0_opt.value();

        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_0.val, 0, 0},
            .metadata = encode_inst_metadata(arg_0),
            .op = unary_opcode,
        });

        if (unary_opcode == Opcode::push) {
            update_stack_slot_id(sm_op_stack_deltas[static_cast<std::size_t>(unary_opcode)], {});
        }

        return true;
    }

    auto Emitter::emit_step(Op op, AbsAddress a0, AbsAddress a1) -> bool {
        auto binary_opcode_opt = ([](Op ir_op) noexcept -> std::optional<Opcode> {
            switch (ir_op) {
                case Op::jump_if: return Opcode::jump_if;
                case Op::jump_else: return Opcode::jump_else;
                case Op::call: return Opcode::call;
                case Op::native_call: return Opcode::native_call;
                default: return {};
            }
        })(op);

        if (!binary_opcode_opt) {
            return false;
        }

        const auto binary_opcode = binary_opcode_opt.value();

        auto arg_0_opt = resolve_abs_addr(a0);
        auto arg_1_opt = resolve_abs_addr(a1);

        if (!arg_0_opt || !arg_1_opt) {
            return false;
        }

        auto arg_0 = arg_0_opt.value();
        auto arg_1 = arg_1_opt.value();
        const auto arg_count = arg_1.val;

        m_result_chunks.back().emplace_back(Instruction {
            .args = {arg_0.val, arg_count, 0},
            .metadata = encode_inst_metadata(arg_0, arg_1),
            .op = binary_opcode,
        });

        if (binary_opcode == Opcode::call) {
            update_stack_slot_id(sm_op_stack_deltas[static_cast<std::size_t>(binary_opcode)], StackDelta {
                .val = 1,
                .shrinks = false,
            });
            update_stack_slot_id(sm_op_stack_deltas[static_cast<std::size_t>(binary_opcode)], StackDelta {
                .val = arg_count,
                .shrinks = true,
            });
        }

        return true;
    }

    /// NOTE: use for ternary op instructions
    // void emit_step(Op op, AbsAddress a0, AbsAddress a1, AbsAddress a2);

    auto Emitter::emit_bb(const IR::CFG::BasicBlock& bb) -> bool {
        auto ok = true;

        for (const auto& ir_step : bb.steps) {
            if (const auto tac_unary_p = std::get_if<IR::Steps::TACUnary>(&ir_step); tac_unary_p) {
                ok = emit_step(tac_unary_p->dest, tac_unary_p->op, tac_unary_p->arg_0);
            } else if (const auto tac_binary_p = std::get_if<IR::Steps::TACBinary>(&ir_step); tac_binary_p) {
                ok = emit_step(tac_binary_p->dest, tac_binary_p->op, tac_binary_p->arg_0, tac_binary_p->arg_1);
            } else if (const auto oper_nonary_p = std::get_if<IR::Steps::OperNonary>(&ir_step); oper_nonary_p) {
                ok = emit_step(oper_nonary_p->op);
            } else if (const auto oper_unary_p = std::get_if<IR::Steps::OperUnary>(&ir_step); oper_unary_p) {
                ok = emit_step(oper_unary_p->op, oper_unary_p->arg_0);
            } else if (const auto oper_binary_p = std::get_if<IR::Steps::OperBinary>(&ir_step); oper_binary_p) {
                ok = emit_step(oper_binary_p->op, oper_binary_p->arg_0, oper_binary_p->arg_1);
            } else {
                /// NOTE: unreachable, invalid case here...
                ok = false;
            }

            if (!ok) {
                break;
            }
        }

        return ok;
    }

    auto Emitter::emit_cfg(const IR::CFG::CFG& cfg) -> bool {
        std::set<int> visited;
        std::stack<int> bb_id_frontier;

        bb_id_frontier.push(0);
        m_result_chunks.emplace_back();

        while (!bb_id_frontier.empty()) {
            auto next_bb_id = bb_id_frontier.top();
            bb_id_frontier.pop();

            if (visited.contains(next_bb_id)) {
                continue;
            }

            auto next_bb_opt = cfg.get_bb(next_bb_id);

            if (!next_bb_opt) {
                continue;
            }

            auto next_bb = next_bb_opt.value();

            if (!emit_bb(*next_bb)) {
                return false;
            }

            visited.emplace(next_bb_id);

            if (const auto falsy_bb_id = next_bb->falsy_id; falsy_bb_id != -1 && !visited.contains(falsy_bb_id)) {
                bb_id_frontier.push(falsy_bb_id);
            }

            if (const auto truthy_bb_id = next_bb->truthy_id; truthy_bb_id != -1 && !visited.contains(truthy_bb_id)) {
                bb_id_frontier.push(truthy_bb_id);
            }
        }

        m_local_patches = {};
        m_tmp_stack_slots.clear();
        m_local_stack_id = 0U;

        return true;
    }
}
