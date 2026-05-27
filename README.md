


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
| GET | `/api/v1/records/{id}` | Получение записи по ID |
| DELETE | `/api/v1/records/{id}` | Удаление записи по ID |
| GET | `/health` | Проверка состояния сервера |

## 🚀 Быстрые скрипты для тестирования

В папке `scripts/` находятся удобные скрипты для проверки API:

```bash
# Проверка здоровья сервера
./scripts/health.sh

# Создание записи (UUID сгенерируется автоматически)
./scripts/create.sh

# Создание записи с указанным UUID и путём
./scripts/create.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee" "/path/to/video.mp4"

# Получение всех записей (пагинация: limit, offset)
./scripts/get.sh 10 0

# Получение записей с сортировкой
./scripts/get_sorted.sh created_at desc 10 0

# Получение записи по ID
./scripts/get_by_id.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"

# Удаление записи по ID
./scripts/delete.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"

# Тестирование обработки ошибок
./scripts/test_errors.sh
```
## 🔍 Фильтрация записей

GET /api/v1/records поддерживает следующие параметры фильтрации:

| Параметр | Описание | Пример |
|----------|----------|--------|
| `codec` | Фильтр по кодеку | `?codec=h264` |
| `from_date` | Записи после указанной даты | `?from_date=2026-05-01` |
| `to_date` | Записи до указанной даты | `?to_date=2026-05-31` |
| `file_path` | Частичное совпадение пути | `?file_path=video` |

### Примеры запросов с фильтрацией:

```bash
# Только кодек h264
./scripts/get_sorted.sh created_at asc 10 0 "codec=h264"

# Записи после 1 мая 2026
./scripts/get_sorted.sh created_at asc 10 0 "from_date=2026-05-01"

# Комбинация фильтров
./scripts/get_sorted.sh created_at asc 5 0 "codec=h264&from_date=2026-05-01"
```
## 🧪 Проверка работы сервера

После запуска сервера откройте **новое окно терминала** (Git Bash) и выполните:

### 1. Проверка здоровья
```bash
# Проверка здоровья
curl --noproxy "localhost" http://localhost:8080/health

# Создание записи с UUID
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records \
  -H "Content-Type: application/json" \
  -d '{"id":"aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee","block_size":10,"fblock":"test","streams":[]}'

# Получение всех записей
curl --noproxy "localhost" http://localhost:8080/api/v1/records

# Получение записи по ID
curl --noproxy "localhost" http://localhost:8080/api/v1/records/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee

# Удаление записи по ID
curl --noproxy "localhost" -X DELETE http://localhost:8080/api/v1/records/aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee
```

## 📁 Структура проекта
```

encoder_project/
├── include/                          # Заголовочные файлы (.hpp)
│   ├── colors.hpp                    # ANSI цвета для консоли
│   ├── utils.hpp                     # Утилиты: current_time(), generate_uuid(), is_valid_uuid()
│   ├── database.hpp                  # Работа с SQLite3 (объявление методов)
│   ├── messages.hpp                  # Структуры сообщений между агентами
│   ├── logger.hpp                    # Логирование в файл (класс Logger)
│   ├── constants.hpp                 # Константы: DEFAULT_LIMIT, MAX_LIMIT, TIMEOUT_SECONDS
│   └── agents/                       # Агенты (объявления)
│       ├── db_agent.hpp              # DB агент — работа с базой данных
│       └── http_agent.hpp            # HTTP агент — обработка запросов
│
├── src/                              # Реализация (.cpp)
│   ├── main.cpp                      # Точка входа, инициализация, graceful shutdown
│   ├── database.cpp                  # Реализация методов Database (SQLite3)
│   └── agents/                       # Агенты (реализация)
│       ├── db_agent.cpp              # DB агент — сохранение, чтение, удаление записей
│       ├── http_agent.cpp            # HTTP агент — основная логика (конструктор, подписки)
│       ├── http_handlers.cpp         # Обработчики маршрутов (POST, GET, DELETE)
│       └── http_print.cpp            # Функции вывода (баннер, рамки, команды)
│
├── scripts/                          # Bash скрипты для тестирования API
│   ├── health.sh                     # Проверка здоровья сервера (GET /health)
│   ├── create.sh                     # Создание записи (POST /api/v1/records)
│   ├── get.sh                        # Получение всех записей с пагинацией
│   ├── get_sorted.sh                 # Получение записей с сортировкой и фильтрацией
│   ├── get_by_id.sh                  # Получение записи по ID
│   ├── delete.sh                     # Удаление записи по ID
│   └── test_errors.sh                # Тестирование обработки ошибок (400, 404, 409)
│
├── data/                             # Папка для данных БД (создаётся автоматически)
│
├── Dockerfile                        # Docker образ для контейнеризации
├── docker-compose.yml                # Docker Compose для удобного запуска
│
├── CMakeLists.txt                    # Конфигурация сборки CMake
├── conanfile.txt                     # Зависимости Conan (cpp-httplib, nlohmann_json, sobjectizer, sqlite3)
│
├── build.sh                          # Скрипт сборки проекта
├── rebuild.sh                        # Полная пересборка с нуля
├── start.sh                          # Запуск сервера (сборка + запуск)
│
└── README.md                         # Документация проекта
```

## 📦 Зависимости

- cpp-httplib/0.14.3
- nlohmann_json/3.11.2
- sobjectizer/5.8.1
- sqlite3/3.45.1

## 🎨 Цветной вывод

Цвета работают в Git Bash, MSYS2, WSL, Linux terminal. В обычной cmd.exe цвета не поддерживаются.

## 🔄 Статус разработки

- [x]HTTP сервер
- [x]POST /api/v1/records
- [x]GET /api/v1/records
- [x]GET /api/v1/records/{id}
- [x]DELETE /api/v1/records/{id}
- [x]GET /health
- [x]Пагинация
- [x]Сортировка
- [x]Фильтрация
- [x]Валидация и обработка ошибок
- [x]Логирование в файл
- [x]Graceful shutdown
- [x]Docker контейнеризация
- [ ]ffmpeg_pool актор

## 📝 Примечание

Флаг --noproxy "localhost" необходим при работе через корпоративный прокси

ID записи должен быть в формате UUID (например, aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee)




## 🐳 Docker

### Сборка образа
```
./docker-build.sh
Запуск контейнера

./docker-run.sh
Запуск через docker-compose

./docker-compose-up.sh
Остановка

./docker-compose-down.sh
Просмотр логов

docker logs -f encoders_gag
Переменные окружения в Docker

docker run -d -p 9090:8080 \
  -e ENCODERS_GAG_PORT=8080 \
  -e ENCODERS_GAG_HOST=0.0.0.0 \
  -e ENCODERS_GAG_DB=/data/records.db \
  -v ./data:/data \
  --name encoders_gag encoders_gag:latest
```
