#ifndef STMTS_HPP
#define STMTS_HPP

#include <variant>
#include <vector>
#include "frontend/lexicals.hpp"
#include "syntax/exprs.hpp"

namespace Minuet::Syntax::Stmts {
    template <typename ... StmtTypes>
    struct StmtNode;
    struct ExprStmt;
    struct LocalDef;
    struct If;
    // struct Match;
    // struct MatchCase;
    struct Return;
    struct Block;
    struct Function;
    struct Import;

    // using StmtPtr = std::unique_ptr<StmtNode<ExprStmt, LocalDef, Match, MatchCase, Block, Function>>;
    using StmtPtr = std::unique_ptr<StmtNode<ExprStmt, LocalDef, If, Return, Block, Function, Import>>;

    struct ExprStmt {
        Exprs::ExprPtr expr;
    };

    struct LocalDef {
        Frontend::Lexicals::Token name;
        Exprs::ExprPtr init_expr;
    };

    struct If {
        Exprs::ExprPtr cond_expr;
        StmtPtr if_body;
        StmtPtr else_body;
    };

    // struct Match {
    //     std::vector<MatchCase> cases;
    // };

    // struct MatchCase {
    //     std::optional<Exprs::ExprPtr> value;
    //     StmtPtr body;
    // };

    struct Return {
        Exprs::ExprPtr result;
    };

    struct Block {
        std::vector<StmtPtr> items;
    };

    struct Function {
        std::vector<Frontend::Lexicals::Token> params;
        Frontend::Lexicals::Token name;
        StmtPtr body;
    };

    struct Import {
        Frontend::Lexicals::Token target;
    };

    template <typename ... StmtTypes>
    struct StmtNode {
        std::variant<StmtTypes...> data;
        uint32_t src_begin;
        uint32_t src_end;
    };

    // using Stmt = StmtNode<ExprStmt, LocalDef, If, Match, MatchCase, Block, Function>;
    using Stmt = StmtNode<ExprStmt, LocalDef, If, Return, Block, Function, Import>;
}

#endif
