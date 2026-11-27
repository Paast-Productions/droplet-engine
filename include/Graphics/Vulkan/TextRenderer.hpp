// #pragma once
//
// #include <Context.hpp>
// #include <vma/vk_mem_alloc.h>
// #include <unordered_map>
//
// #include "FontResource.hpp"
// #include "Image.hpp"
// #include "Text/GlyphInstance.hpp"
//
// namespace Droplet::Graphics::Vulkan
// {
// 	class TextRenderer
// 	{
// 	public:
// 		void Initialize(
// 			const Context& p_context,
// 			const vma::raii::Allocator& p_allocator,
// 			const CommandPool& p_commandPool,
// 			vk::Format p_colorFormat,
// 			vk::Format p_depthFormat,
// 			uint32_t p_framesInFlight = 2
// 		);
// 		
// 		void BeginFrame(uint32_t p_frameIndex);
// 		
// 		void EnsureFont(GUID p_fontGUID, const FontResource& p_font);
// 		
// 		[[nodiscard]] uint32_t Prepare(
// 			uint32_t p_frameIndex,
// 			GUID p_fontGUID,
// 			const Camera& p_camera,
// 			const std::vector<Text::GlyphInstance>& p_instances,
// 			const glm::mat4& p_model,
// 			const glm::vec4& p_color);
// 		
// 		void Record(
// 			const vk::raii::CommandBuffer& p_commandBuffer,
// 			uint32_t p_frameIndex,
// 			uint32_t p_drawIndex,
// 			vk::Extent2D p_extent) const;
// 		
// 		void Shutdown();
// 		
// 	private:
// 		struct GPUFont
// 		{
// 			std::optional<Image> curves;
// 			std::optional<Image> bands;
// 			std::optional<ImageView> curvesView;
// 			std::optional<ImageView> bandsView;
// 		};
// 		
// 		struct PrepareDraw
// 		{
// 			StorageBuffer glyphBuffer{nullptr};
// 			vma::raii::Buffer cameraBuffer{nullptr};
// 			vk::raii::DescriptorSet descriptorSet{nullptr};
// 			glm::mat4x4 model{1.0f};
// 			glm::vec4 color{1.0f};
// 			uint32_t instanceCount = 0;
// 		};
// 		
// 		static constexpr uint32_t MAX_DRAWS_PER_FRAME = 128;
// 		
// 		const Context* m_context = nullptr;
// 		const vma::raii::Allocator* m_allocator = nullptr;
// 		const CommandPool* m_commandPool = nullptr;
// 		
// 		vk::raii::DescriptorSetLayout m_descriptorSetLayout{nullptr};
// 		vk::raii::PipelineLayout m_pipelineLayout{nullptr};
// 		vk::raii::DescriptorPool m_descriptorPool{nullptr};
// 		vk::raii::ShaderModule m_vertexShader{nullptr};
// 		vk::raii::ShaderModule m_fragmentShader{nullptr};
// 		vk::raii::Pipeline m_pipeline{nullptr};
// 		
// 		std::unordered_map<GUID, GPUFont> m_fonts;
// 		std::vector<std::vector<std::unique_ptr<PrepareDraw>>> m_frameDraws;
// 	};
// }
