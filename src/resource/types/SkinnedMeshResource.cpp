#include "resource/types/SkinnedMeshResource.hpp"

namespace Droplet
{
    // TODO: Move definitions here
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
}
