#!/bin/bash
# Фильтрация по диапазону дат
FROM=${1:-"2026-05-01"}
TO=${2:-"2026-05-31"}
curl --noproxy "localhost" "http://localhost:8080/api/v1/records?from_date=$FROM&to_date=$TO"
