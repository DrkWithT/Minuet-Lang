#ifndef MINUET_RUNTIME_MINUET_VM
#define MINUET_RUNTIME_MINUET_VM

#include <cstdint>
#include <optional>
#include <vector>

#include "runtime/value.hpp"
#include "runtime/bytecode.hpp"

namespace Minuet::Runtime::VM {
    namespace Utils {
        struct EngineConfig {
            int reg_buffer_limit;
            int16_t call_frame_max;
        };

        struct CallFrame {
            int16_t old_func_idx;
            int16_t old_func_ip;
            int old_base_ptr;
            int old_mem_top; // old register frame top
            int old_exec_status;
            bool old_flag_val;
        };

        enum class ExecStatus : int {
            ok = 0,
            entry_error,  // invalid main ID
            op_error,     // invalid opcode
            arg_error,    // invalid opcode arg
            mem_error,    // bad VM memory access
            math_error,
            user_error,   // user-caused failure (return nonzero)
            any_error,    // general error
        };
    }

    class Engine {
    public:
        Engine(Utils::EngineConfig config, Code::Program& prgm);

        [[nodiscard]] auto operator()() -> Utils::ExecStatus;

    private:
        [[nodiscard]] auto fetch_value(Code::ArgMode mode, int16_t id) noexcept -> std::optional<Runtime::FastValue>;

        void handle_load_const(uint16_t metadata, int16_t dest, int16_t const_id) noexcept;
        void handle_mov(uint16_t metadata, int16_t dest, int16_t src) noexcept;
        void handle_neg(uint16_t metadata, int16_t dest) noexcept;
        void handle_inc(uint16_t metadata, int16_t dest) noexcept;
        void handle_dec(uint16_t metadata, int16_t dest) noexcept;
        void handle_mul(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_div(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_mod(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_add(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_sub(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs);
        void handle_cmp_eq(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_cmp_ne(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_cmp_lt(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_cmp_gt(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_cmp_gte(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_cmp_lte(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept;
        void handle_jmp(int16_t dest_ip) noexcept;
        void handle_jmp_if(int16_t dest_ip) noexcept;
        void handle_jmp_else(int16_t dest_ip) noexcept;
        void handle_call(int16_t func_id, int16_t arg_count) noexcept;
        void handle_native_call(int16_t native_id) noexcept;
        void handle_ret(uint16_t metadata, int16_t src_id) noexcept;
        // void handle_halt(int16_t metadata, int16_t src_id);

        std::vector<Runtime::FastValue> m_memory;
        std::vector<Utils::CallFrame> m_call_frames;

        const Code::Chunk* m_chunk_view;
        const FastValue* m_const_view;
        Utils::CallFrame* m_call_frame_ptr;

        int16_t m_rfi;  // Contains the callee ID
        int16_t m_rip;  // Contains the instruction index in the callee's chunk
        int m_rbp;  // Contains the base point of the current register frame in memory
        int m_rft;  // Contains highest memory cell used
        int m_rsp;
        int m_res;  // Contains execution status code
        int m_consts_n;
        int16_t m_rrd; // Counts 1-based recursion depth- 0 means done!
        bool m_rfv; // Represents conditional-test flag value
    };
}

#endif
