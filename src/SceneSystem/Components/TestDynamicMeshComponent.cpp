/**
 * @file TestDynamicMeshComponent.cpp
 * @brief Test only: TestDynamicMeshComponent implementation
 */
#include <SceneSystem/Components/TestDynamicMeshComponent.hpp>

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <Resource/GuidUtils.hpp>
#include <Resource/ResourceManager.hpp>
#include <Time.hpp>

#include <array>
#include <cmath>
#include <span>
#include <vector>

#ifdef _EDITOR
#include <ImGui/imgui.h>
#include <Editor/ResourceSlot.hpp>
#endif

using namespace Droplet;
using namespace Droplet::Scene;

namespace
{
    constexpr float STAR_DEGREES_PER_SECOND = 45.0f;
    constexpr int STAR_POINTS = 5;

    /// @brief A vertex of the star: position and an 8-bit colour
    struct StarVertex
    {
        glm::vec3 position;
        std::uint8_t color[4];
    };

    /// @brief A vertex of the quad: position, normal and uv (the default scene shader's layout)
    struct QuadVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    /// @brief The bytes of p_vertices
    template <class Vertex>
    std::vector<std::byte> ToBytes(std::span<const Vertex> p_vertices)
    {
        const std::span<const std::byte> bytes = std::as_bytes(p_vertices);
        return {bytes.begin(), bytes.end()};
    }
}

TestDynamicMeshComponent::TestDynamicMeshComponent()
{
    BuildStar();
}

void TestDynamicMeshComponent::UpdateImpl()
{
    m_angle += STAR_DEGREES_PER_SECOND * Time::Get().GetDeltaTime();
    if (m_mode == Mode::Streamed)
    {
        BuildStar();
    }
}

void TestDynamicMeshComponent::EditorUpdateImpl()
{
    UpdateImpl(); // spins in the editor too
}

void TestDynamicMeshComponent::QueueRenderImpl()
{
    GetOwner()->GetScene()->QueueRenderable(shared_from_this());
}

Common::RenderData TestDynamicMeshComponent::GetRenderDataImpl()
{
    Common::RenderData renderData{};
    renderData.dynamicMesh = m_mode == Mode::Streamed ? m_star : m_quad;
    if (m_mode == Mode::Kept)
    {
        renderData.texData.GUID = m_texture.GetGUID(); // the star has vertex colours and no texture
    }
    return renderData;
}

void TestDynamicMeshComponent::BuildStar()
{
    // a fan around the centre, the points alternating between outer and inner radius
    std::vector<StarVertex> vertices{{glm::vec3(0.0f), {255, 255, 255, 255}}};
    std::vector<std::uint32_t> indices;
    static constexpr int RIM = STAR_POINTS * 2;
    
    for (int rimIndex = 0; rimIndex < RIM; ++rimIndex)
    {
        const float angle = glm::radians(m_angle + 360.0f * static_cast<float>(rimIndex) / RIM);
        const float radius = rimIndex % 2 == 0 ? 1.0f : 0.45f;
        const auto blue = static_cast<std::uint8_t>(rimIndex * 255 / (RIM - 1));

        vertices.push_back({
            .position = glm::vec3(std::cos(angle), std::sin(angle), 0.0f) * radius, 
            .color = {255, 160, blue, 255}
        });
        indices.insert(indices.end(), {
                           0u, static_cast<std::uint32_t>(1 + rimIndex),
                           static_cast<std::uint32_t>(1 + (rimIndex + 1) % RIM)
                       });
    }

    m_star = std::make_shared<const Common::DynamicMeshData>(Common::DynamicMeshData{
        .vertexStride = sizeof(StarVertex),
        .attributes = {
            {.location = 0, .format = Common::VertexFormat::Float3, .offset = offsetof(StarVertex, position)},
            {.location = 1, .format = Common::VertexFormat::UNorm8x4, .offset = offsetof(StarVertex, color)}
        },
        .vertexData = ToBytes(std::span<const StarVertex>(vertices)),
        .indices = std::move(indices)
    });
}

