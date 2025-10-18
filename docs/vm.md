### VM Design Notes

### General
 - Register-based operations on a stack-like value-buffer
 - General Registers: `1 - N` depending on a function's reserved space.
 - Special Registers:
    - `RFI`: function index
    - `RIP`: pointer to an incoming instruction
    - `RBP`: base pointer of register frame
    - `RFT`: position of frame buffer's top
    - `RSP`: pointer of stack top
    - `RES`: error status
    - `RRD`: current recursion depth
    - `RFV`: flag value (for comparisons)
 - Will have a heap and GC.

### Instruction Encoding (from LSB to MSB)
 - Opcode: 1 unsigned byte
 - Metadata: 1 unsigned short
    - `2` prefix bits for arity
    - `4` bits per argument's address mode: `imm, const, reg, heap`
 - Args: `0` to `3` signed 16-bit integers

### Call Frame Format
 - Old `RFI` & `RIP` values for a "caller-return address"
 - Old `RBP` value
 - Old `RFT` value
 - Old `RES` value
 - Old `RFV` value

### Opcodes:
 - `nop`: does nothing except increment `RIP`
 - `make_seq <dest-reg>`: creates an empty sequence on the heap and loads its reference in a register
 - `seq_obj_push <dest-obj-reg> <src-value-reg> <mode>`: appends to the front or back of a sequence (modes 0 or 1) if it's flexible
 - `seq_obj_pop <dest-value-reg> <src-obj-reg> <mode>`: removes an item from the front or back of a sequence (modes 0 or 1) if it's flexible
 - `seq_obj_get <dest-value-reg> <src-obj-reg> <index>`: retrieves the item from a sequence at a given index
 - `frz_seq_obj <dest-obj-reg>`: makes the sequence fixed size _after tuple initialization_
 - `load_const <dest-reg> <imm>`: places a constant by index into a register
 - `mov <dest-reg> <src: const / reg>`: places a copied source value (constant or register) to a destination register
 - `neg <dest-reg>`: negates a register value in-place
 - `inc <dest-reg>`: increments a register value in-place
 - `dec <dest-reg>`: decrements a register value in-place
 - `mul <dest-reg> <lhs: const / reg> <rhs: const / reg>`: ...
 - `div <dest-reg> <lhs: const / reg> <rhs: const / reg>`: ...
 - `mod <dest-reg> <lhs: const / reg> <rhs: const / reg>`: ...
 - `add <dest-reg> <lhs: const / reg> <rhs: const / reg>`: ...
 - `sub <dest-reg> <lhs: const / reg> <rhs: const / reg>`: ...
 - `equ <lhs: const / reg> <rhs: const / reg>`: ...
 - `neq <lhs: const / reg> <rhs: const / reg>`: sets `RFV` to the result of the comparison
 - `lt <lhs: const / reg> <rhs: const / reg>`: similar to previous
 - `gt <lhs: const / reg> <rhs: const / reg>`: similar to previous
 - `lte <lhs: const / reg> <rhs: const / reg>`: similar to previous
 - `gte <lhs: const / reg> <rhs: const / reg>`: similar to previous
 - `jump <target-ip: imm>`: sets `RIP` to the immediate value (absolute code chunk position)
 - `jump_if: <target-ip: imm>`: sets `RIP` to the immediate value if `RFV == true`
 - `jump_else: <target-ip: imm>`: sets `RIP` to the immediate value if `RFV == false`
 - `call <func-id: imm> <arg-count: imm> <local-count: imm>`: saves some special registers (`RES`, `RFV`) and caller state in a call frame, prepares a register frame above the latest argument register, and sets:
    - `RFI` to `func-id` (saved to `ret-func-id` on call frame)
    - `RIP` to 0 (saved to `ret-address` on call frame)
    - `RBP` to `RFT - arg_count + 1`
 - `native_call <native-func-id: imm>`: invokes the registered native function upon VM state:
   - The native function must respect the "calling convention"... It must access by register base offset (`caller_RFT - arg_count + 1`).
   - Native functions must call `Engine::handle_native_fn_return(<result-Value>)` on completion _only if_ anything is returned.
 - `ret <src: const / reg>`: places a return value at the `RBP` location, destroys the current register frame, and restores some special registers (`RFV`, `RES`) and caller state from the top call frame
 - `halt <status-code: imm>`: stops program execution with the specified `status-code`

### Runtime Status Codes:
 - ok: no errors, yippee!
 - entry_error: invalid main ID
 - op_error: invalid opcode
 - arg_error: invalid opcode arg
 - mem_error: bad VM memory access on register slot / stack
 - math_error: illegal math operation e.g division by `0`
 - user_error: user-caused failure (main did not return `int(0)`)
 - any_error: general error
