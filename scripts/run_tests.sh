#!/bin/sh

#
# The MIT License
#
# Copyright 2017-2018, Alexander Saprykin <saprykin.spb@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# 'Software'), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
