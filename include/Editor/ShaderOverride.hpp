#pragma once

namespace Droplet::Editor::Graphics
{
	/// @brief FragmentFlag is used to override the fragment shaders while in Editor mode.
	/// @details Only one shader can override each type at a time
	/// The shaders are simple and should Input/Output matching values to avoid validation errors.
	enum class FragmentFlag : std::uint32_t
	{
		None = 0,		// No flags set -> No shader overriding
		Basic = 1,		// Render objects with simple shading -> BaseFS
		Color = 2,		// Render only color
		Normal = 3,		// Render normals as color
		UV = 4,			// Render UV coordinates as color
		Depth = 5,		// Render depth from camera as color // Not implemented
		Light = 6		// Render only light brightness		 // Not implemented
	};

	/// @brief VertexFlag is used to override the fragment shaders while in Editor mode.
	/// @details Only one shader can override each type at a time
	/// The shaders are simple and should Input/Output matching values to avoid validation errors
	enum class VertexFlag : std::uint32_t
	{
		None = 0,		// No flags set -> No shader overriding
		Basic = 1		// Render objects with basic vertex shader -> BaseVS 
	};

	class ShaderOverride
	{
	public:
		ShaderOverride() = default;
		~ShaderOverride() = default;

		static void SetVertexFlag(VertexFlag p_vFlag) { m_vFlag = p_vFlag; }
		static void SetFragmentFlag(FragmentFlag p_fFlag) { m_fFlag = p_fFlag; }

		[[nodiscard]] static VertexFlag GetVertexFlag() { return m_vFlag; }
		[[nodiscard]] static FragmentFlag GetFragmentFlag() { return m_fFlag;  }

	private:
		static VertexFlag m_vFlag;
		static FragmentFlag m_fFlag;
	};
}
