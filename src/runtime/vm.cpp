#include <utility>
#include <algorithm>
// #include <print>

#include "runtime/bytecode.hpp"
#include "runtime/vm.hpp"

namespace Minuet::Runtime::VM {
    using Minuet::Runtime::Value;

    static constexpr auto ok_res_value = static_cast<int>(Utils::ExecStatus::ok);

    Engine::Engine(Utils::EngineConfig config, Code::Program& prgm)
    : m_memory {}, m_stack {}, m_call_frames {}, m_chunk_view {}, m_const_view {}, m_rfi {}, m_rip {}, m_rbp {}, m_rft {}, m_rsp {}, m_res {}, m_consts_n {}, m_mem_limit {}, m_stack_limit {}, m_recur_limit {}, m_rfv {} {
        const auto [mem_limit, stack_limit, recur_depth_max] = config;
        const auto prgm_entry_fn_id = prgm.entry_id.value_or(-1);

        const auto mem_vec_size = static_cast<std::size_t>(mem_limit);
        m_memory.reserve(mem_vec_size);
        m_memory.resize(mem_vec_size);

        const auto stack_vec_size = static_cast<std::size_t>(stack_limit);
        m_stack.reserve(stack_vec_size);
        m_stack.resize(stack_vec_size);

        m_call_frames.emplace_back(Utils::CallFrame {
            .old_func_idx = 0,
            .old_func_ip = 0,
            .old_base_ptr = 0,
            .old_mem_top = 0,
        });

        m_chunk_view = prgm.chunks.data();
        m_const_view = prgm.constants.data();

        m_rfi = prgm_entry_fn_id;
        m_rip = 0;
        m_rbp = 0;
        m_rft = 0;
        m_rsp = -1;
        m_res = (prgm_entry_fn_id >= 0)
            ? static_cast<int>(Utils::ExecStatus::ok)
            : static_cast<int>(Utils::ExecStatus::entry_error);

        m_consts_n = static_cast<int>(prgm.constants.size());
        m_mem_limit = mem_limit;
        m_stack_limit = stack_limit;
        m_recur_limit = recur_depth_max;
    }

