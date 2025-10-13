#include <utility>
#include <iostream>
#include <print>
#include <variant>

#include "semantics/enums.hpp"
#include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "ir/convert_ast.hpp"

/// TODO: fix emission to handle code with a flat BB after any whole conditional stmt??

namespace Minuet::IR::Convert {
    using Frontend::Lexicals::TokenType;
    using Frontend::Lexicals::token_to_sv;
    using Semantics::Enums::Operator;
    using Semantics::Enums::operator_to_ir_op;
    using Steps::AbsAddrTag;
    using Steps::AbsAddress;
    using Steps::Op;
    using Steps::TACUnary;
    using Steps::TACBinary;
    using Steps::OperNonary;
    using Steps::OperUnary;
    using Steps::OperBinary;
    using IR::CFG::FullIR;
    using Utils::NameLocation;

    ASTConversion::ASTConversion(const Runtime::NativeProcRegistry* native_proc_ids)
    : m_globals {}, m_locals {}, m_pending_links {}, m_result_cfgs {}, m_proto_consts {}, m_native_proc_ids {native_proc_ids}, m_proto_main_id {-1}, m_error_count {0}, m_next_func_aa {0}, m_next_local_aa {0}, m_prepassing {true} {}

    auto ASTConversion::operator()(const Syntax::AST::FullAST& src_mapped_ast, const std::vector<std::string>& source_map) -> std::optional<FullIR> {
        // 1. Prepass top-level definitions of functions, etc. to avoid forward declaration jank.
        for (const auto& [ast, ast_src_id] : src_mapped_ast) {
            if (!emit_stmt(ast, source_map.at(ast_src_id))) {
                return {};
            }
        }

        // 2. Do the actual emitting pass over the AST once all top-level functions, etc. are resolved. This accounts for self / mutually recursive functions which would otherwise cause a false definition error.
        m_prepassing = false;

        for (const auto& [ast, ast_src_id] : src_mapped_ast) {
            if (!emit_stmt(ast, source_map.at(ast_src_id))) {
                return {};
            }
        }

        if (m_error_count > 0) {
            return {};
        }

        return FullIR {
            .cfg_list = std::exchange(m_result_cfgs, {}),
            .constants = std::exchange(m_proto_consts, {}),
            .main_id = m_proto_main_id,
        };
    }


    void ASTConversion::report_error(std::string_view msg_c_str) {
        ++m_error_count;
        std::println(std::cerr, "\033[1;31mIR Error {}:\033[0m {}", m_error_count, msg_c_str);
    }

    auto ASTConversion::gen_fun_aa() -> std::optional<Steps::AbsAddress> {
        const auto next_fun_id = m_next_func_aa;

        ++m_next_func_aa;

        return AbsAddress {
            .id = next_fun_id,
            .tag = AbsAddrTag::immediate,
        };
    }

    auto ASTConversion::gen_temp_aa() -> std::optional<AbsAddress> {
        const auto next_aa_id = m_next_local_aa;

        ++m_next_local_aa;

        return AbsAddress {
            .id = next_aa_id,
            .tag = AbsAddrTag::temp,
        };
    }

    auto ASTConversion::resolve_constant_aa(const std::string& literal, Runtime::FastValue constant_val) -> std::optional<AbsAddress> {
        if (m_globals.contains(literal)) {
            return {m_globals[literal]};
        }

        const auto next_const_id = static_cast<int16_t>(m_proto_consts.size());
        const auto next_aa = AbsAddress {
            .tag = AbsAddrTag::constant,
            .id = next_const_id,
        };

        m_proto_consts.emplace_back(constant_val);
        m_globals.emplace(literal, next_aa);

        return next_aa;
    }

