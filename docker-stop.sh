#!/bin/bash
echo "Остановка контейнера..."
docker stop encoders_gag
docker rm encoders_gag
echo "Контейнер остановлен и удалён"
