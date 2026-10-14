#include "Resource/ResourceBrowser.hpp"
#include "core/FileExplorer.hpp"

#include <ImGui/imgui.h>

#include "MetaUtils.hpp"

using namespace Droplet::Editor::Resource;

void ResourceBrowser::InitImpl()
{
	SetName("Resource Browser");
    BuildBrowserTree(m_engine->GetResourceManager().GetRegisteredResources());
}

void ResourceBrowser::RenderImpl()
{
	using namespace ImGui;
    
    bool updatedRegistry = false;
    
    // --- Registering Resources ---
    
    if (Button("Register Resource"))
    {
        m_fileExplorer.OpenFileDialog(m_engine->GetWindow(), true, m_engine->GetResourceManager().GetRootDirectory(),
            [this](const std::vector<std::filesystem::path>& absPaths)
            {
                if (absPaths.empty()) return;
                
                std::filesystem::path assetRoot = m_engine->GetResourceManager().GetRootDirectory();
                // This is where assets from outside the root dir will be copied to
                std::filesystem::path destinationBrowserDir = m_currentBrowserPath.empty() ? assetRoot : m_currentBrowserPath;
                
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
                    
                    m_engine->GetResourceManager().RegisterAsset(relPath,
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
    TextDisabled(m_engine->GetResourceManager().GetRootDirectory().string().c_str());
    Separator();
    
    // --- Queued Popup Rendering ---
    if (!m_registerPopupQueue.empty())
    {
        OpenPopup("Resource Settings");
    }
    
    if (BeginPopupModal("Resource Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        std::vector<MetaEntry>& currentEntries = m_registerPopupQueue.front();
        
        TextDisabled("%s", currentEntries.front().relAssetPath.c_str());
        Spacing();
        for (size_t i = 0; i < currentEntries.size(); i++)
        {
            MetaEntry &entry = currentEntries[i];
            PushID(static_cast<int>(i));
            
            SeparatorText(ResourceTypeToString(entry.type).c_str());
            
            DrawResourceSettings(entry);
            Spacing();
            
            PopID();
        }
        
        if (Button("Cancel"))
        {
            m_registerPopupQueue.pop();
            CloseCurrentPopup();
        }
        SameLine();
        if (Button("Apply"))
        {
            for (const auto &entry : currentEntries)
            {
                m_engine->GetResourceManager().UpdateMetaEntry(entry.guid, entry);
                updatedRegistry = true;
            }
            
            m_registerPopupQueue.pop(); // Registering has been handled
            CloseCurrentPopup();
        }
        
        if (m_registerPopupQueue.size() > 1)
        {
            TextDisabled("(%zu more files waiting...)", m_registerPopupQueue.size());
        }
        
        EndPopup();
    }
    
    // --- Resource Browser ---
    
	// Split the window into two child windows: one for the resource list and one for the resource details

	ImVec2 windowSize = GetWindowSize();
	ImVec2 cursorStartPos = GetCursorPos();
	ImVec2 listSize = ImVec2(windowSize.x * 0.5f, 0);
    
    std::vector<const MetaEntry *> resources = m_engine->GetResourceManager().GetRegisteredResources();
    if (updatedRegistry)
    {
        BuildBrowserTree(resources);
    }
    
    BrowserNode *currentNode = &m_rootBrowserNode;
    for (const auto &part : m_currentBrowserPath)
    {
        if (currentNode->children.contains(part.generic_string()))
        {
            currentNode = &currentNode->children[part.generic_string()];
        }
        else
        {
            // Fallback
            m_currentBrowserPath.clear();
            currentNode = &m_rootBrowserNode;
            break;
        }
    }

	// Selectable and draggable resource entries
	BeginChild("Resource List", listSize, ImGuiChildFlags_ResizeX);
	{
        TextDisabled("Current Path: /%s", m_currentBrowserPath.generic_string().c_str());
	    Separator();
	    
	    // Draw go back item
	    if (!m_currentBrowserPath.empty())
	    {
	        if (Selectable("[..] (Parent Directory)", false, ImGuiSelectableFlags_AllowDoubleClick))
	        {
	            if (IsMouseDoubleClicked(ImGuiMouseButton_Left))
	            {
	                m_currentBrowserPath = m_currentBrowserPath.parent_path();
	            }
	        }
	        Separator();
	    }
	    
	    // Draw directories
	    for (auto &[childName, childNode] : currentNode->children)
	    {
	        if (childNode.isDirectory)
	        {
	            std::string label = "📁 " + childName;
	            if (Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
	            {
	                if (IsMouseDoubleClicked(ImGuiMouseButton_Left))
	                {
	                    m_currentBrowserPath /= childName;
	                }
	            }
	        }
	    }
	    
	    // Draw files
	    for (auto &[childName, childNode] : currentNode->children)
	    {
	        if (!childNode.isDirectory)
	        {
	            // Multi resource file
	            std::string fileLabel = "= " + childName;
	            if (TreeNodeEx(fileLabel.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow))
	            {
	                for (const MetaEntry *entry : childNode.resources)
	                {
	                    bool isSelected = (m_selectedResourceGuid == entry->guid);
	                    std::string resLabel = "- " + entry->name + " [" + ResourceTypeToString(entry->type) + "]";
	                    if (Selectable(resLabel.c_str(), isSelected))
	                    {
	                        m_selectedResourceGuid = isSelected ? C_INVALID_GUID : entry->guid;
	                    }
	                }
	                TreePop();
	            }
	        }
	    }
	}
	EndChild();

	SetCursorPos(ImVec2(cursorStartPos.x + GetItemRectSize().x, cursorStartPos.y));

	// Details of selected resource
	BeginChild("Resource Inspector");
	{
		if (m_selectedResourceGuid != C_INVALID_GUID)
		{
			for (const MetaEntry *entry : resources)
			{
			    if (entry->guid == m_selectedResourceGuid)
			    {
			        MetaEntry modifiedEntry = *entry;
			        if (DrawResourceSettings(modifiedEntry))
			        {
			            // TODO: This has to keep retrying until it returns true somehow
			            m_engine->GetResourceManager().UpdateMetaEntry(m_selectedResourceGuid, modifiedEntry, true);
			        }
			    }
			}
		}
	}
	EndChild();
}

void ResourceBrowser::BuildBrowserTree(const std::vector<const MetaEntry *> &p_resources)
{
    m_rootBrowserNode.children.clear();
    m_rootBrowserNode.name = "Assets";
    m_rootBrowserNode.isDirectory = true;
    
    for (const MetaEntry *entry : p_resources)
    {
        std::filesystem::path relPath = entry->relAssetPath;
        BrowserNode *currentNode = &m_rootBrowserNode;
        
        // Iterate through the relative path
        auto it = relPath.begin();
        while (it != relPath.end())
        {
            std::string partName = it->generic_string();
            bool isLastNode = (std::next(it)) == relPath.end();
            
            // If the directory or file does not exist in the tree yet -> create it
            if (currentNode->children.find(partName) == currentNode->children.end())
            {
                currentNode->children[partName] = BrowserNode();
                currentNode->children[partName].name = partName;
                currentNode->children[partName].isDirectory = !isLastNode;
            }
            
            currentNode = &currentNode->children[partName];
            if (isLastNode)
            {
                // Node is a file -> Add the resource to it
                currentNode->resources.push_back(entry);
            }
            ++it;
        }
    }
}

void ResourceBrowser::DrawBrowserNode(BrowserNode &p_node)
{
    using namespace ImGui;
    
    if (p_node.isDirectory)
    {
        // Draw directory
        if (TreeNodeEx(p_node.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow))
        {
            for (auto &[childName, childNode] : p_node.children)
            {
                DrawBrowserNode(childNode);
            }
            
            TreePop();
        }
    }
    else
    {
        // Draw asset file
        if (p_node.resources.size() == 1)
        {
            // Singe resource asset
            const MetaEntry *entry = p_node.resources[0];
            bool isSelected = (m_selectedResourceGuid == entry->guid);
            std::string label = p_node.name + " [" + ResourceTypeToString(entry->type) + "]";
            if (Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
            {
                m_selectedResourceGuid = isSelected ? C_INVALID_GUID : entry->guid;
            }
        }
        else if (p_node.resources.size() > 1)
        {
            // Multi resource asset
            if (TreeNodeEx(p_node.name.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow))
            {
                for (const MetaEntry *entry : p_node.resources)
                {
                    bool isSelected = (m_selectedResourceGuid == entry->guid);
                    std::string label = p_node.name + " [" + ResourceTypeToString(entry->type) + "]";
                    if (Selectable(label.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        m_selectedResourceGuid = isSelected ? C_INVALID_GUID : entry->guid;
                    }
                }
                
                TreePop();
            }
        }
    }
}

bool ResourceBrowser::DrawResourceSettings(MetaEntry &p_entry) const
{
    using namespace ImGui;
    
    bool settingsChanged = false;
    
    // Name change here
    char nameBuffer[256];
    strncpy_s(nameBuffer, p_entry.name.c_str(), sizeof(nameBuffer));
    if (InputText("##ResourceName", nameBuffer, sizeof(nameBuffer)))
    {
        p_entry.name = std::string(nameBuffer);
    }
    
    Text("GUID:", p_entry.guid);
    SameLine();
    TextDisabled("%llu", p_entry.guid);
    
    
    // Draw UI based on descriptor
    auto schema = m_engine->GetResourceManager().GetSettingsSchema(p_entry.type);
    for (const auto &desc : schema)
    {
        if (!p_entry.loadSettings.contains(desc.key)) continue;
        
        if (desc.type == MetaLoadSettings::Type::Bool)
        {
            bool val = p_entry.loadSettings[desc.key].get<bool>();
            if (Checkbox(desc.key.c_str(), &val))
            {
                p_entry.loadSettings[desc.key] = val;
                settingsChanged = true;
            }
        }
        else if (desc.type == MetaLoadSettings::Type::Int)
        {
            int val = p_entry.loadSettings[desc.key].get<int>();
            if (InputInt(desc.key.c_str(), &val))
            {
                p_entry.loadSettings[desc.key] = val;
                settingsChanged = true;
            }
        }
        else if (desc.type == MetaLoadSettings::Type::Float)
        {
            float val = p_entry.loadSettings [desc.key].get<float>();
            if (InputFloat(desc.key.c_str(), &val))
            {
                p_entry.loadSettings[desc.key] = val;
                settingsChanged = true;
            }
        }
        else if (desc.type == MetaLoadSettings::Type::String)
        {
            std::string val = p_entry.loadSettings[desc.key].get<std::string>();
            char buffer[512]; 
            strncpy_s(buffer, val.c_str(), sizeof(buffer));
            if (InputText(desc.key.c_str(), buffer, sizeof(buffer)))
            {
                p_entry.loadSettings[desc.key] = std::string(buffer);
                settingsChanged = true;
            }
        }
        else if (desc.type == MetaLoadSettings::Type::Enum)
        {
            int currentIndex = p_entry.loadSettings[desc.key].get<int>();
            const char *previewVal;
            if (currentIndex >= 0 && currentIndex < desc.enumOptions.size())
            {
                previewVal = desc.enumOptions[currentIndex].c_str();
            }
            else
            {
                previewVal = "Unknown";
            }
            
            if (BeginCombo(desc.displayName.c_str(), previewVal))
            {
                for (int i = 0; i < desc.enumOptions.size(); i++)
                {
                    const bool isSelected = (currentIndex == i);
                    if (Selectable(desc.enumOptions[i].c_str(), isSelected))
                    {
                        p_entry.loadSettings[desc.key] = i;
                        settingsChanged = true;
                    }
                    
                    if (isSelected)
                    {
                        SetItemDefaultFocus();
                    }
                }
                
                EndCombo();
            }
        }
    }
    
    // TODO: Dependency modifications should go here when we start using that
    
    return settingsChanged;
}

std::string ResourceBrowser::ResourceTypeToString(ResourceType p_type)
{
    switch (p_type)
    {
        case ResourceType::Animation:
            return "Animation";
        case ResourceType::Material:
            return "Material";
        case ResourceType::Mesh:
            return "Mesh";
        case ResourceType::Shader:
            return "Shader";
        case ResourceType::SkinnedMesh:
            return "Skinned Mesh";
        case ResourceType::Texture2D:
                return "2D Texture";
        case ResourceType::Texture3D:
            return "3D Texture";
        default:
            return "None";
    }
}
