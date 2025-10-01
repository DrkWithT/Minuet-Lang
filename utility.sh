argc=$#

usage_exit() {
    echo "Usage: utility.sh [help | build | unittest | run]\n\tutility.sh build <preset> 'refresh'?\n\tutility.sh unittest\n\tutility.sh run <args>";
    exit $1;
}

if [[ $argc -lt 1 ]]; then
    usage_exit 1;
fi

action="$1"
build_status=0

if [[ $action = "help" ]]; then
    usage_exit 0;
elif [[ $action = "build" && $argc -ge 2 && $argc -le 4 ]]; then
    if [[ $argc -eq 4 && "$3" = "refresh" ]]; then
        cmake --fresh -S . -B build --preset "local-$2-build" && cmake --build build;
        build_status=$?
    else
        cmake -S . -B build --preset "local-$2-build" && cmake --build build;
        build_status=$?
    fi

    mv ./build/compile_commands.json .;
elif [[ $action = "unittest" && $argc -eq 1 ]]; then
    touch ./logs/all.txt;
    ctest --test-dir build --timeout 2 -V 1> ./logs/all.txt;

    if [[ $? -eq 0 ]]; then
        echo "All tests OK";
    else
        echo "Some tests failed: see logs";
    fi
elif [[ $action = "run" ]]; then
    ./build/src/minuetm "${@:2}" && echo "\033[1;32mRUN OK\033[0m" || echo "\033[1;33mRUN FAILED\033[0m"
else
    usage_exit 1;
fi
