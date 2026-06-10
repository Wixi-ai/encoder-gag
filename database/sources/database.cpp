#include "database/database.hpp"
#include "database/connection_pool.hpp"
#include "utils/colors.hpp"
#include <iostream>
#include <sstream>

Database::Database(const std::string& path) 
    : pool_(std::make_unique<ConnectionPool>(path, 5)) 
{
    auto conn = pool_->acquire();
    sqlite3* db = conn->get();
    
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
    
    prepareStatements(db);
    
    std::cout << "[Database] Tables and indexes created" << std::endl;
}

Database::~Database() {
    finalizeStatements();
}

void Database::prepareStatements(sqlite3* db) {
    // INSERT records
    sqlite3_prepare_v2(db, "INSERT INTO records (id, block_size, fblock, codec) VALUES (?, ?, ?, ?);",
                       -1, &insert_record_stmt_, nullptr);
    // INSERT record_files
    sqlite3_prepare_v2(db, "INSERT INTO record_files (record_id, stream_id, stream_type, begin_time, file_path) VALUES (?, ?, ?, ?, ?);",
                       -1, &insert_record_file_stmt_, nullptr);
    // INSERT vaa_blocks
    sqlite3_prepare_v2(db, "INSERT INTO vaa_blocks (record_id, block_index, block_type, pts, duration, data) VALUES (?, ?, ?, ?, ?, ?);",
                       -1, &insert_vaa_block_stmt_, nullptr);
    // DELETE record
    sqlite3_prepare_v2(db, "DELETE FROM records WHERE id = ?;", -1, &delete_record_stmt_, nullptr);
    // GET record by id
    sqlite3_prepare_v2(db, "SELECT id, file_path, created_at FROM records WHERE id = ?;", -1, &get_record_by_id_stmt_, nullptr);
    // GET total records count
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM records;", -1, &get_total_records_count_stmt_, nullptr);
    // GET total vaa blocks count
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM vaa_blocks;", -1, &get_total_vaa_blocks_count_stmt_, nullptr);
    // GET vaa blocks count by record_id
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM vaa_blocks WHERE record_id = ?;", -1, &get_vaa_blocks_count_stmt_, nullptr);
    // GET vaa blocks
    sqlite3_prepare_v2(db, "SELECT block_index, block_type, pts, duration, data FROM vaa_blocks WHERE record_id = ? ORDER BY block_index LIMIT ? OFFSET ?;",
                       -1, &get_vaa_blocks_stmt_, nullptr);
}

void Database::finalizeStatements() {
    if (insert_record_stmt_) sqlite3_finalize(insert_record_stmt_);
    if (insert_record_file_stmt_) sqlite3_finalize(insert_record_file_stmt_);
    if (insert_vaa_block_stmt_) sqlite3_finalize(insert_vaa_block_stmt_);
    if (delete_record_stmt_) sqlite3_finalize(delete_record_stmt_);
    if (get_record_by_id_stmt_) sqlite3_finalize(get_record_by_id_stmt_);
    if (get_total_records_count_stmt_) sqlite3_finalize(get_total_records_count_stmt_);
    if (get_total_vaa_blocks_count_stmt_) sqlite3_finalize(get_total_vaa_blocks_count_stmt_);
    if (get_vaa_blocks_count_stmt_) sqlite3_finalize(get_vaa_blocks_count_stmt_);
    if (get_vaa_blocks_stmt_) sqlite3_finalize(get_vaa_blocks_stmt_);
}

sqlite3* Database::getDb() const { 
    auto* non_const_this = const_cast<Database*>(this);
    auto conn = non_const_this->pool_->acquire();
    return conn->get(); 
}

