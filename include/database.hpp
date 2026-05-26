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
    std::vector<std::pair<std::string, std::string>> getAllRecords();
    std::tuple<bool, std::string, std::string, std::string> getRecordById(const std::string& id);

private:
    sqlite3* db = nullptr;
    void createTables();
};
