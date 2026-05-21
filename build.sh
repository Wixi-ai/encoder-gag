#!/bin/bash

echo "========================================="
echo "  Сборка encoders_gag"
echo "========================================="

conan install . --output-folder=build --build=missing

cmake -S . -B build -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    

cmake --build build

echo "========================================="
echo "  Сборка завершена!"
echo "  Запустите: ./build/encoder_project.exe"
echo "========================================="
