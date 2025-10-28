#ifndef MINUET_RUNTIME_MINUET_VM
#define MINUET_RUNTIME_MINUET_VM

#include <any>
#include <cstdint>
#include <optional>
#include <vector>

#include "runtime/fast_value.hpp"
#include "runtime/heap_storage.hpp"
#include "runtime/bytecode.hpp"
#include "runtime/natives.hpp"

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
            uint8_t old_exec_status;
        };

        enum class ExecStatus : uint8_t {
            ok = 0,
            setup_error,  // invalid setup
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
        Engine(Utils::EngineConfig config, Code::Program& prgm, std::any native_fn_table);

        [[nodiscard]] auto operator()() -> Utils::ExecStatus;

        [[nodiscard]] auto handle_native_fn_access(int16_t arg_count, int16_t offset) & noexcept -> Runtime::FastValue&;

        void handle_native_fn_return(Runtime::FastValue&& result, [[maybe_unused]] int16_t arg_count) noexcept;

    private:
        [[nodiscard]] auto fetch_value(Code::ArgMode mode, int16_t id) noexcept -> std::optional<Runtime::FastValue>;

        void try_mark_and_sweep();

        void handle_make_seq(int16_t dest_reg) noexcept;
        void handle_seq_obj_push(uint16_t metadata, int16_t dest, int16_t src_id, int16_t mode) noexcept;
        void handle_seq_obj_pop(uint16_t metadata, int16_t dest, int16_t src_id, int16_t mode) noexcept;
        void handle_seq_obj_get(uint16_t metadata, int16_t dest, int16_t src_id, int16_t pos_value_id) noexcept;
        void handle_frz_seq_obj(int16_t dest) noexcept;

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

        // void handle_jmp(int16_t dest_ip) noexcept;
        void handle_jmp_if(int16_t check_reg, int16_t dest_ip) noexcept;
        void handle_jmp_else(int16_t check_reg, int16_t dest_ip) noexcept;
        void handle_call(int16_t func_id, int16_t arg_count) noexcept;
        void handle_native_call(int16_t native_id, [[maybe_unused]] int16_t arg_count) noexcept;
        void handle_ret(uint16_t metadata, int16_t src_id) noexcept;
        // void handle_halt(int16_t metadata, int16_t src_id);

        HeapStorage m_heap;
        std::vector<Runtime::FastValue> m_memory;
        std::vector<Utils::CallFrame> m_call_frames;

        const Code::Chunk* m_chunk_view;
        const FastValue* m_const_view;
        Utils::CallFrame* m_call_frame_ptr;
        const Runtime::NativeProcTable* m_native_funcs;

        int16_t m_rfi;  // Contains the callee ID
        int16_t m_rip;  // Contains the instruction index in the callee's chunk
        int m_rbp;  // Contains the base point of the current register frame in memory
        int m_rft;  // Contains highest memory cell used
        int m_rsp;
        int m_consts_n;
        int16_t m_rrd; // Counts 1-based recursion depth- 0 means done!
        uint8_t m_res;  // Contains execution status code
    };
}

#endif
