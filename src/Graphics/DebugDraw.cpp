#include "Graphics/DebugDraw.hpp"
#include <Graphics/VK/Camera.hpp>
#include <math/Bezier.hpp>
#include <math/Curves.hpp>
#include <glm/gtx/quaternion.hpp>
#include <array>
#include <map>
#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

using namespace Droplet::Math;
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

	static void GenerateCircle(int segments, std::vector<glm::vec3> &outVertices)
	{
		float invSegments = 1.0f / float(segments);

		outVertices.resize(segments);
		for (int i = 0; i < segments; ++i)
		{
			float angle = 2.0f * M_PI * float(i) * invSegments;
			outVertices[i] = glm::vec3(std::cos(angle), std::sin(angle), 0.0f);
		}
	}

	static void GenerateBezierCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, int segments, std::vector<glm::vec3> &outVertices)
	{
		glm::vec3 v0 = p_p1;
		glm::vec3 v1 = v0 + p_d;
		glm::vec3 v2 = p_p2;
		
		outVertices.resize((std::size_t)segments + 2);

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

		outVertices.resize((std::size_t)segments + 2);

		outVertices.emplace_back(v0);

		for (int i = 1; i <= segments; ++i)
		{
			float t = float(i) / float(segments + 1);
			glm::vec3 point = Bezier::CubicInterpolate(v0, v1, v2, v3, t);
			outVertices.emplace_back(point);
		}

		outVertices.emplace_back(v3);
	}
}



//
// Renderer Backend
//

void DebugDraw::Initialize(/* VK Resources */)
{
	// Initialize Vulkan resources for rendering debug draw data
	// Generate pipelines, descriptor sets, buffers, etc.
	// TODO
}

void DebugDraw::PrepareRender()
{
	// Generate all but icon draw data
	// Combine all wireframe draw data into two meshes (depth + no depth) and all filled draw data into two other meshes

	std::vector<Vertex> wireMesh, wireDepthMesh;
	std::vector<Vertex> fillMesh, fillDepthMesh;

	for (const auto &[opt, lines] : m_wireLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(wireDepthMesh, lines, opt.color);
		}
		else
		{
			AppendToMesh(wireMesh, lines, opt.color);
		}
	}

	for (const auto &[opt, tris] : m_fillLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(fillDepthMesh, tris, opt.color);
		}
		else
		{
			AppendToMesh(fillMesh, tris, opt.color);
		}
	}

	// TODO: Upload the generated meshes to Vulkan buffers for rendering
}

void DebugDraw::RenderToCamera(const Camera &p_camera)
{
	// Generate icon draw data facing the camera
	// TODO

	glm::vec3 cameraRight = -p_camera.GetRight();
	glm::vec3 cameraUp = p_camera.GetUp();

	std::map<GUID, std::vector<IconVertex>> iconMeshes, iconDepthMeshes;

	for (const auto &[opt, icons] : m_iconLists)
	{
		if (opt.depthTest)
		{
			AppendToMesh(iconDepthMeshes[opt.icon], icons, opt.color, cameraRight, cameraUp);
		}
		else
		{
			AppendToMesh(iconMeshes[opt.icon], icons, opt.color, cameraRight, cameraUp);
		}
	}
	
	// TODO: Upload the generated icon meshes to Vulkan buffers for rendering

	// Render all draw data to the camera output
	// TODO
}

void DebugDraw::ClearDrawCommands()
{
	m_wireLists = {};
	m_fillLists = {};
	m_iconLists = {};

	// TODO: Clear Vulkan buffers for the next frame
}



//
// Mesh Generation
//

void DebugDraw::AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgLine> &p_lines, glm::vec4 p_color)
{
	p_mesh.reserve(p_mesh.size() + p_lines.size() * 2);
	for (const auto &line : p_lines)
	{
		p_mesh.emplace_back(Vertex{ line.v1, p_color });
		p_mesh.emplace_back(Vertex{ line.v2, p_color });
	}
}

void DebugDraw::AppendToMesh(std::vector<Vertex> &p_mesh, const std::vector<DbgTri> &p_tris, glm::vec4 p_color)
{
	p_mesh.reserve(p_mesh.size() + p_tris.size() * 3);
	for (const auto &tri : p_tris)
	{
		p_mesh.emplace_back(Vertex{ tri.v1, p_color });
		p_mesh.emplace_back(Vertex{ tri.v2, p_color });
		p_mesh.emplace_back(Vertex{ tri.v3, p_color });
	}
}

