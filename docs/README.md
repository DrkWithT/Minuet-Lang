# README

### Brief
_Minuet_ is a small, Python & C inspired language for simple programs. Unlike shell scripting dialects across systems, this language aims to look and behave nicer for the programmer.

### Design Principles
 - Concise and semi-familiar syntax
 - Dynamic, _strong_, null-safe typing
 - Imperative & functional
    - First class functions
    - Higher order functions & some closures?
 - Built-in data structures
    - Lists (a rope-like hybrid of `Value` vectors in nodes)
    - Tuples
 - Built-in helper functions
 - Requires a `main` function

### Other Documentation
 - [Usage](./docs/usage.md)
 - [Grammar](/docs/grammar.md)
 - [VM](/docs/vm.md)

### Roadmap
 - `0.6.2`: Fuse separate condition `MOV`s & their conditional jumps.
   - Example: `jump_eq <IP> <LHS> <RHS>`
 - `0.6.3`: Add simple bytecode optimization passes, following dependency injection.
   - Unused variable elimination
   - Reduction of useless `mov` and `nop` instructions
   - Dead branch elimination
 - `0.7.0`: Add dictionaries
 - `0.8.0`: Add strings
   - `String` type
   - `string_do_xyz` intrinsic functions
 - `0.9.0`: Add program argument support (wrapped `char* argv[]` strings):
   - Add `[args]` intrinsics: `get_argv()`, `get_argc()`
 - `0.9.1`: Add file I/O support:
   - Add `open_file(path)` intrinsic, yielding an `fd`
   - Add `append_to_file(fd, mode)` intrinsic
   - Add `close_file(fd)` intrinsic
 - `0.10.0`: Add simple x64 JIT for pure, global functions taking integers.
   - Prototype JIT compiler using CFG IR, regalloc, etc.
   - Create a JIT compilation thread which runs in the background, adding a JIT version of a function when finished.
   - Add JIT / bytecode dispatch in `call` instruction.
