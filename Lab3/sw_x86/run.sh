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
PERF_OUTPUT=false
PERF_RUNS=1
PLOT_MODE=false
ARGS=""

# Print usage/help
print_help() {
    echo "Usage: ./run.sh [OPTION] [ARGUMENTS]"
    echo ""
    echo "Options:"
    echo "  --test                                  Compile and run the selected program with test arguments."
    echo "                                          Output will be stored in $OUTPUT_FILE and compared with $CORRECT_OUTPUT."
    echo "  --asm                                   Store the assembly of the input source code."
    echo "  --perf [runs] [args]                    Run the program under perf stat with the provided arguments."
    echo "  --perf-out [runs] [args]                Store the output of the program when using the perf flag."
    echo "  --plot [runs] [args] [stats to plot]    Runs all the available versions for the specified times and plots the specified stats"
    echo "  --help                                  Show this help message."
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
            PERF_RUNS="$2"
            ARGS="$3"
            shift 3
            ;;
        --perf-out)
            NONE=false
            RUN_PERF=true
            PERF_OUTPUT=true
            PERF_RUNS="$2"
            ARGS="$3"
            shift 3
            ;;
        --plot)
            NONE=false
            PLOT_MODE=true
            PERF_RUNS="$2"
            ARGS="$3"
            shift 3
            STATS=()
            while [[ $# -gt 0 && "$1" != --* ]]; do
                STATS+=("$1")
                shift
            done
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

if [ "$PLOT_MODE" = true ]; then
    echo "Starting data gathering and plotting process..."
    read -r FIRST_DIM SECOND_DIM <<< "$ARGS"
    INPUT="${FIRST_DIM}x${SECOND_DIM}"
    INPUT_DESC="${INPUT} input"
    FINAL_PERF_DIR="$PERF_DIR/$INPUT"
    mkdir -p "$FINAL_PERF_DIR"
    mkdir -p "$OUT_DIR"
    mkdir -p "$TARGET_DIR"

    for file_path in "${c_files[@]}"; do
        src_file=$(basename "$file_path")
        target="${src_file%.c}"
        binary="$TARGET_DIR/$target"
        OUTPUT_FILE="$OUT_DIR/${target}.out"
        PERF_FILE="$FINAL_PERF_DIR/${target}.data"

        echo "▶️ Compiling $target..."
        make clean TARGET="$target" > /dev/null
        make TARGET="$target" > /dev/null

        echo "Running $target under perf stat $PERF_RUNS times..."

        > "$PERF_FILE"  # Clear old data
        for ((i = 1; i <= PERF_RUNS; i++)); do
            percent=$(( (i * 100) / PERF_RUNS ))
            printf "\r🔄 Running %d/%d (%d%% complete)" "$i" "$PERF_RUNS" "$percent"

            {
                echo ""
                echo "### Run $i on $(date) with args: $ARGS"
                perf stat "$binary" $ARGS > "$OUTPUT_FILE"
            } >> "$PERF_FILE" 2>&1

            tail -n 1 "$OUTPUT_FILE" >> "$PERF_FILE"
        done
        printf "\n"
    done

    echo ""
    echo "📈 Generating plots..."
    for stat in "${STATS[@]}"; do
        python3 plot-avg.py "$FINAL_PERF_DIR" "$stat" "$INPUT_DESC"
    done

    echo "✅ All plots generated."
    exit 0
fi

IFS=$'\n' sorted_files=($(printf "%s\n" "${c_files[@]}" | sort))
unset IFS

echo "Select a source file to compile and run:"
for i in "${!sorted_files[@]}"; do
    printf "%d) %s\n" "$i" "$(basename "${sorted_files[$i]}")"
done

read -rp "Enter the number corresponding to the file: " index
file_path="${sorted_files[$index]}"
if [[ -n "$file_path" ]]; then
    src_file=$(basename "$file_path")
    target="${src_file%.c}"
    binary="$TARGET_DIR/$target"
    OUTPUT_FILE="$OUT_DIR/${target}.out"
    
    read -r FIRST_DIM SECOND_DIM <<< "$ARGS"
    INPUT="${FIRST_DIM}x${SECOND_DIM}"
    FINAL_PERF_DIR="$PERF_DIR/$INPUT"
    PERF_FILE="$FINAL_PERF_DIR/${target}.data"
    mkdir -p "$FINAL_PERF_DIR"
    echo "Selected: $src_file"

    if [ "$NONE" = true ]; then
        echo "###################### COMPILE ######################"
        echo "Compiling..."
        make clean TARGET="$target" > /dev/null
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
        make clean TARGET="$target" > /dev/null
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
        make clean TARGET="$target" > /dev/null
        if [ "$PERF_OUTPUT" = true ]; then
            echo "Compiling with test output enabled..."
            make TARGET="$target" TEST=1
        else
            echo "Compiling with test output disabled..."
            make TARGET="$target"
        fi

        if [ ! -x "$binary" ]; then
            echo "Compilation failed or binary not found."
            exit 1
        fi

        echo "Running under perf stat $PERF_RUNS time(s) with arguments: $ARGS"
        
        > "$PERF_FILE"  # Clear old data
        for ((i = 1; i <= PERF_RUNS; i++)); do

            {
                echo ""
                echo "### Run $i on $(date) with args: $ARGS"
                perf stat "$binary" $ARGS > "$OUTPUT_FILE"
            } >> "$PERF_FILE" 2>&1

            if [ $PERF_RUNS -eq 1 ]; then
                echo "▶️ Run $i/$PERF_RUNS"
                echo ""
                echo "🔹 Last line of program output:"
                tail -n 1 "$OUTPUT_FILE" >> "$PERF_FILE"
                tail -n 1 "$OUTPUT_FILE"
                echo ""
            else
                percent=$(( (i * 100) / PERF_RUNS ))
                printf "\r🔄 Running %d/%d (%d%% complete)" "$i" "$PERF_RUNS" "$percent"
                tail -n 1 "$OUTPUT_FILE" >> "$PERF_FILE"
            fi
        done
        printf "\n"

        echo "✅ Perf stat output saved to $PERF_FILE"
        echo "######################################################"
    fi
else
    echo "Invalid selection."
fi
