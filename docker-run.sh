#!/bin/bash
echo "Запуск контейнера..."
docker run -d -p 8080:8080 --name encoders_gag encoders_gag:latest
echo "Контейнер запущен. Логи: docker logs -f encoders_gag"
