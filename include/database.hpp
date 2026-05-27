#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include <utility>
#include <tuple>

class Database {
public:
    Database(const std::string& path);
    ~Database();

    bool saveRecord(const std::string& id, const std::string& file_path);
    std::vector<std::pair<std::string, std::string>> getAllRecords(int limit, int offset, const std::string& sort_by, const std::string& sort_order);
    int getTotalRecordsCount();
    std::tuple<bool, std::string, std::string, std::string> getRecordById(const std::string& id);
    bool deleteRecordById(const std::string& id);

private:
    sqlite3* db = nullptr;
    void createTables();
};
