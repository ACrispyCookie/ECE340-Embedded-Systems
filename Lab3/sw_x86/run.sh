#!/bin/bash

SRC_DIR="./src"
OUT_DIR="./out"
TARGET_DIR="./target"
PERF_DIR="./perf"

CORRECT_OUTPUT="$OUT_DIR/correct_out"
# Will be set later once a file is selected
OUTPUT_FILE=""

NONE=true
STORE_ASM=false
RUN_TEST=false
RUN_PERF=false
ARGS=""

# Print usage/help
print_help() {
    echo "Usage: ./run.sh [OPTION] [ARGUMENTS]"
    echo ""
    echo "Options:"
    echo "  --test       Compile and run the selected program with test arguments."
    echo "               Output will be stored in $OUTPUT_FILE and compared with $CORRECT_OUTPUT."
    echo "  --asm        Store the assembly of the input source code."
    echo "  --perf       Run the program under perf stat with the provided arguments."
    echo "  --help       Show this help message."
    echo ""
    echo "If both --test and --perf are used, the code will be compiled twice:"
    echo "once with output for testing and once without for perf analysis."
    exit 0
}

# Handle flags
while [[ $# -gt 0 ]]; do
    case "$1" in
        --asm)
            NONE=false
            STORE_ASM=true
            shift
            ;;
        --test)
            NONE=false
            RUN_TEST=true
            shift
            ;;
        --perf)
            NONE=false
            RUN_PERF=true
            ARGS="$2"
            shift 2
            ;;
        --help)
            NONE=false
            print_help
            ;;
        *)
            echo "Unknown option: $1"
            print_help
            ;;
    esac
done

# Collect .c files
c_files=($(find "$SRC_DIR" -maxdepth 1 -type f -name '*.c'))

# Exit if no .c files
if [ ${#c_files[@]} -eq 0 ]; then
    echo "No .c files found in $SRC_DIR"
    exit 1
fi

echo "Select a source file to compile and run:"
select file_path in "${c_files[@]}"; do
    if [[ -n "$file_path" ]]; then
        src_file=$(basename "$file_path")
        target="${src_file%.c}"
        binary="$TARGET_DIR/$target"
        OUTPUT_FILE="$OUT_DIR/${target}.out"
        PERF_FILE="$PERF_DIR/${target}.data"
        echo "Selected: $src_file"

        if [ "$NONE" = true ]; then
            echo "###################### COMPILE ######################"
            echo "Compiling..."
            make TARGET="$target"
            echo "######################################################"
        fi

        if [ "$STORE_ASM" = true ]; then
            echo "###################### ASSEMBLY ######################"
            echo "Compiling assembly..."
            make TARGET="$target" assembly
            echo "######################################################"
        fi

        if [ "$RUN_TEST" = true ]; then
            echo "######################## TEST ########################"
            echo "Compiling with test output enabled..."
            make clean > /dev/null
            make TARGET="$target" TEST=1
            if [ ! -x "$binary" ]; then
                echo "Compilation failed or binary not found."
                exit 1
            fi

            echo "Running test against known output..."
            args="256 15000"

            "$binary" $args > "$OUTPUT_FILE"
            diff <(head -n -1 "$OUTPUT_FILE") <(head -n -1 "$CORRECT_OUTPUT") >/dev/null
            if [ $? -eq 0 ]; then
                echo "✅ Test Passed."
            else
                echo "❌ Test Failed. See diff below:"
                diff "$OUTPUT_FILE" "$CORRECT_OUTPUT"
            fi
            echo "######################################################"
        fi

        if [ "$RUN_PERF" = true ]; then
            echo "######################## PERF ########################"
            echo "Compiling with test output disabled..."
            make clean > /dev/null
            make TARGET="$target"
            if [ ! -x "$binary" ]; then
                echo "Compilation failed or binary not found."
                exit 1
            fi

            echo "Running under perf stat with arguments: $ARGS"
            perf stat "$binary" $ARGS > /dev/null 2> >(tee "$PERF_FILE")
            cat $PERF_FILE
            echo "✅ Perf stat output saved to $PERF_FILE"
            echo "######################################################"
        fi
        break
    else
        echo "Invalid selection."
    fi
done
