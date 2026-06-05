#include <gtest/gtest.h>
#include "../database/sources/database.cpp"
#include <cstdio>
#include <sqlite3.h>

class VaaBlocksTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Создаём временную БД
        db = new Database(":memory:");
    }
    
    void TearDown() override {
        delete db;
    }
    
    Database* db;
};

TEST_F(VaaBlocksTest, SaveAndGetVaaBlocks) {
    // Создаём VAA блок
    VaaBlock block;
    block.index = 0;
    block.type = "video";
    block.pts = 0;
    block.duration = 10000;
    block.data = "TEST_DATA";
    
    std::vector<VaaBlock> blocks;
    blocks.push_back(block);
    
    // Сохраняем
    bool saved = db->saveVaaBlocks("test-uuid", blocks);
    EXPECT_TRUE(saved);
    
    // Получаем
    auto result = db->getVaaBlocks("test-uuid", 10, 0);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].index, 0);
    EXPECT_EQ(result[0].type, "video");
    EXPECT_EQ(result[0].pts, 0);
    EXPECT_EQ(result[0].duration, 10000);
    EXPECT_EQ(result[0].data, "TEST_DATA");
}

TEST_F(VaaBlocksTest, GetVaaBlocksCount) {
    VaaBlock block;
    block.index = 0;
    block.type = "video";
    block.pts = 0;
    block.duration = 10000;
    block.data = "DATA";
    
    std::vector<VaaBlock> blocks;
    blocks.push_back(block);
    blocks.push_back(block);
    
    db->saveVaaBlocks("test-uuid", blocks);
    
    int count = db->getVaaBlocksCount("test-uuid");
    EXPECT_EQ(count, 2);
}
