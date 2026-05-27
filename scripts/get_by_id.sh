#!/bin/bash
# Получение записи по ID
# Использование: ./scripts/get_by_id.sh <uuid>

if [ -z "$1" ]; then
    echo "Usage: ./scripts/get_by_id.sh <uuid>"
    exit 1
fi

curl --noproxy "localhost" -s "http://localhost:8080/api/v1/records/$1" | python3 -m json.tool
