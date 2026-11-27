#include <Graphics/Debug/DebugDraw.hpp>
#include <resource/types/MeshResource.hpp>

#include <Math/Bezier.hpp>
#include <Math/Curves.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tracy/public/tracy/Tracy.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstddef>
#include <cstring>
#include <span>
#undef _USE_MATH_DEFINES

using namespace Droplet::Math;
using namespace Droplet::Common;
using namespace Droplet::Graphics;
using namespace Droplet::Graphics::Debug;


//
// Internal
//

namespace
{
	namespace Icosahedron
	{
		// Source: https://schneide.blog/2016/07/15/generating-an-icosphere-in-c/

		using Index = int;

		struct Triangle
		{
			Index vertex[3];

			Triangle(Index v1, Index v2, Index v3) : vertex{ v1, v2, v3 } { }
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
			ZoneScoped;

			Lookup::key_type key(p_first, p_second);
			if (key.first > key.second)
			{
				std::swap(key.first, key.second);
			}

			auto inserted = p_lookup.insert({ key, p_vertices.size() });
			if (inserted.second)
			{
				const glm::vec3 &edge0 = p_vertices[p_first];
				const glm::vec3 &edge1 = p_vertices[p_second];
				p_vertices.push_back(glm::normalize(edge0 + edge1));
			}

			return inserted.first->second;
		}

		static TriangleList Subdivide(VertexList &p_vertices, TriangleList p_triangles)
		{
			ZoneScoped;

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
			ZoneScoped;

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
		ZoneScoped;

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

	static void GenerateCircle(int segments, std::vector<glm::vec3> &outVertices)
	{
		ZoneScoped;

		float invSegments = 1.0f / float(segments);

		outVertices.resize(segments);
		for (int i = 0; i < segments; ++i)
		{
			float angle = 2.0f * std::numbers::pi * float(i) * invSegments;
			outVertices[i] = glm::vec3(std::cos(angle), std::sin(angle), 0.0f);
		}
	}

	static void GenerateBezierCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, int segments, std::vector<glm::vec3> &outVertices)
	{
		glm::vec3 v0 = p_p1;
		glm::vec3 v1 = v0 + p_d;
		glm::vec3 v2 = p_p2;
		
		outVertices.clear();
		outVertices.reserve((std::size_t)segments + 2);

		outVertices.emplace_back(v0);

		for (int i = 1; i <= segments; ++i)
		{
			float t = float(i) / float(segments + 1);
			glm::vec3 point = Bezier::QuadraticInterpolate(v0, v1, v2, t);
			outVertices.emplace_back(point);
		}

		outVertices.emplace_back(v2);
	}

	static void GenerateBezierCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2, int segments, std::vector<glm::vec3> &outVertices)
	{
		glm::vec3 v0 = p_p1;
		glm::vec3 v1 = v0 + p_d1;
		glm::vec3 v2 = p_p2 + p_d2;
		glm::vec3 v3 = p_p2;

		outVertices.clear();
		outVertices.reserve((std::size_t)segments + 2);

		outVertices.emplace_back(v0);

		for (int i = 1; i <= segments; ++i)
		{
			float t = float(i) / float(segments + 1);
			glm::vec3 point = Bezier::CubicInterpolate(v0, v1, v2, v3, t);
			outVertices.emplace_back(point);
		}

		outVertices.emplace_back(v3);
	}

	static int ClampCurveSegments(int detail)
	{
		return std::clamp(detail, 3, 128);
	}

	static int ClampCircleSegments(int detail)
	{
		return std::clamp(detail, 3, 64);
	}

	static int ClampSphereSubdivisions(int detail)
	{
		return std::clamp(detail, 0, 3);
	}
}



//
// Renderer Backend
//

void DebugDraw::PrepareRender()
{
	ZoneScoped;

	// Generate all but icon draw data
	// Combine all wireframe draw data into two meshes (depth + no depth) and all filled draw data into two other meshes

	std::vector<Vertex> wireMesh, wireDepthMesh;
	std::vector<Vertex> fillMesh, fillDepthMesh;

	for (const auto &[opt, lines] : m_wireLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(wireDepthMesh, lines);
		}
		else
		{
			AppendToMesh(wireMesh, lines);
		}
	}

	for (const auto &[opt, tris] : m_fillLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(fillDepthMesh, tris);
		}
		else
		{
			AppendToMesh(fillMesh, tris);
		}
	}

	m_wireMesh = std::move(wireMesh);
	m_wireDepthMesh = std::move(wireDepthMesh);

	m_fillMesh = std::move(fillMesh);
	m_fillDepthMesh = std::move(fillDepthMesh);
}

