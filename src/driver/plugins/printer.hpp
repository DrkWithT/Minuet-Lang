#ifndef MINUET_PLUGINS_PRINTER_HPP
#define MINUET_PLUGINS_PRINTER_HPP

#include <type_traits>
#include <print>
#include <any>

namespace Minuet::Driver::Plugins {
    template <typename P, typename D>
    concept PrinterKind = requires (P printer, std::any arg, bool disabled) {
        {printer(arg)};
        {printer.set_disable_flag(disabled)};
    } && std::is_default_constructible_v<P>;

    class Printer {
    public:
        virtual ~Printer() noexcept = default;

        virtual void set_disable_flag(bool b) noexcept = 0;
        virtual void operator()(std::any ir_ref_wrap) const = 0;
    };
}

#endif
