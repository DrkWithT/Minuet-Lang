#ifndef MINUET_SYNTAX_AST_HPP
#define MINUET_SYNTAX_AST_HPP

#include <vector>
#include "syntax/stmts.hpp"

namespace Minuet::Syntax::AST {
    struct SourcedAST {
        Stmts::StmtPtr stmt_p;
        uint32_t src_id;
    };

    using UnitAST = std::vector<Stmts::StmtPtr>;
    using FullAST = std::vector<SourcedAST>;
}

#endif