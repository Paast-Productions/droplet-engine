#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <Graphics/VK/Context.hpp>

namespace Droplet::Graphics::VK
{
	//TODO: Make sampler modular
	class Sampler
	{
	public:
		Sampler()
		{
			m_sampler = nullptr;
		}

		~Sampler() = default;

		Sampler(nullptr_t p_nullptr)
		{
			m_sampler = { p_nullptr };
		}

		Sampler(const Context &p_context);

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

		[[nodiscard]] const vk::raii::Sampler &Get() { return m_sampler; }

	private:
		vk::raii::Sampler m_sampler { nullptr };
	};
}
