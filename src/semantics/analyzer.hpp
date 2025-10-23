#ifndef MINUET_SEMA_ANALYZER_HPP
#define MINUET_SEMA_ANALYZER_HPP

#include <string>
#include <array>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>

#include "frontend/lexicals.hpp"
#include "semantics/enums.hpp"
#include "syntax/exprs.hpp"
#include "syntax/stmts.hpp"
#include "syntax/ast.hpp"

namespace Minuet::Semantics {
    struct DudAttr {};

    struct SemanticItem {
        std::variant<DudAttr, bool, int, double> extra; // more metadata e.g tuple length
        Enums::EntityKinds entity_kind;
        Enums::ValueGroup value_group;
        bool readonly;

        auto to_lvalue() const noexcept -> SemanticItem;
    };

    struct Scope {
        std::unordered_map<std::string, SemanticItem> items;
        std::string name;
    };
    
    class Analyzer {
    private:
        /// Cross-lookup table for all 5 kinds by ops: access, negate, mul, div, mod, add, sub, equality, inequality, lesser, greater, at_most, at_least, assign... Compacts and hastens semantic checks for type-kinds by their valid operations.
        static constexpr std::array<std::array<bool, 14>, 5> cm_table = {
            std::array<bool, 14> {true, true, true, true, true, true, true, true, true, true, true, true, true},
            std::array<bool, 14> {false, true, true, true, true, true, true, true, true, true, true, true, true}, // lookups for primitive kind
            {true, false, false, false, false, false, false, false, false, false, false, false, false, true}, // lookups for tuple kind
            {true, false, false, false, false, false, false, false, false, false, false, false, false, true}, // lookups for flexible kind
            {false, false, false, false, false, false, false, false, false, false, false, false, false, true}, // lookups for callable
        };
        std::vector<Scope> m_scopes;
        bool m_prepassing;

        /// NOTE: for simple errors which consider an area of code.
        void report_error(const std::string& msg, const std::string& source, int area_begin, int area_end);

        /// NOTE: for very simple errors where no token lookup is needed.
        void report_error(int line, const std::string& msg);

        /// NOTE: for more complex errors needing the offending token's information.
        void report_error(const Frontend::Lexicals::Token& culprit, const std::string& msg, const std::string& source);

        void enter_scope(const std::string& name_str);
        void leave_scope();
        [[nodiscard]] auto lookup_named_item(const std::string& name) const& noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto record_named_item(const std::string& name, SemanticItem item) -> bool;

        /// NOTE: overload for unary exprs
        [[nodiscard]] auto check_op_by_kinds(Enums::Operator op, const SemanticItem& inner) const noexcept -> bool;

        /// NOTE: overload for binary exprs
        [[nodiscard]] auto check_op_by_kinds(Enums::Operator op, const SemanticItem& lhs, const SemanticItem& rhs) const noexcept -> bool;

        /// NOTE: for assignments
        [[nodiscard]] auto check_assignability(const SemanticItem& lhs) const noexcept -> bool;

        [[nodiscard]] auto check_literal(const Syntax::Exprs::Literal& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto check_sequence(const Syntax::Exprs::Sequence& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto check_lambda(const Syntax::Exprs::Lambda& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto check_call(const Syntax::Exprs::Call& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto check_unary(const Syntax::Exprs::Unary& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto check_binary(const Syntax::Exprs::Binary& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;
        [[nodiscard]] auto check_assign(const Syntax::Exprs::Assign& expr, const std::string& source) noexcept -> std::optional<SemanticItem>;

        [[nodiscard]] auto check_expr(const Syntax::Exprs::ExprPtr& expr_p, const std::string& source) noexcept -> std::optional<SemanticItem>;

        [[nodiscard]] auto check_expr_stmt(const Syntax::Stmts::ExprStmt& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_local_def(const Syntax::Stmts::LocalDef& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_if(const Syntax::Stmts::If& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_return(const Syntax::Stmts::Return& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_while(const Syntax::Stmts::While& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_break(const Syntax::Stmts::Break& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_block(const Syntax::Stmts::Block& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_function(const Syntax::Stmts::Function& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_native_stub(const Syntax::Stmts::NativeStub& stmt, const std::string& source) noexcept -> bool;
        [[nodiscard]] auto check_import(const Syntax::Stmts::Import& stmt, const std::string& source) noexcept -> bool;

        [[nodiscard]] auto check_stmt(const Syntax::Stmts::StmtPtr& stmt_p, const std::string& source) noexcept -> bool;

    public:
        Analyzer();

        [[nodiscard]] auto operator()(const Syntax::AST::FullAST& ast, const std::unordered_map<uint32_t, std::string>& src_map) -> bool;
    };
}

#endif
