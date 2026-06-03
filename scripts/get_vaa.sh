#!/bin/bash
# Получение VAA блоков записи
# Использование: ./scripts/get_vaa.sh <record_id> [limit] [offset]

if [ -z "$1" ]; then
    echo "Usage: ./scripts/get_vaa.sh <record_id> [limit] [offset]"
    echo "Example: ./scripts/get_vaa.sh 04cc951e-1c0f-4838-bc37-3d9d1c71b505"
    echo "         ./scripts/get_vaa.sh 04cc951e-1c0f-4838-bc37-3d9d1c71b505 5 0"
    exit 1
fi

RECORD_ID=$1
LIMIT=${2:-100}
OFFSET=${3:-0}

curl --noproxy "localhost" -s "http://localhost:8080/api/v1/archive/records/$RECORD_ID/vva_blocks?limit=$LIMIT&offset=$OFFSET" | python3 -m json.tool
