#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}=========================================${NC}"
echo -e "${CYAN}  Сборка encoders_gag с логами${NC}"
echo -e "${CYAN}=========================================${NC}"

# 1. Проверка зависимостей
echo -e "${YELLOW}[1/4] Проверка зависимостей...${NC}"
if command -v conan &> /dev/null; then
    conan install . --output-folder=build --build=missing 2>&1
else
    echo -e "${YELLOW}  ⚠ Conan не найден, пропускаем...${NC}"
fi

# 2. Создаём папку build если её нет
mkdir -p build

# 3. CMake конфигурация
echo -e "${YELLOW}[2/4] Конфигурация CMake...${NC}"
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}  ✗ CMake конфигурация failed${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ CMake конфигурация успешна${NC}"

# 4. Сборка
echo -e "${YELLOW}[3/4] Компиляция проекта...${NC}"
cmake --build . --config Release 2>&1

if [ $? -ne 0 ]; then
    echo -e "${RED}  ✗ Компиляция failed${NC}"
    exit 1
fi
echo -e "${GREEN}  ✓ Компиляция успешна${NC}"

# 5. Запуск
cd ..
echo -e "${CYAN}=========================================${NC}"
echo -e "${GREEN}  Сборка завершена!${NC}"
echo -e "${CYAN}=========================================${NC}"
echo -e "${YELLOW}  Запуск программы...${NC}"
echo -e "${CYAN}=========================================${NC}"
./build/encoder_project.exe
