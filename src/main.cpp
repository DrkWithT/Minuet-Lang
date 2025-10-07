#include <iostream>
#include <print>

#include "driver/driver.hpp"
// #include "codegen/disassemble.hpp"

constexpr auto minuet_version_major = 0;
constexpr auto minuet_version_minor = 0;
constexpr auto minuet_version_patch = 1;

using namespace Minuet;

int main(int argc, char* argv[]) {
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

    // Codegen::print_program(program_opt.value());
}
