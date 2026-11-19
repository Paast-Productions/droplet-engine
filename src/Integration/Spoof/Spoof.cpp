/**
 * @file Spoof.cpp
 * @brief Implementation of Spoof.hpp
 */
#include "Integration/Spoof/Spoof.hpp"

#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <string>
#include <utility>
#include <span>
#include <vector>


using namespace Spoof;

namespace
{
    using namespace Assets;

    // Colors
    constexpr Rgba ORANGE{240, 140, 40, 255};
    constexpr Rgba COAL{40, 35, 45, 255};
    constexpr Rgba MAGENTA{200, 35, 200, 255};
    constexpr Rgba NEON_GREEN{57, 255, 20, 255};
    constexpr Rgba LIGHT_BLUE{60, 90, 200, 255};
    constexpr Rgba OFF_WHITE{230, 230, 230, 255};
    constexpr Rgba DARK_GREY{80, 80, 85, 255};
    constexpr Rgba LIGHT_GREY{120, 120, 125, 255};

    /// @brief A mesh while it is built: typed vertices in the scene layout, before they become asset bytes
    struct MeshBuilder
    {
        std::vector<Common::MeshVertex> vertices;
        std::vector<std::uint32_t> indices;
    };

    /// @brief The asset for `builder`: its vertices as bytes, and spherical bounds computed from their positions
    MeshAsset ToMeshAsset(const MeshBuilder& builder)
    {
        MeshAsset mesh;
        const std::span<const std::byte> bytes = std::as_bytes(std::span(builder.vertices));
        mesh.vertexData.assign(bytes.begin(), bytes.end());
        mesh.vertexStride = sizeof(Common::MeshVertex);
        mesh.indices = builder.indices;
        if (builder.vertices.empty())
            return mesh;

        glm::vec3 smallest = builder.vertices.front().position;
        glm::vec3 biggest = smallest;
        for (const Common::MeshVertex& vertex : builder.vertices)
        {
            smallest = glm::min(smallest, vertex.position);
            biggest = glm::max(biggest, vertex.position);
        }
        mesh.boundsCenter = (smallest + biggest) * 0.5f;
        for (const Common::MeshVertex& vertex : builder.vertices)
        {
            mesh.boundsRadius = std::max(mesh.boundsRadius, glm::length(vertex.position - mesh.boundsCenter));
        }
        return mesh;
    }

    // Appends one quad (BL, BR, TR, TL) facing along normal, with vec1 = right and vec2 = up (vec1 x vec2 == normal).
    // Winding is counter-clockwise when viewed from outside, uv (0,0) = top-left
    void AddFace(
        MeshBuilder& mesh,
        glm::vec3 center,
        glm::vec3 normal,
        glm::vec3 vec1,
        glm::vec3 vec2,
        float halfWidth,
        float halfHeight)
    {
        const auto base = static_cast<std::uint32_t>(mesh.vertices.size());

        mesh.vertices.push_back({
            .position = center - vec1 * halfWidth - vec2 * halfHeight, .normal = normal, .uv = {0.0f, 1.0f}
        });
        mesh.vertices.push_back({
            .position = center + vec1 * halfWidth - vec2 * halfHeight, .normal = normal, .uv = {1.0f, 1.0f}
        });
        mesh.vertices.push_back({
            .position = center + vec1 * halfWidth + vec2 * halfHeight, .normal = normal, .uv = {1.0f, 0.0f}
        });
        mesh.vertices.push_back({
            .position = center - vec1 * halfWidth + vec2 * halfHeight, .normal = normal, .uv = {0.0f, 0.0f}
        });

        for (const std::uint32_t corner : {0u, 1u, 2u, 0u, 2u, 3u})
        {
            mesh.indices.push_back(base + corner);
        }
    }


    /// @brief A size x size RGBA8 sRGB texture filled by pixelColor(x, y).
    template <class PixelColor>
    TextureAsset MakeTexture(std::uint32_t size, PixelColor&& pixelColor)
    {
        TextureAsset texture;
        texture.width = texture.height = size;
        texture.format = Common::PixelFormat::RGBA8_SRGB;
        texture.pixels.resize(std::size_t{size} * size * 4);

        for (std::uint32_t y = 0; y < size; ++y)
        {
            for (std::uint32_t x = 0; x < size; ++x)
            {
                const Rgba color = pixelColor(x, y);
                const std::size_t pixelOffset = (std::size_t{y} * size + x) * 4;
                for (std::size_t channel = 0; channel < 4; ++channel)
                {
                    texture.pixels[pixelOffset + channel] = static_cast<std::byte>(color[channel]);
                }
            }
        }
        return texture;
    }

