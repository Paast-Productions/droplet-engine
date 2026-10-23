#include <Editor/ResourceSlot.hpp>

#include <ImGui/imgui.h>
#include <string>
#include <Editor/DragDropFields.hpp>
#include <Resource/GuidUtils.hpp>
#include <Resource/IResource.hpp>
#include <Resource/Meta/MetaData.hpp>

namespace Droplet
{
    bool ResourceSlot(
        const char *p_label,
        Droplet::GUID p_current,
        const std::string &p_currentDisplayName,
        Droplet::GUID &p_outGUID,
        std::string &p_outDisplayName,
        Droplet::ResourceType p_expectedType,
        Droplet::ShaderResource::ShaderType p_expectedShader)
    {
        using namespace ImGui;
        using namespace Droplet::Editor;

        bool changed = false;

        PushID(p_label);
        TableNextRow();
        TableNextColumn();
        AlignTextToFramePadding();
        TextUnformatted(p_label);
        TableNextColumn();

        const float clearW = GetFrameHeight();
        const float gap = GetStyle().ItemSpacing.x;

        const std::string text = p_current != Droplet::C_INVALID_GUID
            ? p_currentDisplayName
            : "None (drop resource here)";

        if (Button(text.c_str(), ImVec2(-(clearW + gap), 0.0f)))
        {
            OpenPopup("pick");
        }

        if (BeginDragDropTarget())
        {
            if (const ImGuiPayload *pl =
                AcceptDragDropPayload(PAYLOAD_RESOURCE.data()))
            {
                if (pl->DataSize == sizeof(const Droplet::MetaEntry *))
                {
                    const auto *entryPtr =
                        *static_cast<const Droplet::MetaEntry *const *>(pl->Data);
                    Droplet::ShaderResource::ShaderType shaderType = entryPtr->loadSettings.value(Droplet::MetaLoadSettings::C_SHADER_TYPE.key, Droplet::MetaLoadSettings::C_SHADER_TYPE.defaultValue);
                    if (entryPtr->type == p_expectedType && shaderType == p_expectedShader)
                    {
                        // Access the complete entry.
                        p_outGUID = entryPtr->guid;
                        p_outDisplayName = entryPtr->displayName;
                        changed = true;
                    }
                }
            }
            if (const ImGuiPayload *pl =
                AcceptDragDropPayload(PAYLOAD_ASSET.data()))
            {
                if (pl->DataSize % sizeof(const Droplet::MetaEntry *) == 0)
                {
                    const auto *entries =
                        static_cast<const Droplet::MetaEntry *const *>(pl->Data);

                    const std::size_t count =
                        pl->DataSize / sizeof(const Droplet::MetaEntry *);

                    for (std::size_t i = 0; i < count; ++i)
                    {
                        const Droplet::MetaEntry *entry = entries[i];
                        Droplet::ShaderResource::ShaderType shaderType = entry->loadSettings.value(Droplet::MetaLoadSettings::C_SHADER_TYPE.key, Droplet::MetaLoadSettings::C_SHADER_TYPE.defaultValue);

                        if (entry && entry->type == p_expectedType && shaderType == p_expectedShader)
                        {
                            p_outGUID = entry->guid;
                            p_outDisplayName = entry->displayName;
                            changed = true;
                            break;
                        }
                    }
                }
            }

            EndDragDropTarget();
        }

        if (p_current != Droplet::C_INVALID_GUID && IsItemHovered())
        {
            SetTooltip(
                "GUID: %llu",
                static_cast<unsigned long long>(p_current));
        }

        SameLine();
        BeginDisabled(p_current == Droplet::C_INVALID_GUID);

        if (Button("X", ImVec2(clearW, clearW)))
        {
            p_outGUID = Droplet::C_INVALID_GUID;
            p_outDisplayName.clear();
            changed = true;
        }

        EndDisabled();

        PopID();
        return changed;
    }
}
