#!/bin/bash
# 1. Установка библиотек
conan install . --output-folder=build --build=missing

# 2. Настройка (теперь всё в одной команде)
cmake -S . -B build -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="build/build/Release/generators/conan_toolchain.cmake"

# 3. Сборка
cmake --build build
