#!/bin/bash
# Создание записи
# Использование: ./scripts/create.sh [uuid] [file_path]

UUID=${1:-"$(uuidgen | tr '[:upper:]' '[:lower:]')"}
FILE_PATH=${2:-"/test/video.mp4"}

curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records \
  -H "Content-Type: application/json" \
  -d "{
    \"id\": \"$UUID\",
    \"block_size\": 4194304,
    \"fblock\": 1,
    \"streams\": [
      {
        \"type\": \"video\",
        \"id\": 0,
        \"files\": [
          {\"begin\": \"$(date -u +%Y-%m-%dT%H:%M:%SZ)\", \"path\": \"$FILE_PATH\"}
        ]
      }
    ]
  }"