std::vector<std::shared_ptr<const DynamicMeshData>> DebugDraw::GetFrameMeshes()
{
	ZoneScoped

	DebugDraw& dbgDraw = DebugDraw::Get();
	dbgDraw.PrepareRender();

	// position and colour, streamed every frame (no id)
	std::vector<std::shared_ptr<const DynamicMeshData>> meshes;
	const auto add = [&meshes](const std::vector<Vertex>& p_vertices, MeshTopology p_topology, bool p_depthTest)
	{
		if (p_vertices.empty())
			return;
		const std::span<const std::byte> bytes = std::as_bytes(std::span(p_vertices));
		meshes.push_back(std::make_shared<const DynamicMeshData>(DynamicMeshData{
			.vertexStride = sizeof(Vertex),
			.attributes = {
				{ .location = 0, .format = VertexFormat::Float3, .offset = offsetof(Vertex, position) },
				{ .location = 1, .format = VertexFormat::Float4, .offset = offsetof(Vertex, color) } },
			.vertexData = { bytes.begin(), bytes.end() },
			.topology = p_topology,
			.depthTest = p_depthTest }));
	};
	add(dbgDraw.m_wireDepthMesh, MeshTopology::Lines, true);
	add(dbgDraw.m_fillDepthMesh, MeshTopology::Triangles, true);
	add(dbgDraw.m_wireMesh, MeshTopology::Lines, false);
	add(dbgDraw.m_fillMesh, MeshTopology::Triangles, false);
	
	dbgDraw.ClearDrawCommands();
	return meshes;
}

void DebugDraw::ClearDrawCommands()
{
	ZoneScoped;

	m_wireLists = {};
	m_fillLists = {};
	m_iconLists = {};
	m_wireMesh.clear();
	m_wireDepthMesh.clear();
	m_fillMesh.clear();
	m_fillDepthMesh.clear();
	m_iconMeshes.clear();
	m_iconDepthMeshes.clear();
}



//
// Mesh Generation
//

void DebugDraw::AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgLine> &p_lines)
{
	ZoneScoped;

	p_mesh.reserve(p_mesh.size() + p_lines.size() * 2);
	for (const auto &line : p_lines)
	{
		p_mesh.emplace_back(Vertex{ line.v1, line.color });
		p_mesh.emplace_back(Vertex{ line.v2, line.color });
	}
}

void DebugDraw::AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgTri> &p_tris)
{
	ZoneScoped;

	p_mesh.reserve(p_mesh.size() + p_tris.size() * 3);
	for (const auto &tri : p_tris)
	{
		p_mesh.emplace_back(Vertex{ tri.v1, tri.color });
		p_mesh.emplace_back(Vertex{ tri.v2, tri.color });
		p_mesh.emplace_back(Vertex{ tri.v3, tri.color });
	}
}

void DebugDraw::AppendToMesh(std::vector<IconVertex> &p_mesh, const std::vector<DbgIcon> &p_icons, glm::vec3 p_right, glm::vec3 p_up)
{
	ZoneScoped;

	p_mesh.reserve(p_mesh.size() + p_icons.size() * 6);
	for (const auto &icon : p_icons)
	{
		glm::vec3 center = icon.position;
		glm::vec3 right = p_right * icon.scale.x * 0.5f;
		glm::vec3 up = p_up * icon.scale.y * 0.5f;

		const IconVertex bottomLeft { center - right - up, icon.color, glm::vec2(0.0f, 0.0f) };
		const IconVertex bottomRight { center + right - up, icon.color, glm::vec2(1.0f, 0.0f) };
		const IconVertex topRight { center + right + up, icon.color, glm::vec2(1.0f, 1.0f) };
		const IconVertex topLeft { center - right + up, icon.color, glm::vec2(0.0f, 1.0f) };
		p_mesh.insert(p_mesh.end(), { bottomLeft, bottomRight, topRight, bottomLeft, topRight, topLeft });
	}
}



//
// Drawing Functions
//

void DebugDraw::DrawRay(const Ray &p_ray, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_ray.pos, p_ray.pos + p_ray.dir, p_col);
}

