#ifndef MINUET_PLUGINS_IR_DUMPER_HPP
#define MINUET_PLUGINS_IR_DUMPER_HPP

#include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "driver/plugins/printer.hpp"

namespace Minuet::Driver::Plugins {
    auto fmt_step_arg(IR::Steps::AbsAddress aa) -> std::string;

    class IRDumper : public Printer {
    public:
        IRDumper() noexcept;

        void set_disable_flag(bool b) noexcept override;
        void operator()(std::any ir_ref_wrap) const override;

    private:
        void print_step(const IR::Steps::Step& step) const;
        void print_cfg(const IR::CFG::CFG& cfg, uint32_t id) const;
        void print_ir(const IR::CFG::FullIR& full_ir) const;

        bool m_disabled;

    };
}

#endif
