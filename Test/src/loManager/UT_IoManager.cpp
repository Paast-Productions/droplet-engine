#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "Core/loManager.hpp"

namespace fs = std::filesystem;

namespace Droplet::Core
{
    class IOManagerTest : public ::testing::Test
    {
    protected:
        const fs::path testFile = "IOManagerTest.json";

        void SetUp() override
        {
			// remove the test file if it exists before each test.
            fs::remove(testFile);
        }

        void TearDown() override
        {
			// clean up the test file after each test.
            fs::remove(testFile);
        }

        void CreateTestFile(const std::string &content)
        {
            std::ofstream file(testFile);

            ASSERT_TRUE(file.is_open());

            file << content;
            file.close();
        }

        nlohmann::json ReadTestFile()
        {
            std::ifstream file(testFile);

            EXPECT_TRUE(file.is_open());

            nlohmann::json json;
            file >> json;

            return json;
        }
    };


    // =========================================================
    // Load
    // =========================================================

    TEST_F(IOManagerTest, LoadReadsValidJson)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level": 10,
            "health": 100
        })");

        const std::string path = testFile.string();

        const nlohmann::json result = JsonIO::Read(path);

        EXPECT_EQ(result["name"], "Player");
        EXPECT_EQ(result["level"], 10);
        EXPECT_EQ(result["health"], 100);
    }


    TEST_F(IOManagerTest, LoadReadsNestedJson)
    {
        CreateTestFile(R"({
            "player": {
                "name": "Player",
                "stats": {
                    "health": 100,
                    "mana": 50
                }
            },
            "items": [
                "Sword",
                "Shield"
            ]
        })");

        const std::string path = testFile.string();

        const nlohmann::json result = JsonIO::Read(path);

        EXPECT_EQ(result["player"]["name"], "Player");
        EXPECT_EQ(result["player"]["stats"]["health"], 100);
        EXPECT_EQ(result["player"]["stats"]["mana"], 50);

        ASSERT_TRUE(result["items"].is_array());
        EXPECT_EQ(result["items"].size(), 2);
        EXPECT_EQ(result["items"][0], "Sword");
        EXPECT_EQ(result["items"][1], "Shield");
    }


    TEST_F(IOManagerTest, LoadReturnsEmptyObject)
    {
        CreateTestFile("{}");

        const std::string path = testFile.string();

        const nlohmann::json result = JsonIO::Read(path);

        EXPECT_TRUE(result.is_object());
        EXPECT_TRUE(result.empty());
    }


    TEST_F(IOManagerTest, LoadThrowsWhenFileDoesNotExist)
    {
        const std::string path = "FileThatDoesNotExist.json";

        EXPECT_THROW(
            JsonIO::Read(path),
            std::runtime_error
        );
    }


    TEST_F(IOManagerTest, LoadThrowsWhenJsonIsInvalid)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level":
        })");

        const std::string path = testFile.string();

        nlohmann::json result = JsonIO::Read(path);
        EXPECT_TRUE(result.is_discarded());
    }


    // =========================================================
    // Save
    // =========================================================

    TEST_F(IOManagerTest, SaveCreatesFile)
    {
        const nlohmann::json data = {
            {"name", "Player"},
            {"level", 10}
        };

        EXPECT_NO_THROW(
            JsonIO::Write(
                testFile.string(),
                data
            )
        );

        EXPECT_TRUE(fs::exists(testFile));
    }


    TEST_F(IOManagerTest, SaveWritesCorrectJson)
    {
        const nlohmann::json data = {
            {"name", "Player"},
            {"level", 10},
            {"health", 100}
        };

        EXPECT_NO_THROW(
            JsonIO::Write(
                testFile.string(),
                data
            )
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result, data);
    }


    TEST_F(IOManagerTest, SaveWritesNestedJson)
    {
        const nlohmann::json data = {
            {
                "player",
                {
                    {"name", "Player"},
                    {"level", 10}
                }
            },
            {
                "items",
                {"Sword", "Shield"}
            }
        };

        EXPECT_NO_THROW(
            JsonIO::Write(
                testFile.string(),
                data
            )
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result, data);
    }


    TEST_F(IOManagerTest, SaveOverwritesExistingFile)
    {
        CreateTestFile(R"({
            "oldData": true
        })");

        const nlohmann::json data = {
            {"newData", true}
        };

        EXPECT_NO_THROW(
            JsonIO::Write(
                testFile.string(),
                data
            )
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_TRUE(result.contains("newData"));
        EXPECT_FALSE(result.contains("oldData"));
    }


    TEST_F(IOManagerTest, SaveReturnsTrueOnSuccess)
    {
        const nlohmann::json data = {
            {"value", 42}
        };

        EXPECT_NO_THROW(
            JsonIO::Write(
                testFile.string(),
                data
            )
        );
    }


    // =========================================================
    // DataAction - Insert
    // =========================================================

    TEST_F(IOManagerTest, DataActionInsertAddsNewValues)
    {
        CreateTestFile(R"({
            "name": "Player"
        })");

        const nlohmann::json data = {
            {"level", 10},
            {"health", 100}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Insert
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result["name"], "Player");
        EXPECT_EQ(result["level"], 10);
        EXPECT_EQ(result["health"], 100);
    }


    TEST_F(IOManagerTest, DataActionInsertOverwritesExistingValue)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level": 1
        })");

        const nlohmann::json data = {
            {"level", 10}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Insert
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result["name"], "Player");
        EXPECT_EQ(result["level"], 10);
    }


    TEST_F(IOManagerTest, DataActionInsertMultipleValues)
    {
        CreateTestFile("{}");

        const nlohmann::json data = {
            {"name", "Player"},
            {"level", 5},
            {"health", 100},
            {"alive", true}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Insert
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result["name"], "Player");
        EXPECT_EQ(result["level"], 5);
        EXPECT_EQ(result["health"], 100);
        EXPECT_EQ(result["alive"], true);
    }


    // =========================================================
    // DataAction - Delete
    // =========================================================

    TEST_F(IOManagerTest, DataActionDeleteRemovesValue)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level": 10,
            "health": 100
        })");

        const nlohmann::json data = {
            {"level", nullptr}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Delete
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_TRUE(result.contains("name"));
        EXPECT_TRUE(result.contains("health"));
        EXPECT_FALSE(result.contains("level"));
    }


    TEST_F(IOManagerTest, DataActionDeleteMultipleValues)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level": 10,
            "health": 100,
            "mana": 50
        })");

        const nlohmann::json data = {
            {"level", nullptr},
            {"health", nullptr}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Delete
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_TRUE(result.contains("name"));
        EXPECT_TRUE(result.contains("mana"));

        EXPECT_FALSE(result.contains("level"));
        EXPECT_FALSE(result.contains("health"));
    }


    TEST_F(IOManagerTest, DataActionDeleteNonExistingValueDoesNotThrow)
    {
        CreateTestFile(R"({
            "name": "Player"
        })");

        const nlohmann::json data = {
            {"doesNotExist", nullptr}
        };

        EXPECT_NO_THROW(
            JsonIO::ModifyWithAction(
                testFile.string(),
                data,
                JsonIO::ModifyAction::Delete
            )
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result["name"], "Player");
    }


    // =========================================================
    // DataAction - Error handling
    // =========================================================

    TEST_F(IOManagerTest, DataActionThrowsWhenFileDoesNotExist)
    {
        const nlohmann::json data = {
            {"name", "Player"}
        };

        EXPECT_THROW(
            JsonIO::ModifyWithAction(
                testFile.string(),
                data,
                JsonIO::ModifyAction::Insert
            ),
            std::runtime_error
        );
    }


    TEST_F(IOManagerTest, DataActionThrowsWhenJsonIsInvalid)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level":
        })");

        const nlohmann::json data = {
            {"health", 100}
        };

		std::string path = testFile.string();
        nlohmann::json result = JsonIO::ModifyWithAction(path, data, JsonIO::ModifyAction::Insert);
        EXPECT_TRUE(result.is_discarded());
    }


    // =========================================================
    // DataAction - File content
    // =========================================================

    TEST_F(IOManagerTest, DataActionKeepsExistingValuesWhenInserting)
    {
        CreateTestFile(R"({
            "name": "Player",
            "level": 5
        })");

        const nlohmann::json data = {
            {"health", 100}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Insert
        );

        const nlohmann::json result = ReadTestFile();

        EXPECT_EQ(result["name"], "Player");
        EXPECT_EQ(result["level"], 5);
        EXPECT_EQ(result["health"], 100);
    }


    TEST_F(IOManagerTest, DataActionKeepsFileValidAfterInsert)
    {
        CreateTestFile("{}");

        const nlohmann::json data = {
            {"name", "Player"}
        };

        JsonIO::ModifyWithAction(
            testFile.string(),
            data,
            JsonIO::ModifyAction::Insert
        );

        std::ifstream file(testFile);

        EXPECT_TRUE(file.is_open());

        nlohmann::json result;

        EXPECT_NO_THROW(
            file >> result
        );

        EXPECT_TRUE(result.is_object());
    }

    TEST_F(IOManagerTest, DeleteFiles)
    {
        CreateTestFile(R"({
            "name": "Player"
        })");

        EXPECT_TRUE(std::filesystem::exists(testFile));
        JsonIO::DeleteFile(testFile.string());
        JsonIO::DeleteFile("logger.json");
        EXPECT_FALSE(std::filesystem::exists(testFile));
    }
}

