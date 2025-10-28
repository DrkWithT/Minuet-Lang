#include <utility>
#include <algorithm>
// #include <print>
#include <queue>
#include <set>

#include "runtime/fast_value.hpp"
#include "runtime/bytecode.hpp"
#include "runtime/sequence_value.hpp"
#include "runtime/vm.hpp"

namespace Minuet::Runtime::VM {
    using Minuet::Runtime::FastValue;

    static constexpr auto ok_res_value = static_cast<int>(Utils::ExecStatus::ok);

    Engine::Engine(Utils::EngineConfig config, Code::Program& prgm, std::any native_fn_table_wrap)
    : m_heap {}, m_memory {}, m_call_frames {}, m_chunk_view {}, m_const_view {}, m_call_frame_ptr {nullptr}, m_native_funcs {}, m_rfi {}, m_rip {}, m_rbp {}, m_rft {}, m_rsp {}, m_consts_n {}, m_rrd {}, m_res {} {
        const auto [mem_limit, recur_depth_max] = config;
        const auto prgm_entry_fn_id = prgm.entry_id.value_or(-1);

        const auto mem_vec_size = static_cast<std::size_t>(mem_limit);
        m_memory.reserve(mem_vec_size);
        m_memory.resize(mem_vec_size);

        m_call_frames.reserve(recur_depth_max);
        m_call_frames.resize(recur_depth_max);

        m_chunk_view = prgm.chunks.data();
        m_const_view = prgm.constants.data();
        m_call_frame_ptr = m_call_frames.data();
        m_native_funcs = (native_fn_table_wrap.type() == typeid(Runtime::NativeProcTable*))
            ? std::any_cast<Runtime::NativeProcTable*>(native_fn_table_wrap)
            : nullptr;

        m_rfi = prgm_entry_fn_id;
        m_rip = 0;
        m_rbp = 0;
        m_rft = 0;
        m_rsp = -1;
        m_res = (prgm_entry_fn_id >= 0 && m_native_funcs != nullptr)
            ? static_cast<int>(Utils::ExecStatus::ok)
            : static_cast<int>(Utils::ExecStatus::setup_error);

        m_consts_n = static_cast<int>(prgm.constants.size());

        *m_call_frame_ptr = Utils::CallFrame {
            .old_func_idx = 0,
            .old_func_ip = 0,
            .old_base_ptr = 0,
            .old_mem_top = 0,
            .old_exec_status = ok_res_value,
        };
        ++m_rrd; // NOTE: main is implicitly called if present... call depth is now 1 to count this!
    }

