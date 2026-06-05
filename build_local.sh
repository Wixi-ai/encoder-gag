#!/bin/bash

source ~/conan-env/bin/activate

# Подменяем на Windows версию
cp conanfile_win.txt conanfile.txt

# Сборка
rm -rf build
conan install . --build=missing
cmake --preset conan-release
cmake --build --preset conan-release

# Возвращаем глобальную версию
git checkout conanfile.txt

echo ""
echo "========================================="
echo "  Сборка завершена!"
echo "  Запусти: ./build/Release/encoder_project.exe"
echo "========================================="
