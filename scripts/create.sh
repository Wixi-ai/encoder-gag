#!/bin/bash
# Создание записи по ТЗ

UUID=${1:-"$(uuidgen | tr '[:upper:]' '[:lower:]')"}

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
          {\"begin\": \"2026-06-01T09:00:00Z\", \"path\": \"/test/video.mp4\"}
        ]
      }
    ]
  }"