    auto Engine::operator()() -> Utils::ExecStatus {
        while (m_rrd > 0 && m_res == ok_res_value) {
            const auto& [args, metadata, opcode] = m_chunk_view[m_rfi][m_rip];

            // std::println("RFI = {}, RIP = {}, RBP = {}, RFT = {}, RSP = {}, opcode: {}", m_rfi, m_rip, m_rbp, m_rft, m_rsp, Code::opcode_name(opcode)); // debug

            switch (opcode) {
                case Code::Opcode::nop:
                    ++m_rip;
                    break;
                case Code::Opcode::make_seq:
                    handle_make_seq(args[0]);
                    break;
                case Code::Opcode::seq_obj_push:
                    handle_seq_obj_push(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::seq_obj_pop:
                    handle_seq_obj_pop(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::seq_obj_get:
                    handle_seq_obj_get(metadata, args[0], args[1], args[2]);
                    break;
                case Code::Opcode::frz_seq_obj:
                    handle_frz_seq_obj(args[0]);
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
                    m_rip = args[0];
                    break;
                case Code::Opcode::jump_if:
                    handle_jmp_if(args[0], args[1]);
                    break;
                case Code::Opcode::jump_else:
                    handle_jmp_else(args[0], args[1]);
                    break;
                case Code::Opcode::call:
                    handle_call(args[0], args[1]);
                    break;
                case Code::Opcode::ret:
                    handle_ret(metadata, args[0]);
                    break;
                case Code::Opcode::native_call:
                    handle_native_call(args[0], args[1]);
                    break;
                case Code::Opcode::halt:
                default:
                    m_res = static_cast<int>(Utils::ExecStatus::op_error);
                    break;
            }
        }

        if (m_res != ok_res_value) {
            return static_cast<Utils::ExecStatus>(m_res);
        }

        return (m_memory[0] == FastValue {0}) ? Utils::ExecStatus::ok : Utils::ExecStatus::user_error;
    }

    auto Engine::handle_native_fn_access(int16_t arg_count, int16_t offset) & noexcept -> Runtime::FastValue& {
        const auto native_call_base_slot = m_rft - arg_count + 1;

        return m_memory[native_call_base_slot + offset];
    }

    void Engine::handle_native_fn_return(Runtime::FastValue&& result, [[maybe_unused]] int16_t arg_count) noexcept {
        const auto native_call_base_slot = m_rft - arg_count + 1;

        m_memory[native_call_base_slot] = std::move(result);
    }


    /**
     * @brief Implements the bulk of garbage collection. Specifically, the logic will base itself on craftinginterpreters.com: the GC will stop-the-world for each collection if the heap has a certain "overhead score" given by
     */
    void Engine::try_mark_and_sweep() {
        if (!m_heap.is_ripe()) {
            return;
        }

        std::set<HeapValuePtr> live_object_ptrs;
        std::set<HeapValuePtr> visited;
        std::queue<HeapValuePtr> frontier;

        for (auto abs_reg_id = 0; abs_reg_id <= m_rft; ++abs_reg_id) {
            if (HeapValuePtr object_p = m_memory[abs_reg_id].to_object_ptr(); object_p) {
                frontier.emplace(object_p);
            }
        }

        // 1. Use a BFS traversal to mark all heap values that are reachable from the register frames. During this stage, every marked address will be stored in a "reachable" set...
        while (!frontier.empty()) {
            auto next_ptr = frontier.front();
            frontier.pop();

            live_object_ptrs.emplace(next_ptr);

            if (next_ptr->get_tag() == ObjectTag::sequence) {
                SequenceValue* sequence_ptr = dynamic_cast<SequenceValue*>(next_ptr);

                for (auto& item_value : sequence_ptr->items()) {
                    if (HeapValuePtr item_obj_ptr = item_value.to_object_ptr(); item_obj_ptr != nullptr && !visited.contains(item_obj_ptr)) {
                        frontier.emplace(item_obj_ptr);
                    }
                }
            }

            visited.emplace(next_ptr);
        }

        // 2. Linearly scan through the heap cells for anything NOT in the reachable set... If the value is unmarked, the VM can collect it.
        for (auto cell_id = 0UL; auto& heap_cell : m_heap.get_objects()) {
            if (heap_cell.get()->get_tag() != ObjectTag::dud) {
                if (!m_heap.try_destroy_value(cell_id)) {
                    break;
                }
            }

            ++cell_id;
        }
    }

    void Engine::handle_make_seq(int16_t dest_reg) noexcept {
        HeapValuePtr temp_obj_ref = m_heap.try_create_value(ObjectTag::sequence).get();
        const auto abs_reg_id = m_rbp + dest_reg;

        m_memory[abs_reg_id] = temp_obj_ref;

        ++m_rip;
    }

    void Engine::handle_seq_obj_push([[maybe_unused]] uint16_t metadata, int16_t dest, int16_t src_id, [[maybe_unused]] int16_t mode) noexcept {
        const auto abs_dest_id = m_rbp + dest;
        const auto src_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);

        auto src_value_opt = fetch_value(src_mode, src_id);

        if (!src_value_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        auto src_value = src_value_opt.value();

        if (HeapValuePtr dest_obj_ref = m_memory[abs_dest_id].to_object_ptr(); dest_obj_ref) {
            dest_obj_ref->push_value(src_value);
            ++m_rip;
        } else {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
        }
    }

    void Engine::handle_seq_obj_pop([[maybe_unused]] uint16_t metadata, int16_t dest, int16_t src_id, int16_t mode) noexcept {
        const auto abs_dest_id = m_rbp + dest;
        const auto abs_src_id = m_rbp + src_id;
        const SequenceOpPolicy pop_mode = static_cast<SequenceOpPolicy>(mode);

        HeapValuePtr src_obj_ptr = m_memory[abs_src_id].to_object_ptr();

        if (!src_obj_ptr) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        m_memory[abs_dest_id] = src_obj_ptr->pop_value(pop_mode);

        ++m_rip;
    }

    void Engine::handle_seq_obj_get([[maybe_unused]] uint16_t metadata, int16_t dest, int16_t src_id, int16_t pos_value_id) noexcept {
        const auto abs_dest_id = m_rbp + dest;
        const auto abs_src_id = m_rbp + src_id;
        const auto pos_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto pos_value_opt = fetch_value(pos_mode, pos_value_id);

        if (!pos_value_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::arg_error);
            return;
        }

        const auto pos_i32_opt = pos_value_opt.value().to_scalar();

        if (!pos_i32_opt) {
            m_res = static_cast<int>(Utils::ExecStatus::arg_error);
            return;
        }

        const auto pos_i32 = pos_i32_opt.value();

        if (HeapValuePtr src_obj_ref = m_memory[abs_src_id].to_object_ptr(); src_obj_ref) {
            if (auto item_opt = src_obj_ref->get_value(pos_i32); item_opt) {
                m_memory[abs_dest_id] = {item_opt.value()};
                ++m_rip;
                return;
            }
        }

        m_res = static_cast<int>(Utils::ExecStatus::mem_error);
    }

    void Engine::handle_frz_seq_obj(int16_t dest) noexcept {
        const auto abs_dest_id = m_rbp + dest;

        if (HeapValuePtr obj_ref = m_memory[abs_dest_id].to_object_ptr(); obj_ref) {
            obj_ref->freeze();
            ++m_rip;

            return;
        }

        m_res = static_cast<int>(Utils::ExecStatus::mem_error);
    }

    auto Engine::fetch_value(Code::ArgMode mode, int16_t id) noexcept -> std::optional<FastValue> {
        switch (mode) {
            case Code::ArgMode::constant: return m_const_view[id];
            case Code::ArgMode::reg: return m_memory[m_rbp + id];
            case Code::ArgMode::stack:
            case Code::ArgMode::heap:
            default: return {};
        }
    }

    void Engine::handle_load_const([[maybe_unused]] uint16_t metadata, int16_t dest, int16_t const_id) noexcept {
        auto temp_const = fetch_value(Code::ArgMode::constant, const_id);

        if (!temp_const) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        const auto real_mem_dest_id = m_rbp + dest;

        m_memory[real_mem_dest_id] = temp_const.value();
        m_rft = std::max(m_rft, real_mem_dest_id);
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

        /// NOTE: If the register's FastValue is a primitive, replace it. But if the FastValue contains a reference to the actual value (e.g a list's item) then `FastValue::emplace_other()` is necessary.
        if (auto& dest_ref = m_memory[real_mem_dest_id]; dest_ref.tag() != FVTag::val_ref) {
            dest_ref = std::move(src_value_opt.value());
        } else if (!dest_ref.emplace_other(src_value_opt.value())) {
            m_res = static_cast<int>(Utils::ExecStatus::mem_error);
            return;
        }

        m_rft = std::max(m_rft, real_mem_dest_id);
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

        const auto real_mem_loc = m_rbp + dest;
        m_memory[real_mem_loc] = std::move(lhs_opt.value());
        m_memory[real_mem_loc] *= rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_div(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

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

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = std::move(lhs_opt.value());
        m_memory[real_mem_loc] += rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_sub(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);
        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value();
        m_memory[real_mem_loc] -= rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_eq(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() == rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_ne(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() != rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_lt(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() < rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_gt(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() > rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_gte(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() >= rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_cmp_lte(uint16_t metadata, int16_t dest, int16_t lhs, int16_t rhs) noexcept {
        const auto lhs_mode = static_cast<Code::ArgMode>((metadata & 0b00001111000000) >> 6);
        const auto rhs_mode = static_cast<Code::ArgMode>((metadata & 0b11110000000000) >> 10);

        auto lhs_opt = fetch_value(lhs_mode, lhs);
        auto rhs_opt = fetch_value(rhs_mode, rhs);

        const auto real_mem_loc = m_rbp + dest;

        m_memory[real_mem_loc] = lhs_opt.value() <= rhs_opt.value();
        m_rft = std::max(m_rft, real_mem_loc);
        ++m_rip;
    }

    void Engine::handle_jmp_if(int16_t check_reg, int16_t dest_ip) noexcept {
        if (m_memory[m_rbp + check_reg]) {
            m_rip = dest_ip;
        } else {
            ++m_rip;
        }
    }

    void Engine::handle_jmp_else(int16_t check_reg, int16_t dest_ip) noexcept {
        if (!m_memory[m_rbp + check_reg]) {
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
        const auto old_rfi = m_rfi;
        const int16_t old_rip = m_rip + 1;
        const auto old_rbp = m_rbp;
        const auto old_rft = m_rft;
        const auto old_res = m_res;

        ++m_call_frame_ptr;
        *m_call_frame_ptr = Utils::CallFrame {
            .old_func_idx = old_rfi,
            .old_func_ip = old_rip,
            .old_base_ptr = old_rbp,
            .old_mem_top = old_rft,
            .old_exec_status = old_res,
        };
        ++m_rrd;

        m_rfi = func_id;
        m_rip = 0;
        m_rbp = m_rft - arg_count + 1;
    }

    void Engine::handle_native_call(int16_t native_id, int16_t arg_count) noexcept {
        m_res = (m_native_funcs->data()[native_id](*this, arg_count)) ? ok_res_value : static_cast<int>(Utils::ExecStatus::op_error);

        ++m_rip;
    }

    void Engine::handle_ret(uint16_t metadata, int16_t src_id) noexcept {
        /// 1. Prepare return value in correct slot for caller to hold correctness.
        const auto src_mode = static_cast<Code::ArgMode>((metadata & 0b00000000111100) >> 2);
        auto ret_src_opt = fetch_value(src_mode, src_id);

        m_memory[m_rbp] = std::move(ret_src_opt.value());

        /// 2. Restore the caller's call state
        auto [caller_rfi, caller_rip, caller_rbp, caller_rft, caller_res] = *m_call_frame_ptr;
        --m_call_frame_ptr;
        --m_rrd;

        m_rfi = caller_rfi;
        m_rip = caller_rip;
        m_rbp = caller_rbp;
        m_rft = caller_rft;
        m_res = caller_res;

        try_mark_and_sweep();
    }
}
