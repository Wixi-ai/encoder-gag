#include "../include/database.hpp"
#include "../include/colors.hpp"
#include <iostream>
#include <sstream>

Database::Database(const std::string &path)
{
    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Error opening: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return;
    }
    std::cout << COLOR_DB_COM << "[Database] Connected to " << path << COLOR_RESET << std::endl;
    createTables();
}

Database::~Database()
{
    if (db)
        sqlite3_close(db);
    std::cout << COLOR_DB_COM << "[Database] Connection closed" << COLOR_RESET << std::endl;
}

void Database::createTables()
{
    const char *sql = R"(
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
    char *errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Error creating table: " << errMsg << COLOR_RESET << std::endl;
        sqlite3_free(errMsg);
    }
    
    // Индексы для ускорения запросов
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_vaa_blocks_record_id ON vaa_blocks(record_id);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_record_files_record_id ON record_files(record_id);", nullptr, nullptr, nullptr);
    
    std::cout << COLOR_DB_COM << "[Database] Tables and indexes created" << COLOR_RESET << std::endl;
}

bool Database::saveRecord(const RecordCreateRequest &request)
{
    const char *record_sql = "INSERT INTO records (id, block_size, fblock, codec) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, record_sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Prepare error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, request.id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, request.block_size);
    sqlite3_bind_int(stmt, 3, request.fblock);
    sqlite3_bind_text(stmt, 4, "h264", -1, SQLITE_STATIC);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        if (rc == SQLITE_CONSTRAINT)
        {
            std::cerr << COLOR_DB_COM << "[Database] Record already exists: " << request.id << COLOR_RESET << std::endl;
        }
        else
        {
            std::cerr << COLOR_DB_COM << "[Database] Insert error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        }
        return false;
    }

    const char *file_sql = "INSERT INTO record_files (record_id, stream_id, stream_type, begin_time, file_path) VALUES (?, ?, ?, ?, ?);";

    for (const auto &stream : request.streams)
    {
        for (const auto &file : stream.files)
        {
            if (sqlite3_prepare_v2(db, file_sql, -1, &stmt, nullptr) != SQLITE_OK)
            {
                std::cerr << COLOR_DB_COM << "[Database] Prepare file error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
                continue;
            }

            sqlite3_bind_text(stmt, 1, request.id.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, stream.id);
            sqlite3_bind_text(stmt, 3, stream.type.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, file.begin.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, file.path.c_str(), -1, SQLITE_STATIC);

            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            if (rc != SQLITE_DONE)
            {
                std::cerr << COLOR_DB_COM << "[Database] File insert error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
            }
        }
    }

    std::cout << COLOR_DB_COM << "[Database] Record saved: " << request.id << COLOR_RESET << std::endl;
    return true;
}

int Database::getFilteredCount(const std::string &codec, const std::string &from_date, const std::string &to_date, const std::string &file_path)
{
    std::string sql = "SELECT COUNT(*) FROM records WHERE 1=1";

    if (!codec.empty())
    {
        sql += " AND codec = '" + codec + "'";
    }
    if (!from_date.empty())
    {
        sql += " AND date(created_at) >= '" + from_date + "'";
    }
    if (!to_date.empty())
    {
        sql += " AND date(created_at) <= '" + to_date + "'";
    }
    if (!file_path.empty())
    {
        sql += " AND EXISTS (SELECT 1 FROM record_files WHERE record_files.record_id = records.id AND record_files.file_path LIKE '%" + file_path + "%')";
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Count error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

std::vector<std::pair<std::string, std::string>> Database::getFilteredRecords(int limit, int offset, const std::string &sort_by, const std::string &sort_order, const std::string &codec, const std::string &from_date, const std::string &to_date, const std::string &file_path)
{
    std::vector<std::pair<std::string, std::string>> records;

    std::string valid_sort_by = "created_at";
    if (sort_by == "id")
        valid_sort_by = "id";
    else if (sort_by == "file_path")
        valid_sort_by = "file_path";
    else if (sort_by == "created_at")
        valid_sort_by = "created_at";

    std::string valid_sort_order = "ASC";
    if (sort_order == "desc" || sort_order == "DESC")
        valid_sort_order = "DESC";

    std::string sql = "SELECT id, file_path FROM records WHERE 1=1";

    if (!codec.empty())
    {
        sql += " AND codec = '" + codec + "'";
    }
    if (!from_date.empty())
    {
        sql += " AND date(created_at) >= '" + from_date + "'";
    }
    if (!to_date.empty())
    {
        sql += " AND date(created_at) <= '" + to_date + "'";
    }
    if (!file_path.empty())
    {
        sql += " AND EXISTS (SELECT 1 FROM record_files WHERE record_files.record_id = records.id AND record_files.file_path LIKE '%" + file_path + "%')";
    }

    sql += " ORDER BY " + valid_sort_by + " " + valid_sort_order + " LIMIT ? OFFSET ?;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Read error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return records;
    }

    sqlite3_bind_int(stmt, 1, limit);
    sqlite3_bind_int(stmt, 2, offset);

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        records.emplace_back(
            (const char *)sqlite3_column_text(stmt, 0),
            (const char *)sqlite3_column_text(stmt, 1) ?: "");
    }
    sqlite3_finalize(stmt);
    return records;
}

int Database::getTotalRecordsCount()
{
    const char *sql = "SELECT COUNT(*) FROM records;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Count error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return 0;
    }

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return count;
}

