#pragma once
#include <slang/slang-com-ptr.h>

#include "resource/IResource.hpp"

#include <slang/slang.h>

namespace Droplet
{
    /// @brief Class for shader resources.
    class ShaderResource : public IResource
    {
    public:
        enum class ShaderType
        {
            Vertex,
            Fragment,
            Geometry,
            Compute,
            TessellationControl,
            TessellationEvaluation,
            Mesh,
            Task,
        };
	    
        ShaderResource(ShaderType p_type);
        ShaderType GetType() const;
	    
        uint32_t GetGpuHandle() const;
        void SetGpuHandle(uint32_t handle);
	    
        Slang::ComPtr<slang::IBlob> GetByteCode() const;
        void SetByteCode(Slang::ComPtr<slang::IBlob> p_byteCode);
	    
        void ReleaseRam();

    private:
        ShaderType m_shaderType = ShaderType::Vertex;
        Slang::ComPtr<slang::IBlob> m_byteCode = nullptr;
        uint32_t m_gpuHandle = 0; 
    };
}
