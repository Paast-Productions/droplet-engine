#pragma once
#include <cstdint>
#include <map>
#include <string>

namespace Droplet::Graphics
{
	/// @brief Enum mask representing different draw layers for rendering.
	enum class DrawLayer : uint16_t
	{
		None = 0,
		All = 0xFFFF,

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
		Layer16 = 1 << 15,
	};

	/// @brief Utility class for managing draw layers.
	class DrawLayerUtility
	{
	public:

		// Layer Helpers

		static inline bool IsLayerEnabled(DrawLayer p_layer, DrawLayer p_layerMask)
		{
			return (static_cast<uint16_t>(p_layer) & static_cast<uint16_t>(p_layerMask)) != 0;
		}
		static inline void EnableLayer(DrawLayer p_layer, DrawLayer &p_layerMask)
		{
			p_layerMask = static_cast<DrawLayer>(static_cast<uint16_t>(p_layerMask) | static_cast<uint16_t>(p_layer));
		}
		static inline void DisableLayer(DrawLayer p_layer, DrawLayer &p_layerMask)
		{
			p_layerMask = static_cast<DrawLayer>(static_cast<uint16_t>(p_layerMask) & ~static_cast<uint16_t>(p_layer));
		}
		static inline void SetLayer(DrawLayer p_layer, DrawLayer &p_layerMask, bool p_enabled)
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
		static inline void ToggleLayer(DrawLayer p_layer, DrawLayer &p_layerMask)
		{
			p_layerMask = static_cast<DrawLayer>(static_cast<uint16_t>(p_layerMask) ^ static_cast<uint16_t>(p_layer));
		}
		static inline bool IsSingleLayer(DrawLayer p_layer)
		{
			return (static_cast<uint16_t>(p_layer) & (static_cast<uint16_t>(p_layer) - 1)) == 0;
		}


		// Layer Naming

		static inline const std::string &GetLayerName(DrawLayer p_layer)
		{
			// Ensure only one flag is set in the layer enum value
			assert(
				IsSingleLayer(p_layer),
				"p_layer must be a single layer flag, not a layer mask."
			);

			auto &instance = Get();
			auto it = instance.m_layerNames.find(p_layer);
			if (it != instance.m_layerNames.end())
			{
				return it->second;
			}
			return "Unknown Layer";
		}

		static inline void SetLayerName(DrawLayer p_layer, const std::string &p_name)
		{
			// Ensure only one flag is set in the layer enum value
			assert(
				IsSingleLayer(p_layer),
				"p_layer must be a single layer flag, not a layer mask."
			);

			auto &instance = Get();
			instance.m_layerNames[p_layer] = p_name;
		}

		static inline std::vector<std::string> GetLayerNames(DrawLayer p_layerMask)
		{
			auto &instance = Get();
			std::vector<std::string> layerNames;
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

		static bool EditLayerNamesPopup(bool p_doOpen);


		// General Utilities

		static inline size_t GetLayerCount() // Defined by bits in DrawLayer enum
		{
			return sizeof(DrawLayer) * 8ull;
		}

		static void SerializeLayerNames(const std::string &p_path)
		{
			Get().InternalSerializeLayerNames(p_path);
		}
		static void DeserializeLayerNames(const std::string &p_path)
		{
			Get().InternalDeserializeLayerNames(p_path);
		}

		static bool SetLayerMaskUI(const char *label, DrawLayer &p_layerMask);

		static bool SetLayerUI(const char *label, DrawLayer &p_layer);


	private:
		std::map<DrawLayer, std::string> m_layerNames{
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

		DrawLayerUtility()
		{
			InternalDeserializeLayerNames(ROOT_PATH "/GameData/DrawLayers.json");
		}
		~DrawLayerUtility()
		{
			InternalSerializeLayerNames(ROOT_PATH "/GameData/DrawLayers.json");
		}

		static DrawLayerUtility &Get()
		{
			static DrawLayerUtility instance;
			return instance;
		}

		void InternalSerializeLayerNames(const std::string &p_path) const;
		void InternalDeserializeLayerNames(const std::string &p_path);
	};
}