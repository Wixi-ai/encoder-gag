#pragma once

// Отключаем проверку Windows версии
#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#define CPPHTTPLIB_USE_POLL
#define CPPHTTPLIB_NO_EXCEPTIONS

#include <httplib.h>
