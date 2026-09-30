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
	    
        ShaderResource(ShaderType p_type, Slang::ComPtr<slang::IBlob> p_byteCode);
        
        /// @return The type of shader this shader resource represents.
        [[nodiscard]] ShaderType GetType() const;
        
        /// @return The byte code of the shader blob.
        [[nodiscard]] Slang::ComPtr<slang::IBlob> GetByteCode() const;

        /// @brief Releases the pointer to the byte code stored in RAM.
        void ReleaseRam();

    private:
        ShaderType m_type = ShaderType::Vertex;
        Slang::ComPtr<slang::IBlob> m_byteCode = nullptr;
    };
}