bool Database::saveRecord(const RecordCreateRequest& request) {
    auto conn = pool_->acquire();
    sqlite3* db = conn->get();
    
    sqlite3_bind_text(insert_record_stmt_, 1, request.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(insert_record_stmt_, 2, request.block_size);
    sqlite3_bind_int(insert_record_stmt_, 3, request.fblock);
    sqlite3_bind_text(insert_record_stmt_, 4, "h264", -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(insert_record_stmt_);
    sqlite3_reset(insert_record_stmt_);
    
    if (rc != SQLITE_DONE) {
        if (rc == SQLITE_CONSTRAINT) {
            std::cerr << "[Database] Record already exists: " << request.id << std::endl;
        } else {
            std::cerr << "[Database] Insert error: " << sqlite3_errmsg(db) << std::endl;
        }
        return false;
    }
    
    for (const auto& stream : request.streams) {
        for (const auto& file : stream.files) {
            sqlite3_bind_text(insert_record_file_stmt_, 1, request.id.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(insert_record_file_stmt_, 2, stream.id);
            sqlite3_bind_text(insert_record_file_stmt_, 3, stream.type.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_record_file_stmt_, 4, file.begin.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(insert_record_file_stmt_, 5, file.path.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(insert_record_file_stmt_);
            sqlite3_reset(insert_record_file_stmt_);
            
            if (rc != SQLITE_DONE) {
                std::cerr << "[Database] File insert error: " << sqlite3_errmsg(db) << std::endl;
            }
        }
    }
    
    std::cout << "[Database] Record saved: " << request.id << std::endl;
    return true;
}

int Database::getFilteredCount(const std::string& codec, const std::string& from_date, const std::string& to_date, const std::string& file_path) {
    auto conn = pool_->acquire();
    sqlite3* db = conn->get();
    
    std::string sql = "SELECT COUNT(*) FROM records WHERE 1=1";
    if (!codec.empty()) sql += " AND codec = '" + codec + "'";
    if (!from_date.empty()) sql += " AND date(created_at) >= '" + from_date + "'";
    if (!to_date.empty()) sql += " AND date(created_at) <= '" + to_date + "'";
    if (!file_path.empty()) sql += " AND EXISTS (SELECT 1 FROM record_files WHERE record_files.record_id = records.id AND record_files.file_path LIKE '%" + file_path + "%')";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[Database] Count error: " << sqlite3_errmsg(db) << std::endl;
        return 0;
    }
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}

std::vector<std::pair<std::string, std::string>> Database::getFilteredRecords(int limit, int offset, const std::string& sort_by, const std::string& sort_order, const std::string& codec, const std::string& from_date, const std::string& to_date, const std::string& file_path) {
    auto conn = pool_->acquire();
    sqlite3* db = conn->get();
    
    std::vector<std::pair<std::string, std::string>> records;
    
    std::string valid_sort_by = "created_at";
    if (sort_by == "id") valid_sort_by = "id";
    else if (sort_by == "file_path") valid_sort_by = "file_path";
    else if (sort_by == "created_at") valid_sort_by = "created_at";
    
    std::string valid_sort_order = "ASC";
    if (sort_order == "desc" || sort_order == "DESC") valid_sort_order = "DESC";
    
    std::string sql = "SELECT id, file_path FROM records WHERE 1=1";
    if (!codec.empty()) sql += " AND codec = '" + codec + "'";
    if (!from_date.empty()) sql += " AND date(created_at) >= '" + from_date + "'";
    if (!to_date.empty()) sql += " AND date(created_at) <= '" + to_date + "'";
    if (!file_path.empty()) sql += " AND EXISTS (SELECT 1 FROM record_files WHERE record_files.record_id = records.id AND record_files.file_path LIKE '%" + file_path + "%')";
    sql += " ORDER BY " + valid_sort_by + " " + valid_sort_order + " LIMIT ? OFFSET ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[Database] Read error: " << sqlite3_errmsg(db) << std::endl;
        return records;
    }
    
    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, offset);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records.emplace_back((const char*)sqlite3_column_text(stmt, 0), (const char*)sqlite3_column_text(stmt, 1) ?: "");
    }
    sqlite3_finalize(stmt);
    return records;
}

int Database::getTotalRecordsCount() {
    auto conn = pool_->acquire();
    sqlite3_reset(get_total_records_count_stmt_);
    
    int count = 0;
    if (sqlite3_step(get_total_records_count_stmt_) == SQLITE_ROW) {
        count = sqlite3_column_int(get_total_records_count_stmt_, 0);
    }
    sqlite3_reset(get_total_records_count_stmt_);
    return count;
}

int Database::getTotalVaaBlocksCount() {
    auto conn = pool_->acquire();
    sqlite3_reset(get_total_vaa_blocks_count_stmt_);
    
    int count = 0;
    if (sqlite3_step(get_total_vaa_blocks_count_stmt_) == SQLITE_ROW) {
        count = sqlite3_column_int(get_total_vaa_blocks_count_stmt_, 0);
    }
    sqlite3_reset(get_total_vaa_blocks_count_stmt_);
    return count;
}

std::vector<std::pair<std::string, std::string>> Database::getAllRecords(int limit, int offset, const std::string& sort_by, const std::string& sort_order) {
    return getFilteredRecords(limit, offset, sort_by, sort_order, "", "", "", "");
}

std::pair<std::string, std::string> Database::getRecordTimeRange(const std::string& record_id) {
    auto conn = pool_->acquire();
    sqlite3* db = conn->get();
    const char* sql = "SELECT MIN(begin_time), MAX(begin_time) FROM record_files WHERE record_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return {"", ""};
    sqlite3_bind_text(stmt, 1, record_id.c_str(), -1, SQLITE_STATIC);
    std::string start = "", finish = "";
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* min_time = (const char*)sqlite3_column_text(stmt, 0);
        const char* max_time = (const char*)sqlite3_column_text(stmt, 1);
        if (min_time) start = min_time;
        if (max_time) finish = max_time;
    }
    sqlite3_finalize(stmt);
    return {start, finish};
}

