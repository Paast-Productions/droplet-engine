#include "resource/io/SlangFormat.hpp"
#include "resource/types/ShaderResource.hpp"

#include <gtest/gtest.h>
#include <json/json.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace Droplet;

class SlangLoaderTest : public ::testing::Test
{
protected:
    fs::path m_validShaderPath;
    fs::path m_invalidSyntaxPath;
    fs::path m_emptyShaderPath;
    fs::path m_missingPath;
    
    nlohmann::json m_defaultLoadSettings;
    
    void SetUp() override
    {
        fs::path baseDir(TEST_ASSET_DIR);
        fs::path shaderDir = baseDir / "Shaders";

        m_validShaderPath = shaderDir / "Test_Valid.slang";
        m_invalidSyntaxPath = shaderDir / "Test_InvalidSyntax.slang";
        m_emptyShaderPath = shaderDir / "Test_Empty.slang";
        m_missingPath = shaderDir / "DoesNotExist.slang";

        // Pre-flight checks
        ASSERT_TRUE(fs::exists(m_validShaderPath)) << "Missing test asset: " << m_validShaderPath;
        ASSERT_TRUE(fs::exists(m_invalidSyntaxPath)) << "Missing test asset: " << m_invalidSyntaxPath;
        ASSERT_TRUE(fs::exists(m_emptyShaderPath)) << "Missing test asset: " << m_emptyShaderPath;
        
        m_defaultLoadSettings = nlohmann::json::object();
    }
};

TEST_F(SlangLoaderTest, ListAssetResources)
{
    auto resources = IO::SlangFormat::ListAssetResources(m_validShaderPath);
    
    ASSERT_EQ(resources.size(), 1);
    
    EXPECT_EQ(resources[0].first, ResourceType::Shader); 
    EXPECT_EQ(resources[0].second, "Test_Valid");
}

TEST_F(SlangLoaderTest, CompileValidShader)
{
    std::unique_ptr<ShaderResource> shader = IO::SlangFormat::CompileAndLoad(m_validShaderPath, m_defaultLoadSettings);

    ASSERT_NE(shader, nullptr);
    EXPECT_NE(shader->GetByteCode().get(), nullptr);
    
    shader->ReleaseRam();
    EXPECT_EQ(shader->GetByteCode(), nullptr);
}

TEST_F(SlangLoaderTest, CompileNonExistentFile)
{
    EXPECT_THROW(IO::SlangFormat::CompileAndLoad(m_missingPath, m_defaultLoadSettings), std::runtime_error);
}

TEST_F(SlangLoaderTest, CompileInvalidSyntax)
{
    EXPECT_THROW(IO::SlangFormat::CompileAndLoad(m_invalidSyntaxPath, m_defaultLoadSettings), std::runtime_error);
}

TEST_F(SlangLoaderTest, CompileEmptyFile)
{
    EXPECT_THROW(IO::SlangFormat::CompileAndLoad(m_emptyShaderPath, m_defaultLoadSettings), std::runtime_error);
}