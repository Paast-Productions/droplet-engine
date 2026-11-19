/**
* @file Spoof.hpp
* @brief 'Spoofs' a scene by creating it in memory using constexpr definitions for Guids.
*/
#pragma once

#include <optional>
#include <string_view>

#include "Integration/Assets/Assets.hpp" // dummy definitions of assets
#include "Integration/Game/Scene.hpp"    // simple version of a scene node system

/// Hand-built data standing in for a loaded level, so the renderer can be developed without the real game
namespace Spoof
{
    using Rgba = std::array<std::uint8_t, 4>;
    // == Meshes ==
    inline constexpr Guid CubeMesh         = Guid::FromName("mesh/cube");    // simple cube
    inline constexpr Guid QuadMesh         = Guid::FromName("mesh/quad");    // single quad
    inline constexpr Guid GroundMesh       = Guid::FromName("mesh/ground");  // ground grid
    inline constexpr Guid BigScreenMesh    = Guid::FromName("mesh/monitor"); // quad mesh sampling a camera's output texture as 
    inline constexpr Guid ScreenMesh       = Guid::FromName("mesh/screen");  // larger quad mesh sampling a camera's output as its texture
    inline constexpr Guid CameraMarkerMesh = Guid::FromName("mesh/camera_marker"); // small white cube to mark a camera
    // == Textures ==
    inline constexpr Guid CheckerTex  = Guid::FromName("texture/checker");  // blue checkerboard texture
    inline constexpr Guid TileTex     = Guid::FromName("texture/tiles");    // grey tiled texture
    inline constexpr Guid StripeTex   = Guid::FromName("texture/stripes");  // diagonal stripes texture
    inline constexpr Guid ReverseStripeTex = Guid::FromName("texture/reverse_stripes");  // diagonal stripes texture
    inline constexpr Guid GradientTex = Guid::FromName("texture/gradient"); // color gradient texture
    // == Shaders ==
    inline constexpr Guid ToonShader = Guid::FromName("shader/toon"); // toon shader lighting up whole faces at certain angles
    // == Camera Render Targets ==
    inline constexpr Guid TargetA    = Guid::FromName("target/camera_a");
    inline constexpr Guid TargetB    = Guid::FromName("target/camera_b");
    inline constexpr Guid TargetC    = Guid::FromName("target/camera_c");
    inline constexpr Guid TargetMain = Guid::FromName("target/camera_main");

    // == Camera component data ==
    inline constexpr const char* CameraMainName = "Camera Main";
    inline constexpr glm::vec3 CameraMainStart{ 0.0f, 4.5f, 11.0f };
    inline constexpr glm::vec3 CameraMainTarget{ 0.0f, 1.5f, -1.0f };
    inline constexpr Game::AnimationComponent CameraMainAnimation
    {
        .amplitude = 5.0f,
        .period = 10.0f,
        .axis = {1.0f, 0.0f, 0.0f}
    }; // swings along X, looking at the scene


    inline constexpr const char* CameraAName = "Camera A";
    inline constexpr glm::vec3 CameraAStart{0.0f, 2.4f, 3.6f};  // pos: centred slightly elevated and back from the origin
    inline constexpr glm::vec3 CameraATarget{0.0f, 1.5f, 0.0f}; // lookAt: slightly above the centre to give an angled view
    inline constexpr Game::AnimationComponent CameraAAnimation
    {
        .amplitude = 1.0f,
        .period = 3.0f,
        .axis = {1.0f, 0.0f, 0.0f},
        .kind = Game::AnimationComponent::Kind::Pan
    }; // swings along X axis, looking at the centre cube

    inline constexpr const char* CameraBName = "Camera B";
    inline constexpr glm::vec3 CameraBStart{-6.0f, 1.5f, -3.0f};  // studio B, left, looking -X
    inline constexpr glm::vec3 CameraBTarget{-9.5f, 1.5f, -3.0f};

    inline constexpr const char* CameraCName = "Camera C";
    inline constexpr glm::vec3 CameraCStart{6.0f, 2.0f, -3.0f};   // studio C, right, looking +X
    inline constexpr glm::vec3 CameraCTarget{9.5f, 1.0f, -3.0f};
    inline constexpr Game::AnimationComponent CameraCAnimation
    {
        .amplitude = 1.5f,
        .period = 5.0f,
        .axis = {0.0f, 0.0f, 1.0f}
    }; // swings along Z (sideways for it), looking at the studio cube


    // -- Make Nodes--
    /// Creates a node in the scene with a Camera component attached and an optional animation
    /// as well as a small white cube in the scene that follows it
    Game::Node MakeCamera(const char* name,
                          glm::vec3 pos,
                          glm::vec3 target,
                          float fovY,
                          float aspect,
                          Guid output,
                          bool present = false,
                          const std::optional<Game::AnimationComponent>& animation = std::nullopt);

    // -- Make Assets --
    /// Creates a MeshAsset cube
    Assets::MeshAsset MakeCube(float size);
    /// Creates a single quad as a mesh
    Assets::MeshAsset MakeQuad(float width, float height);
    
    // -- Make Textures --
    /// @brief Checkered texture with dimensions size x size, cells nr of cells alternativ between color1 and color2
    Assets::TextureAsset MakeCheckerTexture(std::uint32_t size, std::uint32_t cells, Rgba color1, Rgba color2);
    /// @brief white and blue checker texture
    Assets::TextureAsset MakeBlueCheckerTexture(std::uint32_t size, std::uint32_t cells);
    /// @brief The checker with two greys
    Assets::TextureAsset MakeTileCheckerTexture(std::uint32_t size, std::uint32_t cells);
    /// @brief Diagonal stripes
    Assets::TextureAsset MakeStripeTexture(std::uint32_t size, std::uint32_t stripes, Rgba color1, Rgba color2, bool inverse=false);
    /// @brief Maps a texture to UV coordinates. Red to green along u, plus blue along v
    Assets::TextureAsset MakeGradientTexture(std::uint32_t size);

    /// @brief Create an asset storage filled with fake 'loaded from disk' resources
    Assets::AssetStorage MakeResources();

    /// @brief Create spoof scene with geometry and cameras
    Game::Scene MakeScene(const Assets::AssetStorage& resources);
}