void DebugDraw::AppendToMesh(std::vector<IconVertex> &p_mesh, const std::vector<DbgIcon> &p_icons, glm::vec4 p_color, glm::vec3 p_right, glm::vec3 p_up)
{
	p_mesh.reserve(p_mesh.size() + p_icons.size() * 4);
	for (const auto &icon : p_icons)
	{
		glm::vec3 center = icon.position;
		glm::vec3 right = p_right * icon.scale.x * 0.5f;
		glm::vec3 up = p_up * icon.scale.y * 0.5f;

		p_mesh.emplace_back(IconVertex{ center - right - up, p_color, glm::vec2(0.0f, 0.0f) });
		p_mesh.emplace_back(IconVertex{ center + right - up, p_color, glm::vec2(1.0f, 0.0f) });
		p_mesh.emplace_back(IconVertex{ center + right + up, p_color, glm::vec2(1.0f, 1.0f) });
		p_mesh.emplace_back(IconVertex{ center - right + up, p_color, glm::vec2(0.0f, 1.0f) });
	}
}



//
// Drawing Functions
//

void DebugDraw::DrawRay(const Ray &p_ray, const DrawOptions &p_opt)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_ray.pos, p_ray.pos + p_ray.dir);
}

void DebugDraw::DrawLine(const glm::vec3 &p_start, const glm::vec3 &p_end, const DrawOptions &p_opt)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);
	dbgDraw.m_wireLists[intOpt].emplace_back(p_start, p_end);
}

void DebugDraw::DrawTri(const glm::vec3 &p_v1, const glm::vec3 &p_v2, const glm::vec3 &p_v3, const DrawOptions &p_opt)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);
	dbgDraw.m_fillLists[intOpt].emplace_back(p_v1, p_v2, p_v3);
}

void DebugDraw::DrawQuad(const glm::vec3 &p_center, const glm::vec3 &p_up, const glm::vec3 &p_right, const DrawOptions &p_opt)
{
	// Calculate the four corners of the quad
	glm::vec3 v1 = p_center - p_right - p_up; // Bottom-left
	glm::vec3 v2 = p_center + p_right - p_up; // Bottom-right
	glm::vec3 v3 = p_center + p_right + p_up; // Top-right
	glm::vec3 v4 = p_center - p_right + p_up; // Top-left

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	dbgDraw.m_fillLists[intOpt].emplace_back(v1, v2, v3);
	dbgDraw.m_fillLists[intOpt].emplace_back(v1, v3, v4);
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d, const glm::vec3 &p_p2, const DrawOptions &p_opt)
{
	std::vector<glm::vec3> curvePoints;
	GenerateBezierCurve(p_p1, p_d, p_p2, std::min(p_opt.detail, 1), curvePoints);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	dbgDraw.m_wireLists[intOpt].reserve(dbgDraw.m_wireLists[intOpt].size() + curvePoints.size() - 1);

	for (std::size_t i = 0; i < curvePoints.size() - 1; ++i)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(curvePoints[i], curvePoints[i + 1]);
	}
}

void DebugDraw::DrawCurve(const glm::vec3 &p_p1, const glm::vec3 &p_d1, const glm::vec3 &p_d2, const glm::vec3 &p_p2, const DrawOptions &p_opt)
{
	std::vector<glm::vec3> curvePoints;
	GenerateBezierCurve(p_p1, p_d1, p_d2, p_p2, std::min(p_opt.detail, 1), curvePoints);

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	dbgDraw.m_wireLists[intOpt].reserve(dbgDraw.m_wireLists[intOpt].size() + curvePoints.size() - 1);

	for (std::size_t i = 0; i < curvePoints.size() - 1; ++i)
	{
		dbgDraw.m_wireLists[intOpt].emplace_back(curvePoints[i], curvePoints[i + 1]);
	}
}

