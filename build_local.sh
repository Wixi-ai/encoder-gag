#!/bin/bash

source ~/conan-env/bin/activate
cp conanfile_win.txt conanfile.txt
rm -rf build
conan install . --build=missing
cmake --preset conan-release
cmake --build --preset conan-release
git checkout conanfile.txt

echo ""
echo "========================================="
echo "  Сборка завершена!"
echo "  Запусти: ./build/Release/encoder_project.exe"
echo "========================================="
