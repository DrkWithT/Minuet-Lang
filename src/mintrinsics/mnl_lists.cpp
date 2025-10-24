#include <utility>
#include "mintrinsics/mnl_lists.hpp"

namespace Minuet::Intrinsics {
    auto native_len_of(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        auto arg_0 = vm.handle_native_fn_access(argc, 0);
        
        if (auto arg_obj_ptr = arg_0.to_object_ptr(); arg_obj_ptr) {
            vm.handle_native_fn_return({arg_obj_ptr->get_size()}, argc);
            return true;
        }

        return false;
    }

    auto native_list_push_back(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        auto target_arg = vm.handle_native_fn_access(argc, 0);
        auto new_item_arg = vm.handle_native_fn_access(argc, 1);

        if (target_arg.tag() != Runtime::FVTag::sequence) {
            return false;
        }

        if (auto obj_ptr = target_arg.to_object_ptr(); obj_ptr) {
            if (obj_ptr->get_tag() == Runtime::ObjectTag::sequence && !obj_ptr->is_frozen()) {
                if (obj_ptr->push_value(std::move(new_item_arg))) {
                    vm.handle_native_fn_return(std::move(target_arg), argc);

                    return true;
                }
            }
        }

        return false;
    }

    auto native_list_pop_back(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        auto target_arg = vm.handle_native_fn_access(argc, 0);

        if (target_arg.tag() != Runtime::FVTag::sequence) {
            return false;
        }

        if (auto obj_ptr = target_arg.to_object_ptr(); obj_ptr) {
            if (obj_ptr->get_tag() == Runtime::ObjectTag::sequence && !obj_ptr->is_frozen()) {
                if (auto old_back = obj_ptr->pop_value(Runtime::SequenceOpPolicy::back); !old_back.is_none()) {
                    vm.handle_native_fn_return(std::move(old_back), argc);

                    return true;
                }
            }
        }

        return false;
    }

    auto native_list_pop_front(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        auto target_arg = vm.handle_native_fn_access(argc, 0);

        if (target_arg.tag() != Runtime::FVTag::sequence) {
            return false;
        }

        if (auto obj_ptr = target_arg.to_object_ptr(); obj_ptr) {
            if (obj_ptr->get_tag() == Runtime::ObjectTag::sequence && !obj_ptr->is_frozen()) {
                if (auto old_front = obj_ptr->pop_value(Runtime::SequenceOpPolicy::front); !old_front.is_none()) {
                    vm.handle_native_fn_return(std::move(old_front), argc);

                    return true;
                }
            }
        }

        return false;
    }

    auto native_list_concat(Runtime::VM::Engine& vm, int16_t argc) -> bool {
        auto source_arg_p = vm.handle_native_fn_access(argc, 1).to_object_ptr();
        auto target_arg_p = vm.handle_native_fn_access(argc, 0).to_object_ptr();

        if (!target_arg_p || !source_arg_p) {
            return false;
        }

        if (target_arg_p->get_tag() != Runtime::ObjectTag::sequence || target_arg_p->is_frozen()) {
            return false;
        }

        for (const auto& source_items = source_arg_p->items(); const auto& item : source_items) {
            if (!target_arg_p->push_value(item)) {
                return false;
            }
        }

        return true;
    }
}
