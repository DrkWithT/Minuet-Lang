argc=$#

usage_exit() {
    echo "Usage: utility.sh [help | build | unittest | run]\n\tutility.sh build <preset>\n\tutility.sh unittest\n\tutility.sh run <args>\n\tprofile <minuet-lang-file>";
    exit $1;
}

if [[ $argc -lt 1 ]]; then
    usage_exit 1;
fi

action="$1"
build_status=0

if [[ $action = "help" ]]; then
    usage_exit 0;
elif [[ $action = "build" && $argc -ge 2 ]]; then
    rm -rf ./build;
    rm -f ./compile_commands.json;
    cmake --fresh -S . -B build --preset "local-$2-build" && cmake --build build;

    if [[ $? -eq 0 ]]; then
        mv ./build/compile_commands.json .;
    fi
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
elif [[ $action = "profile" && $argc -eq 2 ]]; then
    samply record --save-only -o minuet_prof.json -- ./build/src/minuetm $2
else
    usage_exit 1;
fi
