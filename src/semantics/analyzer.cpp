#include <iostream>
#include <print>
#include "semantics/analyzer.hpp"

namespace Minuet::Semantics {
    using Frontend::Lexicals::token_length;
    using Frontend::Lexicals::token_to_sv;
    using Enums::operator_name;


    auto SemanticItem::to_lvalue() const noexcept -> SemanticItem {
        return {
            .extra = this->extra,
            .entity_kind = this->entity_kind,
            .value_group = Enums::ValueGroup::locator,
            .readonly = false,
        };
    }


    void Analyzer::report_error(const std::string& msg, const std::string& source, int area_begin, int area_end) {
        std::string_view src_sv {source};
        
        std::println(std::cerr, "\033[1;31mSemantic Error\033[0m [In scope '{}']: {}\n{}\n", m_scopes.back().name, msg, src_sv.substr(area_begin, area_end - area_begin));
    }

    void Analyzer::report_error(int line, const std::string& msg) {
        std::println(std::cerr, "\033[1;31mSemantic Error\033[0m [In scope '{}', ln {}]: {}",  m_scopes.back().name, line, msg);
    }

    void Analyzer::report_error(const Frontend::Lexicals::Token& culprit, const std::string& msg, const std::string& source) {
        const auto& [type, culprit_start, culprit_end, line, column] = culprit;
        std::string_view src_sv {source};

        // 1. print error header as context of following source snippet...
        std::println(std::cerr, "\033[1;31mSemantic Error\033[0m [In scope '{}', ln {}, col {}]: {}\n", m_scopes.back().name, line, column, msg);

        // 2. color bad token for ease of notice
        std::print(std::cerr, "\033[1;33mCulprit\033[0m: '{}'", token_to_sv(culprit, src_sv));
    }

    void Analyzer::enter_scope(const std::string& name_str) {
        m_scopes.emplace_back(Scope {
            .items = {},
            .name = name_str,
        });
    }

    void Analyzer::leave_scope() {
        m_scopes.pop_back();
    }

    auto Analyzer::lookup_named_item(const std::string& name) const& noexcept -> std::optional<SemanticItem> {
        for (int scope_idx = m_scopes.size() - 1; scope_idx >= 0; --scope_idx) {
            if (const auto& entry_opt = m_scopes[scope_idx].items.find(name); entry_opt != m_scopes[scope_idx].items.cend()) {
                return entry_opt->second;
            }
        }

        return {};
    }

    auto Analyzer::record_named_item(const std::string& name, SemanticItem item) -> bool {
        if (auto& current_scope = m_scopes.back(); !current_scope.items.contains(name)) {
            current_scope.items[name] = item;
            return true;
        }

        return false;
    }

    auto Analyzer::check_op_by_kinds(Enums::Operator op, const SemanticItem& inner) const noexcept -> bool {
        const auto inner_kind_id = static_cast<std::size_t>(inner.entity_kind);
        const auto ast_operator_id = static_cast<std::size_t>(op);

        return cm_table[inner_kind_id][ast_operator_id];
    }

    auto Analyzer::check_op_by_kinds(Enums::Operator op, const SemanticItem& lhs, const SemanticItem& rhs) const noexcept -> bool {
        const auto lhs_type_kind_id = static_cast<std::size_t>(lhs.entity_kind);
        const auto rhs_type_kind_id = static_cast<std::size_t>(rhs.entity_kind);
        const auto ast_operator_id = static_cast<std::size_t>(op);

        return (lhs.entity_kind == Enums::EntityKinds::anything || rhs.entity_kind == Enums::EntityKinds::anything) || (cm_table[lhs_type_kind_id][ast_operator_id] && cm_table[rhs_type_kind_id][ast_operator_id]);
    }

    auto Analyzer::check_assignability(const SemanticItem& lhs) const noexcept -> bool {
        const auto& [foo_0, foo_1, lhs_value_group, lhs_is_immutable] = lhs;

        return !lhs_is_immutable && lhs_value_group == Enums::ValueGroup::locator;
    }


    auto Analyzer::check_literal(const Syntax::Exprs::Literal& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        const auto literal_token = expr.token;
        auto literal_lexeme = source.substr(literal_token.start, token_length(literal_token));

        auto result_info = ([&, this]() -> std::optional<SemanticItem>  {
            switch (const auto expr_token_tag = literal_token.type; expr_token_tag) {
            case Frontend::Lexicals::TokenType::literal_true:
            case Frontend::Lexicals::TokenType::literal_false:
                return SemanticItem {
                    .extra = literal_lexeme == "true",
                    .entity_kind = Enums::EntityKinds::primitive,
                    .value_group = Enums::ValueGroup::temporary,
                    .readonly = true,
                };
            case Frontend::Lexicals::TokenType::literal_int:
                return SemanticItem {
                    .extra = std::stoi(literal_lexeme),
                    .entity_kind = Enums::EntityKinds::primitive,
                    .value_group = Enums::ValueGroup::temporary,
                    .readonly = true,
                };
            case Frontend::Lexicals::TokenType::literal_double:
                return SemanticItem {
                    .extra = std::stof(literal_lexeme),
                    .entity_kind = Enums::EntityKinds::primitive,
                    .value_group = Enums::ValueGroup::temporary,
                    .readonly = true,
                };
            case Frontend::Lexicals::TokenType::identifier:
            default:
                return lookup_named_item(literal_lexeme);
            }
        })();

        if (!result_info) {
            report_error(literal_token, "Undeclared name!", source);
        }

        return result_info;
    }