void DebugDraw::DrawRay(const glm::vec3 &p_pos, const glm::vec3 &p_dir, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_pos, p_pos + p_dir, p_col);
}

void DebugDraw::DrawLine(const glm::vec3 &p_start, const glm::vec3 &p_end, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_start, p_end, p_col);
}

void DebugDraw::DrawTri(const glm::vec3 &p_v1, const glm::vec3 &p_v2, const glm::vec3 &p_v3, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);
	if (p_wire)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(p_v1, p_v2, p_col);
		dbgDraw.m_wireLists[intOpt].emplace_back(p_v2, p_v3, p_col);
		dbgDraw.m_wireLists[intOpt].emplace_back(p_v3, p_v1, p_col);
	}
	else
	{
		dbgDraw.m_fillLists[intOpt].emplace_back(p_v1, p_v2, p_v3, p_col);
	}
}

void DebugDraw::DrawQuad(const glm::vec3 &p_center, const glm::vec3 &p_up, const glm::vec3 &p_right, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	// Calculate the four corners of the quad
	glm::vec3 v1 = p_center - p_right - p_up; // Bottom-left
	glm::vec3 v2 = p_center + p_right - p_up; // Bottom-right
	glm::vec3 v3 = p_center + p_right + p_up; // Top-right
	glm::vec3 v4 = p_center - p_right + p_up; // Top-left

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);

	if (p_wire)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(v1, v2, p_col);
		dbgDraw.m_wireLists[intOpt].emplace_back(v2, v3, p_col);
		dbgDraw.m_wireLists[intOpt].emplace_back(v3, v4, p_col);
		dbgDraw.m_wireLists[intOpt].emplace_back(v4, v1, p_col);
	}
	else
	{
		dbgDraw.m_fillLists[intOpt].emplace_back(v1, v2, v3, p_col);
		dbgDraw.m_fillLists[intOpt].emplace_back(v1, v3, v4, p_col);
	}
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, glm::vec4 p_col, int p_seg, bool p_depth)
{
	std::vector<glm::vec3> curvePoints;
	GenerateBezierCurve(p_p1, p_d, p_p2, ClampCurveSegments(p_seg), curvePoints);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);

	dbgDraw.m_wireLists[intOpt].reserve(dbgDraw.m_wireLists[intOpt].size() + curvePoints.size() - 1);

	for (std::size_t i = 0; i < curvePoints.size() - 1; ++i)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(curvePoints[i], curvePoints[i + 1], p_col);
	}
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2, glm::vec4 p_col, int p_seg, bool p_depth)
{
	std::vector<glm::vec3> curvePoints;
	GenerateBezierCurve(p_p1, p_d1, p_d2, p_p2, ClampCurveSegments(p_seg), curvePoints);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);

	dbgDraw.m_wireLists[intOpt].reserve(dbgDraw.m_wireLists[intOpt].size() + curvePoints.size() - 1);

	for (std::size_t i = 0; i < curvePoints.size() - 1; ++i)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(curvePoints[i], curvePoints[i + 1], p_col);
	}
}