    auto ASTConversion::record_name_aa(Utils::NameLocation mode, const std::string& name, AbsAddress aa) -> bool {
        auto name_exists = false;

        switch (mode) {
            case NameLocation::global_native_slot:
                name_exists = m_globals.contains(name);

                if (!name_exists) {
                    m_globals[name] = aa;
                }

                break;
            case NameLocation::global_function_slot:
                name_exists = m_globals.contains(name);

                if (!name_exists) {
                    m_globals[name] = aa;
                }

                break;
            case NameLocation::local_slot:
                name_exists = m_locals.contains(name);

                if (!name_exists) {
                    m_locals[name] = aa;
                }

                break;
            default:
                break;
        }

        return !name_exists;
    }

    auto ASTConversion::lookup_name_aa(const std::string& name) noexcept -> std::optional<AbsAddress> {
        if (m_native_proc_ids->contains(name)) {
            return AbsAddress {
                .id = static_cast<int16_t>(m_native_proc_ids->at(name)),
                .tag = AbsAddrTag::constant,
            };
        } else if (m_globals.contains(name)) {
            return m_globals[name];
        } else if (m_locals.contains(name)) {
            return m_locals[name];
        }

        return {};
    }

    void ASTConversion::add_cfg() {
        m_result_cfgs.emplace_back();
    }

    auto ASTConversion::apply_pending_links() -> bool {
        if (m_result_cfgs.empty()) {
            return false;
        }

        auto& recent_wip_cfg = m_result_cfgs.back();

        while (!m_pending_links.empty()) {
            const auto [from_id, to_id] = m_pending_links.front();

            if (!recent_wip_cfg.link_bb(from_id, to_id)) {
                return false;
            }

            m_pending_links.pop();
        }

        return true;
    }

    
    auto ASTConversion::emit_literal(const Syntax::Exprs::Literal& literal, std::string_view source) -> std::optional<AbsAddress> {
        const auto literal_tag = literal.token.type;
        std::string literal_lexeme = std::format("{}", token_to_sv(literal.token, source));
        std::optional<AbsAddress> temp;

        if (literal_tag == TokenType::literal_false || literal_tag == TokenType::literal_true) {
            temp = resolve_constant_aa(literal_lexeme, Runtime::FastValue {literal_lexeme == "true"});
        } else if (literal_tag == TokenType::literal_int) {
            temp = resolve_constant_aa(literal_lexeme, Runtime::FastValue {std::stoi(literal_lexeme)});
        } else if (literal_tag == TokenType::literal_double) {
            temp = resolve_constant_aa(literal_lexeme, Runtime::FastValue {std::stod(literal_lexeme)});
        } else if (literal_tag == TokenType::identifier) {
            temp = lookup_name_aa(literal_lexeme);
        } else {
            std::string bad_literal_msg {std::format("Cannot resolve invalid literal: '{}'", literal_lexeme)};
            report_error(bad_literal_msg);
        }

        return temp;
    }

    auto ASTConversion::emit_unary(const Syntax::Exprs::Unary& unary, std::string_view source) -> std::optional<AbsAddress> {
        std::optional<AbsAddress> temp = emit_expr(unary.inner, source);

        if (!temp) {
            return {};
        }

        if (unary.op == Operator::negate) {
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(TACUnary {
                .dest = temp.value(),
                .arg_0 = temp.value(),
                .op = Op::neg,
            });

            return temp;
        }

        const auto unary_src_beg = unary.inner->src_begin;
        const auto unary_src_len = unary.inner->src_end - unary_src_beg + 1;
        std::string bad_unary_op_msg = std::format("Invalid unary operator in expression, only negation(-) is supported.:\n\n\033[1;33mSource:\033[0m\n\n{}\n", source.substr(unary_src_beg, unary_src_len));

        report_error(bad_unary_op_msg);

        return {};
    }

