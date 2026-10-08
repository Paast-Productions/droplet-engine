#pragma once

namespace Droplet
{
	/// @brief UpdateFlags are used to control the execution of the frame loop.
	/// @details Certain functionality may be skipped or modified in case the default frame loop logic is not desired.
	/// For example, node + behaviour logic and automatic rendering should be disabled in the Editor.
	enum class UpdateFlags : std::uint32_t
	{
		None = 0,		// No flags set
		SkipNodeLogic = 1 << 0,	// Skip node logic
		SkipBehaviourLogic = 1 << 1,	// Skip behaviour logic
		SkipScriptLogic = 1 << 2,	// Skip script logic
		SkipAutoRender = 1 << 3,	// Skip implicitly rendering cameras in active scenes. Cameras must be submitted to the renderer manually.
		// Add more flags as needed

		EditorFlags = SkipNodeLogic | SkipBehaviourLogic | SkipScriptLogic | SkipAutoRender,
		DefaultFlags = None
	};

	class EngineFlagsOwner
	{
	public:

		/// @brief Gets the update flags for the engine.
		/// @return The current update flags.
		[[nodiscard]] static UpdateFlags GetUpdateFlags() { return GetInstance().m_updateFlags; }

		/// @brief Sets the update flags for the engine.
		/// @param p_flags The update flags to set.
		static void SetUpdateFlags(UpdateFlags p_flags) { GetInstance().m_updateFlags = p_flags; }

		/// @brief Checks if a specific update flag bit is set.
		/// @param p_flag The update flag bit to check.
		/// @return True if the flag is set, false otherwise.
		[[nodiscard]] static bool IsUpdateFlagSet(UpdateFlags p_flag) 
		{ 
			return (static_cast<std::uint32_t>(GetInstance().m_updateFlags) & static_cast<std::uint32_t>(p_flag)) != 0;
		}

		/// @brief Sets or clears a specific update flag bit.
		/// @param p_flag The update flag bit to set or clear.
		/// @param p_value True to set the flag, false to clear it.
		static void SetUpdateFlagBits(UpdateFlags p_flags, bool p_value)
		{
			EngineFlagsOwner &instance = GetInstance();

			if (p_value)
			{
				// Set the specified flag bits to 1 using bitwise OR
				instance.m_updateFlags = static_cast<UpdateFlags>(
					static_cast<std::uint32_t>(instance.m_updateFlags) |
					static_cast<std::uint32_t>(p_flags)
					);
			}
			else
			{
				// Clear the specified flag bits to 0 using bitwise AND NOT
				instance.m_updateFlags = static_cast<UpdateFlags>(
					static_cast<std::uint32_t>(instance.m_updateFlags) &
					~static_cast<std::uint32_t>(p_flags)
					);
			}
		}

	private:
		UpdateFlags m_updateFlags{ UpdateFlags::DefaultFlags };

		[[nodiscard]] static EngineFlagsOwner &GetInstance()
		{
			static EngineFlagsOwner instance;
			return instance;
		}
	};
}