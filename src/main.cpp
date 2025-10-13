#include <iostream>
#include <print>

#include "runtime/vm.hpp"
#include "driver/driver.hpp"
#include "driver/plugins/disassembler.hpp"
#include "driver/plugins/ir_dumper.hpp"

constexpr auto minuet_version_major = 0;
constexpr auto minuet_version_minor = 2;
constexpr auto minuet_version_patch = 0;


using namespace Minuet;

[[nodiscard]] auto native_print_value(Runtime::VM::Engine& vm, int16_t argc) -> bool {
    const auto& argument_value = vm.handle_native_fn_access(argc, 0);

    std::println("{}", argument_value.to_string());

    return true;
}

class DriverBuilder {
private:
    bool m_ir_printer_on;
    bool m_bc_printer_on;

public:
    DriverBuilder() noexcept
    : m_ir_printer_on {false}, m_bc_printer_on {false} {}

    [[nodiscard]] auto config_ir_dumper(bool enabled_flag) noexcept -> DriverBuilder* {
        m_ir_printer_on = enabled_flag;

        return this;
    }

    [[nodiscard]] auto config_bc_dumper(bool enabled_flag) noexcept -> DriverBuilder* {
        m_bc_printer_on = enabled_flag;

        return this;
    }

    [[nodiscard]] auto build() noexcept -> Driver::Driver {
        Driver::Driver interpreter_driver;

        Driver::Plugins::IRDumper ir_printer;
        ir_printer.set_disable_flag(!m_ir_printer_on);        
        Driver::Plugins::Disassembler bc_printer;
        bc_printer.set_disable_flag(!m_bc_printer_on);

        interpreter_driver.add_ir_dumper(ir_printer);
        interpreter_driver.add_disassembler(bc_printer);

        return interpreter_driver;
    }
};

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        std::println(std::cerr, "Invalid argument count, try 'minuetm info' for help.");

        return 1;
    }

    std::string arg_1 {argv[1]};
    std::string arg_2 { (argc >= 3) ? argv[2] : ""};

    DriverBuilder driver_builder;
    Driver::Driver app;

    if (arg_1 == "info") {
        std::println("minuetm v{}.{}.{}\n\nUsage: ./minuetm [info | compile-only <main-file> | run <main-file>]\n\tinfo []: shows usage info and version.", minuet_version_major, minuet_version_minor, minuet_version_patch);

        return 0;
    } else if (arg_1 == "compile-only" && !arg_2.empty()) {
        app = driver_builder.config_ir_dumper(true)->config_bc_dumper(true)->build();
    } else if (arg_1 == "run" && !arg_2.empty()) {
        app = driver_builder.config_ir_dumper(false)->config_bc_dumper(false)->build();
    } else {
        std::println("minuetm v{}.{}.{}\n\nUsage: ./minuetm [info | compile-only <main-file> | run <main-file>]\n\tinfo []: shows usage info and version.", minuet_version_major, minuet_version_minor, minuet_version_patch);

        return 1;
    }

    app.register_native_proc({"print", native_print_value});

    return app(arg_2) ? 0 : 1 ;
}
