#ifndef PRINTING_HPP
#define PRINTING_HPP

#include "ir/steps.hpp"
#include "ir/cfg.hpp"

namespace Minuet::IR::Printing {
    void print_step(const IR::Steps::Step& step);
    void print_cfg(const CFG::CFG& cfg, uint32_t id);
    void print_ir(const CFG::FullIR& full_ir);
}

#endif
