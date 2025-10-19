#include <memory>
#include <queue>

#include "runtime/sequence_value.hpp"
#include "runtime/heap_storage.hpp"

namespace Minuet::Runtime {
    HeapStorage::HeapStorage()
    : m_hole_list {}, m_objects {}, m_dud {}, m_overhead {0UL}, m_next_id {0UL} {
        m_objects.reserve(cm_normal_obj_capacity);
        m_objects.resize(cm_normal_obj_capacity);
    }

    auto HeapStorage::is_ripe() const& noexcept -> bool {
        return m_overhead >= cm_normal_gc_threshold;
    }

    auto HeapStorage::try_create_value(ObjectTag obj_tag) noexcept -> std::unique_ptr<HeapValueBase>& {
        switch (obj_tag) {
        case ObjectTag::sequence:
            {
                const auto next_object_id = ([this]() {
                    if (!m_hole_list.empty()) {
                        const auto gap_id = m_hole_list.front();
                        m_hole_list.pop();

                        return gap_id;
                    }

                    return m_next_id;
                })();

                m_objects[next_object_id] = std::make_unique<SequenceValue>();
                m_overhead += cm_normal_obj_overhead;
                ++m_next_id;

                return m_objects[next_object_id];
            }
        default:
            return m_dud;
        }
    }

    [[nodiscard]] auto HeapStorage::try_destroy_value(std::size_t id) noexcept -> bool {
        if (auto& object_cell = m_objects[id]; object_cell) {
            object_cell = {};
            m_overhead -= cm_normal_obj_overhead;
            m_hole_list.emplace(id);

            return true;
        }

        return false;
    }

    auto HeapStorage::get_objects() noexcept -> std::vector<std::unique_ptr<HeapValueBase>>& {
        return m_objects;
    }
}
