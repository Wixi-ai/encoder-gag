#!/bin/bash
# Получение записей с сортировкой и пагинацией
# Использование: ./scripts/get_sorted.sh [sort_by] [sort_order] [limit] [offset]
# sort_by: id, created_at, file_path (по умолчанию created_at)
# sort_order: asc, desc (по умолчанию asc)

SORT_BY=${1:-created_at}
SORT_ORDER=${2:-asc}
LIMIT=${3:-10}
OFFSET=${4:-0}

curl --noproxy "localhost" -s "http://localhost:8080/api/v1/records?sort_by=$SORT_BY&sort_order=$SORT_ORDER&limit=$LIMIT&offset=$OFFSET" | python3 -m json.tool