std::vector<std::pair<std::string, std::string>> Database::getAllRecords(int limit, int offset, const std::string &sort_by, const std::string &sort_order)
{
    return getFilteredRecords(limit, offset, sort_by, sort_order, "", "", "", "");
}

std::pair<std::string, std::string> Database::getRecordTimeRange(const std::string &record_id)
{
    const char *sql = "SELECT MIN(begin_time), MAX(begin_time) FROM record_files WHERE record_id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Time range error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return {"", ""};
    }

    sqlite3_bind_text(stmt, 1, record_id.c_str(), -1, SQLITE_STATIC);

    std::string start = "";
    std::string finish = "";

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        const char *min_time = (const char *)sqlite3_column_text(stmt, 0);
        const char *max_time = (const char *)sqlite3_column_text(stmt, 1);
        if (min_time)
            start = min_time;
        if (max_time)
            finish = max_time;
    }

    sqlite3_finalize(stmt);
    return {start, finish};
}

std::tuple<bool, std::string, std::string, std::string> Database::getRecordById(const std::string &id) const
{
    const char *sql = "SELECT id, file_path, created_at FROM records WHERE id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Prepare error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return {false, "", "", ""};
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string found_id = (const char *)sqlite3_column_text(stmt, 0);
        std::string path = (const char *)sqlite3_column_text(stmt, 1) ?: "";
        std::string created = (const char *)sqlite3_column_text(stmt, 2);
        sqlite3_finalize(stmt);
        return {true, found_id, path, created};
    }

    sqlite3_finalize(stmt);
    return {false, "", "", ""};
}

bool Database::deleteRecordById(const std::string &id)
{
    const char *delete_files_sql = "DELETE FROM record_files WHERE record_id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, delete_files_sql, -1, &stmt, nullptr) == SQLITE_OK)
    {
        sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    const char *delete_record_sql = "DELETE FROM records WHERE id = ?;";
    if (sqlite3_prepare_v2(db, delete_record_sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << COLOR_DB_COM << "[Database] Delete prepare error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE && sqlite3_changes(db) > 0)
    {
        std::cout << COLOR_DB_COM << "[Database] Record deleted: " << id << COLOR_RESET << std::endl;
        return true;
    }

    std::cout << COLOR_DB_COM << "[Database] Record not found: " << id << COLOR_RESET << std::endl;
    return false;
}

std::tuple<std::vector<VideoStream>, std::vector<AudioStream>> Database::getStreamsByRecordId(const std::string &)
{
    std::vector<VideoStream> video;
    std::vector<AudioStream> audio;

    VideoStream v;
    v.id = 0;
    v.codec = "h264";
    v.width = 1920;
    v.height = 1080;
    v.time_base = {1, 90000};
    v.extra = "";
    video.push_back(v);

    AudioStream a;
    a.id = 1;
    a.codec = "aac";
    a.sample_rate = 48000;
    a.channels = 2;
    a.channel_layout = "stereo";
    a.time_base = {1, 48000};
    a.extra = "";
    audio.push_back(a);

    return {video, audio};
}

bool Database::saveVaaBlocks(const std::string &record_id, const std::vector<VaaBlock> &blocks)
{
    const char *sql = "INSERT INTO vaa_blocks (record_id, block_index, block_type, pts, duration, data) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    for (const auto &block : blocks)
    {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << COLOR_DB_COM << "[Database] Prepare VAA error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
            return false;
        }

        sqlite3_bind_text(stmt, 1, record_id.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, block.index);
        sqlite3_bind_text(stmt, 3, block.type.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int64(stmt, 4, block.pts);
        sqlite3_bind_int64(stmt, 5, block.duration);
        sqlite3_bind_text(stmt, 6, block.data.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << COLOR_DB_COM << "[Database] Insert VAA error: " << sqlite3_errmsg(db) << COLOR_RESET << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }
        sqlite3_finalize(stmt);
    }

    std::cout << COLOR_DB_COM << "[Database] Saved " << blocks.size() << " VAA blocks for record " << record_id << COLOR_RESET << std::endl;
    return true;
}

std::vector<VaaBlock> Database::getVaaBlocks(const std::string& record_id, int limit, int offset)
{
    std::vector<VaaBlock> blocks;
    const char* sql = "SELECT block_index, block_type, pts, duration, data FROM vaa_blocks WHERE record_id = ? ORDER BY block_index LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[Database] Prepare error: " << sqlite3_errmsg(db) << std::endl;
        return blocks;
    }
    
    sqlite3_bind_text(stmt, 1, record_id.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, limit);
    sqlite3_bind_int(stmt, 3, offset);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        VaaBlock block;
        block.index = sqlite3_column_int(stmt, 0);
        block.type = (const char*)sqlite3_column_text(stmt, 1);
        block.pts = sqlite3_column_int64(stmt, 2);
        block.duration = sqlite3_column_int64(stmt, 3);
        block.data = (const char*)sqlite3_column_text(stmt, 4) ?: "";
        blocks.push_back(block);
    }
    sqlite3_finalize(stmt);
    return blocks;
}

int Database::getVaaBlocksCount(const std::string& record_id)
{
    const char* sql = "SELECT COUNT(*) FROM vaa_blocks WHERE record_id = ?;";
    sqlite3_stmt* stmt;
    int count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, record_id.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    return count;
}