    auto ASTConversion::emit_binary(const Syntax::Exprs::Binary& binary, std::string_view source) -> std::optional<AbsAddress> {
        const auto bin_operator = binary.op;
        auto lhs_aa_opt = emit_expr(binary.left, source);
        auto rhs_aa_opt = emit_expr(binary.right, source);

        if (!lhs_aa_opt || !rhs_aa_opt) {
            return {};
        }

        auto bin_result_aa = ([&, this]() -> std::optional<AbsAddress> {
            switch (bin_operator) {
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
                {
                    if (auto dest_aa = gen_temp_aa(); dest_aa) {
                        auto result_aa = dest_aa.value();

                        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(TACBinary {
                            .dest = dest_aa.value(),
                            .arg_0 = lhs_aa_opt.value(),
                            .arg_1 = rhs_aa_opt.value(),
                            .op = operator_to_ir_op(bin_operator).value(),
                        });

                        return result_aa;
                    }

                    return {};
                }
            default:
                {
                    const auto binary_src_beg = binary.left->src_begin;
                    const auto binary_src_len = binary.right->src_end - binary_src_beg + 1;
                    std::string bad_binary_op_msg = std::format("Unsupported unary operator in expression:\n\n\033[1;33mSource:\033[0m\n\n{}\n", source.substr(binary_src_beg, binary_src_len));

                    report_error(bad_binary_op_msg);
                }
                return {};
        }})();

        return bin_result_aa;
    }

    auto ASTConversion::emit_call(const Syntax::Exprs::Call& call, std::string_view source) -> std::optional<AbsAddress> {
        auto callee_aa_opt = emit_expr(call.callee, source);

        if (!callee_aa_opt) {
            return {};
        }

        /// NOTE: Any call will take the function ID and then N (stack argument count).
        const int16_t real_args_n = call.args.size();
        
        for (int16_t arg_idx = 0; arg_idx < real_args_n; ++arg_idx) {
            if (auto arg_aa_opt = emit_expr(call.args.at(arg_idx), source); arg_aa_opt) {
                const auto arg_dest = gen_temp_aa().value();
                
                m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(TACUnary {
                    .dest = arg_dest,
                    .arg_0 = arg_aa_opt.value(),
                    .op = Op::nop,
                });
                
                continue;
            }
            
            return {};
        }

        const auto call_result_slot_aa = AbsAddress {
            .id = static_cast<int16_t>(m_next_local_aa - real_args_n),
            .tag = AbsAddrTag::temp,
        };

        auto callee_aa = callee_aa_opt.value();
        /// NOTE: the IR `Op` for call expressions will be `native_call` upon an AbsAddress with reused tag `constant`... This denotes a function pointer ID from the native procedure "registry".
        const auto calling_op = (callee_aa.tag == AbsAddrTag::immediate)
            ? Op::call
            : Op::native_call;

        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperBinary {
            .arg_0 = {
                .id = callee_aa.id,
                .tag = AbsAddrTag::immediate,
            },
            .arg_1 = {
                .id = real_args_n,
                .tag = AbsAddrTag::immediate,
            },
            .op = calling_op,
        });

