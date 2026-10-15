#pragma once
#include <string>
#include <vector>

namespace Droplet::Editor
{
	/// @brief A utility class for managing UI styles in the editor.
	class UIStyle
	{
	public:
		/// @brief Retrieves the names of all saved styles.
		/// @param styles A reference to a vector that will be populated with the names of saved styles.
		static void GetStyleNames(std::vector<std::string> &styles);

		/// @brief Saves the current UI style with the specified name.
		/// @param name The name under which to save the current style.
		static void SaveStyle(const std::string &name);

		/// @brief Loads a saved UI style with the specified name.
		/// @param name The name of the style to load.
		static void LoadStyle(const std::string &name);
	};
}
