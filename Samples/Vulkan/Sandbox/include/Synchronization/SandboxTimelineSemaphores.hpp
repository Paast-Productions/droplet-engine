#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_NO_UNION_CONSTRUCTORS 
#include <thread>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_UNION_CONSTRUCTORS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <Time.hpp>

namespace Droplet::Sandbox::Vulkan::Sync
{
    class TimelineSemaphore
    {
    public:
        static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

        /// resources for the graphics worker thread
        struct GraphicsResources
        {
            vk::raii::Queue queue{nullptr};
            vk::raii::CommandPool commandPool{nullptr};
            vk::raii::CommandBuffer commandBuffer{nullptr};
            vk::raii::PipelineLayout pipelineLayout{nullptr};
            vk::raii::Pipeline pipeline{nullptr};

            uint32_t queueFamilyIndex;
        } m_graphicsResources;

        /// resources for the compute worker thread
        struct ComputeResources
        {
            vk::raii::Queue queue{nullptr};
            vk::raii::CommandPool commandPool{nullptr};
            vk::raii::CommandBuffer commandBuffer{nullptr};
            vk::raii::PipelineLayout pipelineLayout{nullptr};
            vk::raii::Pipeline initPipeline{nullptr};
            vk::raii::Pipeline updatePipeline{nullptr};
            vk::raii::Pipeline mutatePipeline{nullptr};

            Time time;
            uint32_t queueFamilyIndex;
        } m_computeResources;

        struct SharedResources
        {
            vk::raii::DescriptorSetLayout storageLayout{nullptr};
            vk::raii::DescriptorSetLayout sampledLayout{nullptr};
            std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT> storageDescriptorSets;
            std::array<vk::raii::DescriptorSet, MAX_FRAMES_IN_FLIGHT> sampledDescriptorSets;
            vk::raii::DescriptorPool descriptorPool{nullptr};
            
            std::unique_ptr<vk::raii::Sampler> immutableSampler{nullptr};
            std::array<std::unique_ptr<vk::raii::Image>, MAX_FRAMES_IN_FLIGHT> images;
            std::array<std::unique_ptr<vk::raii::ImageView>, MAX_FRAMES_IN_FLIGHT> imageViews;
        } m_shared;

        struct Timeline
        {
            // suggestions about picking values for stages
            // https://docs.vulkan.org/tutorial/latest/Synchronization/Timeline_Semaphores/03_monotonic_counter.html
            enum Stages
            {
                submit = 1,
                draw,
                present,
                MAX_STAGES
            };

            vk::raii::Semaphore semaphore{nullptr};
            uint64_t frame;
        };

        struct TimelineWorker
        {
            std::thread thread;
            std::atomic_bool alive;
        };
        
    public:
        TimelineWorker m_graphicsWorker, m_computeWorker;
        
        TimelineSemaphore();
        ~TimelineSemaphore();
        
        void SetupSharedResources();
        void BuildCommandBuffers();
        
        // timeline operations
        void CreateTimelineSemaphores();
        void StartTimelineWorkers();
        void FinishTimelineWorkers();
        void SignalTimeline(Timeline::Stages stage);
        void WaitOnTimeline(Timeline::Stages stage);
        void SignalNextFrame();
        void WaitForNextFrame(const uint64_t nextFrame);
        uint64_t GetTimelineStageValue(const Timeline::Stages stage);

        // compute work
        void DoComputeWork();
        void SetupComputeTimeline();
        void SetupComputePipeline();
        void SetupGameOfLife();
        void BuildComputeCommandBuffers(const float elapsed = 0.0f);
        
        // graphics work
        void DoGraphicsWork();
        void SetupGraphicsResources();
        void SetupGraphicsPipeline();
        void SetupGraphicsCommandBuffer();
        
        void Prepare();
        void Render();
        
        static vk::raii::Semaphore CreateTimelineSemaphore(const vk::raii::Device& device)
        {
            auto typeCreateInfo = vk::SemaphoreTypeCreateInfo{
                .semaphoreType = vk::SemaphoreType::eTimeline,
                .initialValue = 0
            };

            auto createInfo = vk::SemaphoreCreateInfo{
                .pNext = &typeCreateInfo
            };

            return vk::raii::Semaphore(device, createInfo);
        }
    };
}
