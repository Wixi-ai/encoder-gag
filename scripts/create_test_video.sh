#!/bin/bash
# Создание тестового видео для проверки
# ./scripts/create_test_video.sh [duration] [size] [output_path]

DURATION=${1:-10}
SIZE=${2:-640x480}
OUTPUT=${3:-"C:/test/test_video.mp4"}

mkdir -p "$(dirname "$OUTPUT")"

/c/Users/tungiia/ffmpeg-release/ffmpeg-8.1.1-essentials_build/bin/ffmpeg \
  -f lavfi -i testsrc=duration=$DURATION:size=$SIZE:rate=30 \
  -c:v libx264 -t $DURATION "$OUTPUT" -y

echo "Test video created: $OUTPUT ($DURATION sec, $SIZE)"
