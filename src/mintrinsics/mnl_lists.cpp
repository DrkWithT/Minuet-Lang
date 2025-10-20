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
}
