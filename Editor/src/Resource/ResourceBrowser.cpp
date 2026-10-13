#include "Resource/ResourceBrowser.hpp"
#include "core/FileExplorer.hpp"

#include <ImGui/imgui.h>

#include "MetaUtils.hpp"

using namespace Droplet::Editor::Resource;

void ResourceBrowser::InitImpl()
{
	SetName("Resource Browser");
}

void ResourceBrowser::RenderImpl()
{
	using namespace ImGui;
    
    // --- Registering Resources ---
    
    if (Button("Register Resource"))
    {
        m_fileExplorer.OpenFileDialog(m_instance->GetWindow(), true, m_instance->GetResourceManager().GetRootDirectory(),
            [this](const std::vector<std::filesystem::path>& absPaths)
            {
                if (absPaths.empty()) return;
                
                std::filesystem::path assetRoot = m_instance->GetResourceManager().GetRootDirectory();
                // This is where assets from outside the root dir will be copied to
                std::filesystem::path destinationBrowserDir = m_currentBrowserDirectory.empty() ? assetRoot : m_currentBrowserDirectory;
                
                for (auto &absPath : absPaths)
                {
                    std::filesystem::path relPath;
                    std::filesystem::path relPathTemp = absPath.lexically_relative(assetRoot); // Generate relative path
                    
                    if (relPathTemp.empty() || relPathTemp.string().starts_with("..") || relPathTemp.is_absolute())
                    {
                        // The file is from outside the root asset dir -> Copy it
                        std::filesystem::path destination = destinationBrowserDir / absPath.filename();
                        std::filesystem::copy_file(absPath, destination, std::filesystem::copy_options::overwrite_existing);
                        relPath = destination.lexically_relative(assetRoot);
                    }
                    else
                    {
                        relPath = relPathTemp;
                    }
                    
                    m_instance->GetResourceManager().RegisterAsset(relPath,
                        [this](const std::vector<MetaEntry>& entries)
                        {
                            m_registerPopupQueue.push(entries);
                        }
                    );
                }
            }
        );
    }
    SameLine();
    Text("Root Dir: %s", m_instance->GetResourceManager().GetRootDirectory().string().c_str());
    Separator();
    
    // --- Queued Popup Rendering ---
    Text("Queued Popups: %u", m_registerPopupQueue.size());
    if (!m_registerPopupQueue.empty())
    {
        OpenPopup("Import Settings");
    }
    
    if (BeginPopupModal("Import Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::vector<MetaEntry>& currentEntries = m_registerPopupQueue.front();
        
        Text("Importing Asset: %s", currentEntries.front().relAssetPath.c_str());
        Separator();
        
        for (size_t i = 0; i < currentEntries.size(); i++)
        {
            MetaEntry &entry = currentEntries[i];
            PushID(static_cast<int>(i));
            
            Text("Resource GUID: %llu", entry.guid);
            auto schema = m_instance->GetResourceManager().GetSettingsSchema(entry.type);
            if (schema.empty())
            {
                TextDisabled("No settings available");
            }
            
            // Name change here
            
            // Draw UI based on descriptor
            for (const auto &desc : schema)
            {
                if (!entry.loadSettings.contains(desc.key)) continue;
                
                if (desc.type == MetaLoadSettings::Type::Bool)
                {
                    bool val = entry.loadSettings[desc.key].get<bool>();
                    if (Checkbox(desc.key.c_str(), &val))
                    {
                        entry.loadSettings[desc.key] = val;
                    }
                }
                else if (desc.type == MetaLoadSettings::Type::Int)
                {
                    int val = entry.loadSettings[desc.key].get<int>();
                    if (InputInt(desc.key.c_str(), &val))
                    {
                        entry.loadSettings[desc.key] = val;
                    }
                }
            }
            
            PopID();
            
            // TODO: Dependency modifications should go here
        }
        
        if (Button("Cancel"))
        {
            m_registerPopupQueue.pop();
            CloseCurrentPopup();
        }
        if (Button("Apply Changes"))
        {
            for (const auto &entry : currentEntries)
            {
                m_instance->GetResourceManager().UpdateMetaEntry(entry.guid, entry);
            }
            
            m_registerPopupQueue.pop(); // Registering has been handled
            CloseCurrentPopup();
        }
        SameLine();
        
        if (m_registerPopupQueue.size() > 1)
        {
            TextDisabled("(%zu more files waiting...)", m_registerPopupQueue.size());
        }
        
        EndPopup();
    }
    
    // -- Registered Resources List ---
    

	// Split the window into two child windows: one for the resource list and one for the resource details

	ImVec2 windowSize = GetWindowSize();
	ImVec2 cursorStartPos = GetCursorPos();
	ImVec2 listSize = ImVec2(windowSize.x * 0.5f, 0);
    
    std::vector<const MetaEntry *> resources = m_instance->GetResourceManager().GetRegisteredResources();

	// Selectable and draggable resource entries
	BeginChild("Resource List", listSize, ImGuiChildFlags_ResizeX);
	{
	    Text("Registered Resources: %u", resources.size());
	    
		for (std::size_t i = 0; i < resources.size(); i++)
		{
			const MetaEntry *entry = resources[i];

			if (Selectable(entry->name.c_str(), selectedResourceGUID == entry->guid))
			{
				if (selectedResourceGUID == entry->guid)
				{
					selectedResourceGUID = C_INVALID_GUID; // Deselect the resource if it was already selected
				}
				else
				{
					selectedResourceGUID = entry->guid; // Select the resource if it was not already selected
				}
			}
		}
	}
	EndChild();

	SetCursorPos(ImVec2(cursorStartPos.x + GetItemRectSize().x, cursorStartPos.y));

	// Details of selected resource
	BeginChild("Resource Inspector");
	{
		if (selectedResourceGUID != C_INVALID_GUID)
		{
			// TODO: Fetch the resource details using the GUID and render them here
		}
	}
	EndChild();
}
