#pragma once

// Форсируем старые Windows API для совместимости с cpp-httplib 0.39.0
#define _WIN32_WINNT 0x0601
#define WINVER 0x0601
#define NTDDI_VERSION 0x06010000

// Отключаем использование CreateFile2 и GetAddrInfoExCancel
#define CPPHTTPLIB_USE_POLL

#include <httplib.h>
