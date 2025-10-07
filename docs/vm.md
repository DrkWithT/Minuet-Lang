### VM Design Notes

### General
 - Register-based operations on a stack-like value-buffer & a separate stack exists to preserve special registers
 - General Registers: `1 - N` depending on a function's reserved space.
 - Special Registers:
    - `RFI`: function index
    - `RIP`: pointer to an incoming instruction
    - `RBP`: base pointer of register frame
    - `RFT`: position of frame buffer's top
    - `RSP`: pointer of stack top
    - `RRV`: return value
    - `RES`: error status
    - `RFV`: flag value (for comparisons)
 - Will have a heap and GC.

### Instruction Encoding (from MSB to LSB)
 - Opcode: 1 unsigned byte
 - Metadata: 1 unsigned short
    - `2` prefix bits for arity
    - `4` bits per argument's address mode: `imm, const, reg, heap`
 - Args: `0` to `3` signed 16-bit integers

### Opcodes:
 - `nop`: does nothing except increment `RIP`
 - `save`: preserves special register values on the stack
 - `restore`: reloads saved special registers from stack
 - `load_const <dest-reg> <imm>`: places a constant by index into a register
 - `mov <dest-reg> <src-reg>`: places a copied source register value to a destination register
 - `neg <reg>`: negates a register value in-place
 - `inc <reg>`: increments a register value in-place
 - `dec <reg>`: decrements a register value in-place
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
 - `call <func-id: imm> <arg-count: imm> <local-count: imm>`: saves all special registers and caller state, prepares a register frame with size `arg_count + local_count`, and sets:
    - `RFI` to `func-id` (saved to `ret-func-id` on call frame)
    - `RIP` to 0 (saved to `ret-address` on call frame)
    - `RBP` to `RFT - arg_count + 1`
 - `native_call <native-func-id: imm>`: ??
 - `ret <src: const / reg>`: places a return value to `RAV`, destroys the current register frame, and restores all special registers and caller state
 - `halt <status-code: imm>`: stops program execution with the specified `status-code`
