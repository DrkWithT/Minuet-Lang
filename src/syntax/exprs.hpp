#ifndef MINUET_SYNTAX_EXPRS_HPP
#define MINUET_SYNTAX_EXPRS_HPP

#include "frontend/lexicals.hpp"
#include "semantics/enums.hpp"
#include <variant>
#include <memory>
#include <vector>

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
    struct Break;
    struct Block;
    struct Function;
    struct NativeStub;
    struct Import;

    using StmtPtr = std::unique_ptr<StmtNode<ExprStmt, LocalDef, If, Return, While, Break, Block, Function, NativeStub, Import>>;
}

namespace Minuet::Syntax::Exprs {
    template <typename ... ExprTypes>
    struct ExprNode;
    struct Literal;
    struct Sequence;
    struct Lambda;
    struct Call;
    struct Unary;
    struct Binary;
    struct Assign;

    using ExprPtr = std::unique_ptr<ExprNode<Literal, Sequence, Lambda, Call, Unary, Binary, Assign>>;

    struct Literal {
        Frontend::Lexicals::Token token;
    };

    struct Sequence {
        std::vector<ExprPtr> items;
        bool is_tuple;
    };

    struct Lambda {
        std::vector<Frontend::Lexicals::Token> params;
        Stmts::StmtPtr body;
    };

    struct Call {
        std::vector<ExprPtr> args;
        ExprPtr callee;
    };

    struct Unary {
        ExprPtr inner;
        Minuet::Semantics::Enums::Operator op;
    };

    struct Binary {
        ExprPtr left;
        ExprPtr right;
        Minuet::Semantics::Enums::Operator op;
    };

    struct Assign {
        ExprPtr left;
        ExprPtr value;
    };

    template <typename ... ExprTypes>
    struct ExprNode {
        std::variant<ExprTypes...> data;
        uint32_t src_begin;
        uint32_t src_end;
    };

    using Expr = ExprNode<Literal, Sequence, Lambda, Call, Unary, Binary, Assign>;
}

#endif
