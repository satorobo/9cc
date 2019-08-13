#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

try 0 "0;"
try 42 "42;"
try 21 "5+20-4;"
try 41 "12 + 34 - 5;"
try 66 "3 * 5 * 4 + 2 * 3;"
try 100 "10 * 10 / 2 + 20 * 3 - 10;"
try 15 "5*(9-6);"
try 4 "(3+5)/2;"
try 10 "-10 + 20;"
try 1 "2 < 3;"
try 1 "2+5 > 3+2;"
try 1 "2+5 >= 3+4;"
try 1 "3==3;"
try 1 "10 * 10 / 2 + 20 * 3 - 10 == 100;"
try 1 "10 * 10 / 2 + 20 * 3 - 10 != 10;"
try 1 "a = 3-2;"
try 9 "a = 3-2; b = 10; b-a;"
try 55 "a = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;"
try 1 "hoge = 3-2;"
try 9 "hoge = 3-2; foo = 10; foo-hoge;"

echo OK
