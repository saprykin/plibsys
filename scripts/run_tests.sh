#!/bin/sh

# Copyright 2017-2018, Alexander Saprykin <xelfium@gmail.com>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
#===========================================================================

if [[ $# -ne 2 ]]; then
    echo "Usage: run_tests.sh <testing_dir> <shared_library>"
    echo "Where: <testing_dir> is a directory containing tests to run"
    echo "       <shared_library> is a path to the shared library to be tested"
    exit 1
fi

total_counter=0
pass_counter=0

IFS=$'\n'
files=$(ls $1)

export LD_LIBRARY_PATH=$1:$LD_LIBRARY_PATH

selfname=$(basename $0)

echo "Running tests..."

for file in $files
do
    if [[ $file == *"_test"* && $file != $selfname ]]; then
        test_name=${file%.*}
        total_counter=$((total_counter + 1))
        echo "[RUN ] $test_name"

        if [[ $test_name == "plibraryloader_test" ]]; then
            $($1/${file} $2 > /dev/null 2>&1)
        else
            $($1/${file} > /dev/null 2>&1)
        fi

        if [[ $? -ne 0 ]]; then
            echo "[FAIL] *** Test failed: $test_name"
        else
            echo "[PASS] Test passed: $test_name"
            pass_counter=$((pass_counter + 1))
        fi
    fi
done

echo "Tests passed: $pass_counter/$total_counter"
