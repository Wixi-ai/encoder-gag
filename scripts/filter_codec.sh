#!/bin/bash
# Фильтрация по codec
CODEC=${1:-"h264"}
curl --noproxy "localhost" "http://localhost:8080/api/v1/records?codec=$CODEC"