    Game::Node MakeMeshNode(const char* name, glm::vec3 position, Guid mesh, Common::ImageRef baseColor,
                            Guid shader = {})
    {
        Game::Node node;
        node.name = name;
        node.transform.position = position;
        node.mesh = Game::MeshComponent{
            .mesh = mesh,
            .imageTexture = baseColor,
            .shader = shader
        };
        return node;
    }
}

namespace Spoof
{
    MeshAsset MakeCube(float size)
    {
        const float half = size * 0.5f;
        MeshBuilder mesh;
        mesh.vertices.reserve(24);
        mesh.indices.reserve(36);

        struct Face
        {
            glm::vec3 normal, v1, v2;
        };

        static constexpr std::array<Face, 6> faces{
            {
                {.normal = {1.f, 0.f, 0.f}, .v1 = {0.f, 0.f, -1.f}, .v2 = {0.f, 1.f, 0.f}},
                {.normal = {-1.f, 0.f, 0.f}, .v1 = {0.f, 0.f, 1.f}, .v2 = {0.f, 1.f, 0.f}},
                {.normal = {0.f, 1.f, 0.f}, .v1 = {1.f, 0.f, 0.f}, .v2 = {0.f, 0.f, -1.f}},
                {.normal = {0.f, -1.f, 0.f}, .v1 = {1.f, 0.f, 0.f}, .v2 = {0.f, 0.f, 1.f}},
                {.normal = {0.f, 0.f, 1.f}, .v1 = {1.f, 0.f, 0.f}, .v2 = {0.f, 1.f, 0.f}},
                {.normal = {0.f, 0.f, -1.f}, .v1 = {-1.f, 0.f, 0.f}, .v2 = {0.f, 1.f, 0.f}},
            }
        };
        for (const Face& face : faces)
        {
            AddFace(mesh, face.normal * half, face.normal, face.v1, face.v2, half, half);
        }

        return ToMeshAsset(mesh);
    }

