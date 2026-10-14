#pragma once

namespace Droplet
{
	class Engine;

	/// @brief EngineProvider is a singleton that provides access to the engine instance.
	class EngineProvider
	{
	public:
		EngineProvider() = delete;

		/// @brief Gets the engine instance.
		/// @return The engine instance.
		[[nodiscard]] static Engine &Get()
		{
			return *s_engine;
		}

	private:
		friend class Engine;

		/// @brief Constructs the EngineProvider with the engine instance.
		/// @param p_engine The engine instance.
		EngineProvider(Engine *p_engine)
		{
			s_engine = p_engine;
		}

		static Engine *s_engine;
	};
}