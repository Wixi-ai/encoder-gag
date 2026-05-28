#!/bin/bash
# Тестирование обработки ошибок

echo "========================================="
echo "  ТЕСТИРОВАНИЕ ОБРАБОТКИ ОШИБОК"
echo "========================================="

echo ""
echo "1. Неверный UUID (GET) - ожидается 400"
curl --noproxy "localhost" -s -o /dev/null -w "Статус: %{http_code}\n" "http://localhost:8080/api/v1/records/123"

echo ""
echo "2. Неверный UUID (DELETE) - ожидается 400"
curl --noproxy "localhost" -s -o /dev/null -w "Статус: %{http_code}\n" -X DELETE "http://localhost:8080/api/v1/records/123"

echo ""
echo "3. Пустое тело (POST) - ожидается 400"
curl --noproxy "localhost" -s -o /dev/null -w "Статус: %{http_code}\n" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d ""

echo ""
echo "4. Несуществующий UUID - ожидается 404"
curl --noproxy "localhost" -s -o /dev/null -w "Статус: %{http_code}\n" "http://localhost:8080/api/v1/records/00000000-0000-0000-0000-000000000000"

echo ""
echo "5. Дубликат записи - ожидается 409"
# Генерируем чистый UUID без префиксов
UUID=$(uuidgen | tr '[:upper:]' '[:lower:]')
echo "   Используем UUID: $UUID"
# Первое создание
curl --noproxy "localhost" -s -o /dev/null -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d "{\"id\":\"$UUID\",\"block_size\":10,\"fblock\":\"test\",\"streams\":[]}" 2>/dev/null
# Второе создание (дубликат) - должен вернуть 409
curl --noproxy "localhost" -s -o /dev/null -w "Статус: %{http_code}\n" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d "{\"id\":\"$UUID\",\"block_size\":10,\"fblock\":\"test\",\"streams\":[]}"

echo ""
echo "6. Невалидный JSON - ожидается 400"
curl --noproxy "localhost" -s -o /dev/null -w "Статус: %{http_code}\n" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d "not json"

echo ""
echo "========================================="
echo "  ТЕСТИРОВАНИЕ ЗАВЕРШЕНО"
echo "========================================="
