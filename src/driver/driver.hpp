#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <filesystem>
#include "frontend/lexing.hpp"

namespace Minuet::Driver::Compilation {   
    class CompileDriver {
    public:
        CompileDriver();

        /// TODO: refactor to return full program AST from traversing includes.
        [[nodiscard]] bool parse_sources(const std::filesystem::path& main_path);

    private:
        Frontend::Lexing::Lexer m_lexer;
    };
}

#endif