void DebugDraw::DrawBox(const AABB &p_aabb, const glm::vec4 p_col, bool p_depth, bool p_wire)
{
	glm::vec3 min = p_aabb.GetMin();
	glm::vec3 max = p_aabb.GetMax();

	glm::vec3 v1 = glm::vec3(min.x, min.y, min.z);
	glm::vec3 v2 = glm::vec3(max.x, min.y, min.z);
	glm::vec3 v3 = glm::vec3(max.x, max.y, min.z);
	glm::vec3 v4 = glm::vec3(min.x, max.y, min.z);
	glm::vec3 v5 = glm::vec3(min.x, min.y, max.z);
	glm::vec3 v6 = glm::vec3(max.x, min.y, max.z);
	glm::vec3 v7 = glm::vec3(max.x, max.y, max.z);
	glm::vec3 v8 = glm::vec3(min.x, max.y, max.z);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);

	if (p_wire)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v1, v2}, {v2, v3}, {v3, v4}, {v4, v1}, // Bottom face
			{v5, v6}, {v6, v7}, {v7, v8}, {v8, v5}, // Top face
			{v1, v5}, {v2, v6}, {v3, v7}, {v4, v8}  // Vertical edges
		};
		for (auto &line : lines) line.color = p_col;
		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		// Draw filled faces
		std::vector<DbgTri> triangles = {
			// Bottom face
			{v1, v2, v3}, {v1, v3, v4},
			// Top face
			{v5, v6, v7}, {v5, v7, v8},
			// Front face
			{v1, v2, v6}, {v1, v6, v5},
			// Back face
			{v4, v3, v7}, {v4, v7, v8},
			// Left face
			{v1, v4, v8}, {v1, v8, v5},
			// Right face
			{v2, v3, v7}, {v2, v7, v6}
		};
		for (auto &triangle : triangles) triangle.color = p_col;
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawBox(const OBB &p_obb, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	// Calculate the corners
	std::vector<glm::vec3> v;
	v.resize(8);
	p_obb.GetCorners(v.data());

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);

	if (p_wire)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v[0], v[1]}, {v[1], v[2]}, {v[2], v[3]}, {v[3], v[0]}, // Bottom face
			{v[4], v[5]}, {v[5], v[6]}, {v[6], v[7]}, {v[7], v[4]}, // Top face
			{v[0], v[4]}, {v[1], v[5]}, {v[2], v[6]}, {v[3], v[7]}  // Vertical edges
		};
		for (auto &line : lines) line.color = p_col;
		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		// Draw filled faces
		std::vector<DbgTri> triangles = {
			// Bottom face
			{v[0], v[1], v[2]}, {v[0], v[2], v[3]},
			// Top face
			{v[4], v[5], v[6]}, {v[4], v[6], v[7]},
			// Front face
			{v[0], v[1], v[5]}, {v[0], v[5], v[4]},
			// Back face
			{v[3], v[2], v[6]}, {v[3], v[6], v[7]},
			// Left face
			{v[0], v[3], v[7]}, {v[0], v[7], v[4]},
			// Right face
			{v[1], v[2], v[6]}, {v[1], v[6], v[5]}
		};
		for (auto &triangle : triangles) triangle.color = p_col;
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawSphere(const Sphere &p_sphere, glm::vec4 p_col, int p_subdiv, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	const int subdivisions = ClampSphereSubdivisions(p_subdiv);

	if (dbgDraw.m_icosphereCache.find(subdivisions) == dbgDraw.m_icosphereCache.end())
	{
		std::vector<glm::vec3> vertices;
		std::vector<int> indices;
		::GenerateIcoSphere(subdivisions, vertices, indices);

		std::vector<DbgTri> triangles;
		triangles.reserve(indices.size() / 3);

		for (std::size_t i = 0; i < indices.size(); i += 3)
		{
			triangles.emplace_back(
				vertices[indices[i + 0]],
				vertices[indices[i + 1]],
				vertices[indices[i + 2]],
				p_col
			);
		}

		dbgDraw.m_icosphereCache[subdivisions] = std::move(triangles);
	}

	std::vector<DbgTri> triangles = dbgDraw.m_icosphereCache[subdivisions];

	for (auto &tri : triangles)
	{
		tri.v1 = p_sphere.center + tri.v1 * p_sphere.radius;
		tri.v2 = p_sphere.center + tri.v2 * p_sphere.radius;
		tri.v3 = p_sphere.center + tri.v3 * p_sphere.radius;
		tri.color = p_col;
	}

	InternalDrawOptions intOpt(p_depth);

	dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
}

