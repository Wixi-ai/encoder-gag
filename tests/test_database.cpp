#include <gtest/gtest.h>
#include "../database/sources/database.cpp"

class DatabaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = new Database(":memory:");
    }
    
    void TearDown() override {
        delete db;
    }
    
    Database* db;
};

TEST_F(DatabaseTest, SaveAndGetRecord) {
    RecordCreateRequest request;
    request.id = "test-uuid";
    request.block_size = 1000;
    request.fblock = 1;
    
    bool saved = db->saveRecord(request);
    EXPECT_TRUE(saved);
    
    auto [found, id, path, created] = db->getRecordById("test-uuid");
    EXPECT_TRUE(found);
    EXPECT_EQ(id, "test-uuid");
}

TEST_F(DatabaseTest, DeleteRecord) {
    RecordCreateRequest request;
    request.id = "delete-uuid";
    request.block_size = 1000;
    request.fblock = 1;
    
    db->saveRecord(request);
    
    bool deleted = db->deleteRecordById("delete-uuid");
    EXPECT_TRUE(deleted);
    
    auto [found, id, path, created] = db->getRecordById("delete-uuid");
    EXPECT_FALSE(found);
}
