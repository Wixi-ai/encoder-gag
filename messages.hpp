#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <string>
#include <vector>
#include <so_5/all.hpp>

// ===== Вспомогательные структуры =====

struct TimeBase {
    int num;
    int den;
};

struct VideoStream {
    int id;
    std::string codec;
    int width;
    int height;
    TimeBase time_base;
    std::string extra;
};

struct AudioStream {
    int id;
    std::string codec;
    int sample_rate;
    int channels;
    std::string channel_layout;
    TimeBase time_base;
    std::string extra;
};

// ===== Запрос на создание записи =====

struct RecordCreateRequest {
    std::string id;
    int block_size;
    std::string fblock;
    std::vector<VideoStream> video_streams;
    std::vector<AudioStream> audio_streams;
};

// ===== Сообщения для акторов =====

struct msg_hello {
    std::string name;
};

struct msg_bye {
    std::string name;
};

struct msg_create_record {
    std::string id;
    std::string file_path;
    std::string request_body;
};

struct msg_create_response {
    bool success;
    std::string error_message;
};

struct msg_get_records {
    // пустое сообщение-запрос
};

struct msg_get_records_response {
    std::vector<std::pair<std::string, std::string>> records;
};

#endif

// Запрос на получение всех записей (с указанием, куда отвечать)
struct msg_get_records {
    so_5::mbox_t reply_to;  // обратный адрес для ответа
};

// Ответ со списком записей
struct msg_get_records_response {
    std::vector<std::pair<std::string, std::string>> records;
};

