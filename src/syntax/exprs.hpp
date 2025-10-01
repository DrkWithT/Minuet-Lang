#include "frontend/lexicals.hpp"
#include "semantics/enums.hpp"
#include <variant>
#include <memory>
#include <vector>

namespace Minuet::Syntax::Exprs {
    template <typename ... ExprTypes>
    struct ExprNode;
    struct Literal;
    struct Call;
    struct Unary;
    struct Binary;
    struct Assign;

    using ExprPtr = std::unique_ptr<ExprNode<Literal, Call, Unary, Binary, Assign>>;
    
    struct Literal {
        Frontend::Lexicals::Token token;
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

    using Expr = ExprNode<Literal, Call, Unary, Binary, Assign>;
}