    auto Analyzer::check_sequence(const Syntax::Exprs::Sequence& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        for (const auto& item_expr : expr.items) {
            if (!check_expr(item_expr, source).has_value()) {
                return {};
            }
        }

        return SemanticItem {
            .extra = DudAttr {},
            .entity_kind = (expr.is_tuple)
                ? Enums::EntityKinds::sequence_fixed
                : Enums::EntityKinds::sequence_flexible,
            .value_group = Enums::ValueGroup::temporary,
            .readonly = expr.is_tuple,
        };
    }

    auto Analyzer::check_lambda(const Syntax::Exprs::Lambda& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        report_error("Lambdas are currently unsupported.", source, expr.body->src_begin, expr.body->src_end);

        return {};
    }

    auto Analyzer::check_call(const Syntax::Exprs::Call& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        auto callee_info_opt = check_expr(expr.callee, source);

        if (!callee_info_opt) {
            return {};
        }

        const auto& callee_info = callee_info_opt.value();

        if (callee_info.entity_kind != Enums::EntityKinds::anything && callee_info.entity_kind != Enums::EntityKinds::callable) {
            report_error("Variable is not a function.", source, expr.callee->src_begin, expr.args.back()->src_end);

            return {};
        }

        if (const auto callee_arity = static_cast<std::size_t>(std::get<int>(callee_info.extra)), callee_argc = expr.args.size(); callee_arity != callee_argc) {
            report_error(std::format("Function recieved {} arguments rather than {}.", callee_argc, callee_arity), source, expr.callee->src_begin, expr.callee->src_end);

            return {};
        }

        return SemanticItem {
            .extra = DudAttr {},
            .entity_kind = Enums::EntityKinds::anything,
            .value_group = Enums::ValueGroup::temporary,
            .readonly = true,
        };
    }

    auto Analyzer::check_unary(const Syntax::Exprs::Unary& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        auto inner_info_opt = check_expr(expr.inner, source);

        if (!inner_info_opt) {
            return {};
        }

        const auto& inner_info = inner_info_opt.value();

        if (const auto op = expr.op; !check_op_by_kinds(op, inner_info)) {
            report_error(std::format("The operand does not support the {} operator.", operator_name(op)), source, expr.inner->src_begin - 1, expr.inner->src_end + 1);

            return {};
        }

        return SemanticItem {
            .extra = {},
            .entity_kind = Enums::EntityKinds::anything,
            .value_group = Enums::ValueGroup::temporary,
            .readonly = true,
        };
    }

    auto Analyzer::check_binary(const Syntax::Exprs::Binary& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        using Enums::Operator;
        using Enums::EntityKinds;

        auto lhs_info_opt = check_expr(expr.left, source);
        auto rhs_info_opt = check_expr(expr.right, source);

        if (!lhs_info_opt || !rhs_info_opt) {
            return {};
        }

        const auto& lhs_info = lhs_info_opt.value();
        const auto& rhs_info = rhs_info_opt.value();
        auto has_special_access_case = false;

        const auto check_ok = ([&, this]() {
            switch (const auto binary_op = expr.op; binary_op) {
            case Operator::mul:
            case Operator::div:
            case Operator::modulo:
            case Operator::add:
            case Operator::sub:
            case Operator::equality:
            case Operator::inequality:
            case Operator::lesser:
            case Operator::greater:
            case Operator::at_most:
            case Operator::at_least:
                return check_op_by_kinds(binary_op, lhs_info, rhs_info);
            case Operator::access:
            default:
                const auto lhs_kind = lhs_info.entity_kind;
                const auto rhs_kind = rhs_info.entity_kind;

                has_special_access_case = true;

                return (lhs_kind == EntityKinds::anything || lhs_kind == EntityKinds::sequence_fixed || lhs_kind == EntityKinds::sequence_flexible) && (rhs_kind == EntityKinds::anything || rhs_kind == EntityKinds::primitive);
            }
        })();

        if (!check_ok) {
            if (has_special_access_case) {
                report_error("Invalid sequence access- Requires a named location.", source, expr.left->src_begin, expr.right->src_end + 1);
            } else {
                report_error("Invalid operands to {} operator.", source, expr.left->src_begin, expr.left->src_end + 1);
            }

            return {};
        }

        if (has_special_access_case) {
            return SemanticItem {
                .extra = DudAttr {},
                .entity_kind = EntityKinds::anything,
                .value_group = Enums::ValueGroup::locator,
                .readonly = false,
            };
        }

        return SemanticItem {
            .extra = {},
            .entity_kind = Enums::EntityKinds::anything,
            .value_group = Enums::ValueGroup::temporary,
            .readonly = true,
        };
    }