std::tuple<bool, std::string, std::string, std::string> Database::getRecordById(const std::string& id) const {
    auto* non_const_this = const_cast<Database*>(this);
    auto conn = non_const_this->pool_->acquire();
    
    sqlite3_reset(get_record_by_id_stmt_);
    sqlite3_bind_text(get_record_by_id_stmt_, 1, id.c_str(), -1, SQLITE_STATIC);
    
    if (sqlite3_step(get_record_by_id_stmt_) == SQLITE_ROW) {
        std::string found_id = (const char*)sqlite3_column_text(get_record_by_id_stmt_, 0);
        std::string path = (const char*)sqlite3_column_text(get_record_by_id_stmt_, 1) ?: "";
        std::string created = (const char*)sqlite3_column_text(get_record_by_id_stmt_, 2);
        sqlite3_reset(get_record_by_id_stmt_);
        return {true, found_id, path, created};
    }
    sqlite3_reset(get_record_by_id_stmt_);
    return {false, "", "", ""};
}

bool Database::deleteRecordById(const std::string& id) {
    auto conn = pool_->acquire();
    
    sqlite3_reset(delete_record_stmt_);
    sqlite3_bind_text(delete_record_stmt_, 1, id.c_str(), -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(delete_record_stmt_);
    sqlite3_reset(delete_record_stmt_);
    
    if (rc == SQLITE_DONE && sqlite3_changes(conn->get()) > 0) {
        std::cout << "[Database] Record deleted: " << id << std::endl;
        return true;
    }
    std::cout << "[Database] Record not found: " << id << std::endl;
    return false;
}

std::tuple<std::vector<VideoStream>, std::vector<AudioStream>> Database::getStreamsByRecordId(const std::string&) {
    std::vector<VideoStream> video;
    std::vector<AudioStream> audio;
    VideoStream v; v.id = 0; v.codec = "h264"; v.width = 1920; v.height = 1080; v.time_base = {1, 90000}; v.extra = "";
    video.push_back(v);
    AudioStream a; a.id = 1; a.codec = "aac"; a.sample_rate = 48000; a.channels = 2; a.channel_layout = "stereo"; a.time_base = {1, 48000}; a.extra = "";
    audio.push_back(a);
    return {video, audio};
}

bool Database::saveVaaBlocks(const std::string& record_id, const std::vector<VaaBlock>& blocks) {
    auto conn = pool_->acquire();
    
    for (const auto& block : blocks) {
        sqlite3_reset(insert_vaa_block_stmt_);
        sqlite3_bind_text(insert_vaa_block_stmt_, 1, record_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(insert_vaa_block_stmt_, 2, block.index);
        sqlite3_bind_text(insert_vaa_block_stmt_, 3, block.type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(insert_vaa_block_stmt_, 4, block.pts);
        sqlite3_bind_int64(insert_vaa_block_stmt_, 5, block.duration);
        sqlite3_bind_text(insert_vaa_block_stmt_, 6, block.data.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(insert_vaa_block_stmt_) != SQLITE_DONE) {
            std::cerr << "[Database] Insert VAA error" << std::endl;
            return false;
        }
    }
    sqlite3_reset(insert_vaa_block_stmt_);
    std::cout << "[Database] Saved " << blocks.size() << " VAA blocks for record " << record_id << std::endl;
    return true;
}

std::vector<VaaBlock> Database::getVaaBlocks(const std::string& record_id, int limit, int offset) {
    auto conn = pool_->acquire();
    std::vector<VaaBlock> blocks;
    
    sqlite3_reset(get_vaa_blocks_stmt_);
    sqlite3_bind_text(get_vaa_blocks_stmt_, 1, record_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(get_vaa_blocks_stmt_, 2, limit);
    sqlite3_bind_int(get_vaa_blocks_stmt_, 3, offset);
    
    while (sqlite3_step(get_vaa_blocks_stmt_) == SQLITE_ROW) {
        VaaBlock block;
        block.index = sqlite3_column_int(get_vaa_blocks_stmt_, 0);
        block.type = (const char*)sqlite3_column_text(get_vaa_blocks_stmt_, 1);
        block.pts = sqlite3_column_int64(get_vaa_blocks_stmt_, 2);
        block.duration = sqlite3_column_int64(get_vaa_blocks_stmt_, 3);
        block.data = (const char*)sqlite3_column_text(get_vaa_blocks_stmt_, 4) ?: "";
        blocks.push_back(block);
    }
    sqlite3_reset(get_vaa_blocks_stmt_);
    return blocks;
}

int Database::getVaaBlocksCount(const std::string& record_id) {
    auto conn = pool_->acquire();
    
    sqlite3_reset(get_vaa_blocks_count_stmt_);
    sqlite3_bind_text(get_vaa_blocks_count_stmt_, 1, record_id.c_str(), -1, SQLITE_STATIC);
    
    int count = 0;
    if (sqlite3_step(get_vaa_blocks_count_stmt_) == SQLITE_ROW) {
        count = sqlite3_column_int(get_vaa_blocks_count_stmt_, 0);
    }
    sqlite3_reset(get_vaa_blocks_count_stmt_);
    return count;
}
