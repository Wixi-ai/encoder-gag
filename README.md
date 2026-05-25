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
2. Сборка проекта
bash
./logs.sh
Скрипт автоматически:

Установит зависимости через Conan

Сконфигурирует CMake

Соберёт проект

Запустит сервер

3. Запуск вручную
bash
./build/encoder_project.exe
🔧 API Эндпоинты
Метод	Эндпоинт	Описание
POST	/api/v1/records	Создание новой записи
GET	/api/v1/records	Получение списка записей
GET	/health	Проверка состояния сервера
Пример запроса
bash
# Создание записи
curl -X POST http://localhost:8080/api/v1/records \
  -H "Content-Type: application/json" \
  -d '{"test": "data"}'

# Получение всех записей
curl http://localhost:8080/api/v1/records

# Проверка здоровья
curl http://localhost:8080/health
📁 Структура проекта
text
encoder_project/
├── src/
│   ├── main.cpp                 # Точка входа
│   ├── colors.hpp               # Цвета для консоли
│   ├── database.hpp             # Работа с SQLite3
│   ├── utils.hpp                # Вспомогательные функции
│   └── agents/
│       ├── db_agent.hpp         # DB агент (сохранение записей)
│       └── http_agent.hpp       # HTTP агент (сервер)
├── messages.hpp                 # Сообщения между агентами
├── CMakeLists.txt               # Конфигурация сборки
├── conanfile.txt                # Зависимости
├── logs.sh                      # Скрипт сборки и запуска
└── README.md                    # Этот файл
🎨 Цветной вывод
Цвета работают в:

✅ Git Bash

✅ MSYS2

✅ WSL

✅ Linux / macOS terminal

В обычной cmd.exe цвета не поддерживаются (рекомендуется использовать Git Bash).

📦 Зависимости
Все зависимости устанавливаются автоматически через Conan:

cpp-httplib/0.14.3

nlohmann_json/3.11.2

sobjectizer/5.8.1

sqlite3/3.45.1

🧪 Проверка работы сервера
После запуска сервера (./logs.sh) откройте новое окно терминала (Git Bash) и выполните команды:

1. Проверка здоровья сервера
bash
curl --noproxy "localhost" http://localhost:8080/health
Ожидаемый ответ: OK

2. Создание записи (POST)
bash
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records \
  -H "Content-Type: application/json" \
  -d '{"test": "data"}'
Ожидаемый ответ: {"status":"created","id":"сгенерированный-uuid"}

3. Получение всех записей (GET)
bash
curl --noproxy "localhost" http://localhost:8080/api/v1/records
Ожидаемый ответ: JSON-массив с созданными записями

4. Просмотр базы данных (опционально)
bash
sqlite3 records.db "SELECT * FROM records;"
📝 Примечание
Флаг --noproxy "localhost" необходим при работе через корпоративный прокси, чтобы запросы к локальному серверу не уходили через прокси.

🐛 Возможные проблемы
Conan не найден
Установите Conan:

bash
pip install conan
Компилятор не найден
Установите MinGW через MSYS2:

bash
pacman -S mingw-w64-ucrt-x86_64-gcc
Цвета не работают
Используйте Git Bash вместо cmd.exe:

Откройте Git Bash

Перейдите в папку проекта

Запустите ./logs.sh

🔄 Статус разработки
HTTP сервер

POST /api/v1/records

GET /api/v1/records (реальные данные из БД)

SQLite3 сохранение

Цветной вывод

GET /api/v1/records/{id}

DELETE /api/v1/records/{id}

ffmpeg_pool актор