    auto Analyzer::check_assign(const Syntax::Exprs::Assign& expr, const std::string& source) noexcept -> std::optional<SemanticItem> {
        auto target_info_opt = check_expr(expr.left, source);
        auto rhs_opt = check_expr(expr.value, source);

        if (!target_info_opt || !rhs_opt) {
            return {};
        }

        const auto target_info = target_info_opt.value();

        if (!check_assignability(target_info)) {
            report_error("LHS is not assignable.", source, expr.left->src_begin, expr.value->src_end + 1);

            return {};
        }

        return SemanticItem {
            .extra = {},
            .entity_kind = Enums::EntityKinds::anything,
            .value_group = Enums::ValueGroup::temporary,
            .readonly = true,
        };
    }


    auto Analyzer::check_expr(const Syntax::Exprs::ExprPtr& expr_p, const std::string& source) noexcept -> std::optional<SemanticItem> {
        if (auto literal_p = std::get_if<Syntax::Exprs::Literal>(&expr_p->data); literal_p) {
            return check_literal(*literal_p, source);
        } else if (auto sequence_p = std::get_if<Syntax::Exprs::Sequence>(&expr_p->data); sequence_p) {
            return check_sequence(*sequence_p, source);
        } else if (auto lambda_p = std::get_if<Syntax::Exprs::Lambda>(&expr_p->data); lambda_p) {
            return check_lambda(*lambda_p, source);
        } else if (auto call_p = std::get_if<Syntax::Exprs::Call>(&expr_p->data); call_p) {
            return check_call(*call_p, source);
        } else if (auto unary_p = std::get_if<Syntax::Exprs::Unary>(&expr_p->data); unary_p) {
            return check_unary(*unary_p, source);
        } else if (auto binary_p = std::get_if<Syntax::Exprs::Binary>(&expr_p->data); binary_p) {
            return check_binary(*binary_p, source);
        } else if (auto assignment_p = std::get_if<Syntax::Exprs::Assign>(&expr_p->data); assignment_p) {
            return check_assign(*assignment_p, source);
        }

        return {};
    }


    auto Analyzer::check_expr_stmt(const Syntax::Stmts::ExprStmt& stmt, const std::string& source) noexcept -> bool {
        return check_expr(stmt.expr, source).has_value();
    }

    auto Analyzer::check_local_def(const Syntax::Stmts::LocalDef& stmt, const std::string& source) noexcept -> bool {
        std::string var_name = source.substr(stmt.name.start, token_length(stmt.name));

        auto var_initializer_opt = check_expr(stmt.init_expr, source);

        if (!var_initializer_opt) {
            return false;
        }

        if (!record_named_item(var_name, var_initializer_opt.value().to_lvalue())) {
            report_error(stmt.name.line, std::format("Illegal redeclaration of variable '{}'.", var_name));

            return false;
        }

        return true;
    }

    auto Analyzer::check_if(const Syntax::Stmts::If& stmt, const std::string& source) noexcept -> bool {
        if (!check_expr(stmt.cond_expr, source)) {
            return false;
        }

        if (!check_stmt(stmt.if_body, source)) {
            return false;
        }

        if (!check_stmt(stmt.else_body, source)) {
            return false;
        }

        return true;
    }

    auto Analyzer::check_return(const Syntax::Stmts::Return& stmt, const std::string& source) noexcept -> bool {
        return check_expr(stmt.result, source).has_value();
    }

    auto Analyzer::check_while(const Syntax::Stmts::While& stmt, const std::string& source) noexcept -> bool {
        if (!check_expr(stmt.check, source)) {
            return false;
        }

        if (!check_stmt(stmt.body, source)) {
            return false;
        }

        return true;
    }

    auto Analyzer::check_break([[maybe_unused]] const Syntax::Stmts::Break& stmt, [[maybe_unused]] const std::string& source) noexcept -> bool {
        return true;
    }

    auto Analyzer::check_block(const Syntax::Stmts::Block& stmt, const std::string& source) noexcept -> bool {
        for (const auto& item : stmt.items) {
            if (!check_stmt(item, source)) {
                return false;
            }
        }

        return true;
    }

