#ifndef EMITTER_HPP
#define EMITTER_HPP

#include <optional>
#include <stack>
#include <unordered_map>
#include <vector>

// #include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "ir/pass.hpp"
#include "runtime/bytecode.hpp"

namespace Minuet::Codegen {
    namespace Utils {
        struct StackDelta {
            uint16_t val;
            bool shrinks;
        };

        struct JumpPatch {
            std::size_t instruction_pos;
            uint16_t dest_ip;
            bool cf_forward;
        };

        struct PseudoArg {
            uint16_t val;
            Runtime::Code::ArgMode tag;

            friend constexpr auto operator==(PseudoArg lhs, PseudoArg rhs) noexcept -> bool {
                return static_cast<std::size_t>(lhs.tag) == static_cast<std::size_t>(rhs.tag) && lhs.val == rhs.val;
            }
        };

        enum class OpcodeArity : uint8_t {
            nonary,
            unary,
            binary,
            ternary,
        };

        /// NOTE: Use this for nonary-opcode instructions only!
        [[nodiscard]] constexpr auto encode_inst_metadata() -> uint8_t {
            // The metadata encoding in a no-args opcode will be completely zeroed since the arity bits are `00` and the args are absent.
            return static_cast<uint8_t>(0b00000000);
        }

        [[nodiscard]] constexpr auto encode_inst_metadata(PseudoArg a0) noexcept -> uint8_t {
            const auto arity_prefix = static_cast<uint8_t>(0b01 << 6);
            const auto op_a0_n = static_cast<uint8_t>((static_cast<uint8_t>(a0.tag) & 0b00000011) << 4);

            return arity_prefix + op_a0_n;
        }

        [[nodiscard]] constexpr auto encode_inst_metadata(PseudoArg a0, PseudoArg a1) noexcept -> uint8_t {
            const auto arity_prefix = static_cast<uint8_t>(0b10 << 6);
            const auto op_a0_n = static_cast<uint8_t>((static_cast<uint8_t>(a0.tag) & 0b00000011) << 4);
            const auto op_a1_n = static_cast<uint8_t>((static_cast<uint8_t>(a1.tag) & 0b00000011) << 2);

            return arity_prefix + op_a0_n + op_a1_n;
        }

        [[nodiscard]] constexpr auto encode_inst_metadata(PseudoArg a0, PseudoArg a1, PseudoArg a2) noexcept -> uint8_t {
            const auto arity_prefix = static_cast<uint8_t>(0b11 << 6);
            const auto op_a0_n = static_cast<uint8_t>((static_cast<uint8_t>(a0.tag) & 0b00000011) << 4);
            const auto op_a1_n = static_cast<uint8_t>((static_cast<uint8_t>(a1.tag) & 0b00000011) << 2);
            const auto op_a2_n = static_cast<uint8_t>(static_cast<uint8_t>(a2.tag) & 0b00000011);

            return arity_prefix + op_a0_n + op_a1_n + op_a2_n;
        }
    }

    class Emitter : public IR::Pass::PassBase<bool> {
    public:
        Emitter();

        // void use_native_catalog(/* const NativeCatalog& native_ctg */) noexcept;

        [[nodiscard]] auto apply(const IR::CFG::CFG &cfg) -> bool override;

        [[maybe_unused]] auto apply(IR::CFG::CFG& cfg) -> bool override;

        [[nodiscard]] auto operator()(IR::CFG::FullIR& ir) -> std::optional<Runtime::Code::Program>;

    private:
        [[nodiscard]] auto next_stack_slot_id() noexcept -> std::optional<uint16_t>;

        void update_stack_slot_id(Utils::StackDelta normal_delta, std::optional<Utils::StackDelta> opt_delta) noexcept;

        [[nodiscard]] auto find_stack_slot_of(IR::Steps::AbsAddress aa) noexcept -> std::optional<Utils::PseudoArg>;

        /// TODO: Fix this function- It may cause an extra slot allocation during the wrong case if a TAC step is reached... a2 = a0 + a1 would accidentally allocate an extra slot for the `dest-value` at `a2`.
        [[nodiscard]] auto resolve_abs_addr(IR::Steps::AbsAddress aa) noexcept -> std::optional<Utils::PseudoArg>;

        /// NOTE: use for unary value operations i.e arithmetic or comparison computations
        [[nodiscard]] auto emit_step(IR::Steps::AbsAddress dest, IR::Steps::Op op, IR::Steps::AbsAddress a0) -> bool;

        /// NOTE: use for binary value operations similar to the previous overload `emit_step(PseudoArg, Op, PseudoArg)`
        [[nodiscard]] auto emit_step(IR::Steps::AbsAddress dest, IR::Steps::Op op, IR::Steps::AbsAddress a0, IR::Steps::AbsAddress a1) -> bool;

        /// NOTE: use for nonary op instructions
        [[nodiscard]] auto emit_step(IR::Steps::Op op) -> bool;

        /// NOTE: use for unary op instructions
        [[nodiscard]] auto emit_step(IR::Steps::Op op, IR::Steps::AbsAddress a0) -> bool;

        /// NOTE: use for binary op instructions
        [[nodiscard]] auto emit_step(IR::Steps::Op op, IR::Steps::AbsAddress a0, IR::Steps::AbsAddress a1) -> bool;

        // void emit_step(Runtime::Code::Opcode op, IR::Steps::AbsAddress a0, IR::Steps::AbsAddress a1, IR::Steps::AbsAddress a2);

        [[nodiscard]] auto emit_bb(const IR::CFG::BasicBlock& bb) -> bool;
        [[nodiscard]] auto emit_cfg(const IR::CFG::CFG& cfg) -> bool;
    
        static constexpr auto sm_dud_stack_delta = static_cast<int16_t>(100);
        static constexpr std::array<Utils::StackDelta, static_cast<std::size_t>(Runtime::Code::Opcode::last)> sm_op_stack_deltas = {
            Utils::StackDelta {.val = 0, .shrinks = false },
            Utils::StackDelta {.val = 1, .shrinks = false },
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 0, .shrinks = false},
            Utils::StackDelta {.val = 0, .shrinks = false},
            Utils::StackDelta {.val = 0, .shrinks = false},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 0, .shrinks = false},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = 1, .shrinks = true},
            Utils::StackDelta {.val = sm_dud_stack_delta, .shrinks = false},
            Utils::StackDelta {.val = sm_dud_stack_delta, .shrinks = false},
            Utils::StackDelta {.val = sm_dud_stack_delta, .shrinks = false},
            Utils::StackDelta {.val = 0, .shrinks = false},
        };

        std::stack<Utils::JumpPatch> m_local_patches;
        std::unordered_map<uint16_t, uint16_t> m_tmp_stack_slots;
        std::unordered_map<uint16_t, Utils::PseudoArg> m_other_locations;
        std::vector<Runtime::Code::Chunk> m_result_chunks;
        // const NativeCatalog* m_catalog_p;
        uint16_t m_local_stack_id;
    };
}

#endif
