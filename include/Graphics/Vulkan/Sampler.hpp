#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
	//TODO: (RENDERING) Make sampler modular
	class Sampler
	{
	public:
		/// @brief Default maximum anisotropy used for sampling. Clamped to the device limit.
		static constexpr float C_DEFAULT_MAX_ANISOTROPY = 8.0f;

		Sampler()
		{
			m_sampler = nullptr;
		}

		~Sampler() = default;

		Sampler(nullptr_t p_nullptr)
		{
			m_sampler = { p_nullptr };
		}

		/// @brief Constructs a sampler
		/// @param p_physicalDevice Vulkan Physical Device
		/// @param p_device Vulkan Device
		Sampler(const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device);

		Sampler(Sampler &&p_other) noexcept
		{
			m_sampler = std::move(p_other.m_sampler);
		}

		Sampler &operator=(Sampler &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}

			m_sampler = std::move(p_other.m_sampler);

			return *this;
		}

		bool operator==(const Sampler &p_other) const
		{
			return m_sampler == p_other.m_sampler;
		}
		
		/// @brief Getter function for the sampler
		/// @returns Vulkan Sampler
		[[nodiscard]] const vk::raii::Sampler &Get() const { return m_sampler; }

	private:
		vk::raii::Sampler m_sampler { nullptr };
	};
}
