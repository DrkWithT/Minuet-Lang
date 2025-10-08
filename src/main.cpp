#include <iostream>
#include <print>
#include <chrono>

#include "driver/driver.hpp"
#include "codegen/disassemble.hpp"
#include "runtime/vm.hpp"

constexpr auto minuet_version_major = 0;
constexpr auto minuet_version_minor = 0;
constexpr auto minuet_version_patch = 1;

using namespace Minuet;

int main(int argc, char* argv[]) {
    using Runtime::VM::Utils::EngineConfig;
    using Runtime::VM::Utils::ExecStatus;

    if (argc < 2) {
        std::println(std::cerr, "Usage: ./minuetm [info | <program-file>]\n\tinfo []: shows usage info and version.");
        return 1;
    }

    std::string arg_1 {argv[1]};

    if (arg_1 == "info") {
        std::println("minuetm v{}.{}.{}\n\nUsage: ./minuetm [info | <program-file>]\n\tinfo []: shows usage info and version.", minuet_version_major, minuet_version_minor, minuet_version_patch);
        return 0;
    }

    Driver::Compilation::CompileDriver compiler;
    
    auto program_opt = compiler(argv[1]);

    if (!program_opt) {
        return 1;
    }

    auto program = program_opt.value();

    Codegen::print_program(program);

    Runtime::VM::Engine engine {EngineConfig {
        .reg_buffer_limit = 8192,
        .stack_limit = 1024,
        .call_frame_max = 512,
    }, program};

    auto run_start = std::chrono::steady_clock::now();
    const auto exec_status = engine();
    auto run_end = std::chrono::steady_clock::now();

    std::println("Finished in: {}\n", std::chrono::duration_cast<std::chrono::milliseconds>(run_end - run_start));

    switch (exec_status) {
        case ExecStatus::ok:
            std::println("\033[1;32mStatus OK\033[0m\n");
            return 0;
        default:
            std::println(std::cerr, "\033[1;31mRuntime Error: Exited with ExecStatus #{}, see vm.md for details.\n", static_cast<int>(exec_status));
            return 1;
    }
}
