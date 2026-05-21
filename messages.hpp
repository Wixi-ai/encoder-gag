#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <string>
#include <vector>

// ===== Вспомогательные структуры =====

struct TimeBase
{
  int num;
  int den;
};

struct VideoStream
{
  int id;
  std::string codec;
  int width;
  int height;
  TimeBase time_base;
  std::string extra;
};

struct AudioStream
{
  int id;
  std::string codec;
  int sample_rate;
  int channels;
  std::string channel_layout;
  TimeBase time_base;
  std::string extra;
};

// ===== Запрос на создание записи (POST /api/v1/records) =====

struct RecordCreateRequest
{
  std::string id; // UUID
  int block_size;
  std::string fblock;
  std::vector<VideoStream> video_streams;
  std::vector<AudioStream> audio_streams;
};

// ===== Сообщения для SObjectizer =====

struct msg_hello
{
  std::string name;
};

struct msg_bye
{
  std::string name;
};

// Сообщение для создания записи (от HTTP к DB)
struct msg_create_record
{
  std::string id;           // UUID записи
  std::string file_path;    // путь к файлу
  std::string request_body; // сырое тело запроса
};

#endif
