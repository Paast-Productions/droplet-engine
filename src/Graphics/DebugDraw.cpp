#include "Graphics/DebugDraw.hpp"
#include <array>
#include <map>

using namespace Droplet::Math;
using namespace Droplet::Graphics;
using namespace Droplet::Graphics::Debug;

namespace
{
	namespace Icosahedron
	{
		// Source: https://schneide.blog/2016/07/15/generating-an-icosphere-in-c/

		using Index = int;

		struct Triangle
		{
			Index vertex[3];
		};

		using TriangleList = std::vector<Triangle>;
		using VertexList = std::vector<glm::vec3>;

		const float X = .525731112119133606f;
		const float Z = .850650808352039932f;
		const float N = 0.f;

		static const VertexList IcosahedronVertices =
		{
			{-X,N,Z},	{X,N,Z},	{-X,N,-Z},	{X,N,-Z},
			{N,Z,X},	{N,Z,-X},	{N,-Z,X},	{N,-Z,-X},
			{Z,X,N},	{-Z,X, N},	{Z,-X,N},	{-Z,-X, N}
		};

		static const TriangleList IcosahedronTriangles =
		{
			{0,4,1},	{0,9,4},	{9,5,4},	{4,5,8},	{4,8,1},
			{8,10,1},	{8,3,10},	{5,3,8},	{5,2,3},	{2,7,3},
			{7,10,3},	{7,6,10},	{7,11,6},	{11,0,6},	{0,1,6},
			{6,1,10},	{9,0,11},	{9,11,2},	{9,2,5},	{7,2,11}
		};

		using Lookup = std::map<std::pair<Index, Index>, Index>;

		static Index VertexForEdge(Lookup &p_lookup, VertexList &p_vertices, Index p_first, Index p_second)
		{
			Lookup::key_type key(p_first, p_second);
			if (key.first > key.second)
				std::swap(key.first, key.second);

			auto inserted = p_lookup.insert({ key, p_vertices.size() });
			if (inserted.second)
			{
				const glm::vec3 &edge0 = p_vertices[p_first];
				const glm::vec3 &edge1 = p_vertices[p_second];
				p_vertices.emplace_back(glm::normalize(edge0 + edge1));
			}

			return inserted.first->second;
		}

		static TriangleList Subdivide(VertexList &p_vertices, TriangleList p_triangles)
		{
			Lookup lookup;
			TriangleList result;

			for (auto &&each : p_triangles)
			{
				std::array<Index, 3> mid{};
				for (int edge = 0; edge < 3; ++edge)
				{
					mid[edge] = VertexForEdge(lookup, p_vertices,
						each.vertex[edge], each.vertex[(edge + 1) % 3]);
				}

				result.emplace_back( each.vertex[0], mid[0], mid[2] );
				result.emplace_back( each.vertex[1], mid[1], mid[0] );
				result.emplace_back( each.vertex[2], mid[2], mid[1] );
				result.emplace_back( mid[0],		 mid[1], mid[2] );
			}

			return result;
		}

		using IndexedMesh = std::pair<VertexList, TriangleList>;

		static IndexedMesh MakeIcosphere(int subdivisions)
		{
			VertexList vertices = IcosahedronVertices;
			TriangleList triangles = IcosahedronTriangles;

			for (int i = 0; i < subdivisions; ++i)
			{
				triangles = Subdivide(vertices, triangles);
			}

			return{ vertices, triangles };
		}
	};

	static void GenerateIcoSphere(int subdivisions, std::vector<glm::vec3> &outVertices, std::vector<int> &outIndices)
	{
		auto [vertices, triangles] = Icosahedron::MakeIcosphere(subdivisions);

		outVertices.resize(vertices.size());
		std::copy(vertices.begin(), vertices.end(), outVertices.begin());

		outIndices.resize(triangles.size() * 3);
		for (std::size_t i = 0; i < triangles.size(); ++i)
		{
			outIndices[i * 3 + 0] = triangles[i].vertex[0];
			outIndices[i * 3 + 1] = triangles[i].vertex[1];
			outIndices[i * 3 + 2] = triangles[i].vertex[2];
		}
	}
}


void DebugDraw::ClearDrawCommands()
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	dbgDraw.m_wireLists.clear();
	dbgDraw.m_fillLists.clear();
	dbgDraw.m_spriteLists.clear();
}


