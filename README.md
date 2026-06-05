Понял, обновлю зависимости в README.md под текущую структуру.



# encoders_gag

Сервер для модуля архива проекта Ригель. Реализует API для работы с записями и VAA блоками.

## 📋 Требования

- CMake 3.20+
- Conan 2.x
- MinGW (или Visual Studio)
- Git Bash / MSYS2 / Linux terminal (для цветного вывода)
- FFmpeg (для анализа видео)

## Установка на другом компьютере

### Требования

- Windows 10/11
- MSYS2 с установленными mingw64, gcc, cmake, make
- Git Bash (для скриптов)
- Python 3.8+ и pip

### Шаг 1. Установка MSYS2

Скачайте msys2 с официального сайта. Запустите MSYS2 UCRT64 и выполните:

```
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake make
```

### Шаг 2. Установка Python и Conan

```
pacman -S mingw-w64-ucrt-x86_64-python mingw-w64-ucrt-x86_64-python-pip
python -m venv ~/conan-env
source ~/conan-env/bin/activate
pip install conan
```

### Шаг 3. Установка FFmpeg

Скачайте сборку с https://www.gyan.dev/ffmpeg/builds/ (выберите essentials). Распакуйте в C:/ffmpeg. Добавьте C:/ffmpeg/bin в PATH системной переменной.

### Шаг 4. Клонирование и сборка

```
git clone https://gitlab.rigel.bolid.ru/rigel/services/archive/encoders_gag.git
cd encoders_gag
source ~/conan-env/bin/activate
./build_local.sh
./build/Release/encoder_project.exe
```

### Нюансы

- Запускайте сервер только в MSYS2 UCRT64 или Git Bash, иначе цветной вывод не работает.
- Пути к видео в запросах указывайте в Windows формате C:/folder/video.mp4
- Если ffprobe не найден, проверьте PATH или укажите путь в коде вручную.
- База данных records.db создаётся в папке с сервером. Для переноса на другой компьютер скопируйте её вместе с бинарником.
- При ошибке сборки удалите папки build и CMakeFiles и повторите шаг 4.

## 🔧 API Эндпоинты

| Метод | Эндпоинт | Описание |
|-------|----------|----------|
| POST | /api/v1/records | Создание новой записи |
| GET | /api/v1/records | Получение списка записей |
| GET | /api/v1/records/{id} | Получение записи по ID |
| DELETE | /api/v1/records/{id} | Удаление записи по ID |
| GET | /api/v1/archive/records/{id}/vva_blocks | Получение VAA блоков |
| GET | /health | Проверка состояния сервера |

## 🚀 Быстрые скрипты для тестирования

В папке `scripts/` находятся удобные скрипты для проверки API:

```
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

# Получение VAA блоков
./scripts/get_vaa.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"

# Удаление записи по ID
./scripts/delete.sh "aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee"

# Тестирование обработки ошибок
./scripts/test_errors.sh
```

## 🔍 Фильтрация записей

GET /api/v1/records поддерживает следующие параметры фильтрации:

| Параметр | Описание | Пример |
|----------|----------|--------|
| codec | Фильтр по кодеку | ?codec=h264 |
| from_date | Записи после указанной даты | ?from_date=2026-05-01 |
| to_date | Записи до указанной даты | ?to_date=2026-05-31 |
| file_path | Частичное совпадение пути | ?file_path=video |

## 📁 Структура проекта

```
encoder_project/
├── http/                     # HTTP компонент
│   ├── include/http/         # публичные заголовки
│   └── sources/              # исходники
├── db/                       # DB агент компонент
│   ├── include/db/
│   └── sources/
├── ffmpeg/                   # FFmpeg агент компонент
│   ├── include/ffmpeg/
│   └── sources/
├── database/                 # База данных (PImpl)
│   ├── include/database/     # Database (публичный)
│   └── sources/              # Database + DatabaseImpl
├── utils/                    # Утилиты
│   └── include/utils/        # только заголовки
├── messages/                 # Сообщения
│   └── include/messages/
├── encoder/                  # Точка входа
│   └── sources/              # main.cpp
├── tests/                    # Юнит-тесты
├── scripts/                  # Bash скрипты
├── CMakeLists.txt
├── conanfile.txt             # для CI (новые версии)
├── conanfile_win.txt         # для Windows (старые версии)
├── build_local.sh            # локальная сборка
└── README.md
```

## 📦 Зависимости

### Для CI (Linux)

```
cpp-httplib/0.39.0
nlohmann_json/3.12.0
sobjectizer/5.8.1
sqlite3/3.53.0
gtest/1.17.0
```

### Для локальной сборки (Windows)

```
cpp-httplib/0.14.3
nlohmann_json/3.11.2
sobjectizer/5.7.5
sqlite3/3.45.1
gtest/1.17.0
```

### Системные

- FFmpeg (ffprobe) — для анализа видео

## 🎨 Цветной вывод

Цвета работают в Git Bash, MSYS2, WSL, Linux terminal.

## 🔄 Статус разработки

