#pragma once

#include <sqlite3.h>
#include <string>

class DatabaseImpl {
public:
    DatabaseImpl(const std::string& path);
    ~DatabaseImpl();
    
    sqlite3* getDb() { return db; }
    void createTables();
    
private:
    sqlite3* db = nullptr;
};
