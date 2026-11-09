#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>

namespace Droplet::Graphics
{
	// Forward declarations
	class Renderer;
	class Camera;
}

namespace Droplet::Graphics::VK::Slug
{
	/// @brief Width, in texels, of the generated curve and band textures.
	inline constexpr uint32_t TEXTURE_WIDTH = 4096;
	
	/// @brief Magic identifier used to recognize Slug binary files.
	inline constexpr char SLUG_HEADER_DATA[] = "SLUG";
	
	/// @brief Number of bytes written for the Slug file identifier.
	inline constexpr size_t SLUG_HEADER_LEN = 4;
	
	
	std::vector<char> ReadShader(const std::filesystem::path &p_path);
	vk::raii::ShaderModule CreateShaderModule(const vk::raii::Device &p_device, const std::filesystem::path &p_path);
	vma::raii::Buffer CreateHostBuffer(const vma::raii::Allocator &p_allocator, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage);
	
	struct FrameResources
	{
		vma::raii::Buffer wireBuffer { nullptr };
		vma::raii::Buffer fillBuffer { nullptr };
		vma::raii::Buffer iconBuffer { nullptr };
		vma::raii::Buffer indexBuffer { nullptr };
		vma::raii::Buffer cameraBuffer { nullptr };
		vk::raii::DescriptorSet descriptorSet { nullptr };
		vk::DeviceSize wireCapacity = 0;
		vk::DeviceSize fillCapacity = 0;
		vk::DeviceSize iconCapacity = 0;
		vk::DeviceSize indexCapacity = 0;
		std::uint32_t wireDepthFirst = 0;
		std::uint32_t fillDepthFirst = 0;
		std::uint32_t iconDepthFirst = 0;
		std::uint32_t wireDepthCount = 0;
		std::uint32_t fillDepthCount = 0;
	};
	
	struct CameraUniform
	{
		glm::mat4 view;
		glm::mat4 projection;
		float aspectRatio;
	};
	
	struct IconVertex
	{
		glm::vec3 position;
		glm::vec4 color;
		glm::vec2 texCoord;
	};
	
#pragma pack(push, 1)
	/// @brief Metadata required to render a processed Slug glyph.
	///
	/// Describes the glyph's dimensions, band configuration, and
	/// location of its band headers within the shared band texture.
	///
	/// @note Serialized directly into the Slug binary file.
	///       The structure must retain its expected 28-byte layout.
	struct SlugCodePoint
	{
		/// @brief Unicode code point represented by the glyph.
		uint32_t codePoint;
		
		/// @brief Width of the glyph's bounding box in font units.
		uint32_t width;
		
		/// @brief Height of the glyph's bounding box in font units.
		uint32_t height;
		
		/// @brief Number of horizontal and vertical bands per glyph.
		uint32_t bandCount;
		
		/// @brief Width of each vertical band in glyph-local units.
		uint32_t bandDimX;
		
		/// @brief Height of each horizontal band in glyph-local units.
		uint32_t bandDimY;
		
		/// @brief X texel coordinate of the glyph's first band header.
		uint16_t bandsTexCoordX;
		
		/// @brief Y texel coordinate of the glyph's first band header.
		uint16_t bandsTexCoordY;
	};
#pragma pack(pop)
	
	class SlugPrototype
	{
	public:
		void Initialize(const vk::raii::Device &p_device, const vma::raii::Allocator &p_allocator, const vk::raii::DescriptorPool &p_descriptorPool, vk::Format p_colorFormat,
		vk::Format p_depthFormat);
		void Record(vk::raii::CommandBuffer& p_commandBuffer, vk::Extent2D extent);
		
		void RenderToCamera(
			const Camera &p_camera, 
			const vk::raii::CommandBuffer &p_commandBuffer, 
			vk::Extent2D p_extent, 
			std::uint32_t p_frameIndex);
	
		void Shutdown();
	
	private:
		bool LoadFont(const char *p_inputPath);
		
		vk::raii::DescriptorSetLayout m_descriptorSetLayout { nullptr };
		vk::raii::PipelineLayout m_pipelineLayout { nullptr };
		std::array<vk::raii::ShaderModule, 2> m_shaderModules
		{
			vk::raii::ShaderModule { nullptr },
			vk::raii::ShaderModule { nullptr }
		};
		std::vector<FrameResources> m_frameResources {};
		const vk::raii::DescriptorPool *m_descriptorPool = nullptr;
		vk::Format m_colorFormat {};
		vk::Format m_depthFormat {}; 
		vk::raii::Pipeline m_pipeline { nullptr };
		const vk::raii::Device *m_device = nullptr;
		const vma::raii::Allocator *m_allocator = nullptr;
		
		std::vector<SlugCodePoint> m_codePoints;
		std::vector<glm::vec4> m_curvesTexture;
		std::vector<glm::u16vec2> m_bandsTexture; 
	};
}