#pragma once
#include <Core/Layer.hpp>
#include <cstdint>
#include <map>
#include <string>

namespace Droplet::Graphics
{
	typedef Core::Layer DrawLayer;

	/// @brief Utility class for managing draw layers.
	class DrawLayerUtility
	{
	private:
		Core::LayerUtility m_layerUtility;
		std::string m_path = ROOT_PATH "/GameData/DrawLayers.json";

		DrawLayerUtility()
		{
			m_layerUtility.DeserializeLayerNames(m_path);
		}
		~DrawLayerUtility()
		{
			m_layerUtility.SerializeLayerNames(m_path);
		}

		static DrawLayerUtility &Get()
		{
			static DrawLayerUtility instance;
			return instance;
		}

	public:

		// Layer Naming

		static inline const std::string &GetLayerName(DrawLayer p_layer)
		{
			return Get().m_layerUtility.GetLayerName(p_layer);
		}

		static inline void SetLayerName(DrawLayer p_layer, const std::string &p_name)
		{
			Get().m_layerUtility.SetLayerName(p_layer, p_name);
		}

		static inline std::vector<std::string> GetLayerNames(DrawLayer p_layerMask)
		{
			return Get().m_layerUtility.GetLayerNames(p_layerMask);
		}

		static inline DrawLayer GetLayerFromName(const std::string &p_name)
		{
			return Get().m_layerUtility.GetLayerFromName(p_name);
		}

		static inline DrawLayer GetLayerFromNames(const std::vector<std::string> &p_names)
		{
			return Get().m_layerUtility.GetLayerFromNames(p_names);
		}

		static bool EditLayerNamesPopup(bool p_doOpen)
		{
			return Get().m_layerUtility.EditLayerNamesPopup(p_doOpen);
		}


		// General Utilities

		static inline size_t GetLayerCount()
		{
			return Get().m_layerUtility.GetLayerCount();
		}

		void SerializeLayerNames() const
		{
			Get().m_layerUtility.SerializeLayerNames(m_path);
		}
		void DeserializeLayerNames() const
		{
			Get().m_layerUtility.DeserializeLayerNames(m_path);
		}

		static bool SetLayerMaskUI(const char *label, DrawLayer &p_layerMask)
		{
			return Get().m_layerUtility.SetLayerMaskUI(label, p_layerMask);
		}

		static bool SetLayerUI(const char *label, DrawLayer &p_layer)
		{
			return Get().m_layerUtility.SetLayerUI(label, p_layer);
		}
	};
}