- [x] HTTP сервер
- [x] POST /api/v1/records
- [x] GET /api/v1/records
- [x] GET /api/v1/records/{id}
- [x] DELETE /api/v1/records/{id}
- [x] GET /health
- [x] GET /api/v1/archive/records/{id}/vva_blocks
- [x] Пагинация
- [x] Сортировка
- [x] Фильтрация
- [x] Валидация и обработка ошибок
- [x] Логирование в файл
- [x] Graceful shutdown
- [x] Docker контейнеризация
- [x] FFmpeg агент (реальный анализ видео через ffprobe)
- [x] VAA блоки (сегментация видео и аудио по 10 секунд)
- [x] Проверка существования файла (400)
- [x] Индексы в БД
- [x] Кеширование
- [x] Rate limiter
- [x] PImpl для Database
- [x] Юнит-тесты (GTest)
- [x] Компонентная структура CMake

## VAA блоки

При создании записи FFmpeg агент генерирует VAA блоки:

- Видео нарезается на сегменты по 10 секунд
- Аудио нарезается на сегменты по 10 секунд
- Блоки сохраняются в таблицу vaa_blocks

### Получение VAA блоков

```
./scripts/get_vaa.sh 04cc951e-1c0f-4838-bc37-3d9d1c71b505
```

### Пример ответа

```
{
  "record_id": "04cc951e-1c0f-4838-bc37-3d9d1c71b505",
  "total": 6,
  "blocks": [
    {"index": 0, "type": "video", "pts": 0, "duration": 10000},
    {"index": 1, "type": "video", "pts": 10000, "duration": 10000},
    {"index": 2, "type": "video", "pts": 20000, "duration": 10000},
    {"index": 3, "type": "audio", "pts": 0, "duration": 10000},
    {"index": 4, "type": "audio", "pts": 10000, "duration": 10000},
    {"index": 5, "type": "audio", "pts": 20000, "duration": 10000}
  ]
}
```

## Кеширование

Для ускорения работы используется кеширование:

| Эндпоинт | Кеш | TTL | Инвалидация |
|----------|-----|-----|-------------|
| GET /api/v1/records | m_records_cache | 60 сек | POST/DELETE |
| GET /api/v1/records/{id} | m_record_cache | 60 сек | POST/DELETE |
| GET /api/v1/archive/records/{id}/vva_blocks | m_vaa_cache | бессрочно | POST/DELETE |

## 🏗 Как работает проект

### Архитектура

Проект построен на асинхронной модели акторов с использованием фреймворка SObjectizer. В системе работают три независимых агента:

1. **HTTP-агент** — запускает веб-сервер на порту 8080, принимает REST-запросы, валидирует данные, проверяет существование файла, отправляет сообщения другим агентам
2. **FFmpeg-агент** — выполняет реальный анализ видео через ffprobe (определяет codec, разрешение, длительность), генерирует VAA блоки
3. **DB-агент** — получает сообщения от HTTP и FFmpeg агентов, выполняет операции с SQLite3 (сохранение записей, VAA блоков, чтение, удаление)

### Схема работы

```
Client -> HTTP Agent -> FFmpeg Agent -> DB Agent -> SQLite3
              |             |              |
          Response     Video params    Save record
```

### Полный цикл создания записи

1. Пользователь отправляет POST запрос на /api/v1/records с JSON-данными
2. HTTP-агент валидирует UUID, block_size, fblock, streams
3. HTTP-агент проверяет существование видеофайла (если нет — возвращает 400)
4. HTTP-агент отправляет FFmpeg-агенту сообщение на анализ видео
5. FFmpeg-агент через ffprobe определяет параметры видео (codec, ширина, высота, длительность)
6. FFmpeg-агент генерирует VAA блоки (сегменты по 10 секунд)
7. FFmpeg-агент отправляет DB-агенту сообщение на сохранение VAA блоков
8. HTTP-агент отправляет DB-агенту сообщение на сохранение записи
9. DB-агент сохраняет запись и VAA блоки в SQLite3
10. HTTP-агент возвращает клиенту статус 201 Created

### Обработка ошибок

| Статус | Ситуация |
|--------|----------|
| 400 | Неверный формат UUID, пустое тело, невалидный JSON, block_size ≤ 0, видеофайл не существует |
| 404 | Запись с указанным ID не найдена |
| 409 | Попытка создать запись с уже существующим ID |
| 429 | Слишком много запросов (rate limiter) |
| 504 | Таймаут ожидания ответа от агента |

### Технологический стек

- **C++17** — язык программирования
- **CMake + Conan** — сборка и управление зависимостями
- **SObjectizer** — асинхронный фреймворк для обмена сообщениями
- **cpp-httplib** — HTTP-сервер
- **nlohmann/json** — парсинг и сериализация JSON
- **SQLite3** — база данных
- **FFmpeg** — анализ видео (ffprobe)
- **GTest** — юнит-тесты

## 🐳 Docker

### Сборка образа
```
./docker-build.sh
```

### Запуск контейнера
```
./docker-run.sh
```

### Запуск через docker-compose
```
./docker-compose-up.sh
```

### Остановка
```
./docker-compose-down.sh
```

## 📝 Примечание

- Флаг `--noproxy "localhost"` необходим при работе через корпоративный прокси
- ID записи должен быть в формате UUID (например, `aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee`)
- FFmpeg должен быть установлен и доступен в PATH (для работы ffprobe)
EOF
```
