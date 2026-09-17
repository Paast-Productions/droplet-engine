#include "PipelineStorage.hpp"

using namespace DropletEngine;

void PipelineStorage::Insert(const PipelineStorageKey& p_key, const vk::raii::Pipeline& p_pipeline)
{
    m_pipelineHashMap.insert(std::make_pair<const PipelineStorageKey&, const vk::raii::Pipeline&>(p_key, p_pipeline));
}

vk::raii::Pipeline& PipelineStorage::GetPipeline(const PipelineStorageKey& p_key)
{
    return m_pipelineHashMap.at(p_key);
}