void DebugDraw::DrawBox(const AABB &p_aabb, const DrawOptions &p_opt)
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
	InternalDrawOptions intOpt(p_opt);

	if (p_opt.wireframe)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v1, v2}, {v2, v3}, {v3, v4}, {v4, v1}, // Bottom face
			{v5, v6}, {v6, v7}, {v7, v8}, {v8, v5}, // Top face
			{v1, v5}, {v2, v6}, {v3, v7}, {v4, v8}  // Vertical edges
		};
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
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawBox(const OBB &p_obb, const DrawOptions &p_opt)
{
	// Calculate the corners
	std::vector<glm::vec3> v;
	v.resize(8);
	p_obb.GetCorners(v.data());

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	if (p_opt.wireframe)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v[0], v[1]}, {v[1], v[2]}, {v[2], v[3]}, {v[3], v[0]}, // Bottom face
			{v[4], v[5]}, {v[5], v[6]}, {v[6], v[7]}, {v[7], v[4]}, // Top face
			{v[0], v[4]}, {v[1], v[5]}, {v[2], v[6]}, {v[3], v[7]}  // Vertical edges
		};
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
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawSphere(const Sphere &p_sphere, const DrawOptions &p_opt)
{
	static std::map<int, std::vector<DbgTri>> s_icosphereCache;

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

	std::vector<DbgTri> triangles = s_icosphereCache[p_opt.detail];

	for (auto &tri : triangles)
	{
		tri.v1 = p_sphere.center + tri.v1 * p_sphere.radius;
		tri.v2 = p_sphere.center + tri.v2 * p_sphere.radius;
		tri.v3 = p_sphere.center + tri.v3 * p_sphere.radius;
	}

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
}

void DebugDraw::DrawCircle(const glm::vec3 &p_center, const glm::vec3 &p_normal, float p_radius, const DrawOptions &p_opt)
{
	static std::map<int, std::vector<glm::vec3>> s_circleCache;

	if (s_circleCache.find(p_opt.detail) == s_circleCache.end())
	{
		std::vector<glm::vec3> points;

		::GenerateCircle(p_opt.detail, points);

		s_circleCache[p_opt.detail] = std::move(points);
	}

	std::vector<glm::vec3> points = s_circleCache[p_opt.detail];

	// points is a unit circle in the XY plane, we need to transform it to the desired position and orientation
	// Create a rotation matrix that aligns the Z axis with the normal
	glm::vec3 zAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 axis = glm::cross(zAxis, p_normal);
	float angle = std::acos(glm::dot(zAxis, p_normal));
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

	for (auto &p : points)
	{
		p = p_center + glm::vec3(rotation * glm::vec4(p * p_radius, 1.0f));
	}

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	if (p_opt.wireframe)
	{
		std::vector<DbgLine> lines;
		lines.reserve(points.size());

		for (std::size_t i = 0; i < points.size(); ++i)
		{
			lines.emplace_back(points[i], points[(i + 1) % points.size()]);
		}

		dbgDraw.m_wireLists[intOpt].insert(dbgDraw.m_wireLists[intOpt].end(), lines.begin(), lines.end());
	}
	else
	{
		std::vector<DbgTri> triangles;
		triangles.reserve(points.size());

		for (std::size_t i = 0; i < points.size(); ++i)
		{
			triangles.emplace_back(p_center, points[i], points[(i + 1) % points.size()]);
		}

		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawFrustum(const Frustum &p_frustum, const DrawOptions &p_opt)
{
	// Calculate the eight corners of the frustum
	std::vector<glm::vec3> v;
	v.resize(8);
	p_frustum.GetCorners(v.data());

	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt);

	if (p_opt.wireframe)
	{
		// Draw wireframe edges
		std::vector<DbgLine> lines = {
			{v[0], v[1]}, {v[1], v[3]}, {v[3], v[2]}, {v[2], v[0]}, // Near face
			{v[4], v[5]}, {v[5], v[7]}, {v[7], v[6]}, {v[6], v[4]}, // Far face
			{v[0], v[4]}, {v[1], v[5]}, {v[2], v[6]}, {v[3], v[7]}  // Connecting edges
		};
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
		dbgDraw.m_fillLists[intOpt].insert(dbgDraw.m_fillLists[intOpt].end(), triangles.begin(), triangles.end());
	}
}

void DebugDraw::DrawIcon(const GUID &p_icon, const glm::vec3 &p_center, const glm::vec2 &p_scale, const DrawOptions &p_opt)
{
	DebugDraw &dbgDraw = DebugDraw::Get();
	InternalDrawOptions intOpt(p_opt, p_icon);

	dbgDraw.m_iconLists[intOpt].emplace_back(
		p_center,
		p_scale
	);
}
