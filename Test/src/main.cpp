#include <gtest/gtest.h>
#include "Core/IO-Manager.hpp"

#include <fstream>
#include <string>
#include <cstdio>

using Droplet::Core::IOManager;

class IOManagerTest : public ::testing::Test
{
protected:
    const std::string testFile = "test_save.json";

    void TearDown() override
    {
        std::remove(testFile.c_str());
    }
};

// Test 1: save should work
TEST_F(IOManagerTest, SavePlayerData)
{
    IOManager::PlayerSaveData player;
    player.name = "TestPlayer";
    player.level = 5;
    player.health = 75.5f;

    std::string file = testFile;

    bool result = IOManager::Save(file, player);

    EXPECT_TRUE(result);

    std::ifstream input(file);
    EXPECT_TRUE(input.is_open());
}

// Test 2: saves and load load same data
TEST_F(IOManagerTest, SaveAndLoadPlayerData)
{
    IOManager::PlayerSaveData original;
    original.name = "TestPlayer";
    original.level = 10;
    original.health = 80.0f;

    std::string file = testFile;

    ASSERT_TRUE(IOManager::Save(file, original));

    IOManager::PlayerSaveData loaded;

    IOManager::Load(file, loaded);

    EXPECT_EQ(loaded.name, original.name);
    EXPECT_EQ(loaded.level, original.level);
    EXPECT_FLOAT_EQ(loaded.health, original.health);
}

// Test 3: Load from a file that do not exist 
TEST_F(IOManagerTest, LoadNonExistingFileThrows)
{
    std::string file = "does_not_exist.json";
    IOManager::PlayerSaveData player;

    EXPECT_THROW(
        IOManager::Load(file, player),
        std::runtime_error
    );
}

// Test 4: Load from non invalid json
TEST_F(IOManagerTest, LoadInvalidJsonThrows)
{
    std::string file = testFile;

    {
        std::ofstream output(file);
        ASSERT_TRUE(output.is_open());

        output << "{ invalid json }";
    }

    IOManager::PlayerSaveData player;

    EXPECT_THROW(
        IOManager::Load(file, player),
        std::runtime_error
    );
}

// Test 5: Controll the values saves 
TEST_F(IOManagerTest, SavedJsonContainsCorrectValues)
{
    IOManager::PlayerSaveData player;
    player.name = "Alice";
    player.level = 20;
    player.health = 42.5f;

    std::string file = testFile;

    ASSERT_TRUE(IOManager::Save(file, player));

    std::ifstream input(file);
    ASSERT_TRUE(input.is_open());

    nlohmann::json json;
    input >> json;

    EXPECT_EQ(json["name"], "Alice");
    EXPECT_EQ(json["level"], 20);
    EXPECT_FLOAT_EQ(json["health"], 42.5f);
}

// Test 6: DefaultValues
TEST_F(IOManagerTest, PlayerSaveDataHasCorrectDefaults)
{
    IOManager::PlayerSaveData player;

    EXPECT_EQ(player.name, "Player");
    EXPECT_EQ(player.level, 1);
    EXPECT_FLOAT_EQ(player.health, 100.0f);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}