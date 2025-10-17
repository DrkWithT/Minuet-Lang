#include <utility>
#include <memory>
#include <set>
#include <queue>

#include "runtime/sequence_value.hpp"
#include "runtime/heap_storage.hpp"

namespace Minuet::Runtime {
    HeapStorage::HeapStorage(std::vector<std::unique_ptr<HeapValueBase>> preloaded_items)
    : m_hole_list {}, m_objects (std::move(preloaded_items)), m_dud {} {}

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

                    return m_objects.size();
                })();

                m_objects[next_object_id] = std::make_unique<SequenceValue>();

                return m_objects[next_object_id];
            }
        default:
            return m_dud;
        }
    }

    auto HeapStorage::get_objects() noexcept -> std::vector<std::unique_ptr<HeapValueBase>>& {
        return m_objects;
    }
}
