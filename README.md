


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
| GET | `/api/v1/records/{id}` | Получение списка записей |
| DELETE | `/api/v1/records/{id}` | Получение списка записей |
| GET | `/health` | Проверка состояния сервера |

## 🚀 Быстрые скрипты для тестирования

В папке `scripts/` находятся удобные скрипты для проверки API:

```bash
# Проверка здоровья сервера
./scripts/health.sh

# Создание новой записи (укажите UUID)
./scripts/create.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"

# Получение всех записей
./scripts/get.sh

# Получение записи по ID
./scripts/get_by_id.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"

# Удаление записи по ID
./scripts/delete.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"
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
│
├── include/                                 # Заголовочные файлы (.hpp)
│   ├── colors.hpp                           # Цвета для консоли
│   ├── utils.hpp                            # Утилиты (UUID, время, валидация)
│   ├── database.hpp                         # Работа с SQLite3 (объявление)
│   ├── messages.hpp                         # Сообщения между агентами
│   ├── logger.hpp                           # Логирование в файл
│   ├── constants.hpp                        # Константы (лимиты, таймауты)
│   │
│   └── agents/                              # Агенты (объявления)
│       ├── db_agent.hpp
│       └── http_agent.hpp
│
├── src/                                     # Реализация (.cpp)
│   ├── main.cpp                             # Точка входа
│   ├── database.cpp                         # Реализация SQLite3
│   │
│   └── agents/                              # Агенты (реализация)
│       ├── db_agent.cpp
│       ├── http_agent.cpp                   # Основная логика HTTP агента
│       ├── http_handlers.cpp                # Обработчики маршрутов
│       └── http_print.cpp                   # Вывод в консоль (баннер, рамки)
│
├── scripts/                                 # Скрипты для тестирования API
│   ├── health.sh                            # Проверка здоровья
│   ├── create.sh                            # Создание записи
│   ├── get.sh                               # Получение всех записей
│   ├── get_sorted.sh                        # Получение с сортировкой
│   ├── get_by_id.sh                         # Получение записи по ID
│   └── delete.sh                            # Удаление записи
│
├── data/                                    # Папка для данных БД (создаётся автоматически)
│
├── CMakeLists.txt                           # Конфигурация сборки
├── conanfile.txt                            # Зависимости Conan
│
├── Dockerfile                               # Docker образ
├── docker-compose.yml                       # Docker Compose
├── .dockerignore                            # Что игнорировать в Docker
│
├── build.sh                                 # Скрипт сборки
├── rebuild.sh                               # Полная пересборка с нуля
├── start.sh                                 # Запуск сервера
│
└── README.md                                # Документация
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
- [x] GET /api/v1/records/{id}
- [x] DELETE /api/v1/records/{id}
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
