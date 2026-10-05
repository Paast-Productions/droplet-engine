#include "resource/GuidUtils.hpp"
#include <gtest/gtest.h>
#include <unordered_set>

using namespace Droplet;

TEST(GuidUtilsTest, IsValid)
{
    // C_INVALID_GUID (0) must always be invalid
    EXPECT_FALSE(GuidUtils::IsValid(C_INVALID_GUID));
    EXPECT_FALSE(GuidUtils::IsValid(0));

    // Any non-zero number should be valid
    EXPECT_TRUE(GuidUtils::IsValid(1));
    EXPECT_TRUE(GuidUtils::IsValid(0xFFFFFFFFFFFFFFFF)); // Max uint64_t
}

TEST(GuidUtilsTest, GenerateIsValid)
{
    GUID newGuid = GuidUtils::Generate();
    EXPECT_NE(newGuid, C_INVALID_GUID);
    EXPECT_TRUE(GuidUtils::IsValid(newGuid));
}

TEST(GuidUtilsTest, GenerateIsUnique)
{
    // We generate a large batch of GUIDs and put them in a Hash Set.
    // Since std::unordered_set only allows unique values, if the size of the set
    // matches the number of generated GUIDs, we guarantee there were no duplicates
    
    constexpr int generationCount = 10000;
    std::unordered_set<GUID> generatedGuids;

    for (int i = 0; i < generationCount; ++i)
    {
        GUID newGuid = GuidUtils::Generate();
        
        // Assert that the GUID wasn't already in the set
        EXPECT_TRUE(generatedGuids.find(newGuid) == generatedGuids.end()) 
            << "Collision detected! Generated a duplicate GUID: " << newGuid;
            
        generatedGuids.insert(newGuid);
    }

    EXPECT_EQ(generatedGuids.size(), generationCount);
}