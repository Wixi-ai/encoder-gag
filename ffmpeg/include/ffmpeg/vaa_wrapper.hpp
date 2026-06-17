#pragma once

#include <string>
#include <vector>
#include "messages/messages.hpp"

namespace vaa_wrapper {

// Структура для VAA блока (внутреннее представление)
struct VaaBlockData {
    int index;
    std::string type;      // "video" или "audio"
    int64_t pts;
    int64_t duration;
    std::string data;
};

// Создаёт VAA блок
VaaBlockData createBlock(int index, const std::string& type, int64_t pts, int64_t duration, const std::string& data = "");

// Преобразует внутренний блок в сообщение
VaaBlock toMessageBlock(const VaaBlockData& block);

// Генерирует блоки для видеофайла
std::vector<VaaBlockData> generateBlocksForFile(
    const std::string& file_path,
    const std::string& record_id,
    int duration_seconds,
    int& block_counter
);

// Преобразует набор блоков в сообщения
std::vector<VaaBlock> toMessageBlocks(const std::vector<VaaBlockData>& blocks);

// Проверяет валидность блока
bool isValidBlock(const VaaBlockData& block);

// Формирует данные блока (base64 или сырые)
std::string encodeBlockData(const std::string& data);

} // namespace vaa_wrapper
