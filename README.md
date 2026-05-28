


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

- [x] HTTP сервер
- [x] POST /api/v1/records
- [x] GET /api/v1/records
- [x] GET /api/v1/records/{id}
- [x] DELETE /api/v1/records/{id}
- [x] GET /health
- [x] Пагинация
- [x] Сортировка
- [x] Фильтрация
- [x] Валидация и обработка ошибок
- [x] Логирование в файл
- [x] Graceful shutdown
- [x] Docker контейнеризация
- [ ] ffmpeg_pool актор

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

## 🏗 Как работает проект

### Архитектура

Проект построен на асинхронной модели акторов с использованием фреймворка SObjectizer. В системе работают два независимых актора:

1. **HTTP-агент** — запускает веб-сервер на порту 8080, принимает REST-запросы, валидирует данные, отправляет сообщения DB-агенту
2. **DB-агент** — получает сообщения от HTTP-агента, выполняет операции с SQLite3 базой данных

Акторы общаются через асинхронные сообщения, что позволяет разнести логику получения запросов и логику работы с диском.

### Полный цикл создания записи

1. Пользователь отправляет POST запрос на `/api/v1/records` с JSON-данными
2. HTTP-агент валидирует UUID, block_size, fblock, streams
3. Из streams извлекается codec первого потока
4. HTTP-агент отправляет DB-агенту сообщение `msg_create_record`
5. HTTP-агент ожидает ответа через promise/future (таймаут 5 секунд)
6. DB-агент сохраняет запись в SQLite3 (id, file_path, codec)
7. DB-агент отправляет ответ с результатом операции
8. HTTP-агент возвращает клиенту статус 201 Created или 409 Conflict

### Полный цикл получения всех записей

1. Пользователь отправляет GET запрос на `/api/v1/records?limit=10&offset=0&sort_by=created_at&sort_order=desc&codec=h264`
2. HTTP-агент парсит параметры пагинации, сортировки и фильтрации
3. HTTP-агент отправляет DB-агенту сообщение `msg_get_records` с этими параметрами
4. DB-агент формирует SQL-запрос с учетом фильтров, сортировки и пагинации
5. DB-агент выполняет запрос и получает общее количество записей
6. DB-агент отправляет ответ с массивом записей и мета-информацией
7. HTTP-агент формирует JSON и возвращает клиенту

### Полный цикл получения записи по ID

1. Пользователь отправляет GET запрос на `/api/v1/records/{id}`
2. HTTP-агент проверяет формат UUID (должен быть 36 символов, только hex и дефисы)
3. HTTP-агент отправляет DB-агенту сообщение `msg_get_record_by_id`
4. DB-агент ищет запись в БД по id
5. Если найдена — отправляет обратно, если нет — отправляет `found=false`
6. HTTP-агент возвращает 200 OK с данными или 404 Not Found

### Полный цикл удаления записи

1. Пользователь отправляет DELETE запрос на `/api/v1/records/{id}`
2. HTTP-агент проверяет формат UUID
3. HTTP-агент отправляет DB-агенту сообщение `msg_delete_record_by_id`
4. DB-агент удаляет запись из БД
5. DB-агент отправляет ответ с результатом
6. HTTP-агент возвращает 200 OK или 404 Not Found

### Обработка ошибок

| Статус | Ситуация |
|--------|----------|
| 400 | Неверный формат UUID, пустое тело запроса, невалидный JSON, отсутствие обязательных полей, block_size ≤ 0 |
| 404 | Запись с указанным ID не найдена |
| 409 | Попытка создать запись с уже существующим ID |
| 504 | Таймаут ожидания ответа от DB-агента (5 секунд) |

### Технологический стек

- **C++17** — язык программирования
- **CMake + Conan** — сборка и управление зависимостями
- **SObjectizer** — асинхронный фреймворк для обмена сообщениями между акторами
- **cpp-httplib** — HTTP-сервер
- **nlohmann/json** — парсинг и сериализация JSON
- **SQLite3** — база данных

### Дополнительные возможности

- **Graceful shutdown** — корректное завершение при нажатии Ctrl+C
- **Логирование в файл** — все действия записываются в `encoders_gag.log`
- **Конфигурация через переменные окружения** — порт, хост, путь к БД, файл лога
- **Цветной вывод в консоль** — разные цвета для HTTP, DB и MAIN компонентов
- **Docker контейнеризация** — запуск одной командой `docker-compose up -d`