    MeshAsset MakeQuad(float width, float height)
    {
        MeshBuilder mesh;
        AddFace(mesh, {0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {0, 1, 0}, width * 0.5f, height * 0.5f);

        return ToMeshAsset(mesh);
    }

    /// Makes a checker texture
    TextureAsset MakeCheckerTexture(std::uint32_t size, std::uint32_t cells, Rgba color1, Rgba color2)
    {
        const std::uint32_t cell = std::max(1u, size / std::max(1u, cells));

        return MakeTexture(size, [&](std::uint32_t x, std::uint32_t y)
        {
            return ((x / cell) + (y / cell)) % 2 == 0 ? color1 : color2;
        });
    }

    TextureAsset MakeBlueCheckerTexture(std::uint32_t size, std::uint32_t cells)
    {
        return MakeCheckerTexture(size, cells, OFF_WHITE, LIGHT_BLUE);
    }

    TextureAsset MakeTileCheckerTexture(std::uint32_t size, std::uint32_t cells)
    {
        return MakeCheckerTexture(size, cells, LIGHT_GREY, DARK_GREY);
    }

    TextureAsset MakeStripeTexture(std::uint32_t size, std::uint32_t stripes, Rgba color1, Rgba color2, bool inverse)
    {
        const std::uint32_t stripeWidth = std::max(1u, size / std::max(1u, stripes));

        return MakeTexture(size, [&](std::uint32_t x, std::uint32_t y)
        {
            uint32_t factor = inverse ? x - y : x + y;
            return (factor / stripeWidth) % 2 == 0 ? color1 : color2;
        });
    }

    TextureAsset MakeGradientTexture(std::uint32_t size)
    {
        const float scale = 255.0f / static_cast<float>(std::max(1u, size - 1));
        return MakeTexture(size, [&](std::uint32_t x, std::uint32_t y)
        {
            const auto u = static_cast<std::uint8_t>(static_cast<float>(x) * scale);
            const auto v = static_cast<std::uint8_t>(static_cast<float>(y) * scale);

            return Rgba{static_cast<std::uint8_t>(255 - u), u, v, 255};
        });
    }

    AssetStorage MakeResources()
    {
        AssetStorage store;

        store.Add(CubeMesh, MakeCube(1.5f));
        store.Add(QuadMesh, MakeQuad(2.0f, 2.0f));
        store.Add(GroundMesh, MakeQuad(20.0f, 20.0f));
        store.Add(BigScreenMesh, MakeQuad(3.0f, 3.0f));
        store.Add(ScreenMesh, MakeQuad(1.6f, 1.6f));
        store.Add(CameraMarkerMesh, MakeCube(0.3f));

        // one tile per metre on the 20 m ground, seen at grazing angles: anisotropic filtering keeps it sharp
        TextureAsset tiles = MakeCheckerTexture(512, 20, LIGHT_GREY, DARK_GREY);
        tiles.sampling.maxAnisotropy = 16.0f;
        store.Add(TileTex, std::move(tiles));
        store.Add(CheckerTex, MakeCheckerTexture(256, 8, OFF_WHITE, LIGHT_BLUE));
        store.Add(ReverseStripeTex, MakeStripeTexture(256, 16, ORANGE, COAL));
        store.Add(StripeTex, MakeStripeTexture(256, 8, MAGENTA, NEON_GREEN, /*invert*/ true));
        store.Add(GradientTex, MakeGradientTexture(256));

        // Add additional shader
        store.Add(ToonShader, ShaderAsset{
                      .moduleName = "scene_toon",
                      .sourcePath = "Shaders/scene_toon.slang"
                  });

        return store;
    }

    Game::Node MakeCamera(
        const char* name,
        glm::vec3 pos,
        glm::vec3 target,
        float fovY,
        float aspect,
        Guid output,
        bool present,
        const std::optional<Game::AnimationComponent>& animation)
    {
        Game::Node node;
        node.name = name;
        node.transform.position = pos;
        node.transform.rotation = glm::quatLookAtRH(glm::normalize(target - pos), glm::vec3(0, 1, 0));
        node.camera = Game::CameraComponent{
            .fovYDegrees = fovY,
            .aspect = aspect,
            .output = output,
            .present = present,
            .startPos = pos,
            .lookAtTarget = target,
            .upDir = {0.0f, 1.0f, 0.0f}
        };
        if (animation)
        {
            node.animation = animation;
        }
        // a small white cube where the camera is (no texture: drawn white); the camera itself skips it
        node.mesh = Game::MeshComponent{.mesh = CameraMarkerMesh, .imageTexture = {}, .shader = {}};
        return node;
    }

    Game::Scene MakeScene(const AssetStorage& resources)
    {
        using Common::PostEffect;
        using Effect = Game::PostProcessComponent::Effect;


        Game::Scene scene;

        // == Cameras == 
        // The scene cameras are placed deliberately to form a dependency chain between them: 
        // Camera A sees screen B and outputs to the screen A quad, which it also sees.
        // Camera B films screen C and outputs to screen B.
        // Camera C films a cube and outputs to screen C.
        //
        // The dependency is C -> B -> A, where C is independent
        // Main camera sees the entire scene and depends on all of them.
        // They are inserted in declared order, opposite of the dependency order, to force RenderGraph to sort them by dependency


        // Render Targets are declared, not created. Renderer builds the images the first time it sees ids.
        // This has them usable on the CPU side
        scene.renderTargets = {
            {.id = TargetA, .width = 512, .height = 512},
            {.id = TargetB, .width = 512, .height = 512},
            {.id = TargetC, .width = 512, .height = 512},
            {.id = TargetMain, .width = 1280, .height = 720},
        };

        // camera A gets a gresscale post-processing effect
        Game::Node cameraA = MakeCamera(CameraAName, CameraAStart, CameraATarget, 60.0f, 1.0f,
                                        TargetA, false, CameraAAnimation);
        cameraA.postProcess = Game::PostProcessComponent{
            .effects = {
                Effect{
                    .effect = PostEffect::Grayscale
                }
            }
        };
        scene.nodes.push_back(cameraA);

        // camera B gets no post-processing
        scene.nodes.push_back(MakeCamera(CameraBName, CameraBStart, CameraBTarget, 40.0f, 1.0f, TargetB));

        Game::Node cameraC = MakeCamera(CameraCName, CameraCStart, CameraCTarget, 45.0f, 1.0f,
                                        TargetC, false, CameraCAnimation);
        cameraC.postProcess = Game::PostProcessComponent{.effects = {Effect{.effect = PostEffect::Invert}}};
        scene.nodes.push_back(cameraC);

        Game::Node cameraMain = MakeCamera(CameraMainName, CameraMainStart, CameraMainTarget, 55.0f, 16.0f / 9.0f,
                                           TargetMain, /*present*/true, CameraMainAnimation);
        cameraMain.postProcess = Game::PostProcessComponent{
            .effects = {
                Effect{.effect = PostEffect::Grayscale, .enabled = false},
                Effect{.effect = PostEffect::Contrast, .amount = 1.6f, .enabled = false},
                Effect{.effect = PostEffect::Invert, .enabled = false},
                Effect{.effect = PostEffect::Blur, .amount = 6.0f, .enabled = false},
                Effect{.effect = PostEffect::AutoExposure, .amount = 1.0f, .enabled = false},
            }
        };
        scene.nodes.push_back(cameraMain);

        // == Meshes == 

        // the quad faces +Z, so it is turned to face +Y
        Game::Node ground = MakeMeshNode("Ground", {0.0f, 0.0f, 0.0f}, GroundMesh,
                                         Common::ImageRef::FromTexture(TileTex));
        ground.transform.rotation = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0));
        scene.nodes.push_back(ground);

