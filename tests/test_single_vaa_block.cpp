#include <gtest/gtest.h>
#include "../database/sources/database.cpp"

class SingleVaaBlockTest : public ::testing::Test {
protected:
    void SetUp() override {
        db = new Database(":memory:");
    }
    
    void TearDown() override {
        delete db;
    }
    
    Database* db;
};

TEST_F(SingleVaaBlockTest, CreateSingleBlock) {
    VaaBlock block;
    block.index = 5;
    block.type = "audio";
    block.pts = 50000;
    block.duration = 10000;
    block.data = "AUDIO_DATA";
    
    std::vector<VaaBlock> blocks;
    blocks.push_back(block);
    
    bool saved = db->saveVaaBlocks("single-uuid", blocks);
    EXPECT_TRUE(saved);
    
    auto result = db->getVaaBlocks("single-uuid", 10, 0);
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].index, 5);
    EXPECT_EQ(result[0].type, "audio");
    EXPECT_EQ(result[0].pts, 50000);
    EXPECT_EQ(result[0].duration, 10000);
    EXPECT_EQ(result[0].data, "AUDIO_DATA");
}

TEST_F(SingleVaaBlockTest, BlockWithoutData) {
    VaaBlock block;
    block.index = 0;
    block.type = "video";
    block.pts = 0;
    block.duration = 10000;
    block.data = "";
    
    std::vector<VaaBlock> blocks;
    blocks.push_back(block);
    
    bool saved = db->saveVaaBlocks("empty-uuid", blocks);
    EXPECT_TRUE(saved);
    
    auto result = db->getVaaBlocks("empty-uuid", 10, 0);
    EXPECT_EQ(result[0].data, "");
}
