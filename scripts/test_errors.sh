#!/bin/bash
echo "=== Test 1: Invalid UUID (GET) ==="
curl --noproxy "localhost" http://localhost:8080/api/v1/records/123

echo -e "\n=== Test 2: Invalid UUID (DELETE) ==="
curl --noproxy "localhost" -X DELETE http://localhost:8080/api/v1/records/123

echo -e "\n=== Test 3: Empty body (POST) ==="
curl --noproxy "localhost" -X POST http://localhost:8080/api/v1/records -H "Content-Type: application/json" -d ''

echo -e "\n=== Test 4: Non-existent UUID (404) ==="
curl --noproxy "localhost" http://localhost:8080/api/v1/records/00000000-0000-0000-0000-000000000000
