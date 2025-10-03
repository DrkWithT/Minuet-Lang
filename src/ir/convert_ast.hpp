#ifndef CONVERT_AST_HPP
#define CONVERT_AST_HPP

#include <optional>
#include <string>
#include <queue>
#include <unordered_map>
#include <vector>

#include "syntax/exprs.hpp"
#include "syntax/ast.hpp"
#include "syntax/stmts.hpp"
#include "ir/steps.hpp"
#include "ir/cfg.hpp"
#include "runtime/value.hpp"

namespace Minuet::IR::Convert {
    namespace Utils {
        struct BBLink {
            int from;
            int to;
        };

        enum class NameLocation : uint8_t {
            global_native_slot,
            global_function_slot,
            local_slot,
        };
    }


    class ASTConversion {
    public:
        ASTConversion();

        [[nodiscard]] auto operator()(const Syntax::AST::FullAST& src_mapped_ast, const std::vector<std::string>& source_map) -> std::optional<CFG::FullIR>;

    private:
        void report_error(std::string_view msg_c_str);

        [[nodiscard]] auto gen_fun_aa() -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto gen_temp_aa() -> std::optional<Steps::AbsAddress>;

        [[nodiscard]] auto resolve_constant_aa(const std::string& literal, Runtime::Value value) -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto record_name_aa(Utils::NameLocation mode, const std::string& name, Steps::AbsAddress aa) -> bool;
        [[nodiscard]] auto lookup_name_aa(const std::string& name) noexcept -> std::optional<Steps::AbsAddress>;

        void add_cfg();
        [[nodiscard]] auto apply_pending_links() -> bool;

        [[nodiscard]] auto emit_literal(const Syntax::Exprs::Literal& literal, std::string_view source) -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto emit_unary(const Syntax::Exprs::Unary& unary, std::string_view source) -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto emit_binary(const Syntax::Exprs::Binary& binary, std::string_view source) -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto emit_call(const Syntax::Exprs::Call& call, std::string_view source) -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto emit_assign(const Syntax::Exprs::Assign& assign, std::string_view source) -> std::optional<Steps::AbsAddress>;
        [[nodiscard]] auto emit_expr(const Syntax::Exprs::ExprPtr& expr, std::string_view source) -> std::optional<Steps::AbsAddress>;

        [[nodiscard]] auto emit_expr_stmt(const Syntax::Stmts::ExprStmt& expr_stmt, std::string_view source) -> bool;
        [[nodiscard]] auto emit_def(const Syntax::Stmts::LocalDef& def, std::string_view source) -> bool;
        [[nodiscard]] auto emit_if(const Syntax::Stmts::If& cond, std::string_view source) -> bool;
        [[nodiscard]] auto emit_return(const Syntax::Stmts::Return& ret, std::string_view source) -> bool;
        [[nodiscard]] auto emit_block(const Syntax::Stmts::Block& block, std::string_view source) -> bool;
        [[nodiscard]] auto emit_function(const Syntax::Stmts::Function& fun, std::string_view source) -> bool;
        [[nodiscard]] auto emit_stmt(const Syntax::Stmts::StmtPtr& stmt, std::string_view source) -> bool;

        std::unordered_map<std::string, Steps::AbsAddress> m_globals;
        std::unordered_map<std::string, Steps::AbsAddress> m_locals;
        std::queue<Utils::BBLink> m_pending_links;
        std::vector<CFG::CFG> m_result_cfgs;
        std::vector<Runtime::Value> m_proto_consts;
        int m_proto_main_id;
        int m_error_count;
        uint16_t m_next_func_aa;
        uint16_t m_next_local_aa;
        bool m_prepassing;
        // bool m_continuing_bb;
    };
}

#endif
