#include "OctreeBindings.hpp"
#include "LuaBindings.hpp"
#include <SceneSystem/Octree.hpp>

using namespace Droplet::Script;
using namespace Droplet::Scene;

void Bindings::RegisterOctree(sol::state_view p_luaState)
{
    p_luaState.new_usertype<Octree>(
        "Octree",

        "GetNodeFromRaycast",
        [](Octree &p_octree, const Math::Ray &p_ray, sol::optional<float> p_maxLength)
        {
            Math::RayHit hit;

            std::shared_ptr<Node> node =
                p_octree.GetNodeFromRaycast(p_ray, &hit, nullptr, Droplet::CollisionLayer::All, p_maxLength.value_or(std::numeric_limits<float>::infinity())).lock();

            return std::make_tuple(node, hit);
        },

        "GetNodesFromRaycast",
        [&p_luaState](Octree &p_octree, const Math::Ray &p_ray, sol::optional<float> p_maxLength)
        {
            sol::table result = p_luaState.create_table();

            auto nodes = p_octree.GetNodesFromRaycast(p_ray, Droplet::CollisionLayer::All, p_maxLength.value_or(std::numeric_limits<float>::infinity()));

            int index = 1;

            for (const auto &weakNode : nodes)
            {
                if (auto node = weakNode.lock())
                {
                    result[index++] = node;
                }
            }

            return result;
        },

        "ToGraphviz", &Octree::ToGraphviz,
        "ToGraphvizTree", &Octree::ToGraphvizTree
    );

    LuaClassDefinition octreeDef;
    octreeDef.name = "Octree";

    octreeDef.functions = {
        {"AddNode", "void", {
            {"node", "Node"}
        }},

        {"RemoveNode", "void", {
            {"node", "Node"}
        }},

        {"GetNodesFromCulling", "Node[]", {
            {"frustum", "Frustum"}
        }},

        {"GetNodesFullyInFrustum", "Node[]", {
            {"frustum", "Frustum"}
        }},

        {"GetNodeFromRaycast", "tuple<Node, RayHit>", {
            {"ray", "Ray"},
            {"maxLength", "number"}
        }},

        {"GetNodesFromRaycast", "Node[]", {
            {"ray", "Ray"},
            {"maxLength", "number"}
        }},

        {"GetNodesFromOBB", "Node[]", {
            {"obb", "OBB"}
        }},

        {"ToGraphviz", "string", {}},

        {"ToGraphvizTree", "string", {}}
    };

	LuaBindings::PushLuaClassDefinition(octreeDef);
}