void TestDynamicMeshComponent::SetTextureGUID(GUID p_guid)
{
    m_texture = {};
    m_textureDisplayName.clear();
    if (p_guid == C_INVALID_GUID)
    {
        return;
    }
    ResourceManager& resMan = ResourceManager::GetInstance();
    m_texture = resMan.LoadResource<Texture2DResource>(p_guid);
    if (MetaEntry entry; resMan.GetResourceMetaEntry(p_guid, entry))
    {
        m_textureDisplayName = entry.displayName;
    }
}

void TestDynamicMeshComponent::BuildQuad()
{
    if (m_quadId == C_INVALID_GUID)
    {
        m_quadId = GuidUtils::Generate();
    }
    ++m_quadRevision;

    // each revision a little wider, so a rebuild shows; six vertices drawn in order, facing +Z
    const float halfWidth = 0.5f + 0.1f * static_cast<float>((m_quadRevision - 1) % 5);
    const std::array<QuadVertex, 6> vertices{
        {
            {{-halfWidth, -0.5f, 0.0f}, {0, 0, 1}, {0, 0}}, {{halfWidth, -0.5f, 0.0f}, {0, 0, 1}, {1, 0}},
            {{halfWidth, 0.5f, 0.0f}, {0, 0, 1}, {1, 1}}, {{-halfWidth, -0.5f, 0.0f}, {0, 0, 1}, {0, 0}},
            {{halfWidth, 0.5f, 0.0f}, {0, 0, 1}, {1, 1}}, {{-halfWidth, 0.5f, 0.0f}, {0, 0, 1}, {0, 1}}
        }
    };

    m_quad = std::make_shared<const Common::DynamicMeshData>(Common::DynamicMeshData{
        .id = m_quadId,
        .revision = m_quadRevision,
        .vertexStride = sizeof(QuadVertex),
        .attributes = {
            {.location = 0, .format = Common::VertexFormat::Float3, .offset = offsetof(QuadVertex, position)},
            {.location = 1, .format = Common::VertexFormat::Float3, .offset = offsetof(QuadVertex, normal)},
            {.location = 2, .format = Common::VertexFormat::Float2, .offset = offsetof(QuadVertex, uv)}
        },
        .vertexData = ToBytes(std::span<const QuadVertex>(vertices)),
        .indices = {} // no indices
    });
}

void TestDynamicMeshComponent::RenderUIImpl()
{
#ifdef _EDITOR
    int mode = static_cast<int>(m_mode);
    ImGui::RadioButton("Streamed star", &mode, static_cast<int>(Mode::Streamed));
    ImGui::SameLine();
    ImGui::RadioButton("Kept quad", &mode, static_cast<int>(Mode::Kept));
    m_mode = static_cast<Mode>(mode);
    if (m_mode != Mode::Kept)
    {
        return;
    }

    if (!m_quad)
    {
        BuildQuad();
    }
    if (ImGui::Button("Rebuild (new revision)"))
    {
        BuildQuad();
    }
    ImGui::Text("Revision %llu", static_cast<unsigned long long>(m_quadRevision));

    ImGui::PushID(this);
    if (ImGui::BeginTable("TestDynamicMeshProps", 2, ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
        GUID newGUID = C_INVALID_GUID;
        std::string newName;
        if (ResourceSlot("Texture", m_texture.GetGUID(), m_textureDisplayName, newGUID, newName,
                         StaticResourceType::Texture2D))
        {
            SetTextureGUID(newGUID);
        }
        ImGui::EndTable();
    }
    ImGui::PopID();
#endif
}

nlohmann::json TestDynamicMeshComponent::SerializeImpl()
{
    nlohmann::json json;
    json["mode"] = static_cast<std::int32_t>(m_mode);
    json["textureGUID"] = m_texture.GetGUID();
    return json;
}

void TestDynamicMeshComponent::DeserializeImpl(nlohmann::json p_compJson)
{
    if (p_compJson.contains("mode"))
    {
        m_mode = static_cast<Mode>(p_compJson["mode"].get<std::int32_t>());
    }
    if (p_compJson.contains("textureGUID"))
    {
        SetTextureGUID(p_compJson["textureGUID"].get<GUID>());
    }
    if (m_mode == Mode::Kept)
    {
        BuildQuad();
    }
}
