#include "../include/database.hpp"
#include "../include/colors.hpp"
#include <iostream>

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Error opening: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return;
    }
    std::cout << COLOR_DB_COM << "[Database] Connected to " << path << COLOR_RESET << std::endl;
    createTables();
}

Database::~Database() {
    if (db) sqlite3_close(db);
    std::cout << COLOR_DB_COM << "[Database] Connection closed" << COLOR_RESET << std::endl;
}

void Database::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS records (
            id TEXT PRIMARY KEY,
            file_path TEXT NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Error creating table: " << errMsg << COLOR_RESET << std::endl;
        sqlite3_free(errMsg);
    } else {
        std::cout << COLOR_DB_COM << "[Database] Table 'records' ready" << COLOR_RESET << std::endl;
    }
}

bool Database::saveRecord(const std::string& id, const std::string& file_path) {
    const char* sql = "INSERT INTO records (id, file_path) VALUES (?, ?);";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Prepare error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, file_path.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_CONSTRAINT) {
        std::cerr << COLOR_DB_COM << "[Database] Record already exists: " << id << COLOR_RESET << std::endl;
        return false;
    }
    if (rc != SQLITE_DONE) {
        std::cerr << COLOR_DB_COM << "[Database] Insert error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return false;
    }
    
    std::cout << COLOR_DB_COM << "[Database] Record saved: " << id << COLOR_RESET << std::endl;
    return true;
}

int Database::getTotalRecordsCount() {
    const char* sql = "SELECT COUNT(*) FROM records;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Count error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

std::vector<std::pair<std::string, std::string>> Database::getAllRecords(int limit, int offset) {
    std::vector<std::pair<std::string, std::string>> records;
    const char* sql = "SELECT id, file_path FROM records LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Read error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return records;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, offset);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.emplace_back(
            (const char*)sqlite3_column_text(stmt, 0),
            (const char*)sqlite3_column_text(stmt, 1)
        );
    }
    sqlite3_finalize(stmt);
    return records;
}

std::tuple<bool, std::string, std::string, std::string> Database::getRecordById(const std::string& id) {
    const char* sql = "SELECT id, file_path, created_at FROM records WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Prepare error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return {false, "", "", ""};
    }
    
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string found_id = (const char*)sqlite3_column_text(stmt, 0);
        std::string path = (const char*)sqlite3_column_text(stmt, 1);
        std::string created = (const char*)sqlite3_column_text(stmt, 2);
        sqlite3_finalize(stmt);
        return {true, found_id, path, created};
    }
    
    sqlite3_finalize(stmt);
    return {false, "", "", ""};
}

bool Database::deleteRecordById(const std::string& id) {
    const char* sql = "DELETE FROM records WHERE id = ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << COLOR_DB_COM << "[Database] Delete prepare error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_DONE && sqlite3_changes(db) > 0) {
        std::cout << COLOR_DB_COM << "[Database] Record deleted: " << id << COLOR_RESET << std::endl;
        return true;
    }
    
    std::cout << COLOR_DB_COM << "[Database] Record not found: " << id << COLOR_RESET << std::endl;
    return false;
}