void DebugDraw::DrawWireSphere(const Math::Sphere &p_sphere, glm::vec4 p_col, int p_seg, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	const int segments = ClampCircleSegments(p_seg);

	if (dbgDraw.m_circleCache.find(segments) == dbgDraw.m_circleCache.end())
	{
		std::vector<glm::vec3> points;

		::GenerateCircle(segments, points);

		dbgDraw.m_circleCache[segments] = std::move(points);
	}

	std::vector<glm::vec3> points = dbgDraw.m_circleCache[segments];

	InternalDrawOptions intOpt(p_depth);

	for (size_t i = 0; i < 3; i++)
	{
		glm::vec3 normal = glm::vec3(0.0f);
		normal[i] = 1.0f;

		std::vector<glm::vec3> tPoints;
		tPoints.resize(points.size());

		if (i == 2)
		{
			// Z axis, no need to rotate, just scale and translate
			for (int j = 0; j < points.size(); ++j)
			{
				tPoints[j] = p_sphere.center + points[j] * p_sphere.radius;
			}
		}
		else
		{
			// points is a unit circle in the XY plane, we need to transform it to the desired position and orientation
			// Create a rotation matrix that aligns the Z axis with the normal
			glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);

			float dot = glm::clamp(glm::dot(zAxis, normal), -1.0f, 1.0f);
			glm::mat4 rotation = glm::mat4(1.0f);
			if (dot <= -1.0f + 1e-6f)
			{
				rotation = glm::rotate(glm::mat4(1.0f), float(std::numbers::pi), glm::vec3(1.0f, 0.0f, 0.0f));
			}
			else if (dot < 1.0f - 1e-6f)
			{
				glm::vec3 axis = glm::normalize(glm::cross(zAxis, normal));
				float angle = std::acos(dot);
				rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
			}

			for (int j = 0; j < points.size(); ++j)
			{
				tPoints[j] = p_sphere.center + glm::vec3(rotation * glm::vec4(points[j] * p_sphere.radius, 1.0f));
			}
		}

		std::vector<DbgLine> lines;
		lines.reserve(tPoints.size());

		for (std::size_t i = 0; i < tPoints.size(); ++i)
		{
			lines.emplace_back(tPoints[i], tPoints[(i + 1) % tPoints.size()], p_col);
		}

		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
}

