#pragma once

class PipelineLayout
{
public:
    PipelineLayout() = default;
    PipelineLayout(const PipelineLayout &p_other) = delete;
    PipelineLayout &operator=(const PipelineLayout &p_other) = delete;
    PipelineLayout(PipelineLayout &&p_other) noexcept = delete;
    PipelineLayout &operator=(PipelineLayout &&p_other) noexcept = delete;

    ~PipelineLayout() = default;
 
private:
    
    
};
