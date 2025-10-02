#ifndef PASS_HPP
#define PASS_HPP

#include <concepts>

#include "ir/cfg.hpp"

namespace Minuet::IR::Pass {
    template <typename IRGraph, template <typename> typename Pass, typename Result>
    concept PassKind = requires(IRGraph cfg, Pass<Result> pass) {
        {auto(pass.apply(cfg))} -> std::same_as<Result>;
    };

    template <typename Result>
    class PassBase {
    public:
        virtual ~PassBase() noexcept = default;

        virtual auto apply(const CFG::CFG& cfg) -> Result = 0;
        virtual auto apply(CFG::CFG& cfg) -> Result = 0;
    };
}

#endif
