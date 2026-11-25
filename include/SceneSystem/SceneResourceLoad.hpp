#pragma once

#include <memory>
#include <functional>
#include <utility>

#include <resource/ResourceManager.hpp>
#include <SceneSystem/Scene.hpp>

namespace Droplet::Scene
{
    /// @brief Loads a static resource and keeps the Scene from reporting its resources as loaded until the load finishes or fails.
    ///
    /// The Scene tracks the guid while it is loading, so multiple loads of the same guid count once. Completion is
    /// reported once per call, from the main thread, when the ResourceManager invokes the load callback.
    ///
    /// @tparam T The resource type.
    /// @param p_scene The Scene that waits for the load. If null, the resource is loaded without being tracked.
    /// @param p_guid The guid of the resource.
    /// @param p_onLoadCallback Optional callback triggered (on main thread) when the resource's state is ready, as with ResourceManager::LoadResource.
    /// @return A handle to the resource. Make sure to check its validity before use.
    template<typename T>
    [[nodiscard]] StaticResourceHandle<T> LoadTrackedResource(const std::shared_ptr<Scene> &p_scene, GUID p_guid, std::function<void(StaticResourceHandle<T>)> p_onLoadCallback = nullptr)
    {
        ResourceManager &resourceManager = ResourceManager::GetInstance();
        if (!p_scene)
        {
            return resourceManager.LoadResource<T>(p_guid, std::move(p_onLoadCallback));
        }

        p_scene->BeginResourceLoad(p_guid);

        // Guards against reporting completion twice, e.g. when the callback runs synchronously
        auto finished = std::make_shared<bool>(false);
        std::weak_ptr<Scene> weakScene = p_scene;
        auto endLoad = [finished, weakScene, p_guid]()
        {
            if (*finished)
            {
                return;
            }
            *finished = true;

            if (auto scene = weakScene.lock())
            {
                scene->EndResourceLoad(p_guid);
            }
        };

        StaticResourceHandle<T> handle = resourceManager.LoadResource<T>(p_guid, [endLoad, p_onLoadCallback](StaticResourceHandle<T> p_loaded)
        {
            if (p_onLoadCallback)
            {
                p_onLoadCallback(p_loaded);
            }
            endLoad();
        });

        // The callback is never invoked for unregistered guids, or for resources that already failed
        if (handle.HasFailed())
        {
            endLoad();
        }

        return handle;
    }
}
