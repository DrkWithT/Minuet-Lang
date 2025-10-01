#ifndef STMTS_HPP
#define STMTS_HPP

#include <optional>
#include <variant>
#include <vector>
#include "frontend/lexicals.hpp"
#include "syntax/exprs.hpp"

namespace Minuet::Syntax::Stmts {
    template <typename ... StmtTypes>
    struct StmtNode;
    struct ExprStmt;
    struct LocalDef;
    struct Match;
    struct MatchCase;
    struct Block;
    struct Function;

    using StmtPtr = std::unique_ptr<StmtNode<ExprStmt, LocalDef, Match, MatchCase, Block, Function>>;

    struct ExprStmt {
        Exprs::ExprPtr expr;
    };

    struct LocalDef {
        Frontend::Lexicals::Token name;
        Exprs::ExprPtr init_expr;
    };

    struct Match {
        std::vector<MatchCase> cases;
    };

    struct MatchCase {
        std::optional<Exprs::ExprPtr> value;
        StmtPtr body;
    };

    struct Block {
        std::vector<StmtPtr> items;
    };

    struct Function {
        Frontend::Lexicals::Token name;
    };

    template <typename ... StmtTypes>
    struct StmtNode {
        std::variant<StmtTypes...> data;
        uint32_t src_begin;
        uint32_t src_end;
    };
}

#endif