        // the tilted checker cube, spinning about Y-axis
        Game::Node cube = MakeMeshNode("Checker Cube", {0.0f, 1.5f, 0.0f}, CubeMesh,
                                       Common::ImageRef::FromTexture(CheckerTex));
        cube.transform.rotation = glm::angleAxis(glm::radians(30.0f), glm::normalize(glm::vec3(1, 1, 0)));
        cube.spin = Game::SpinComponent{
            .axis = {0.0f, 1.0f, 0.0f},
            .degreesPerSecond = 30.0f,
            .restRotation = cube.transform.rotation
        };
        scene.nodes.push_back(cube);

        // cube seen by camera C
        Game::Node cubeC = MakeMeshNode("Cube (seen by Camera C)", CameraCTarget, CubeMesh,
                                        Common::ImageRef::FromTexture(StripeTex), ToonShader);
        cubeC.spin = Game::SpinComponent
        {
            .axis = {1.0f, 1.0f, 0.0f},
            .degreesPerSecond = 40.0f
        };
        scene.nodes.push_back(cubeC);

        // A ring of six around it: cubes and pillars, three textures, every other one with the toon shader
        struct RingItem
        {
            const char* name;
            Guid texture;
            Guid shader;
            bool pillar; ///< tall and thin instead of a cube
            float spinSpeed; ///< degrees per second about Y; 0 = still
        };
        static constexpr std::array<RingItem, 6> RING{
            {
                {.name = "Stripe Pillar", .texture = StripeTex, .shader = Guid{}, .pillar = true, .spinSpeed = 0.0f},
                {
                    .name = "Toon Gradient Cube", .texture = GradientTex, .shader = ToonShader, .pillar = false,
                    .spinSpeed = 45.0f
                },
                {.name = "Checker Pillar", .texture = CheckerTex, .shader = Guid{}, .pillar = true, .spinSpeed = 0.0f},
                {
                    .name = "Reverse Toon Stripe Cube", .texture = ReverseStripeTex, .shader = ToonShader,
                    .pillar = false, .spinSpeed = -60.0f
                },
                {
                    .name = "Gradient Pillar", .texture = GradientTex, .shader = Guid{}, .pillar = true,
                    .spinSpeed = 0.0f
                },
                {
                    .name = "Toon Checker Cube", .texture = CheckerTex, .shader = ToonShader, .pillar = false,
                    .spinSpeed = 90.0f
                },
            }
        };
        for (std::size_t index = 0; index < RING.size(); ++index)
        {
            constexpr float RING_RADIUS = 4.5f;
            const RingItem& item = RING[index];

            // offset by 30 degrees so the gap behind the centre leaves the monitor visible
            const float angle = glm::radians(30.0f + 60.0f * static_cast<float>(index));
            const float height = item.pillar ? 3.0f : 1.5f;

            Game::Node node = MakeMeshNode(
                item.name,
                {RING_RADIUS * std::sin(angle), height * 0.5f, RING_RADIUS * std::cos(angle)},
                CubeMesh,
                Common::ImageRef::FromTexture(item.texture),
                item.shader);
            if (item.pillar)
                node.transform.scale = {0.5f, 2.0f, 0.5f};
            if (item.spinSpeed != 0.0f)
                node.spin = Game::SpinComponent{.axis = {0.0f, 1.0f, 0.0f}, .degreesPerSecond = item.spinSpeed};
            scene.nodes.push_back(node);
        }

        // -- the chain of screens --
        // Screen A behind the ring
        scene.nodes.push_back(MakeMeshNode("Screen A (shows Camera A)", {0.0f, 3.0f, -6.5f}, BigScreenMesh,
                                           Common::ImageRef::FromTarget(TargetA)));

        // Scree B off to the side
        Game::Node screenB = MakeMeshNode("Screen B (shows Camera B)", {-2.0f, 1.6f, 0.0f}, ScreenMesh,
                                          Common::ImageRef::FromTarget(TargetB));
        screenB.transform.rotation = glm::angleAxis(glm::radians(30.0f), glm::vec3(0, 1, 0)); // turned towards A
        scene.nodes.push_back(screenB);

        // Screen C
        Game::Node screenC = MakeMeshNode("Screen C (shows Camera C)", CameraBTarget, ScreenMesh,
                                          Common::ImageRef::FromTarget(TargetC));
        screenC.transform.rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0)); // faces +X, towards B
        screenC.animation = CameraAAnimation;
        scene.nodes.push_back(screenC);

        // the scene holds each mesh's bounds, as it would in an octree: copied from the asset the node refers to
        for (Game::Node& node : scene.nodes)
        {
            if (!node.mesh)
                continue;
            if (const MeshAsset* mesh = resources.FindMesh(node.mesh->mesh))
            {
                node.mesh->boundsCenter = mesh->boundsCenter;
                node.mesh->boundsRadius = mesh->boundsRadius;
            }
        }

        return scene;
    }
}
