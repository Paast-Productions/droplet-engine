#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_precision.hpp>

#include "CommandPool.hpp"
#include "Context.hpp"
#include "Image.hpp"
#include "ImageView.hpp"
#include "StorageBuffer.hpp"

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
	
	struct SlugGlyphData
	{
		glm::vec2 glyphScale;
		glm::vec2 bandScale;
		glm::uvec2 bandMax;
		glm::uvec2 bandsTexCoords;
	};
	
	/// @brief Contains the per-glyph data required for instanced text rendering.
	/// @details Stores the glyph's transformation, dimensions, color, and
	/// band metadata used by the Slug rendering shader.
	struct GlyphInstance
	{
		/// @brief Glyph quad scale (xy) and position (zw).
		glm::vec4 scaleBias;

		/// @brief Glyph dimensions (xy) and band scaling factors (zw).
		glm::vec4 glyphBandScale;

		/// @brief RGBA color of the glyph.
		glm::vec4 glyphColor;

		/// @brief Maximum horizontal and vertical band indices (xy),
		/// and starting coordinates in the band texture (zw).
		glm::uvec4 bandMaxTexCoords;
	};
	
// TODO: Move important definitions used by the generator and the renderer into a common space. (Linus)
#pragma pack(push, 1)
	/// @brief Font metadata
	/// 
	/// Contains information that is common for all glyphs within the font.
	/// 
	/// @note Serialized directly into the Slug binary file. Changes require regeneration of .slug files.
	struct SlugFontHeader
	{
		uint16_t unitsPerEm = 0;
		
		uint32_t ascent = 0; 
		uint32_t descent = 0;
		uint32_t lineGap = 0;
	};
#pragma pack(pop)
	
#pragma pack(push, 1)
	/// @brief Metadata required to render a processed Slug glyph.
	///
	/// Describes the glyph's dimensions, band configuration, and
	/// location of its band headers within the shared band texture.
	///
	/// @note Serialized directly into the Slug binary file. Changes require regeneration of .slug files.
	struct SlugCodePoint
	{
		/// @brief Unicode code point represented by the glyph.
		uint32_t codePoint;
		
		/// @brief Width of the glyph's bounding box in font units.
		uint32_t width;
		
		/// @brief Height of the glyph's bounding box in font units.
		uint32_t height;
		
		/// @brief Spacing to the next character.
		int32_t advanceWidth;
		
		/// @brief Minimum x-value of the glyph bounding box.
		int32_t xMin;
		
		/// @brief Minimum y-value of the glyph bounding box.
		int32_t yMin;
		
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
		void Initialize(const Context &p_context, const vma::raii::Allocator &p_allocator, const vk::raii::DescriptorPool &p_descriptorPool, vk::Format p_colorFormat,
		vk::Format p_depthFormat, const CommandPool &p_commandPool);
		void Record(vk::raii::CommandBuffer& p_commandBuffer, vk::Extent2D extent);
		
		void RenderToCamera(
			const Camera &p_camera, 
			const vk::raii::CommandBuffer &p_commandBuffer, 
			vk::Extent2D p_extent, 
			std::uint32_t p_frameIndex);
	
		void Shutdown();
	
	private:
		bool LoadFont(const char *p_inputPath);
		const SlugCodePoint* GetGlyph(uint32_t p_codePoint) const;
		
		// TODO: Consider this. Are we okay with keeping this in the codebase?
		static std::vector<std::uint32_t> DecodeUTF8(std::string_view p_text); // Wholly generated by ChatGPT.
		
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
		
		std::optional<Image> m_curveTexture;
		std::optional<Image> m_bandTexture;
		
		std::optional<ImageView> m_curveTextureView;
		std::optional<ImageView> m_bandTextureView;
		
		std::vector<SlugCodePoint> m_codePoints;
		std::vector<glm::vec4> m_curvesTextureData;
		std::vector<glm::u16vec2> m_bandsTextureData;
		SlugFontHeader m_fontHeader;
		
		StorageBuffer m_storageBuffer {nullptr};
				
		uint16_t m_curveTextureWidth = 0;
		uint16_t m_curveTextureHeight = 0;
		
		uint16_t m_bandsTextureWidth = 0;
		uint16_t m_bandsTextureHeight = 0;
		
		std::string_view m_text = "Droplet Engine!\nÅh ja";
		std::vector<GlyphInstance> m_glyphInstances;
	};
}