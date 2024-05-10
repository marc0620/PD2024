#!/bin/bash

# Check if the correct number of arguments are provided
#if [ "$#" -ne 3 ]; then
#    echo "Usage: $0 <alpha_value> <input_name> <evaluator_path>"
#    exit 1
#fi

# Assigning arguments to variables
alpha_value="$1"
input="$2"
inputs=("ami33" "ami49" "apte" "hp" "xerox")

if ! [[ " ${inputs[@]} " =~ " ${input} " ]]; then
    echo "Invalid input name. Available inputs are: ${inputs[@]}"
    exit 1
fi
evaluator_path="../evaluator/evaluator.sh"

# Run the program with the specified inputs
make
./bin/fp "$alpha_value" "../input_pa2/$input.block" "../input_pa2/$input.nets" "../output/result"
