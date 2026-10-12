#include "resource/types/SkinnedMeshResource.hpp"
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet;

std::unique_ptr<SkinnedMeshResource> SkinnedMeshResource::CreateFallback()
{
    ZoneScoped;

    auto fallback = std::make_unique<SkinnedMeshResource>();
    
    struct SkinnedVertex
    {
        glm::vec3 pos;
        int b[4];   // Bone indices
        float w[4]; // Bone weights
    };
    
    // 8 vertices of a 1x1x1 unit cube
    float rawPos[8][3] = {
        {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f},
        {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f}
    };
    
    SkinnedVertex vertices[8];
    for (int i = 0; i < 8; i++)
    {
        vertices[i].pos = glm::vec3(rawPos[i][0], rawPos[i][1], rawPos[i][2]);
            
        // Rig 100% to Bone 0
        vertices[i].b[0] = 0; vertices[i].b[1] = 0; vertices[i].b[2] = 0; vertices[i].b[3] = 0;
        vertices[i].w[0] = 1.0f; vertices[i].w[1] = 0.0f; vertices[i].w[2] = 0.0f; vertices[i].w[3] = 0.0f;
    }

    // 36 indices (standard cube)
    std::vector<std::uint32_t> indices = {
        0, 1, 2, 2, 3, 0, // Front
        1, 5, 6, 6, 2, 1, // Right
        7, 6, 5, 5, 4, 7, // Back
        4, 0, 3, 3, 7, 4, // Left
        4, 5, 1, 1, 0, 4, // Bottom
        3, 2, 6, 6, 7, 3  // Top
    };
    
    // Copy data into byte vector
    std::size_t vertexByteSize = sizeof(SkinnedVertex);
    std::vector<std::byte> vertexData(8 * vertexByteSize);
    std::memcpy(vertexData.data(), vertices, vertexData.size());
    
    // NOTE: I'm unsure if these name strings are correct?
    std::vector<MeshResource::VertexAttribute> layout = {
        {"POSITION", sizeof(float) * 3},
        {"bone_indices", sizeof(int) * 4},
        {"bone_weights", sizeof(float) * 4}
    };
    
    fallback->SetMeshData(vertexData, indices, vertexByteSize, layout);
    try
    {
        int index = fallback->AddBone("root", -1, glm::mat4(1.0f));
        index; // I love nodiscard + treat warnings as errors
        fallback->GenerateBoneBounds();
    }
    catch (...)
    {
        // TODO: Log error (this should never be possible though)
    }
    
    return fallback;
}

int SkinnedMeshResource::AddBone(const std::string &p_name, int p_parentIndex, const glm::mat4 &p_offsetMat)
{
    ZoneScoped;

    if (m_boneMap.find(p_name) != m_boneMap.end())
    {
        throw std::runtime_error("Bone with the same name already exists: " + p_name);
    }

    if (p_parentIndex < -1 || p_parentIndex >= static_cast<int>(m_bones.size()))
    {
        throw std::out_of_range("Parent index is out of range: " + std::to_string(p_parentIndex));
    }

    Bone bone {
        .name = p_name, 
        .parentIndex = p_parentIndex, 
        .offsetMat = p_offsetMat
    };

    m_bones.push_back(bone);
    m_boneMap[p_name] = static_cast<int>(m_bones.size()) - 1;

    return m_boneMap[p_name];
}

bool SkinnedMeshResource::ValidateSkinnedVertexLayout() const
{
    bool hasBoneIndices = false;
    bool hasBoneWeights = false;
    std::size_t size = static_cast<std::size_t>(-1);

    for (const auto &attribute : m_vertexLayout)
    {
        if (attribute.first == "bone_indices")
        {
            if (size == -1)
            {
                size = attribute.second;
            }
            else if (size != attribute.second)
            {
                return false; // Bone indices and weights must have the same size
            }

            hasBoneIndices = true;
        }
        else if (attribute.first == "bone_weights")
        {
            if (size == -1)
            {
                size = attribute.second;
            }
            else if (size != attribute.second)
            {
                return false; // Bone indices and weights must have the same size
            }

            hasBoneWeights = true;
        }
    }

    return hasBoneIndices && hasBoneWeights;
}

const std::vector<SkinnedMeshResource::Bone> & SkinnedMeshResource::GetBones() const
{
    return m_bones;
}

const std::unordered_map<std::string, int> & SkinnedMeshResource::GetBoneMap() const
{
    return m_boneMap;
}

void SkinnedMeshResource::GenerateBoneBounds()
{
    ZoneScoped;

    // Ensure layout has bone_indices attribute
    std::size_t boneIndexDataOffset = 0;
    std::size_t boneIndexDataSize = 0;

    if (!FindVertexAttribute("bone_indices", boneIndexDataOffset, boneIndexDataSize))
    {
        throw std::runtime_error("SkinnedMeshResource vertex layout does not contain bone_indices attribute.");
    }

	// Ensure layout has bone_weights attribute
    std::size_t boneWeightsDataOffset = 0;
    std::size_t boneWeightsDataSize = 0;

    if (!FindVertexAttribute("bone_weights", boneWeightsDataOffset, boneWeightsDataSize))
    {
        throw std::runtime_error("SkinnedMeshResource vertex layout does not contain bone_weights attribute.");
    }

	// Get position attribute offset and size
    std::size_t posDataOffset = 0;
    std::size_t posDataSize = 0;

    if (!FindVertexAttribute("POSITION", posDataOffset, posDataSize))
    {
        throw std::runtime_error("SkinnedMeshResource vertex layout does not contain POSITION attribute.");
    }

	std::size_t numBoneIndices = boneIndexDataSize / sizeof(int);

	// For each bone: find all vertices influenced by this bone, transform them to bone space using the offset matrix, and compute the OBB.
    for (int boneIndex = 0; boneIndex < m_bones.size(); ++boneIndex)
    {
        std::vector<glm::vec3> boneSpaceVertices;

        for (std::size_t vertexIndex = 0; vertexIndex < m_vertexData.size() / m_vertexByteSize; ++vertexIndex)
        {
            const std::byte *vertexPtr = m_vertexData.data() + vertexIndex * m_vertexByteSize;
            const int *boneIndices = reinterpret_cast<const int *>(vertexPtr + boneIndexDataOffset);
            const float *boneWeights = reinterpret_cast<const float *>(vertexPtr + boneWeightsDataOffset);

            // Check if the current bone influences this vertex
            for (std::size_t i = 0; i < numBoneIndices; ++i)
            {
                if (boneIndices[i] != boneIndex)
                {
                    continue; // This bone does not influence this vertex
				}

                if (boneWeights[i] <= 0.0f)
                {
                    break; // Skip if the weight is zero or negative
				}

                // Get the position of the vertex
                const glm::vec3 *positionPtr = reinterpret_cast<const glm::vec3 *>(vertexPtr + posDataOffset);
                glm::vec4 meshSpacePosition = glm::vec4(*positionPtr, 1.0f);

                // Transform to bone space using the offset matrix
                glm::vec4 boneSpacePosition = m_bones[boneIndex].offsetMat * meshSpacePosition;
                boneSpaceVertices.push_back(glm::vec3(boneSpacePosition));

                break; // No need to check other indices for this vertex
            }
        }

        // Compute the OBB for the collected bone space vertices
        if (!boneSpaceVertices.empty())
        {
            m_bones[boneIndex].bounds = Math::OBB::FromPoints(boneSpaceVertices);
		}
	}
}
