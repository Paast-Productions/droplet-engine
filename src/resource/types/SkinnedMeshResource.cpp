#include "resource/types/SkinnedMeshResource.hpp"

using namespace Droplet;

int SkinnedMeshResource::AddBone(const std::string &p_name, int p_parentIndex, const glm::mat4 &p_offsetMat)
{
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
