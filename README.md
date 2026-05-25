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
3. Запуск вручную
bash
./build/encoder_project.exe
🔧 API Эндпоинты
Метод	Эндпоинт	Описание
POST	/api/v1/records	Создание новой записи
GET	/api/v1/records	Получение списка записей
GET	/health	Проверка состояния сервера
🧪 Проверка работы сервера
После запуска сервера откройте новое окно терминала и выполните:

bash
# Проверка здоровья
curl --noproxy "localhost" http://localhost:8080/health

# Создание записи
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d '{"test":"data"}'

# Получение всех записей
curl --noproxy "localhost" http://localhost:8080/api/v1/records
📁 Структура проекта
text
encoder_project/
├── src/
│   ├── main.cpp
│   ├── colors.hpp
│   ├── database.hpp
│   ├── utils.hpp
│   └── agents/
│       ├── db_agent.hpp
│       └── http_agent.hpp
├── messages.hpp
├── CMakeLists.txt
├── conanfile.txt
├── logs.sh
└── README.md
📦 Зависимости
Все зависимости устанавливаются автоматически через Conan:

cpp-httplib/0.14.3

nlohmann_json/3.11.2

sobjectizer/5.8.1

sqlite3/3.45.1

🎨 Цветной вывод
Цвета работают в Git Bash, MSYS2, WSL, Linux terminal. В обычной cmd.exe цвета не поддерживаются.

🔄 Статус разработки
HTTP сервер

POST /api/v1/records

GET /api/v1/records (реальные данные из БД)

SQLite3 сохранение

Цветной вывод

GET /api/v1/records/{id}

DELETE /api/v1/records/{id}

ffmpeg_pool актор

📝 Примечание
Флаг --noproxy "localhost" необходим при работе через корпоративный прокси.

📝 Лицензия
© 2026 Rigel. Все права защищены.

text

---

**Как вставить:**
1. Открой VS Code
2. Открой файл `README.md`
3. Выдели всё (Ctrl+A)
4. Вставь этот текст (Ctrl+V)
5. Сохрани (Ctrl+S)

Или в Git Bash выполни:

```bash
echo '# encoders_gag

Сервер-заглушка для модуля архива проекта Ригель. Имитирует работу сервисов msm и controller для encoder.

## Требования

- CMake 3.20+
- Conan 2.x
- MinGW
- Git Bash / MSYS2

## Быстрый старт

```bash
git clone <репозиторий>
cd encoders_gag
./logs.sh
API Эндпоинты
Метод	Эндпоинт	Описание
POST	/api/v1/records	Создание записи
GET	/api/v1/records	Список записей
GET	/health	Проверка здоровья
Проверка работы
bash
curl --noproxy "localhost" http://localhost:8080/health
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d '"'"'{"test":"data"}'"'"'
curl --noproxy "localhost" http://localhost:8080/api/v1/records
Статус разработки
POST /api/v1/records

GET /api/v1/records

SQLite3

GET /api/v1/records/{id}

DELETE /api/v1/records/{id}' > README.md

text

Но первый вариант (через VS Code) проще.
