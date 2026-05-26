#!/bin/bash
response=$(curl --noproxy "localhost" -s http://localhost:8080/api/v1/records)

if [ -z "$response" ] || [ "$response" = "[]" ]; then
    echo -e "\033[33mНет записей\033[0m"
    exit 0
fi

echo -e "\033[36m"
echo "СПИСОК ЗАПИСЕЙ:"
echo "----------------------------------------"

# Быстрый вывод через grep и cut
echo "$response" | grep -o '"id":"[^"]*"' | cut -d'"' -f4 | while read id; do
    path=$(echo "$response" | grep -o '"file_path":"[^"]*"' | cut -d'"' -f4 | head -1)
    echo "ID:   $id"
    echo "Path: $path"
    echo "----------------------------------------"
done
echo -e "\033[0m"
