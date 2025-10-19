#ifndef MINUET_RUNTIME_HEAP_STORAGE_HPP
#define MINUET_RUNTIME_HEAP_STORAGE_HPP

#include <memory>
#include <queue>
#include <vector>

#include "runtime/fast_value.hpp"

namespace Minuet::Runtime {
    class HeapStorage {
    private:
        /// NOTE: stores constant for default memory "capacity" of VM heap
        static constexpr auto cm_normal_max_overhead = 16384UL;
        static constexpr auto cm_normal_gc_threshold = 8192UL;
        static constexpr auto cm_normal_obj_overhead = 16UL;
        static constexpr auto cm_normal_obj_capacity = cm_normal_max_overhead / cm_normal_obj_overhead;

        /// NOTE: tracks freed object slots in the VM "heap" remaining between live slots
        std::queue<std::size_t> m_hole_list;

        /// NOTE: tracks actual object slots (live / unreachable)
        std::vector<std::unique_ptr<HeapValueBase>> m_objects;

        /// NOTE: holds a null dud for invalid object references
        std::unique_ptr<HeapValueBase> m_dud;

        std::size_t m_overhead;
        std::size_t m_next_id;

    public:
        /// NOTE: preload "heap literals" from IR & codegen stages here!
        HeapStorage();

        [[nodiscard]] auto is_ripe() const& noexcept -> bool;

        [[nodiscard]] auto try_create_value(ObjectTag obj_tag) noexcept -> std::unique_ptr<HeapValueBase>&;

        [[nodiscard]] auto try_destroy_value(std::size_t id) noexcept -> bool;

        [[nodiscard]] auto get_objects() noexcept -> std::vector<std::unique_ptr<HeapValueBase>>&;
    };
}

#endif
