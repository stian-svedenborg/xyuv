#!/usr/bin/env bash
# --------------------------------------------------------
# This script regenerates the minicalc scanner and parser.
# --------------------------------------------------------

set -e
# Generate Parser
lemon -m parser.ypp
# Rename resulting file to C++.
mv parser.c parser.cpp
# Auto generate headerfile
makeheaders parser.cpp
# Rename header file to the convention of the remaining code.
mv parser.hpp parser.h
# Generate scanner.
re2c minicalc.re > minicalc.cpp
# Uncomment this line if you want to test the generated parser.
#g++ -std=c++11 -DCOMPILE_TEST_PROGRAM -g  -o test minicalc.cpp parser.cpp ast.cpp operations.cpp
echo "Done"