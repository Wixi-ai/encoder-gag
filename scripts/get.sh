#!/bin/bash
# Получение всех записей с поддержкой пагинации
# Использование: ./scripts/get.sh [limit] [offset]

LIMIT=${1:-10}
OFFSET=${2:-0}

curl --noproxy "localhost" -s "http://localhost:8080/api/v1/records?limit=$LIMIT&offset=$OFFSET" | python3 -m json.tool
