#!/bin/bash

if type "clang" > /dev/null 2>&1; then
  clang *.c -Werror -O0 -g -o tinyosc
else
  gcc *.c -Werror -std=c99 -O0 -g -o tinyosc
fi
