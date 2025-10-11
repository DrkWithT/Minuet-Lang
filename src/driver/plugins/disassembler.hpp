#ifndef MINUET_PLUGINS_DISASSEMBLER
#define MINUET_PLUGINS_DISASSEMBLER

#include <string_view>

#include "runtime/bytecode.hpp"
#include "driver/plugins/printer.hpp"

namespace Minuet::Driver::Plugins {
    template <std::size_t Idx>
    [[nodiscard]] auto arg_mode_name_of(const Runtime::Code::Instruction& inst) noexcept -> std::string_view {
        if constexpr (Idx >= 0 && Idx < 3) {
            constexpr uint16_t meta_tag_offset = static_cast<uint16_t>(2 + Idx * 4);
            const uint16_t meta_arg_tag_n = (inst.metadata & static_cast<uint16_t>(0b1111) << meta_tag_offset) >> meta_tag_offset;

            switch (meta_arg_tag_n) {
                case 0: return "imm";
                case 1: return "const";
                case 2: return "reg";
                case 3: return "stack";
                case 4: default: return "heap";
            }
        }

        return "??";
    }

    class Disassembler : public Printer {
    public:
        Disassembler() noexcept;

        void set_disable_flag(bool b) noexcept;
        void operator()(std::any bytecode_ref_wrap) const;

    private:
        void print_instruction(const Runtime::Code::Instruction& inst) const;
        void print_chunk(const Runtime::Code::Chunk& chunk) const;
        void print_program(const Runtime::Code::Program& program) const;

        bool m_disabled;

    };
}

#endif
