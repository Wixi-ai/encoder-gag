#!/bin/bash

echo -e "\033[0;34m=========================================\033[0m"
echo -e "\033[0;34m  Сборка encoders_gag\033[0m"
echo -e "\033[0;34m=========================================\033[0m"

conan install . --output-folder=build --build=missing
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake"
cmake --build . --config Release

echo -e "\033[0;32m=========================================\033[0m"
echo -e "\033[0;32m  Сборка завершена!\033[0m"
echo -e "\033[0;32m  Запустите: ./build/encoder_project.exe\033[0m"
echo -e "\033[0;32m=========================================\033[0m"
