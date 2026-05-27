#!/bin/bash
# Создание записи
# Использование: ./scripts/create.sh [uuid] [file_path]

# Генерируем UUID если не передан
if [ -z "$1" ]; then
    UUID=$(uuidgen | tr '[:upper:]' '[:lower:]')
else
    UUID=$1
fi

FILE_PATH=${2:-"C:/test/video.mp4"}

curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records \
  -H "Content-Type: application/json" \
  -d "{\"id\":\"$UUID\",\"block_size\":10,\"fblock\":\"test\",\"streams\":[],\"file_path\":\"$FILE_PATH\"}"