    auto Analyzer::check_function(const Syntax::Stmts::Function& stmt, const std::string& source) noexcept -> bool {
        std::string fn_name = source.substr(stmt.name.start, token_length(stmt.name));

        if (m_prepassing) {
            const auto fn_arity = static_cast<int>(stmt.params.size());

            if (!record_named_item(fn_name, SemanticItem {
                .extra = fn_arity,
                .entity_kind = Enums::EntityKinds::callable,
                .value_group = Enums::ValueGroup::locator,
                .readonly = true,
            })) {
                report_error(stmt.name.line, "Redefinition of function disallowed.");

                return false;
            }

            return true;
        }

        enter_scope(fn_name);

        for (const auto& param_decl : stmt.params) {
            if (std::string param_name = source.substr(param_decl.start, token_length(param_decl)); !record_named_item(param_name, SemanticItem {
                .extra = {},
                .entity_kind = Enums::EntityKinds::anything,
                .value_group = Enums::ValueGroup::locator,
                .readonly = true,
            })) {
                report_error(param_decl.line, std::format("Cannot redeclare function parameter '{}'.", param_name));

                return false;
            }
        }

        if (!check_stmt(stmt.body, source)) {
            leave_scope();
            return false;
        }

        leave_scope();

        return true;
    }

    auto Analyzer::check_native_stub(const Syntax::Stmts::NativeStub& stmt, const std::string& source) noexcept -> bool {
        if (!m_prepassing) {
            return true;
        }

        std::string fn_name = source.substr(stmt.name.start, token_length(stmt.name));
        const auto fn_arity = static_cast<int>(stmt.params.size());

        if (!record_named_item(fn_name, SemanticItem {
            .extra = fn_arity,
            .entity_kind = Enums::EntityKinds::callable,
            .value_group = Enums::ValueGroup::locator,
            .readonly = true,
        })) {
            report_error(stmt.name.line, "Redefinition of native function disallowed.");

            return false;
        }

        return true;
    }

    auto Analyzer::check_import([[maybe_unused]] const Syntax::Stmts::Import& stmt, [[maybe_unused]] const std::string& source) noexcept -> bool {
        return true;
    }

    auto Analyzer::check_stmt(const Syntax::Stmts::StmtPtr& stmt_p, const std::string& source) noexcept -> bool {
        if (auto expr_stmt_p = std::get_if<Syntax::Stmts::ExprStmt>(&stmt_p->data); expr_stmt_p) {
            return check_expr_stmt(*expr_stmt_p, source);
        } else if (auto local_def_p = std::get_if<Syntax::Stmts::LocalDef>(&stmt_p->data); local_def_p) {
            return check_local_def(*local_def_p, source);
        } else if (auto if_stmt_p = std::get_if<Syntax::Stmts::If>(&stmt_p->data); if_stmt_p) {
            return check_if(*if_stmt_p, source);
        } else if (auto ret_stmt_p = std::get_if<Syntax::Stmts::Return>(&stmt_p->data); ret_stmt_p) {
            return check_return(*ret_stmt_p, source);
        } else if (auto while_stmt_p = std::get_if<Syntax::Stmts::While>(&stmt_p->data); while_stmt_p) {
            return check_while(*while_stmt_p, source);
        } else if (auto break_stmt_p = std::get_if<Syntax::Stmts::Break>(&stmt_p->data); break_stmt_p) {
            return check_break(*break_stmt_p, source);
        } else if (auto block_p = std::get_if<Syntax::Stmts::Block>(&stmt_p->data); block_p) {
            return check_block(*block_p, source);
        } else if (auto function_decl_p = std::get_if<Syntax::Stmts::Function>(&stmt_p->data); function_decl_p) {
            return check_function(*function_decl_p, source);
        } else if (auto stub_p = std::get_if<Syntax::Stmts::NativeStub>(&stmt_p->data); stub_p) {
            return check_native_stub(*stub_p, source);
        } else if (auto import_stmt_p = std::get_if<Syntax::Stmts::Import>(&stmt_p->data); import_stmt_p) {
            return check_import(*import_stmt_p, source);
        }

        return true;
    }


    Analyzer::Analyzer()
    : m_scopes {}, m_prepassing {true} {}

    auto Analyzer::operator()(const Syntax::AST::FullAST& ast, const std::unordered_map<uint32_t, std::string>& src_map) -> bool {
        enter_scope("global");

        for (const auto& [decl_ast, src_id] : ast) {
            if (!check_stmt(decl_ast, src_map.at(src_id))) {
                return false;
            }
        }

        m_prepassing = false;

        for (const auto& [decl_ast, src_id] : ast) {
            if (!check_stmt(decl_ast, src_map.at(src_id))) {
                return false;
            }
        }

        leave_scope();

        return true;
    }
}