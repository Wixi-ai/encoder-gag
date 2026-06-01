#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include "messages.hpp"

class Database
{
public:
    Database(const std::string &path);
    ~Database();

    bool saveRecord(const RecordCreateRequest &request);
    std::vector<std::pair<std::string, std::string>> getAllRecords(int limit, int offset, const std::string &sort_by, const std::string &sort_order);
    int getTotalRecordsCount();
    int getFilteredCount(const std::string &codec, const std::string &from_date, const std::string &to_date, const std::string &file_path);
    std::vector<std::pair<std::string, std::string>> getFilteredRecords(int limit, int offset, const std::string &sort_by, const std::string &sort_order, const std::string &codec, const std::string &from_date, const std::string &to_date, const std::string &file_path);
    std::pair<std::string, std::string> getRecordTimeRange(const std::string &record_id);
    std::tuple<bool, std::string, std::string, std::string> getRecordById(const std::string &id) const;
    bool deleteRecordById(const std::string &id);
    std::tuple<std::vector<VideoStream>, std::vector<AudioStream>> getStreamsByRecordId(const std::string &record_id);
    bool saveVaaBlocks(const std::string &record_id, const std::vector<VaaBlock> &blocks);

private:
    sqlite3 *db = nullptr;
    void createTables();
};
