#ifndef MINUET_SYNTAX_STMTS_HPP
#define MINUET_SYNTAX_STMTS_HPP

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
    struct While;
    struct Block;
    struct Function;
    struct NativeStub;
    struct Import;

    // using StmtPtr = std::unique_ptr<StmtNode<ExprStmt, LocalDef, Match, MatchCase, Block, Function>>;
    using StmtPtr = std::unique_ptr<StmtNode<ExprStmt, LocalDef, If, Return, While, Block, Function, NativeStub, Import>>;

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

    struct While {
        Exprs::ExprPtr check;
        Stmts::StmtPtr body;
    };

    struct Block {
        std::vector<StmtPtr> items;
    };

    struct Function {
        std::vector<Frontend::Lexicals::Token> params;
        Frontend::Lexicals::Token name;
        StmtPtr body;
    };

    struct NativeStub {
        std::vector<Frontend::Lexicals::Token> params;
        Frontend::Lexicals::Token name;
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

    using Stmt = StmtNode<ExprStmt, LocalDef, If, Return, While, Block, Function, NativeStub, Import>;
}

#endif