        return call_result_slot_aa;
    }

    auto ASTConversion::emit_assign(const Syntax::Exprs::Assign& assign, std::string_view source) -> std::optional<AbsAddress> {
        auto lhs_aa_opt = emit_expr(assign.left, source);
        auto setting_aa_opt = emit_expr(assign.value, source);

        if (!lhs_aa_opt || !setting_aa_opt) {
            return {};
        }

        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(TACUnary {
            .dest = lhs_aa_opt.value(),
            .arg_0 = setting_aa_opt.value(),
            .op = Op::nop,
        });

        return lhs_aa_opt;
    }

    auto ASTConversion::emit_expr(const Syntax::Exprs::ExprPtr& expr, std::string_view source) -> std::optional<AbsAddress> {
        using namespace Syntax::Exprs;

        if (std::holds_alternative<Literal>(expr->data)) {
            return emit_literal(std::get<Literal>(expr->data), source);
        } else if (std::holds_alternative<Syntax::Exprs::Call>(expr->data)) {
            return emit_call(std::get<Call>(expr->data), source);
        } else if (std::holds_alternative<Syntax::Exprs::Unary>(expr->data)) {
            return emit_unary(std::get<Unary>(expr->data), source);
        } else if (std::holds_alternative<Syntax::Exprs::Binary>(expr->data)) {
            return emit_binary(std::get<Binary>(expr->data), source);
        } else if (std::holds_alternative<Syntax::Exprs::Assign>(expr->data)) {
            return emit_assign(std::get<Assign>(expr->data), source);
        }

        return {};
    }


    auto ASTConversion::emit_expr_stmt(const Syntax::Stmts::ExprStmt& expr_stmt, std::string_view source) -> bool {
        return emit_expr(expr_stmt.expr, source).has_value();
    }

    auto ASTConversion::emit_def(const Syntax::Stmts::LocalDef& def, std::string_view source) -> bool {
        std::string var_name = std::format("{}", token_to_sv(def.name, source));
        auto var_init_opt = emit_expr(def.init_expr, source);
        auto var_dest_opt = gen_temp_aa();

        if (!var_init_opt || !var_dest_opt) {
            return false;
        }

        auto var_init_aa = var_init_opt.value();
        auto var_dest_aa = var_dest_opt.value();

        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(TACUnary {
            .dest = var_dest_aa,
            .arg_0 = var_init_aa,
            .op = Op::nop,
        });

        if (!record_name_aa(NameLocation::local_slot, var_name, var_dest_aa)) {
            const auto def_src_beg = def.name.start;
            const auto def_name_len = def.name.end - def_src_beg + 1;
            const auto def_src_len = def.init_expr->src_end - def_src_beg + 1;
            std::string bad_redef_msg = std::format("Invalid re-definition of local variable '{}'.\n\n\033[1;33mSource:\033[0m\n\n{}\n", source.substr(def_src_beg, def_name_len), source.substr(def_src_beg, def_src_len));

            report_error(bad_redef_msg);

            return false;
        }

        return true;
    }

    /**
     * @brief Constructs the parts of a CFG that represent forward branching constructs like if / else stmts.
     * @note Assumes that the enclosing block visited has a BB placed prior... Patching dud jumps of step pos 0 will be handled later.
     * @param cond 
     * @param source 
     * @return true 
     * @return false 
     */
    auto ASTConversion::emit_if(const Syntax::Stmts::If& cond, std::string_view source) -> bool {
        // Case 1:
        //     (Pre-Block)-*
        //     |           |
        //     (T-Block)   (F-Block)
        //             |          |
        //             (Post-Block)
        // Case 2:
        //     (Pre-Block)-*
        //     |           |
        //   (T-Block)     F
        //     |          /
        //     (Post-Block)

        /// 1: handle the initial basic block- here, the if-stmt check is done before the actual branching by JUMP_ELSE... that instruction pops off a boolean and then jumps if the boolean is false.
        const auto pre_if_bb_id = m_result_cfgs.back().bb_count() - 1;
        const auto if_true_bb_id = pre_if_bb_id + 1;

        auto cond_result_aa = emit_expr(cond.cond_expr, source);

        if (!cond_result_aa) {
            return false;
        }

        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperUnary {
            .arg_0 = {
                .id = 0,
                .tag = AbsAddrTag::immediate,
            },
            .op = Op::jump_else,
        });
        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
            .op = Op::meta_save_patch,
        });

        /// 2: handle the truthy body- then place a JUMP past the falsy body.
        if (!emit_stmt(cond.if_body, source)) {
            return false;
        }

        m_pending_links.push({
            .from = pre_if_bb_id,
            .to = if_true_bb_id,
        });

        /// 3: handle the falsy body if present?
        if (cond.else_body) {
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperUnary {
                .arg_0 = {
                    .id = 0,
                    .tag = AbsAddrTag::immediate,
                },
                .op = Op::jump,
            });
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::meta_save_patch,
            });
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::nop,
            });
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::meta_patch_jmp_else,
            });

            if (!emit_stmt(cond.else_body, source)) {
                return false;
            }

            const auto else_body_bb_id = m_result_cfgs.back().bb_count() - 1;

            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::nop,
            });
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::meta_patch_jmp,
            });
            m_pending_links.push({
                .from = pre_if_bb_id,
                .to = else_body_bb_id,
            });

            const auto post_if_else_bb_id = m_result_cfgs.back().add_bb();

            m_pending_links.push({
                .from = if_true_bb_id,
                .to = post_if_else_bb_id,
            });
            m_pending_links.push({
                .from = else_body_bb_id,
                .to = post_if_else_bb_id,
            });
        } else {
            const auto post_if_body_bb_id = m_result_cfgs.back().add_bb();

            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::nop,
            });
            m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperNonary {
                .op = Op::meta_patch_jmp_else,
            });
            m_pending_links.push({
                .from = if_true_bb_id,
                .to = post_if_body_bb_id,
            });
            m_pending_links.push({
                .from = pre_if_bb_id,
                .to = post_if_body_bb_id,
            });
        }

        return true;
    }

    auto ASTConversion::emit_return(const Syntax::Stmts::Return& ret, std::string_view source) -> bool {
        auto result_aa_opt = emit_expr(ret.result, source);

        if (!result_aa_opt) {
            return false;
        }

        m_result_cfgs.back().get_newest_bb().value()->steps.emplace_back(OperUnary {
            .arg_0 = result_aa_opt.value(),
            .op = Op::ret,
        });

        return true;
    }

    auto ASTConversion::emit_block(const Syntax::Stmts::Block& block, std::string_view source) -> bool {
        m_result_cfgs.back().add_bb();

        for (const auto& stmt : block.items) {
            if (!emit_stmt(stmt, source)) {
                return false;
            }
        }

        return true;
    }

    auto ASTConversion::emit_function(const Syntax::Stmts::Function& fun, std::string_view source) -> bool {
        if (m_prepassing) {
            std::string func_name = std::format("{}", token_to_sv(fun.name, source));
            const auto next_func_aa_opt = gen_fun_aa();
    
            if (!next_func_aa_opt) {
                return false;
            }

            const auto func_aa = next_func_aa_opt.value();

            if (func_name == "main" && m_proto_main_id == -1) {
                m_proto_main_id = func_aa.id;
            }

            return record_name_aa(NameLocation::global_function_slot, func_name, func_aa);
        }

        add_cfg();

        auto generation_ok = true;

        for (const auto& param_token : fun.params) {
            std::string param_name = std::format("{}", token_to_sv(param_token, source));
            auto param_aa_opt = gen_temp_aa();

            if (!param_aa_opt) {
                generation_ok = false;
                break;
            }

            if (!record_name_aa(NameLocation::local_slot, param_name, param_aa_opt.value())) {
                generation_ok = false;
                break;
            }
        }

        if (generation_ok) {
            generation_ok = emit_stmt(fun.body, source);
        }

        if (generation_ok) {
            generation_ok = apply_pending_links();
        }

        m_locals.clear();
        m_next_local_aa = 0U;

        return generation_ok;
    }

    auto ASTConversion::emit_stmt(const Syntax::Stmts::StmtPtr& stmt, std::string_view source) -> bool {
        using namespace Syntax::Stmts;

        if (auto func_p = std::get_if<Function>(&stmt->data); func_p) {
            return emit_function(*func_p, source);
        } else if (auto block_p = std::get_if<Block>(&stmt->data); block_p) {
            return emit_block(*block_p, source);
        } else if (auto ret_p = std::get_if<Return>(&stmt->data); ret_p) {
            return emit_return(*ret_p, source);
        } else if (auto if_p = std::get_if<If>(&stmt->data); if_p) {
            return emit_if(*if_p, source);
        } else if (auto def_p = std::get_if<LocalDef>(&stmt->data); def_p) {
            return emit_def(*def_p, source);
        } else if (auto expr_stmt_p = std::get_if<ExprStmt>(&stmt->data); expr_stmt_p) {
            return emit_expr_stmt(*expr_stmt_p, source);
        }

        return true;
    }
}
