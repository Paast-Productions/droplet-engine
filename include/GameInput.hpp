#pragma once

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <cstdint>

namespace Droplet
{
	/// @brief Customized keycodes using SDL_Scancode
	enum class Key : std::uint32_t
	{
		KeyA = SDL_SCANCODE_A,
		KeyB = SDL_SCANCODE_B,
		KeyC = SDL_SCANCODE_C,
		KeyD = SDL_SCANCODE_D,
		KeyE = SDL_SCANCODE_E,
		KeyF = SDL_SCANCODE_F,
		KeyG = SDL_SCANCODE_G,
		KeyH = SDL_SCANCODE_H,
		KeyI = SDL_SCANCODE_I,
		KeyJ = SDL_SCANCODE_J,
		KeyK = SDL_SCANCODE_K,
		KeyL = SDL_SCANCODE_L,
		KeyM = SDL_SCANCODE_M,
		KeyN = SDL_SCANCODE_N,
		KeyO = SDL_SCANCODE_O,
		KeyP = SDL_SCANCODE_P,
		KeyQ = SDL_SCANCODE_Q,
		KeyR = SDL_SCANCODE_R,
		KeyS = SDL_SCANCODE_S,
		KeyT = SDL_SCANCODE_T,
		KeyU = SDL_SCANCODE_U,
		KeyV = SDL_SCANCODE_V,
		KeyW = SDL_SCANCODE_W,
		KeyX = SDL_SCANCODE_X,
		KeyY = SDL_SCANCODE_Y,
		KeyZ = SDL_SCANCODE_Z,

		Key1 = SDL_SCANCODE_1,
		Key2 = SDL_SCANCODE_2,
		Key3 = SDL_SCANCODE_3,
		Key4 = SDL_SCANCODE_4,
		Key5 = SDL_SCANCODE_5,
		Key6 = SDL_SCANCODE_6,
		Key7 = SDL_SCANCODE_7,
		Key8 = SDL_SCANCODE_8,
		Key9 = SDL_SCANCODE_9,
		Key0 = SDL_SCANCODE_0,

		KeyEnter = SDL_SCANCODE_RETURN,
		KeyEscape = SDL_SCANCODE_ESCAPE,
		KeyBackspace = SDL_SCANCODE_BACKSPACE,
		KeyTab = SDL_SCANCODE_TAB,
		KeySpace = SDL_SCANCODE_SPACE,

		KeyF1 = SDL_SCANCODE_F1,
		KeyF2 = SDL_SCANCODE_F2,
		KeyF3 = SDL_SCANCODE_F3,
		KeyF4 = SDL_SCANCODE_F4,
		KeyF5 = SDL_SCANCODE_F5,
		KeyF6 = SDL_SCANCODE_F6,
		KeyF7 = SDL_SCANCODE_F7,
		KeyF8 = SDL_SCANCODE_F8,
		KeyF9 = SDL_SCANCODE_F9,
		KeyF10 = SDL_SCANCODE_F10,
		KeyF11 = SDL_SCANCODE_F11,
		KeyF12 = SDL_SCANCODE_F12,

		KeyLCTRL = SDL_SCANCODE_LCTRL,
		KeyRCTRL = SDL_SCANCODE_RCTRL,
		KeyShift = SDL_SCANCODE_LSHIFT
	};

	/// @brief Customized mouse codes using ...
	enum class Mouse : std::uint8_t
	{
		LMB = 0,
		RMB = 1,
	};

	/// @brief A universal singleton GameInput class
	class GameInput
	{
	public:
		GameInput() = default;
		~GameInput() = default;

		/// @brief Retrieves the static instance of the GameInput.
		/// @return Reference to the GameInput instance.
		[[nodiscard]] static GameInput &Get()
		{
			static GameInput gameInput;
			return gameInput;
		}

		/// @brief Updates the internal mouse and key states.
		void Update();

		/// @brief Process SDL events from the main loop
		/// @param The event that was created in main
		void ProcessEvent(const SDL_Event &p_event);

		/// @brief Check whether a key is pressed.
		/// @param p_key The key to be checked.
		/// @return True if key was pressed current frame, else false.
		[[nodiscard]] bool KeyPressed(const Key p_key);

		/// @brief Check whether a key is held.
		/// @param p_key The key to be checked.
		/// @returns True if key is held, else false.
		[[nodiscard]] bool KeyHeld(const Key p_key);

		/// @brief Check whether a key is released.
		/// @param p_key The key to be checked.
		/// @returns True if key was released current frame, else false.
		[[nodiscard]] bool KeyReleased(const Key p_key);

		/// @brief Toggles a key.
		/// @param p_key The key to be toggled.
		/// @returns True or false, depending on the key state. Returns false by default.
		[[nodiscard]] bool KeyToggle(const Key p_key);

		/// @brief Check whether a mouse button is pressed.
		/// @param p_mouse The mouse button to be checked.
		/// @returns True if mouse button was pressed current frame, else false.
		[[nodiscard]] bool MousePressed(const Mouse p_mouse);

		/// @brief Check whether a mouse button is held.
		/// @param p_mouse The mouse button to be checked.
		/// @returns True if mouse button is held, else false.
		[[nodiscard]] bool MouseHeld(const Mouse p_mouse);

		/// @brief Check whether a mouse button is released.
		/// @param p_mouse The mouse button to be checked.
		/// @returns True if mouse button was released current frame, else false.
		[[nodiscard]] bool MouseReleased(const Mouse p_mouse);

		/// @brief Get the current cursor x position.
		/// @returns The cursor x position.
		[[nodiscard]] float GetCursorX() const;

		/// @brief Get the current cursor y position.
		/// @returns The cursor y position.
		[[nodiscard]] float GetCursorY() const;

		/// @brief Get the delta mouse x.
		/// @returns The cursor x delta.
		[[nodiscard]] float GetDeltaMouseX() const;

		/// @brief Get the delta mouse y.
		/// @returns The cursor y delta.
		[[nodiscard]] float GetDeltaMouseY() const;

		/// @brief Set the cursor position.
		/// @param p_window The pointer to the SDL window instance.
		/// @param p_x The new cursor x position.
		/// @param p_y The new cursor y position.
		/// @note SDL3 only wants this to be called in the main thread.
		void SetCursorPosition(SDL_Window *&p_window, float p_x = 0.0f, float p_y = 0.0f); // Assuming (0.0, 0.0) is center of window

	private:
		// Keyboard variables
		std::unordered_set<Key> m_currentKeys{};
		std::unordered_set<Key> m_previousKeys{};
		std::unordered_map<Key, bool> m_toggledKeys{};

		// Mouse variables
		float m_mouseX = 0.0f;
		float m_mouseY = 0.0f;
		float m_prevMouseX = 0.0f;
		float m_prevMouseY = 0.0f;
		std::array<bool, 2> m_currentMouse = { false };
		std::array<bool, 2> m_previousMouse = { false };

	};
}