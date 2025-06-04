#!/bin/bash

TARGET_DIR="./target"
PERF_DIR="./perf"

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
    echo "  --perf [runs] [args]                    Run the program under perf stat with the provided arguments."
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
c_files=($(find "$TARGET_DIR" -maxdepth 1 -type f -name '*'))

IFS=$'\n' sorted_files=($(printf "%s\n" "${c_files[@]}" | sort))
unset IFS

echo "Select a target file to run:"
for i in "${!sorted_files[@]}"; do
    printf "%d) %s\n" "$i" "$(basename "${sorted_files[$i]}")"
done

read -rp "Enter the number corresponding to the file: " index
file_path="${sorted_files[$index]}"
if [[ -n "$file_path" ]]; then
    target=$(basename "$file_path")
    binary="$TARGET_DIR/$target"
    OUTPUT_FILE="$PERF_DIR/${target}.out"
    
    read -r FIRST_DIM SECOND_DIM <<< "$ARGS"
    INPUT="${FIRST_DIM}x${SECOND_DIM}"
    FINAL_PERF_DIR="$PERF_DIR/$INPUT"
    PERF_FILE="$FINAL_PERF_DIR/${target}.data"
    mkdir -p "$FINAL_PERF_DIR"
    echo "Selected: $target"

    if [ "$RUN_PERF" = true ]; then
        echo "######################## PERF ########################"

        if [ ! -x "$binary" ]; then
            echo "Binary not found."
            exit 1
        fi

        echo "Running $PERF_RUNS time(s) with arguments: $ARGS"
        
        > "$PERF_FILE"  # Clear old data
        for ((i = 1; i <= PERF_RUNS; i++)); do

            {
                echo ""
                echo "### Run $i on $(date) with args: $ARGS"
                "$binary" $ARGS > "$OUTPUT_FILE"
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

        echo "✅ Output saved to $PERF_FILE"
        echo "######################################################"
    fi
else
    echo "Invalid selection."
fi
