#!/bin/bash

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Добавляем пути к Conan
export PATH="/c/Users/tungiia/.local/bin:$PATH"
export PATH="/c/Python310/Scripts:$PATH"
export PATH="/c/Python39/Scripts:$PATH"
export PATH="/opt/conan/bin:$PATH"

echo -e "${CYAN}=========================================${NC}"
echo -e "${CYAN}  Сборка encoders_gag с логами${NC}"
echo -e "${CYAN}=========================================${NC}"

# 1. Проверка Conan и установка зависимостей (только если есть изменения)
echo -e "${YELLOW}[1/4] Проверка зависимостей...${NC}"
if command -v conan &> /dev/null; then
    echo -e "${GREEN}  Conan найден${NC}"
    if conan install . --output-folder=build --build=missing 2>&1; then
        echo -e "${GREEN}  ✓ Conan install успешен${NC}"
    else
        echo -e "${YELLOW}  ⚠ Conan install пропущен (возможно, уже установлено)${NC}"
    fi
else
    echo -e "${YELLOW}  ⚠ Conan не найден, пропускаем установку зависимостей${NC}"
    echo -e "${YELLOW}  (библиотеки уже должны быть установлены в build/)${NC}"
fi

# 2. CMake конфигурация
echo -e "${YELLOW}[2/4] Конфигурация CMake...${NC}"
cd build
if cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release 2>&1; then
    echo -e "${GREEN}  ✓ CMake конфигурация успешна${NC}"
else
    echo -e "${RED}  ✗ CMake конфигурация failed${NC}"
    exit 1
fi

# 3. Сборка
echo -e "${YELLOW}[3/4] Компиляция проекта...${NC}"
if cmake --build . --config Release 2>&1; then
    echo -e "${GREEN}  ✓ Компиляция успешна${NC}"
else
    echo -e "${RED}  ✗ Компиляция failed${NC}"
    exit 1
fi

# 4. Запуск
cd ..
echo -e "${CYAN}=========================================${NC}"
echo -e "${GREEN}  Сборка завершена!${NC}"
echo -e "${CYAN}=========================================${NC}"
echo -e "${BLUE}  Запуск программы...${NC}"
echo -e "${CYAN}=========================================${NC}"
./build/encoder_project.exe
