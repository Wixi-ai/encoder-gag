#!/bin/bash

echo "========================================="
echo "  Сборка encoders_gag"
echo "========================================="

# Устанавливаем зависимости
conan install . --output-folder=build --build=missing

# Переходим в build
cd build

# Конфигурируем CMake с правильным путём к toolchain
cmake .. -G "MinGW Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="build/Release/generators/conan_toolchain.cmake"

# Собираем
cmake --build . --config Release

echo "========================================="
echo "  Сборка завершена!"
echo "  Запустите: ./build/encoder_project.exe"
echo "========================================="
