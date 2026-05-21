#ifndef DATABASE_HPP
#define DATABASE_HPP

#include <sqlite3.h>
#include <string>
#include <iostream>

class Database {
public:
    Database(const std::string& path) {
        if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Ошибка открытия БД: " << sqlite3_errmsg(db) << std::endl;
        } else {
            std::cout << "[Database] Connected to " << path << std::endl;
            createTables();
        }
    }
    
    ~Database() {
        if (db) {
            sqlite3_close(db);
            std::cout << "[Database] Connection closed" << std::endl;
        }
    }
    
    void createTables() {
        const char* sql = R"(
            CREATE TABLE IF NOT EXISTS records (
                id TEXT PRIMARY KEY,
                file_path TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            );
        )";
        
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Ошибка создания таблицы: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } else {
            std::cout << "[Database] Table 'records' ready" << std::endl;
        }
    }
    
    bool saveRecord(const std::string& id, const std::string& file_path) {
        const char* sql = "INSERT INTO records (id, file_path) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Ошибка подготовки запроса: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, file_path.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Ошибка вставки: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        
        sqlite3_finalize(stmt);
        std::cout << "[Database] Record saved: " << id << std::endl;
        return true;
    }

private:
    sqlite3* db = nullptr;
};

#endif
