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
 - [Grammar](/docs/grammar.md)
 - [VM](/docs/vm.md)

### Roadmap
 - `0.4.0`: Add tuple support (requires an object heap & hidden object references)
 - `0.5.0`: Add basic list support
 - `0.5.1`: Add simple bytecode optimization passes, following dependency injection.
   - Unused variable elimination
   - Reduction of useless `mov` and `nop` instructions
   - Dead branch elimination
