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
        Unknown = SDL_SCANCODE_UNKNOWN,

        A = SDL_SCANCODE_A,
        B = SDL_SCANCODE_B,
        C = SDL_SCANCODE_C,
        D = SDL_SCANCODE_D,
        E = SDL_SCANCODE_E,
        F = SDL_SCANCODE_F,
        G = SDL_SCANCODE_G,
        H = SDL_SCANCODE_H,
        I = SDL_SCANCODE_I,
        J = SDL_SCANCODE_J,
        K = SDL_SCANCODE_K,
        L = SDL_SCANCODE_L,
        M = SDL_SCANCODE_M,
        N = SDL_SCANCODE_N,
        O = SDL_SCANCODE_O,
        P = SDL_SCANCODE_P,
        Q = SDL_SCANCODE_Q,
        R = SDL_SCANCODE_R,
        S = SDL_SCANCODE_S,
        T = SDL_SCANCODE_T,
        U = SDL_SCANCODE_U,
        V = SDL_SCANCODE_V,
        W = SDL_SCANCODE_W,
        X = SDL_SCANCODE_X,
        Y = SDL_SCANCODE_Y,
        Z = SDL_SCANCODE_Z,

        D1 = SDL_SCANCODE_1,
        D2 = SDL_SCANCODE_2,
        D3 = SDL_SCANCODE_3,
        D4 = SDL_SCANCODE_4,
        D5 = SDL_SCANCODE_5,
        D6 = SDL_SCANCODE_6,
        D7 = SDL_SCANCODE_7,
        D8 = SDL_SCANCODE_8,
        D9 = SDL_SCANCODE_9,
        D0 = SDL_SCANCODE_0,
          
        Return = SDL_SCANCODE_RETURN,
        Escape = SDL_SCANCODE_ESCAPE,
        Backspace = SDL_SCANCODE_BACKSPACE,
        Tab = SDL_SCANCODE_TAB,
        Space = SDL_SCANCODE_SPACE,
          
        Minus = SDL_SCANCODE_MINUS,
        Equals = SDL_SCANCODE_EQUALS,
        LeftBracket = SDL_SCANCODE_LEFTBRACKET,
        RightBracket = SDL_SCANCODE_RIGHTBRACKET,
        Backslash = SDL_SCANCODE_BACKSLASH,
        NonUSHash = SDL_SCANCODE_NONUSHASH,
        Semicolon = SDL_SCANCODE_SEMICOLON,
        Apostrophe = SDL_SCANCODE_APOSTROPHE,
        Grave = SDL_SCANCODE_GRAVE,
        Comma = SDL_SCANCODE_COMMA,
        Period = SDL_SCANCODE_PERIOD,
        Slash = SDL_SCANCODE_SLASH,
          
        CapsLock = SDL_SCANCODE_CAPSLOCK,
          
        F1 = SDL_SCANCODE_F1,
        F2 = SDL_SCANCODE_F2,
        F3 = SDL_SCANCODE_F3,
        F4 = SDL_SCANCODE_F4,
        F5 = SDL_SCANCODE_F5,
        F6 = SDL_SCANCODE_F6,
        F7 = SDL_SCANCODE_F7,
        F8 = SDL_SCANCODE_F8,
        F9 = SDL_SCANCODE_F9,
        F10 = SDL_SCANCODE_F10,
        F11 = SDL_SCANCODE_F11,
        F12 = SDL_SCANCODE_F12,
          
        PrintScreen = SDL_SCANCODE_PRINTSCREEN,
        ScrollLock = SDL_SCANCODE_SCROLLLOCK,
        Pause = SDL_SCANCODE_PAUSE,
        Insert = SDL_SCANCODE_INSERT,
        Home = SDL_SCANCODE_HOME,
        PageUp = SDL_SCANCODE_PAGEUP,
        Delete = SDL_SCANCODE_DELETE,
        End = SDL_SCANCODE_END,
        PageDown = SDL_SCANCODE_PAGEDOWN,
        Right = SDL_SCANCODE_RIGHT,
        Left = SDL_SCANCODE_LEFT,
        Down = SDL_SCANCODE_DOWN,
        Up = SDL_SCANCODE_UP,
          
        NumLock = SDL_SCANCODE_NUMLOCKCLEAR,
        NumDivide = SDL_SCANCODE_KP_DIVIDE,
        NumMultiply = SDL_SCANCODE_KP_MULTIPLY,
        NumMinus = SDL_SCANCODE_KP_MINUS,
        NumPlus = SDL_SCANCODE_KP_PLUS,
        NumEnter = SDL_SCANCODE_KP_ENTER,
        Num1 = SDL_SCANCODE_KP_1,
        Num2 = SDL_SCANCODE_KP_2,
        Num3 = SDL_SCANCODE_KP_3,
        Num4 = SDL_SCANCODE_KP_4,
        Num5 = SDL_SCANCODE_KP_5,
        Num6 = SDL_SCANCODE_KP_6,
        Num7 = SDL_SCANCODE_KP_7,
        Num8 = SDL_SCANCODE_KP_8,
        Num9 = SDL_SCANCODE_KP_9,
        Num0 = SDL_SCANCODE_KP_0,
        NumPeriod = SDL_SCANCODE_KP_PERIOD,
          
        NonUSBackslash = SDL_SCANCODE_NONUSBACKSLASH,
        Application = SDL_SCANCODE_APPLICATION,
        Power = SDL_SCANCODE_POWER,
        NumEquals = SDL_SCANCODE_KP_EQUALS,
        F13 = SDL_SCANCODE_F13,
        F14 = SDL_SCANCODE_F14,
        F15 = SDL_SCANCODE_F15,
        F16 = SDL_SCANCODE_F16,
        F17 = SDL_SCANCODE_F17,
        F18 = SDL_SCANCODE_F18,
        F19 = SDL_SCANCODE_F19,
        F20 = SDL_SCANCODE_F20,
        F21 = SDL_SCANCODE_F21,
        F22 = SDL_SCANCODE_F22,
        F23 = SDL_SCANCODE_F23,
        F24 = SDL_SCANCODE_F24,
        Execute = SDL_SCANCODE_EXECUTE,
        Help = SDL_SCANCODE_HELP,
        Menu = SDL_SCANCODE_MENU,
        Select = SDL_SCANCODE_SELECT,
        Stop = SDL_SCANCODE_STOP,
        Again = SDL_SCANCODE_AGAIN,
        Undo = SDL_SCANCODE_UNDO,
        Cut = SDL_SCANCODE_CUT,
        Copy = SDL_SCANCODE_COPY,
        Paste = SDL_SCANCODE_PASTE,
        Find = SDL_SCANCODE_FIND,
        Mute = SDL_SCANCODE_MUTE,
        VolumeUp = SDL_SCANCODE_VOLUMEUP,
        VolumeDown = SDL_SCANCODE_VOLUMEDOWN,
        KPComma = SDL_SCANCODE_KP_COMMA,
        KPEqualsAS400 = SDL_SCANCODE_KP_EQUALSAS400,
          
        International1 = SDL_SCANCODE_INTERNATIONAL1,
        International2 = SDL_SCANCODE_INTERNATIONAL2,
        International3 = SDL_SCANCODE_INTERNATIONAL3,
        International4 = SDL_SCANCODE_INTERNATIONAL4,
        International5 = SDL_SCANCODE_INTERNATIONAL5,
        International6 = SDL_SCANCODE_INTERNATIONAL6,
        International7 = SDL_SCANCODE_INTERNATIONAL7,
        International8 = SDL_SCANCODE_INTERNATIONAL8,
        International9 = SDL_SCANCODE_INTERNATIONAL9,

        Lang1 = SDL_SCANCODE_LANG1,
        Lang2 = SDL_SCANCODE_LANG2,
        Lang3 = SDL_SCANCODE_LANG3,
        Lang4 = SDL_SCANCODE_LANG4,
        Lang5 = SDL_SCANCODE_LANG5,
        Lang6 = SDL_SCANCODE_LANG6,
        Lang7 = SDL_SCANCODE_LANG7,
        Lang8 = SDL_SCANCODE_LANG8,
        Lang9 = SDL_SCANCODE_LANG9,
          
        AltErase = SDL_SCANCODE_ALTERASE,
        SysReq = SDL_SCANCODE_SYSREQ,
        Cancel = SDL_SCANCODE_CANCEL,
        Clear = SDL_SCANCODE_CLEAR,
        Prior = SDL_SCANCODE_PRIOR,
        Return2 = SDL_SCANCODE_RETURN2,
        Separator = SDL_SCANCODE_SEPARATOR,
        Out = SDL_SCANCODE_OUT,
        Oper = SDL_SCANCODE_OPER,
        ClearAgain = SDL_SCANCODE_CLEARAGAIN,
        CrSel = SDL_SCANCODE_CRSEL,
        ExSel = SDL_SCANCODE_EXSEL,
          
        KP00 = SDL_SCANCODE_KP_00,
        KP000 = SDL_SCANCODE_KP_000,
        ThousandsSeparator = SDL_SCANCODE_THOUSANDSSEPARATOR,
        DecimalSeparator = SDL_SCANCODE_DECIMALSEPARATOR,
        CurrencyUnit = SDL_SCANCODE_CURRENCYUNIT,
        CurrencySubUnit = SDL_SCANCODE_CURRENCYSUBUNIT,
        KPLeftParen = SDL_SCANCODE_KP_LEFTPAREN,
        KPRightParen = SDL_SCANCODE_KP_RIGHTPAREN,
        KPLeftBrace = SDL_SCANCODE_KP_LEFTBRACE,
        KPRightBrace = SDL_SCANCODE_KP_RIGHTBRACE,
        KPTab = SDL_SCANCODE_KP_TAB,
        KPBackspace = SDL_SCANCODE_KP_BACKSPACE,
        KPA = SDL_SCANCODE_KP_A,
        KPB = SDL_SCANCODE_KP_B,
        KPC = SDL_SCANCODE_KP_C,
        KPD = SDL_SCANCODE_KP_D,
        KPE = SDL_SCANCODE_KP_E,
        KPF = SDL_SCANCODE_KP_F,
        KPXOR = SDL_SCANCODE_KP_XOR,
        KPPower = SDL_SCANCODE_KP_POWER,
        KPPercent = SDL_SCANCODE_KP_PERCENT,
        KPLess = SDL_SCANCODE_KP_LESS,
        KPGreater = SDL_SCANCODE_KP_GREATER,
        KPAmperstand = SDL_SCANCODE_KP_AMPERSAND,
        KPDblAmperstand = SDL_SCANCODE_KP_DBLAMPERSAND,
        KPVerticalBar = SDL_SCANCODE_KP_VERTICALBAR,
        KPDblVerticalBar = SDL_SCANCODE_KP_DBLVERTICALBAR,
        KPColon = SDL_SCANCODE_KP_COLON,
        KPHash = SDL_SCANCODE_KP_HASH,
        KPSpace = SDL_SCANCODE_KP_SPACE,
        KPAt = SDL_SCANCODE_KP_AT,
        KPExclam = SDL_SCANCODE_KP_EXCLAM,
        KPMemStore = SDL_SCANCODE_KP_MEMSTORE,
        KPMemRecall = SDL_SCANCODE_KP_MEMRECALL,
        KPMemClear = SDL_SCANCODE_KP_MEMCLEAR,
        KPMemAdd = SDL_SCANCODE_KP_MEMADD,
        KPMemSubtract = SDL_SCANCODE_KP_MEMSUBTRACT,
        KPMemMultiply = SDL_SCANCODE_KP_MEMMULTIPLY,
        KPMemDivide = SDL_SCANCODE_KP_MEMDIVIDE,
        KPPlusMinus = SDL_SCANCODE_KP_PLUSMINUS,
        KPClear = SDL_SCANCODE_KP_CLEAR,
        KPClearEntry = SDL_SCANCODE_KP_CLEARENTRY,
        KPBinary = SDL_SCANCODE_KP_BINARY,
        KPOctal = SDL_SCANCODE_KP_OCTAL,
        KPDecimal = SDL_SCANCODE_KP_DECIMAL,
        KPHexadecimal = SDL_SCANCODE_KP_HEXADECIMAL,
          
        LCtrl = SDL_SCANCODE_LCTRL,
        LShift = SDL_SCANCODE_LSHIFT,
        LAlt = SDL_SCANCODE_LALT,
        LGUI = SDL_SCANCODE_LGUI,
        RCtrl = SDL_SCANCODE_RCTRL,
        RShift = SDL_SCANCODE_RSHIFT,
        RAlt = SDL_SCANCODE_RALT,
        RGUI = SDL_SCANCODE_RGUI,
          
        Mode = SDL_SCANCODE_MODE,
          
        Sleep = SDL_SCANCODE_SLEEP,
        Wake = SDL_SCANCODE_WAKE,
          
        ChannelIncrement = SDL_SCANCODE_CHANNEL_INCREMENT,
        ChannelDecrement = SDL_SCANCODE_CHANNEL_DECREMENT,
          
        MediaPlay = SDL_SCANCODE_MEDIA_PLAY,         
        MediaPause = SDL_SCANCODE_MEDIA_PAUSE,        
        MediaRecord = SDL_SCANCODE_MEDIA_RECORD,   
        MediaFastForward = SDL_SCANCODE_MEDIA_FAST_FORWARD,
        MediaRewind = SDL_SCANCODE_MEDIA_REWIND,
        MediaNextTrack = SDL_SCANCODE_MEDIA_NEXT_TRACK,
        MediaPreviousTrack = SDL_SCANCODE_MEDIA_PREVIOUS_TRACK,
        MediaStop = SDL_SCANCODE_MEDIA_STOP,      
        MediaEject = SDL_SCANCODE_MEDIA_EJECT,   
        MediaPlayPause = SDL_SCANCODE_MEDIA_PLAY_PAUSE,
        MediaSelect = SDL_SCANCODE_MEDIA_SELECT,  
          
        ACNew = SDL_SCANCODE_AC_NEW,         
        ACOpen = SDL_SCANCODE_AC_OPEN,    
        ACClose = SDL_SCANCODE_AC_CLOSE,     
        ACExit = SDL_SCANCODE_AC_EXIT,      
        ACSave = SDL_SCANCODE_AC_SAVE,       
        ACPrint = SDL_SCANCODE_AC_PRINT,            
        ACProperties = SDL_SCANCODE_AC_PROPERTIES,       
          
        ACSearch = SDL_SCANCODE_AC_SEARCH,           
        ACHome = SDL_SCANCODE_AC_HOME,             
        ACBack = SDL_SCANCODE_AC_BACK,             
        ACForward = SDL_SCANCODE_AC_FORWARD,          
        ACStop = SDL_SCANCODE_AC_STOP,             
        ACRefresh = SDL_SCANCODE_AC_REFRESH,          
        ACBookmarks = SDL_SCANCODE_AC_BOOKMARKS,
         
        SoftLeft = SDL_SCANCODE_SOFTLEFT,
        SoftRight = SDL_SCANCODE_SOFTRIGHT,
        Call = SDL_SCANCODE_CALL,
        EndCall = SDL_SCANCODE_ENDCALL,
        Reserved = SDL_SCANCODE_RESERVED,

        Count = SDL_SCANCODE_COUNT,
	};

	/// @brief Customized mouse codes using ...
	enum class Mouse : std::uint8_t
	{
		LMB = 0,
		RMB = 1,

		// TODO: Add the rest of the mouse buttons
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
		/// @param p_mainWindow The pointer to the SDL window instance.
		void Update(SDL_Window *&p_mainWindow);

		/// @brief Allows the internal state to register event changes.
		/// @param p_mainWindow The pointer to the SDL window instance.
		void UpdatePostEvents(SDL_Window *&p_mainWindow);

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
		/// @param p_relativeMouse Whether the cursor position is relative to the window or absolute on the screen.
		/// @param p_x The new cursor x position.
		/// @param p_y The new cursor y position.
		/// @param p_teleport Ensures that mouse delta is not affected by the movement.
		/// @note SDL3 only wants this to be called in the main thread.
		void SetCursorPosition(SDL_Window* &p_window, bool p_relativeMouse, float p_x = 0.0f, float p_y = 0.0f, bool p_teleport = true);
		
		/// @brief Function to queue up a cursor movement at the next GameInput update.
		/// @param p_newScreenPos The new screen position to move the cursor to.
		/// @param p_windowRelative Whether the new position is relative to the window or absolute on the screen.
		/// @param p_teleport Ensures that mouse delta is not affected by the movement.
		void MoveCursor(glm::vec2 p_newPos, bool p_windowRelative, bool p_teleport = true);

		/// @brief Function to hide or show cursor
		/// @param p_active Bool that when true hide cursor and show when false
		void HideCursor(bool p_active);

		/// @brief Function to lock the cursor to the center of the window
		/// @param p_window The pointer to the SDL window instance. Set to nullptr to unlock the cursor.
		void LockCursor(SDL_Window *p_window);

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
		float m_deltaMouseX = 0.0f;
		float m_deltaMouseY = 0.0f;
		std::array<bool, 2> m_currentMouse = { false };
		std::array<bool, 2> m_previousMouse = { false };

		glm::vec2 m_queuedCursorPos{ 0.0f, 0.0f };
		bool m_queuedCursorPosRelative = false;
		bool m_queuedCursorPosTeleport = false;
		bool m_hasQueuedCursorPos = false;
	};
}