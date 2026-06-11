#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include "messages/messages.hpp"

class ConnectionPool;

class Database {
public:
    Database(const std::string& path);
    ~Database();

    bool saveRecord(const RecordCreateRequest& request);
    std::vector<std::pair<std::string, std::string>> getAllRecords(int limit, int offset, const std::string& sort_by, const std::string& sort_order);
    int getTotalRecordsCount();
    int getFilteredCount(const std::string& codec, const std::string& from_date, const std::string& to_date, const std::string& file_path);
    std::vector<std::pair<std::string, std::string>> getFilteredRecords(int limit, int offset, const std::string& sort_by, const std::string& sort_order, const std::string& codec, const std::string& from_date, const std::string& to_date, const std::string& file_path);
    std::pair<std::string, std::string> getRecordTimeRange(const std::string& record_id);
    std::tuple<bool, std::string, std::string, std::string> getRecordById(const std::string& id) const;
    bool deleteRecordById(const std::string& id);
    std::tuple<std::vector<VideoStream>, std::vector<AudioStream>> getStreamsByRecordId(const std::string& record_id);
    bool saveVaaBlocks(const std::string& record_id, const std::vector<VaaBlock>& blocks);
    std::vector<VaaBlock> getVaaBlocks(const std::string& record_id, int limit, int offset);
    int getVaaBlocksCount(const std::string& record_id);
    int getTotalVaaBlocksCount();
    sqlite3* getDb() const;

private:
    std::unique_ptr<ConnectionPool> pool_;
    
    // Подготовленные выражения
    sqlite3_stmt* insert_record_stmt_ = nullptr;
    sqlite3_stmt* insert_record_file_stmt_ = nullptr;
    sqlite3_stmt* insert_vaa_block_stmt_ = nullptr;
    sqlite3_stmt* get_filtered_count_stmt_ = nullptr;
    sqlite3_stmt* get_filtered_records_stmt_ = nullptr;
    sqlite3_stmt* get_total_records_count_stmt_ = nullptr;
    sqlite3_stmt* get_record_by_id_stmt_ = nullptr;
    sqlite3_stmt* delete_record_stmt_ = nullptr;
    sqlite3_stmt* get_vaa_blocks_stmt_ = nullptr;
    sqlite3_stmt* get_vaa_blocks_count_stmt_ = nullptr;
    sqlite3_stmt* get_total_vaa_blocks_count_stmt_ = nullptr;
    
    void prepareStatements(sqlite3* db);
    void finalizeStatements();
};
    int getTotalRecordsCount();
    int getTotalVaaBlocksCount();
