handle_usage_and_exit() {
    echo "USAGE:\n\n./try_test_suite.sh [help | run] [args...]\n\thelp []: prints usage information.\n\trun [neg | pos] [simple | unused]: runs the specified groups of test programs.\n";
    exit $1;
}

handle_suite_group() {
    check_status=$((0));

    if [[ $1 = "neg" ]]; then
        check_status=$((1));
    fi

    tests=$( find ./test_suite/$2/*.mnl );

    for test_path in $tests
    do
        ./build/src/minuetm run $test_path;

        if [[ $? -ne $check_status ]]; then
            echo "\033[1;31mFAILED on demo '$test_path'\033[0m";
        else
            echo "\033[1;32mCOMPLETED demo '$test_path'\033[0m";
        fi
    done
}

handle_action() {
    argc=$#;
    action="$1";

    if [[ $argc -lt 1 ]]; then
        handle_usage_exit 1;
    fi

    if [[ $action = "help" ]]; then
        handle_usage_and_exit 0;
    elif [[ $action = "run" && $argc -eq 3 ]]; then
        handle_suite_group "$2" "$3"
    else
        handle_usage_and_exit 1;
    fi
}

handle_action "${@:1}"
