#pragma once

// ANSI цветовые коды для консоли
#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

// Фоновые цвета
#define BG_RED    "\033[41m"
#define BG_GREEN  "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE   "\033[44m"

// Стили текста
#define BOLD      "\033[1m"
#define DIM       "\033[2m"
#define UNDERLINE "\033[4m"

// Цвета для компонентов проекта
#define COLOR_HTTP   COLOR_CYAN BOLD
#define COLOR_DB     COLOR_GREEN BOLD
#define COLOR_MAIN   COLOR_MAGENTA BOLD
#define COLOR_DB_COM COLOR_BLUE

// HTTP статусы
enum class HttpStatus {
    OK = 200,
    CREATED = 201,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    CONFLICT = 409,
    INTERNAL_SERVER_ERROR = 500,
    GATEWAY_TIMEOUT = 504
};