void DebugDraw::DrawArc(const glm::vec3 &p_center, const glm::vec3 &p_normal, const glm::vec3 &p_centerToStart, float p_radius, float p_angle, glm::vec4 p_col, int p_seg, bool p_depth, bool p_wire)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	const int segments = ClampCircleSegments(p_seg);

	std::vector<glm::vec3> points;
	points.resize(segments + 2);

	glm::vec3 startPoint = p_center + glm::normalize(p_centerToStart) * p_radius;
	glm::vec3 endPoint = p_center + glm::vec3(glm::normalize(glm::rotate(glm::mat4(1.0f), glm::radians(p_angle), p_normal) * glm::vec4(p_centerToStart, 0.0f)) * p_radius);

	points[0] = startPoint;
	points[segments + 1] = endPoint;

	for (int i = 1; i < segments + 1; i++)
	{
		points[i] = p_center + glm::vec3(glm::normalize(glm::rotate(glm::mat4(1.0f), glm::radians(p_angle * i / (segments + 1)), p_normal) * glm::vec4(p_centerToStart, 0.0f)) * p_radius);
	}

	InternalDrawOptions intOpt(p_depth);

	if (p_wire)
	{
		std::vector<DbgLine> lines;
		lines.reserve(points.size());

		for (int i = 0; i < points.size() - 1; ++i)
		{
			lines.emplace_back(points[i], points[i + 1], p_col);
		}

		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		std::vector<DbgTri> triangles;
		triangles.reserve(points.size());

		for (int i = 0; i < points.size() - 1; ++i)
		{
			triangles.emplace_back(p_center, points[i], points[i + 1], p_col);
		}

		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawCircle(const glm::vec3 &p_center, const glm::vec3 &p_normal, float p_radius, glm::vec4 p_col, int p_seg, bool p_depth, bool p_wire)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	const int segments = ClampCircleSegments(p_seg);

	if (dbgDraw.m_circleCache.find(segments) == dbgDraw.m_circleCache.end())
	{
		std::vector<glm::vec3> points;

		::GenerateCircle(segments, points);

		dbgDraw.m_circleCache[segments] = std::move(points);
	}

	std::vector<glm::vec3> points = dbgDraw.m_circleCache[segments];

	// points is a unit circle in the XY plane, we need to transform it to the desired position and orientation
	// Create a rotation matrix that aligns the Z axis with the normal
	glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 normal = p_normal;
	if (glm::dot(normal, normal) < 1e-8f)
	{
		normal = zAxis;
	}
	else
	{
		normal = glm::normalize(normal);
	}

	float dot = glm::clamp(glm::dot(zAxis, normal), -1.0f, 1.0f);
	glm::mat4 rotation = glm::mat4(1.0f);
	if (dot <= -1.0f + 1e-6f)
	{
		rotation = glm::rotate(glm::mat4(1.0f), float(std::numbers::pi), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else if (dot < 1.0f - 1e-6f)
	{
		glm::vec3 axis = glm::normalize(glm::cross(zAxis, normal));
		float angle = std::acos(dot);
		rotation = glm::rotate(glm::mat4(1.0f), angle, axis);
	}

	for (auto &p : points)
	{
		p = p_center + glm::vec3(rotation * glm::vec4(p * p_radius, 1.0f));
	}

	InternalDrawOptions intOpt(p_depth);

	if (p_wire)
	{
		std::vector<DbgLine> lines;
		lines.reserve(points.size());

		for (std::size_t i = 0; i < points.size(); ++i)
		{
			lines.emplace_back(points[i], points[(i + 1) % points.size()], p_col);
		}

		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		std::vector<DbgTri> triangles;
		triangles.reserve(points.size());

		for (std::size_t i = 0; i < points.size(); ++i)
		{
			triangles.emplace_back(p_center, points[i], points[(i + 1) % points.size()], p_col);
		}

		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawFrustum(const Frustum &p_frustum, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	// Calculate the eight corners of the frustum
	std::vector<glm::vec3> v;
	v.resize(8);
	p_frustum.GetCorners(v.data());

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);

	if (p_wire)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v[0], v[1]}, {v[1], v[3]}, {v[3], v[2]}, {v[2], v[0]}, // Near face
			{v[4], v[5]}, {v[5], v[7]}, {v[7], v[6]}, {v[6], v[4]}, // Far face
			{v[0], v[4]}, {v[1], v[5]}, {v[2], v[6]}, {v[3], v[7]}  // Connecting edges
		};
		for (auto &line : lines) line.color = p_col;
		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		// Draw filled faces
		std::vector<DbgTri> triangles = {
			// Near face
			{v[0], v[1], v[3]}, {v[0], v[3], v[2]},
			// Far face
			{v[4], v[5], v[7]}, {v[4], v[7], v[6]},
			// Left face
			{v[0], v[2], v[6]}, {v[0], v[6], v[4]},
			// Right face
			{v[1], v[3], v[7]}, {v[1], v[7], v[5]},
			// Top face
			{v[0], v[1], v[5]}, {v[0], v[5], v[4]},
			// Bottom face
			{v[2], v[3], v[7]}, {v[2], v[7], v[6]}
		};
		for (auto &triangle : triangles) triangle.color = p_col;
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawMesh(const MeshResource &p_mesh, const glm::mat4 &p_world, glm::vec4 p_col, bool p_depth, bool p_wire)
{
	std::size_t posOffset = 0;
	std::size_t posSize = 0;
	if (!p_mesh.FindVertexAttribute("POSITION", posOffset, posSize))
		return;

	const std::vector<std::byte> &vertexData = p_mesh.GetVertexData();
	const std::vector<std::uint32_t> &indexData = p_mesh.GetIndexData();
	const std::size_t vertexByteSize = p_mesh.GetVertexByteSize();
	if (vertexByteSize == 0 || posSize < sizeof(glm::vec3))
		return;

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth);
	std::vector<DbgLine> &lines = dbgDraw.m_wireLists[intOpt];
	std::vector<DbgTri> &tris = dbgDraw.m_fillLists[intOpt];

	for (std::size_t i = 0; i + 2 < indexData.size(); i += 3)
	{
		glm::vec3 v[3];
		bool valid = true;
		for (std::size_t j = 0; j < 3 && valid; ++j)
		{
			const std::size_t offset = static_cast<std::size_t>(indexData[i + j]) * vertexByteSize + posOffset;
			if (offset + sizeof(glm::vec3) > vertexData.size())
			{
				valid = false;
				break;
			}

			glm::vec3 local{};
			std::memcpy(&local, &vertexData[offset], sizeof(glm::vec3));
			v[j] = glm::vec3(p_world * glm::vec4(local, 1.0f));
		}

		if (!valid)
			continue;

		if (p_wire)
		{
			lines.emplace_back(v[0], v[1], p_col);
			lines.emplace_back(v[1], v[2], p_col);
			lines.emplace_back(v[2], v[0], p_col);
		}
		else
		{
			tris.emplace_back(v[0], v[1], v[2], p_col);
		}
	}
}

void DebugDraw::DrawIcon(const GUID &p_icon, const glm::vec3 &p_pos, const glm::vec2 &p_scale, glm::vec4 p_col, bool p_depth)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_depth, p_icon);

	dbgDraw.m_iconLists[intOpt].emplace_back(
		p_pos,
		p_scale,
		p_col
	);
}
