#ifndef EMITTER_HPP
#define EMITTER_HPP

#include <optional>
#include <vector>

#include "ir/cfg.hpp"
#include "runtime/bytecode.hpp"

namespace Minuet::Codegen {
    namespace Utils {
        struct Patch {
            std::size_t instruction_pos;
            int16_t target_ip;
            bool cf_forward;
        };

        struct PseudoArg {
            int16_t value;
            Runtime::Code::ArgMode tag;

            [[nodiscard]] friend constexpr auto operator==(PseudoArg lhs, PseudoArg rhs) noexcept {
                return lhs.tag == rhs.tag && lhs.value == rhs.value;
            }
        };

        [[nodiscard]] constexpr auto encode_metadata() -> uint16_t {
            return static_cast<uint16_t>(0);
        }

        [[nodiscard]] constexpr auto encode_metadata(PseudoArg a0) -> uint16_t {
            constexpr auto arity_n = static_cast<uint16_t>(0b01);
            const auto arg_mode_0 = static_cast<uint16_t>(a0.tag) << 2;

            return arity_n + arg_mode_0;
        }

        [[nodiscard]] constexpr auto encode_metadata(PseudoArg a0, PseudoArg a1) -> uint16_t {
            constexpr auto arity_n = static_cast<uint16_t>(0b10);
            const auto arg_mode_0 = static_cast<uint16_t>(a0.tag) << 2;
            const auto arg_mode_1 = static_cast<uint16_t>(a1.tag) << 6;

            return arity_n + arg_mode_0 + arg_mode_1;
        }

        [[nodiscard]] constexpr auto encode_metadata(PseudoArg a0, PseudoArg a1, PseudoArg a2) -> uint16_t {
            constexpr auto arity_n = static_cast<uint16_t>(0b11);
            const auto arg_mode_0 = static_cast<uint16_t>(a0.tag) << 2;
            const auto arg_mode_1 = static_cast<uint16_t>(a1.tag) << 6;
            const auto arg_mode_2 = static_cast<uint16_t>(a2.tag) << 10;

            return arity_n + arg_mode_0 + arg_mode_1 + arg_mode_2;
        }
    }

    class Emitter {
    public:
        Emitter();

        [[nodiscard]] auto operator()(IR::CFG::FullIR& ir) -> std::optional<Runtime::Code::Program>;

    private:
        [[nodiscard]] auto translate_value_aa(IR::Steps::AbsAddress aa) noexcept -> std::optional<Utils::PseudoArg>;
        [[nodiscard]] auto gen_function_id() noexcept -> std::optional<int16_t>;

        [[nodiscard]] auto emit_tac_unary(const IR::Steps::TACUnary& tac_unary) -> bool;
        [[nodiscard]] auto emit_tac_binary(const IR::Steps::TACBinary& tac_binary) -> bool;
        [[nodiscard]] auto emit_oper_nonary(const IR::Steps::OperNonary& oper_nonary) -> bool;
        [[nodiscard]] auto emit_oper_unary(const IR::Steps::OperUnary& oper_unary) -> bool;
        [[nodiscard]] auto emit_oper_binary(const IR::Steps::OperBinary& oper_binary) -> bool;
        [[nodiscard]] auto emit_step(const IR::Steps::Step& step) -> bool;

        [[nodiscard]] auto emit_bb(const IR::CFG::BasicBlock& bb) -> bool;
        [[nodiscard]] auto emit_chunk(const IR::CFG::CFG& cfg) -> bool;

        std::vector<Runtime::Code::Chunk> m_result_chunks;
        std::vector<Utils::Patch> m_patches;
        int16_t m_next_fun_id;
    };
}

#endif
