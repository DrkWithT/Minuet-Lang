#ifndef DISASSEMBLE_HPP
#define DISASSEMBLE_HPP

#include "runtime/bytecode.hpp"

namespace Minuet::Codegen {
    void print_instruction(const Runtime::Code::Instruction& instr);

    void print_chunk(const Runtime::Code::Chunk& chunk);

    void print_program(const Runtime::Code::Program& prgm);
}

#endif