    auto Engine::operator()() -> Utils::ExecStatus {
        push_value(Value {m_res});
        push_value(Value {m_rfv});

        while (!m_call_frames.empty() && m_res == ok_res_value) {
            const auto [args, metadata, opcode] = m_chunk_view[m_rfi][m_rip];

            // std::println("RFI = {}, RIP = {}, RBP = {}, RFT = {}, RSP = {}, opcode: {}", m_rfi, m_rip, m_rbp, m_rft, m_rsp, Code::opcode_name(opcode)); // debug

            switch (opcode) {
                case Code::Opcode::nop:
                    ++m_rip;
                    break;
                case Code::Opcode::load_const:
                    handle_load_const(metadata, args[0], args[1]);
                    break;
                case Code::Opcode::mov:
                    handle_mov(metadata, args[0], args[1]);
                    break;
                case Code::Opcode::neg:
                    handle_neg(metadata, args[0]);
                    break;
                case Code::Opcode::inc:
                    handle_inc(metadata, args[0]);
                    break;
                case Code::Opcode::dec:
                    handle_dec(metadata, args[0]);
                    break;
                case Code::Opcode::mul:
                    handle_mul(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::div:
                    handle_div(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::mod:
                    handle_mod(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::add:
                    handle_add(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::sub:
                    handle_sub(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::equ:
                    handle_cmp_eq(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::neq:
                    handle_cmp_ne(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::lt:
                    handle_cmp_lt(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::gt:
                    handle_cmp_gt(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::lte:
                    handle_cmp_lte(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::gte:
                    handle_cmp_gte(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::jump:
                    handle_jmp(args[0]);
                    break;
                case Code::Opcode::jump_if:
                    handle_jmp_if(args[0]);
                    break;
                case Code::Opcode::jump_else:
                    handle_jmp_else(args[0]);
                    break;
                case Code::Opcode::call:
                    handle_call(args[0], args[1]);
                    break;
                case Code::Opcode::ret:
                    handle_ret(metadata, args[0]);
                    break;
                case Code::Opcode::native_call:
                case Code::Opcode::halt:
                default:
                    m_res = static_cast<int>(Utils::ExecStatus::op_error);
                    break;
            }
        }

        if (m_res != ok_res_value) {
            return static_cast<Utils::ExecStatus>(m_res);
        }

        return (m_memory[0] == Value {0}) ? Utils::ExecStatus::ok : Utils::ExecStatus::user_error;
    }


    auto Engine::fetch_value(Code::ArgMode mode, int16_t id) noexcept -> std::optional<Value> {
        switch (mode) {
            case Code::ArgMode::constant:
                if (id >= 0 && id < m_consts_n) {
                    return m_const_view[id];
                } else {
                    return {};
                }
            case Code::ArgMode::reg:
                if (const auto real_mem_pos = m_rbp + id; real_mem_pos >= m_rbp && real_mem_pos < m_mem_limit) {
                    return m_memory[m_rbp + id];
                } else {
                    return {};
                }
            case Code::ArgMode::stack:
                if (!m_stack.empty()) {
                    return m_stack[m_rsp];
                } else {
                    return {};
                }
            case Code::ArgMode::heap:
            default:
                return {};
        }
    }

    auto Engine::push_value(Value value) noexcept -> bool {
        if (m_rsp >= m_stack_limit) {
            return false;
        }

        ++m_rsp;
        m_stack[m_rsp] = std::move(value);

        return true;
    }

    auto Engine::pop_value() noexcept -> std::optional<Value> {
        if (m_rsp < 0) {
            return {};
        }

        auto temp = m_stack[m_rsp];

        --m_rsp;

        return temp;
    }

    void Engine::handle_load_const([[maybe_unused]] uint16_t metadata, int16_t dest, int16_t const_id) noexcept {
        auto temp_const = fetch_value(Code::ArgMode::constant, const_id);
        
        if (!temp_const) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_dest_id = m_rbp + dest;

        m_rft = std::max(m_rft, real_mem_dest_id);
        m_memory[real_mem_dest_id] = temp_const.value();
        ++m_rip;
    }

    void Engine::handle_mov(uint16_t metadata, int16_t dest, int16_t src) noexcept {
        const auto src_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        auto src_value_opt = fetch_value(src_mode, src);

        if (!src_value_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_dest_id = m_rbp + dest;

        m_rft = std::max(m_rft, real_mem_dest_id);
        m_memory[real_mem_dest_id] = src_value_opt.value();
        ++m_rip;
    }

    void Engine::handle_neg([[maybe_unused]] uint16_t metadata, int16_t dest) noexcept {        
        if (const auto real_mem_dest_id = m_rbp + dest; m_memory[real_mem_dest_id].negate()) {
            m_rft = std::max(m_rft, real_mem_dest_id);
            m_res = static_cast<int>(Utils::ExecStatus::arg_error);
            return;
        }

        ++m_rip;
    }

    /// TODO: implement this instruction later!
    void Engine::handle_inc([[maybe_unused]] uint16_t metadata, [[maybe_unused]] int16_t dest) noexcept {
        m_res = static_cast<int>(Utils::ExecStatus::op_error);
    }

    /// TODO: implement this instruction later!
    void Engine::handle_dec([[maybe_unused]] uint16_t metadata, [[maybe_unused]] int16_t dest) noexcept {
        m_res = static_cast<int>(Utils::ExecStatus::op_error);
    }

    void Engine::handle_mul(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;
        m_memory[real_mem_loc] = lhs_opt.value() * rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_div(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }
        
        if (auto temp = lhs_opt.value() / rhs_opt.value(); !temp.is_none()) {
            const auto real_mem_loc = m_rbp + dest;

            m_memory[real_mem_loc] = std::move(temp);
            m_rft = std::max(m_rft, real_mem_loc);
            ++m_rip;
        } else {
            m_res = static_cast<int>(Utils::ExecStatus::math_error);
        }
    }

    void Engine::handle_mod(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }
        
        if (auto temp = lhs_opt.value() % rhs_opt.value(); !temp.is_none()) {
            const auto real_mem_loc = m_rbp + dest;

            m_memory[real_mem_loc] = std::move(temp);
            m_rft = std::max(m_rft, real_mem_loc);
            ++m_rip;
        } else {
            m_res = static_cast<int>(Utils::ExecStatus::math_error);
        }
    }

    void Engine::handle_add(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() + rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_sub(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() - rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_eq(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto dest_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto dest_opt = fetch_value(dest_mode, dest);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!dest_opt || !lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_rfv = lhs_opt.value() == rhs_opt.value();
        m_memory[real_mem_loc] = {m_rfv};
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_ne(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto dest_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto dest_opt = fetch_value(dest_mode, dest);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!dest_opt || !lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_rfv = lhs_opt.value() != rhs_opt.value();
        m_memory[real_mem_loc] = {m_rfv};
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_lt(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto dest_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto dest_opt = fetch_value(dest_mode, dest);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!dest_opt || !lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_rfv = lhs_opt.value() < rhs_opt.value();
        m_memory[real_mem_loc] = {m_rfv};
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_gt(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto dest_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto dest_opt = fetch_value(dest_mode, dest);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!dest_opt || !lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_rfv = lhs_opt.value() > rhs_opt.value();
        m_memory[real_mem_loc] = {m_rfv};
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_gte(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto dest_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto dest_opt = fetch_value(dest_mode, dest);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!dest_opt || !lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_rfv = lhs_opt.value() >= rhs_opt.value();
        m_memory[real_mem_loc] = {m_rfv};
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_lte(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto dest_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto dest_opt = fetch_value(dest_mode, dest);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        if (!dest_opt || !lhs_opt || !rhs_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_loc = m_rbp + dest;

        m_rfv = lhs_opt.value() <= rhs_opt.value();
        m_memory[real_mem_loc] = {m_rfv};
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_jmp(int16_t dest_ip) noexcept {
        m_rip = dest_ip;
    }

    void Engine::handle_jmp_if(int16_t dest_ip) noexcept {
        if (m_rfv) {
            m_rip = dest_ip;
        } else {
            ++m_rip;
        }
    }

    void Engine::handle_jmp_else(int16_t dest_ip) noexcept {
        if (!m_rfv) {
            m_rip = dest_ip;
        } else {
            ++m_rip;
        }
    }

    /**
     * @brief Executes logic for a bytecode function call. Specified operations in `vm.md` under the `call` note are done. Only special registers of RES and RFV are preserved since the call frames already track special register-related values. The stack will pop-off properly where only those 2 special regs mentioned earlier are saved.
     * 
     * @param metadata 
     * @param func_id 
     * @param arg_count 
     */
    void Engine::handle_call(int16_t func_id, int16_t arg_count) noexcept {
        const auto res_save_ok = push_value(Value {m_res});
        const auto rfv_save_ok = push_value(Value {m_rfv});

        if (!res_save_ok || !rfv_save_ok) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto old_rfi = m_rfi;
        const int16_t old_rip = m_rip + 1;
        const auto old_rbp = m_rbp;
        const auto old_rft = m_rft;

        m_call_frames.emplace_back(Utils::CallFrame {
            .old_func_idx = old_rfi,
            .old_func_ip = old_rip,
            .old_base_ptr = old_rbp,
            .old_mem_top = old_rft,
        });

        m_rfi = func_id;
        m_rip = 0;
        m_rbp = m_rft - arg_count + 1;
    }

    void Engine::handle_native_call([[maybe_unused]] int16_t native_id) noexcept {
        m_res = static_cast<int>(Utils::ExecStatus::op_error);
    }

    void Engine::handle_ret(uint16_t metadata, int16_t src_id) noexcept {
        /// 1. Prepare return value in correct slot for caller to hold correctness.
        const auto src_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        auto ret_src_opt = fetch_value(src_mode, src_id);

        if (!ret_src_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        m_memory[m_rbp] = ret_src_opt.value();

        /// 2. Restore the caller's call state
        auto [caller_rfi, caller_rip, caller_rbp, caller_rft] = m_call_frames.back();
        m_call_frames.pop_back();

        m_rfi = caller_rfi;
        m_rip = caller_rip;
        m_rbp = caller_rbp;
        m_rft = caller_rft;

        if (auto saved_rfv_box = pop_value(); saved_rfv_box) {
            m_rfv = saved_rfv_box.value().unbox_type<bool>().value();
        } else {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        if (auto saved_res_box = pop_value(); saved_res_box) {
            m_res = saved_res_box.value().unbox_type<int>().value();
        } else {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }
    }
}
