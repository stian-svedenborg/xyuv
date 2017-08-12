#!/usr/bin/env bash
# --------------------------------------------------------
# This script regenerates the minicalc scanner and parser.
# --------------------------------------------------------

LEMON=${LEMON:-lemon}
MAKEHEADERS=${MAKEHEADERS:-makeheaders}

set -e
# Generate Parser
${LEMON} -m parser.ypp
# Rename resulting file to C++.
mv parser.c parser.cpp
# Auto generate headerfile
${MAKEHEADERS} parser.cpp
# Rename header file to the convention of the remaining code.
mv parser.hpp parser.h
# Generate scanner.
re2c minicalc.re > minicalc.cpp
# Uncomment this line if you want to test the generated parser.
#g++ -std=c++11 -DCOMPILE_TEST_PROGRAM -g  -o test minicalc.cpp parser.cpp ast.cpp operations.cpp
echo "Done"
