### Project Usage

#### Building the Interpreter

Steps:
 - 1. Ensure that CMake 3.30+ (preferably), a C++23 supporting compiler, and `ninja` are all installed.
    - Installing `samply` through Rust's `cargo` utility is optional if profiling is not desired.
 - 2. Run `chmod u+x ./utility.sh`
 - 3. Run `./utility.sh build release` or `./utility.sh build debug`

#### Usage
 - Run `./utility.sh help` for utility script help. This script is meant to build, test, and run the program.
