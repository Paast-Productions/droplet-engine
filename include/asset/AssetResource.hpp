#pragma once


namespace engine
{
	enum class AssetType
	{
		Texture2D,
		Texture3D,
		TextureCube,
		TextureAtlas,
		Mesh,
		SkinnedMesh,
		Animation,
		Shader,
		Font,
		Material,
	};

	/// @brief Interface for all resources that can be loaded by the AssetManager. All resources must implement this interface.
	class IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Abstract class for all texture resources.
	class TextureResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for 2D texture resources.
	class Texture2DResource : public TextureResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for 3D texture resources.
	class Texture3DResource : public TextureResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for cube texture resources.
	class TextureCubeResource : public TextureResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for atlas texture resources.
	class TextureAtlasResource : public TextureResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Abstract class for all mesh resources.
	class MeshBaseResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for mesh resources.
	class MeshResource : public MeshBaseResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for skinned mesh resources.
	class SkinnedMeshResource : public MeshBaseResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for animation resources.
	class AnimationResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for shader resources.
	class ShaderResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for font resources.
	class FontResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

	/// @brief Class for material resources.
	class MaterialResource : public IResource
	{
		// TODO

	public:

	protected:

	private:

	};

}