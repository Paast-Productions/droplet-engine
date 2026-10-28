#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>

namespace Droplet::Graphics
{
	/// @brief Enum mask representing different draw layers for rendering.
	enum class DrawLayer : uint16_t
	{
		Layer1	= 1 << 0,
		Layer2	= 1 << 1,
		Layer3	= 1 << 2,
		Layer4	= 1 << 3,
		Layer5	= 1 << 4,
		Layer6	= 1 << 5,
		Layer7	= 1 << 6,
		Layer8	= 1 << 7,
		Layer9	= 1 << 8,
		Layer10 = 1 << 9,
		Layer11 = 1 << 10,
		Layer12 = 1 << 11,
		Layer13 = 1 << 12,
		Layer14 = 1 << 13,
		Layer15 = 1 << 14,
		Layer16 = 1 << 15
	};

	/// @brief Utility class for managing draw layers.
	class DrawLayerUtility
	{
	public:

		// Layer Helpers

		static inline bool IsLayerEnabled(DrawLayer p_layer, uint16_t p_layerMask)
		{
			return (static_cast<uint16_t>(p_layer) & p_layerMask) != 0;
		}
		static inline void EnableLayer(DrawLayer p_layer, uint16_t &p_layerMask)
		{
			p_layerMask |= static_cast<uint16_t>(p_layer);
		}
		static inline void DisableLayer(DrawLayer p_layer, uint16_t &p_layerMask)
		{
			p_layerMask &= ~static_cast<uint16_t>(p_layer);
		}
		static inline void SetLayer(DrawLayer p_layer, uint16_t &p_layerMask, bool p_enabled)
		{
			if (p_enabled)
			{
				EnableLayer(p_layer, p_layerMask);
			}
			else
			{
				DisableLayer(p_layer, p_layerMask);
			}
		}
		static inline void ToggleLayer(DrawLayer p_layer, uint16_t &p_layerMask)
		{
			p_layerMask ^= static_cast<uint16_t>(p_layer);
		}


		// Layer Naming

		static inline const std::string &GetLayerName(DrawLayer p_layer)
		{
			// Ensure only one flag is set in the layer enum value
			static_assert(
				(static_cast<uint16_t>(DrawLayer::Layer1) & (static_cast<uint16_t>(DrawLayer::Layer1) - 1)) == 0, 
				"layer"
			);

			auto &instance = Get();
			auto it = instance.m_layerNames.find(layer);
			if (it != instance.m_layerNames.end())
			{
				return it->second;
			}
			return "Unknown Layer";
		}

		static inline void SetLayerName(DrawLayer p_layer, const std::string &p_name)
		{
			auto &instance = Get();
			instance.m_layerNames[p_layer] = p_name;
		}

		static inline std::vector<std::string> &GetLayerNames(DrawLayer p_layerMask)
		{
			auto &instance = Get();
			static std::vector<std::string> layerNames;
			layerNames.clear();

			for (const auto &pair : instance.m_layerNames)
			{
				if (IsLayerEnabled(pair.first, p_layerMask))
				{
					layerNames.push_back(pair.second);
				}
			}
			return layerNames;
		}

		static inline DrawLayer GetLayerFromName(const std::string &p_name)
		{
			auto &instance = Get();

			for (const auto &pair : instance.m_layerNames)
			{
				if (pair.second == p_name)
				{
					return pair.first;
				}
			}

			return static_cast<DrawLayer>(0); // Return 0 if not found
		}

		static inline DrawLayer GetLayerFromNames(const std::vector<std::string> &p_names)
		{
			auto &instance = Get();
			DrawLayer result = static_cast<DrawLayer>(0);

			for (const auto &name : p_names)
			{
				for (const auto &pair : instance.m_layerNames)
				{
					if (pair.second != name)
					{
						continue;
					}

					result = static_cast<DrawLayer>(static_cast<uint16_t>(result) | static_cast<uint16_t>(pair.first));
					break;
				}
			}

			return result;
		}


		// General Utilities

		static inline size_t GetLayerCount() // Defined by bits in DrawLayer enum
		{
			return sizeof(DrawLayer) * 8ull;
		}

		static inline void SerializeLayerNames(const std::string &p_path);
		static inline void DeserializeLayerNames(const std::string &p_path);


	private:
		DrawLayerUtility() = default;

		static DrawLayerUtility &Get()
		{
			static DrawLayerUtility instance;
			return instance;
		}

		std::unordered_map<DrawLayer, std::string> m_layerNames{
			{ DrawLayer::Layer1,	"Layer 1" },
			{ DrawLayer::Layer2,	"Layer 2" },
			{ DrawLayer::Layer3,	"Layer 3" },
			{ DrawLayer::Layer4,	"Layer 4" },
			{ DrawLayer::Layer5,	"Layer 5" },
			{ DrawLayer::Layer6,	"Layer 6" },
			{ DrawLayer::Layer7,	"Layer 7" },
			{ DrawLayer::Layer8,	"Layer 8" },
			{ DrawLayer::Layer9,	"Layer 9" },
			{ DrawLayer::Layer10,	"Layer 10" },
			{ DrawLayer::Layer11,	"Layer 11" },
			{ DrawLayer::Layer12,	"Layer 12" },
			{ DrawLayer::Layer13,	"Layer 13" },
			{ DrawLayer::Layer14,	"Layer 14" },
			{ DrawLayer::Layer15,	"Layer 15" },
			{ DrawLayer::Layer16,	"Layer 16" },
		};
	};
}