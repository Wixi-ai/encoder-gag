


# encoders_gag

Сервер-заглушка для модуля архива проекта Ригель. Имитирует работу сервисов msm и controller для encoder.

## 📋 Требования

- CMake 3.20+
- Conan 2.x
- MinGW (или Visual Studio)
- Git Bash / MSYS2 / Linux terminal (для цветного вывода)

## 🚀 Быстрый старт

### 1. Клонирование репозитория

```bash
git clone https://gitlab.rigel.bolid.ru/rigel/services/archive/encoders_gag.git
cd encoders_gag
```

### 2. Сборка проекта

```bash
./Start.sh
```

Скрипт автоматически:
- Установит зависимости через Conan
- Сконфигурирует CMake
- Соберёт проект
- Запустит сервер

### 3. Запуск вручную

```bash
./build/encoder_project.exe
```

## 🔧 API Эндпоинты

| Метод | Эндпоинт | Описание |
|-------|----------|----------|
| POST | `/api/v1/records` | Создание новой записи |
| GET | `/api/v1/records` | Получение списка записей |
| GET | `/health` | Проверка состояния сервера |

## 🚀 Быстрые скрипты для тестирования

В папке `scripts/` находятся удобные скрипты для проверки API:

```bash
# Проверка здоровья сервера
./scripts/health.sh

# Создание новой записи
./scripts/create.sh

# Получение всех записей
./scripts/get.sh
```

## 🧪 Проверка работы сервера

После запуска сервера откройте **новое окно терминала** (Git Bash) и выполните:

### 1. Проверка здоровья
```bash
curl --noproxy "localhost" http://localhost:8080/health
```
**Ожидаемый ответ:** `OK`

### 2. Создание записи
```bash
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d '{"test":"data"}'
```
**Ожидаемый ответ:** `{"status":"created","id":"..."}`

### 3. Получение всех записей
```bash
curl --noproxy "localhost" http://localhost:8080/api/v1/records
```
**Ожидаемый ответ:** JSON-массив с записями

## 📁 Структура проекта

```
encoder_project/
├── include/ # Заголовочные файлы
│ ├── colors.hpp # Цвета для консоли
│ ├── utils.hpp # Утилиты (UUID, время)
│ ├── database.hpp # Работа с SQLite3 (объявление)
│ ├── messages.hpp # Сообщения между агентами
│ └── agents/
│ ├── db_agent.hpp # DB агент (объявление)
│ └── http_agent.hpp # HTTP агент (объявление)
├── src/ # Реализация
│ ├── main.cpp # Точка входа
│ ├── database.cpp # Реализация SQLite3
│ └── agents/
│ ├── db_agent.cpp # Реализация DB агента
│ └── http_agent.cpp # Реализация HTTP агента
├── scripts/ # Скрипты для тестирования
│ ├── health.sh
│ ├── create.sh
│ └── get.sh
├── CMakeLists.txt
├── conanfile.txt
├── start.sh
├── build.sh
├── rebuild.sh
└── README.md
```

## 📦 Зависимости

- cpp-httplib/0.14.3
- nlohmann_json/3.11.2
- sobjectizer/5.8.1
- sqlite3/3.45.1

## 🎨 Цветной вывод

Цвета работают в Git Bash, MSYS2, WSL, Linux terminal. В обычной cmd.exe цвета не поддерживаются.

## 🔄 Статус разработки

- [x] HTTP сервер
- [x] POST /api/v1/records
- [x] GET /api/v1/records (реальные данные из БД)
- [x] SQLite3 сохранение
- [x] Цветной вывод
- [ ] GET /api/v1/records/{id}
- [ ] DELETE /api/v1/records/{id}
- [ ] ffmpeg_pool актор

## 📝 Примечание

Флаг `--noproxy "localhost"` необходим при работе через корпоративный прокси.

## 📝 Лицензия

© 2026 Rigel. Все права защищены.
```

Сохрани и закоммить:

```bash
git add README.md
git commit -m "fix: restore README"
git push gitlab feature/init
```
