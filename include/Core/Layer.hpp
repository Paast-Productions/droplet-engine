#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace Droplet::Core
{
	/// @brief Enum mask representing different layers
	enum class Layer : uint16_t
	{
		None = 0,
		All = 0xFFFF,

		Layer1 = 1 << 0,
		Layer2 = 1 << 1,
		Layer3 = 1 << 2,
		Layer4 = 1 << 3,
		Layer5 = 1 << 4,
		Layer6 = 1 << 5,
		Layer7 = 1 << 6,
		Layer8 = 1 << 7,
		Layer9 = 1 << 8,
		Layer10 = 1 << 9,
		Layer11 = 1 << 10,
		Layer12 = 1 << 11,
		Layer13 = 1 << 12,
		Layer14 = 1 << 13,
		Layer15 = 1 << 14,
		Layer16 = 1 << 15,
	};

	/// @brief Utility class for managing layers.
	class LayerUtility
	{
	public:

		// Layer Helpers

		static inline bool IsLayerEnabled(Layer p_layer, Layer p_layerMask)
		{
			return (static_cast<uint16_t>(p_layer) & static_cast<uint16_t>(p_layerMask)) != 0;
		}
		static inline void EnableLayer(Layer p_layer, Layer &p_layerMask)
		{
			p_layerMask = static_cast<Layer>(static_cast<uint16_t>(p_layerMask) | static_cast<uint16_t>(p_layer));
		}
		static inline void DisableLayer(Layer p_layer, Layer &p_layerMask)
		{
			p_layerMask = static_cast<Layer>(static_cast<uint16_t>(p_layerMask) & ~static_cast<uint16_t>(p_layer));
		}
		static inline void SetLayer(Layer p_layer, Layer &p_layerMask, bool p_enabled)
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
		static inline void ToggleLayer(Layer p_layer, Layer &p_layerMask)
		{
			p_layerMask = static_cast<Layer>(static_cast<uint16_t>(p_layerMask) ^ static_cast<uint16_t>(p_layer));
		}
		static inline bool IsSingleLayer(Layer p_layer)
		{
			return (static_cast<uint16_t>(p_layer) & (static_cast<uint16_t>(p_layer) - 1)) == 0;
		}


		// Layer Naming

		inline const std::string &GetLayerName(Layer p_layer)
		{
			// Ensure only one flag is set in the layer enum value
			assert(
				IsSingleLayer(p_layer) &&
				"p_layer must be a single layer flag, not a layer mask."
			);

			auto it = m_layerNames.find(p_layer);
			if (it != m_layerNames.end())
			{
				return it->second;
			}
			return "Unknown Layer";
		}

		inline void SetLayerName(Layer p_layer, const std::string &p_name)
		{
			// Ensure only one flag is set in the layer enum value
			assert(
				IsSingleLayer(p_layer) &&
				"p_layer must be a single layer flag, not a layer mask."
			);

			m_layerNames[p_layer] = p_name;
		}

		inline std::vector<std::string> GetLayerNames(Layer p_layerMask)
		{
			std::vector<std::string> layerNames;
			for (const auto &pair : m_layerNames)
			{
				if (IsLayerEnabled(pair.first, p_layerMask))
				{
					layerNames.push_back(pair.second);
				}
			}
			return layerNames;
		}

		inline Layer GetLayerFromName(const std::string &p_name)
		{
			for (const auto &pair : m_layerNames)
			{
				if (pair.second == p_name)
				{
					return pair.first;
				}
			}

			return static_cast<Layer>(0); // Return 0 if not found
		}

		inline Layer GetLayerFromNames(const std::vector<std::string> &p_names)
		{
			Layer result = static_cast<Layer>(0);

			for (const auto &name : p_names)
			{
				for (const auto &pair : m_layerNames)
				{
					if (pair.second != name)
					{
						continue;
					}

					result = static_cast<Layer>(static_cast<uint16_t>(result) | static_cast<uint16_t>(pair.first));
					break;
				}
			}

			return result;
		}

		bool EditLayerNamesPopup(bool p_doOpen);


		// General Utilities

		inline size_t GetLayerCount() // Defined by bits in Layer enum
		{
			return sizeof(Layer) * 8ull;
		}

		void SerializeLayerNames(const std::string &p_path)
		{
			InternalSerializeLayerNames(p_path);
		}
		void DeserializeLayerNames(const std::string &p_path)
		{
			InternalDeserializeLayerNames(p_path);
		}

		bool SetLayerMaskUI(const char *label, Layer &p_layerMask);

		bool SetLayerUI(const char *label, Layer &p_layer);


	private:
		std::map<Layer, std::string> m_layerNames{
			{ Layer::Layer1,	"Layer 1" },
			{ Layer::Layer2,	"Layer 2" },
			{ Layer::Layer3,	"Layer 3" },
			{ Layer::Layer4,	"Layer 4" },
			{ Layer::Layer5,	"Layer 5" },
			{ Layer::Layer6,	"Layer 6" },
			{ Layer::Layer7,	"Layer 7" },
			{ Layer::Layer8,	"Layer 8" },
			{ Layer::Layer9,	"Layer 9" },
			{ Layer::Layer10,	"Layer 10" },
			{ Layer::Layer11,	"Layer 11" },
			{ Layer::Layer12,	"Layer 12" },
			{ Layer::Layer13,	"Layer 13" },
			{ Layer::Layer14,	"Layer 14" },
			{ Layer::Layer15,	"Layer 15" },
			{ Layer::Layer16,	"Layer 16" },
		};

		void InternalSerializeLayerNames(const std::string &p_path) const;
		void InternalDeserializeLayerNames(const std::string &p_path);
	};
}