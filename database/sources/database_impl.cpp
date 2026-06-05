#include "database/database_impl.hpp"
#include "utils/colors.hpp"
#include <iostream>

DatabaseImpl::DatabaseImpl(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "[Database] Error opening: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    std::cout << "[Database] Connected to " << path << std::endl;
    createTables();
}

DatabaseImpl::~DatabaseImpl() {
    if (db) sqlite3_close(db);
    std::cout << "[Database] Connection closed" << std::endl;
}

void DatabaseImpl::createTables() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS records (
            id TEXT PRIMARY KEY,
            file_path TEXT,
            codec TEXT DEFAULT 'h264',
            block_size INTEGER DEFAULT 0,
            fblock INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
        CREATE TABLE IF NOT EXISTS record_files (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            record_id TEXT NOT NULL,
            stream_id INTEGER NOT NULL,
            stream_type TEXT NOT NULL,
            begin_time TEXT NOT NULL,
            file_path TEXT NOT NULL,
            FOREIGN KEY (record_id) REFERENCES records(id) ON DELETE CASCADE
        );
        CREATE TABLE IF NOT EXISTS vaa_blocks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            record_id TEXT NOT NULL,
            block_index INTEGER NOT NULL,
            block_type TEXT NOT NULL,
            pts INTEGER NOT NULL,
            duration INTEGER NOT NULL,
            data TEXT NOT NULL,
            FOREIGN KEY (record_id) REFERENCES records(id) ON DELETE CASCADE
        );
    )";
    
    char* errMsg = nullptr;
    sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (errMsg) {
        std::cerr << "[Database] Error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_vaa_blocks_record_id ON vaa_blocks(record_id);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_record_files_record_id ON record_files(record_id);", nullptr, nullptr, nullptr);
    
    std::cout << "[Database] Tables and indexes created" << std::endl;
}