void DebugDraw::DrawRay(const Ray &p_ray, const DrawOptions &p_opt)
{
	InternalDrawOptions intOpt(p_opt);

	DbgLine line{
		.v1 = p_ray.pos,
		.v2 = p_ray.pos + p_ray.dir
	};

	DebugDraw &dbgDraw = DebugDraw::Get();

	auto wireList = dbgDraw.m_wireLists.find(intOpt);
	if (wireList == dbgDraw.m_wireLists.end())
	{
		dbgDraw.m_wireLists[intOpt] = std::vector<DbgLine>();
	}
	dbgDraw.m_wireLists[intOpt].push_back(line);
}

void DebugDraw::DrawLine(const glm::vec3 &p_start, const glm::vec3 &p_end, const DrawOptions &p_opt)
{
	InternalDrawOptions intOpt(p_opt);

	DbgLine line{
		.v1 = p_start,
		.v2 = p_end
	};

	DebugDraw &dbgDraw = DebugDraw::Get();

	auto wireList = dbgDraw.m_wireLists.find(intOpt);
	if (wireList == dbgDraw.m_wireLists.end())
	{
		dbgDraw.m_wireLists[intOpt] = std::vector<DbgLine>();
	}
	dbgDraw.m_wireLists[intOpt].push_back(line);
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, const DrawOptions &p_opt)
{
	// TODO
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2, const DrawOptions &p_opt)
{
	// TODO
}

void DebugDraw::DrawBox(const AABB &p_aabb, const DrawOptions &p_opt)
{
	// TODO
}

void DebugDraw::DrawBox(const OBB &p_obb, const DrawOptions &p_opt)
{
	// TODO
}

void DebugDraw::DrawSphere(const Sphere &p_sphere, const DrawOptions &p_opt)
{
	static std::unordered_map<int, std::vector<DbgTri>> s_icosphereCache;

	if (s_icosphereCache.find(p_opt.detail) == s_icosphereCache.end())
	{
		std::vector<glm::vec3> vertices;
		std::vector<int> indices;
		::GenerateIcoSphere(p_opt.detail, vertices, indices);

		std::vector<DbgTri> triangles;
		triangles.reserve(indices.size() / 3);

		for (std::size_t i = 0; i < indices.size(); i += 3)
		{
			triangles.emplace_back(
				vertices[indices[i + 0]],
				vertices[indices[i + 1]],
				vertices[indices[i + 2]]
			);
		}

		s_icosphereCache[p_opt.detail] = std::move(triangles);
	}

	const std::vector<DbgTri> &triangles = s_icosphereCache[p_opt.detail];

	InternalDrawOptions intOpt(p_opt);

	DebugDraw &dbgDraw = DebugDraw::Get();

	if (dbgDraw.m_fillLists.find(intOpt) == dbgDraw.m_fillLists.end())
	{
		dbgDraw.m_fillLists[intOpt] = std::vector<DbgTri>();
	}
	dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());

}

void DebugDraw::DrawCircle(const glm::vec3 &p_center, float p_radius, const DrawOptions &p_opt)
{
	// Create a line loop to approximate the circle
	InternalDrawOptions intOpt(p_opt);

	DebugDraw &dbgDraw = DebugDraw::Get();
	
	if (dbgDraw.m_wireLists.find(intOpt) == dbgDraw.m_wireLists.end())
	{
		dbgDraw.m_wireLists[intOpt] = std::vector<DbgLine>();
	}


}

void DebugDraw::DrawFrustum(const Frustum &p_frustum, const DrawOptions &p_opt)
{
	// TODO
}

void DebugDraw::DrawIcon(const GUID &p_icon, const glm::vec3 &p_position, const glm::vec2 &p_scale, const DrawOptions &p_opt)
{
	InternalDrawOptions intOpt(p_opt, p_icon);

	DbgSprite sprite{
		.position = p_position,
		.scale = p_scale
	};

	DebugDraw &dbgDraw = DebugDraw::Get();

	auto spriteList = dbgDraw.m_spriteLists.find(intOpt);
	if (spriteList == dbgDraw.m_spriteLists.end())
	{
		dbgDraw.m_spriteLists[intOpt] = std::vector<DbgSprite>();
	}
	dbgDraw.m_spriteLists[intOpt].push_back(sprite);
}
