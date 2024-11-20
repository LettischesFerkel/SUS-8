@echo off
gcc main.c -Wall -pedantic --version -o build\main
.\build\main test