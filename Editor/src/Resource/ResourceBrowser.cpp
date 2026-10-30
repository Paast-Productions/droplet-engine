#include "Resource/ResourceBrowser.hpp"

#include <IconFontCppHeaders/IconsLucide.h>

#include <core/FileExplorer.hpp>
#include <resource/meta/MetaUtils.hpp>
#include <Editor/DragDropFields.hpp>

#include <ImGui/imgui.h>
#include <ImGui/imgui_stdlib.h>
#include <resource/types/TextureResource.hpp>

using namespace Droplet::Editor::Resource;

ResourceBrowser::ResourceBrowser(EditorContext *p_editorContext) : m_editorContext(p_editorContext)
{}

void ResourceBrowser::InitImpl()
{
	SetName("Resource Browser");
	BuildBrowserTree(ResourceManager::GetInstance().GetRegisteredResources());
}

void ResourceBrowser::RenderImpl()
{
	using namespace ImGui;
	
	bool updatedRegistry = false;

	ResourceManager &resManInstance = ResourceManager::GetInstance();
	
	// --- Registering Resources ---
	
	if (Button("Register Resource"))
	{
		RegisterAsset();
	}
	SameLine();
	TextDisabled(std::filesystem::absolute(resManInstance.GetRootDirectory()).string().c_str());
	Separator();
	
	// --- Queued Popup Rendering ---
	if (m_showUnsupportedPopup)
	{
		OpenPopup("Unsupported Assets");
		m_showUnsupportedPopup = false;
	}
	
	bool isUnsupportedPopupOpen = true;
	if (BeginPopupModal("Unsupported Assets", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		Text("The following files are not supported:");
		Spacing();
		TextDisabled("%s", m_unsupportedFilesList.c_str());
		Spacing();
		Spacing();
		
		float buttonWidth = 100.0f;
		SetCursorPosX(GetWindowWidth() - buttonWidth - GetStyle().WindowPadding.x);
		if (Button("Ok", ImVec2(buttonWidth, 0)))
		{
			CloseCurrentPopup();
		}
	
		EndPopup();
	}
	else
	{
		isUnsupportedPopupOpen = false;
	}
	
	if (!m_registerPopupQueue.empty() && !isUnsupportedPopupOpen && !IsPopupOpen("Resource Settings"))
	{
		OpenPopup("Resource Settings");
		m_selectedResourceGuid = C_INVALID_GUID;
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
				static_cast<void>(resManInstance.UpdateMetaEntry(entry.guid, entry));
				updatedRegistry = true;
			}
			
			m_registerPopupQueue.pop(); // Registering has been handled
			CloseCurrentPopup();
		}
		
		if (m_registerPopupQueue.size() > 1)
		{
			TextDisabled("(%zu more files waiting...)", m_registerPopupQueue.size() - 1);
		}
		
		EndPopup();
	}
	
	// --- Resource Browser ---
	
	// Split the window into two child windows: one for the resource list and one for the resource details

	ImVec2 windowSize = GetWindowSize();
	ImVec2 cursorStartPos = GetCursorPos();
	ImVec2 listSize = ImVec2(windowSize.x * 0.5f, 0);
	
	std::vector<const MetaEntry *> resources = resManInstance.GetRegisteredResources();
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
	DrawResourceList(resources, currentNode);
	EndChild();

	SetCursorPos(ImVec2(cursorStartPos.x + GetItemRectSize().x, cursorStartPos.y));

	// Details of selected resource
	BeginChild("Resource Inspector", ImVec2(0, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);
	{
		if (m_selectedResourceGuid != C_INVALID_GUID)
		{
			for (const MetaEntry *entry : resources)
			{
				if (entry->guid != m_selectedResourceGuid)
					continue;
				
				DrawResourceInspector(*entry);
				break;
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

void ResourceBrowser::DrawResourceList(const std::vector<const MetaEntry *> &p_resources, BrowserNode *p_currentNode)
{
	using namespace ImGui;

	ResourceManager &resManInstance = ResourceManager::GetInstance();

	TextDisabled("Current Path: /%s", m_currentBrowserPath.generic_string().c_str());
	Separator();

	// Draw go back item
	if (!m_currentBrowserPath.empty())
	{
		if (Selectable(ICON_LC_CORNER_LEFT_UP "..", false))
		{
			m_currentBrowserPath = m_currentBrowserPath.parent_path();
		}
		Separator();
	}

	// Draw directories
	for (auto &[childName, childNode] : p_currentNode->children)
	{
		if (childNode.isDirectory)
		{
			std::string label = ICON_LC_FOLDER " " + childName;
			if (Selectable(label.c_str(), false))
			{
				m_currentBrowserPath /= childName;
			}
		}
	}

	// Draw files and their resources
	for (auto &[childName, childNode] : p_currentNode->children)
	{
		if (!childNode.isDirectory)
		{
			// --- Asset ---
			std::string fileLabel = ICON_LC_FILE " " + childName;
			bool isNodeOpen = TreeNodeEx(fileLabel.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
			
			// Asset drag source
			if (BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				// Create a list of pointers to the resource metadata entries.
				std::vector<const MetaEntry *> payloads;
				payloads.reserve(childNode.resources.size());

				for (const MetaEntry *entry : childNode.resources)
				{
					payloads.push_back(entry);
				}

				SetDragDropPayload(
					PAYLOAD_ASSET.data(),
					payloads.data(),
					payloads.size() * sizeof(const MetaEntry *));

				TextDisabled(
					"Asset: %s (%zu resources)",
					childName.c_str(),
					payloads.size());

				EndDragDropSource();
			}

			// Resources it contains
			if (isNodeOpen)
			{
				for (const MetaEntry *entry : childNode.resources)
				{
					// --- Resource ---
					bool isSelected = (m_selectedResourceGuid == entry->guid);
					std::string resLabel = ICON_LC_PACKAGE " " + entry->displayName + " [" + ResourceTypeToString(entry->type) + "]";
					if (Selectable(resLabel.c_str(), isSelected))
					{
						if (isSelected)
						{
							m_selectedResourceGuid = C_INVALID_GUID;
						}
						else
						{
							m_selectedResourceGuid = entry->guid;
							m_inspectedEntry = *entry;
						}
					}

					// Resource drag source
					if (BeginDragDropSource())
					{
						const MetaEntry *payload = entry;

						SetDragDropPayload(
							PAYLOAD_RESOURCE.data(),
							&payload,
							sizeof(payload));

						TextDisabled("Resource: %s", entry->displayName.c_str());

						EndDragDropSource();
					}
				}
				TreePop();
			}
		}
	}
}

void ResourceBrowser::DrawResourceInspector(const MetaEntry &p_entry)
{
	using namespace ImGui;

	ResourceManager &resManInstance = ResourceManager::GetInstance();

	ImVec2 windowSize = GetWindowSize();
	ImVec2 cursorStartPos = GetCursorPos();
	ImVec2 settingsSize = ImVec2(0, windowSize.y * 0.6f);

	SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, windowSize.y - 12));

	BeginChild("Resource Settings", settingsSize, ImGuiChildFlags_ResizeY);

	DrawResourceSettings(m_inspectedEntry);

	bool isDirty = (m_inspectedEntry != p_entry);
	if (isDirty)
	{
		Spacing();
		Separator();
		Spacing();

		if (Button("Revert", ImVec2(100, 0)))
		{
			m_inspectedEntry = p_entry; // Reset to the original
		}
		SameLine();
		if (Button("Apply", ImVec2(100, 0)))
		{
			static_cast<void>(resManInstance.UpdateMetaEntry(m_selectedResourceGuid, m_inspectedEntry, true));
		}
	}
	EndChild();

	float settingsHeight = GetItemRectSize().y;
	SetCursorPosY(cursorStartPos.y + settingsHeight - GetTextLineHeightWithSpacing() / 2);
	SeparatorText("Preview");
	SetCursorPosY(cursorStartPos.y + settingsHeight + GetTextLineHeightWithSpacing() / 2);

	BeginChild("Resource Preview");
	DrawResourcePreview(m_inspectedEntry);
	EndChild();
}

void ResourceBrowser::DrawResourceSettings(MetaEntry &p_entry) const
{
	using namespace ImGui;

	// Name change here
	InputText("##ResourceName", &p_entry.displayName);

	Text("GUID:", p_entry.guid);
	SameLine();
	TextDisabled("%llu", p_entry.guid);


	// Draw UI based on descriptor
	auto schema = ResourceManager::GetInstance().GetSettingsSchema(p_entry.type);
	for (const auto &desc : schema)
	{
		if (!p_entry.loadSettings.contains(desc.key)) continue;

		if (desc.type == MetaLoadSettings::Type::Bool)
		{
			bool val = p_entry.loadSettings[desc.key].get<bool>();
			if (Checkbox(desc.key.c_str(), &val))
			{
				p_entry.loadSettings[desc.key] = val;
			}
		}
		else if (desc.type == MetaLoadSettings::Type::Int)
		{
			int val = p_entry.loadSettings[desc.key].get<int>();
			if (InputInt(desc.key.c_str(), &val))
			{
				p_entry.loadSettings[desc.key] = val;
			}
		}
		else if (desc.type == MetaLoadSettings::Type::Float)
		{
			float val = p_entry.loadSettings[desc.key].get<float>();
			if (InputFloat(desc.key.c_str(), &val))
			{
				p_entry.loadSettings[desc.key] = val;
			}
		}
		else if (desc.type == MetaLoadSettings::Type::String)
		{
			std::string val = p_entry.loadSettings[desc.key].get<std::string>();
			char buffer[512];
			snprintf(buffer, sizeof(buffer), "%s", val.c_str());
			if (InputText(desc.key.c_str(), buffer, sizeof(buffer)))
			{
				p_entry.loadSettings[desc.key] = std::string(buffer);
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
}

void ResourceBrowser::DrawResourcePreview(const MetaEntry &p_entry)
{
	using namespace ImGui;

	ResourceManager &resManInstance = ResourceManager::GetInstance();

	ImVec2 regionSize = GetContentRegionAvail();
	ImVec2 regionMin = GetCursorPos();
	ImVec2 regionMax = regionMin + regionSize;
	ImVec2 regionCenter = (regionMax - regionMin) * 0.5f;

	switch (p_entry.type)
	{
	case ResourceType::Texture2D:
		if (m_previewResourceHandle.GetGUID() != p_entry.guid)
		{
			if (m_previewResourceHandle.IsValid())
			{
				IntermediaryStorage::GetInstance().RemoveImGuiTexture(m_previewResourceHandle.GetGUID());
			}

			m_isPreviewResourceLoaded = false;

			ResourceHandle<Texture2DResource> textureHandle = resManInstance.LoadResource<Texture2DResource>(
				p_entry.guid,
				[this](ResourceHandle<Texture2DResource> handle) { m_isPreviewResourceLoaded = true; }
			);

			m_previewResourceHandle = ResourceHandle<IResource>(textureHandle.GetGUID(), &resManInstance);
		}

		if (m_isPreviewResourceLoaded)
		{
			GUID guid = m_previewResourceHandle.GetGUID();

			ResourceHandle<Texture2DResource> texHandle = ResourceHandle<Texture2DResource>(guid, &resManInstance);
			Texture2DResource *texture = texHandle.Get();

			// Calculate the size to maintain aspect ratio
			ImVec2 imageSize = ImVec2(static_cast<float>(texture->GetWidth()), static_cast<float>(texture->GetHeight()));

			float imageAspect = imageSize.x / imageSize.y;
			float regionAspect = regionSize.x / regionSize.y;

			float imageScale = 1.0f;
			if (imageAspect > regionAspect)
			{
				imageScale = regionSize.x / imageSize.x;
			}
			else
			{
				imageScale = regionSize.y / imageSize.y;
			}

			ImVec2 scaledImageSize = imageSize * imageScale;
			ImVec2 previewPos = regionCenter - scaledImageSize * 0.5f;

			//ImTextureRef textureRef = m_editorContext->GetEngine().GetOrCreateImGuiTexture(guid);
			ImTextureRef textureRef = IntermediaryStorage::GetInstance().GetOrCreateImGuiTexture(guid);
			
			SetCursorPos(previewPos);
			BeginChild("Preview Image", scaledImageSize);
			ImGui::Image(textureRef, GetContentRegionAvail(), ImVec2(0, 0), ImVec2(1, 1));
			EndChild();
		}

		break;

	default:
	case ResourceType::None:
	case ResourceType::Texture3D:
	case ResourceType::Mesh:
	case ResourceType::SkinnedMesh:
	case ResourceType::Animation:
	case ResourceType::Shader:
	case ResourceType::Material:
		// Draw text in center of the preview area
		ImVec2 textSize = CalcTextSize("No Preview Available");
		SetCursorPosX(regionCenter.x - textSize.x * 0.5f);
		SetCursorPosY(regionCenter.y - textSize.y * 0.5f);
		TextDisabled("No Preview Available");
		break;
	}
}

void ResourceBrowser::RegisterAsset()
{
	ResourceManager &resManInstance = ResourceManager::GetInstance();

	auto absPaths = FileExplorer::OpenFileDialog(true, resManInstance.GetRootDirectory());
	if (!absPaths.empty())
	{
		std::filesystem::path assetRoot = resManInstance.GetRootDirectory();
		std::filesystem::path normAssetRoot = std::filesystem::weakly_canonical(assetRoot);
		
		std::filesystem::path destinationBrowserDir = normAssetRoot;
		if (!m_currentBrowserPath.empty())
		{
			destinationBrowserDir /= m_currentBrowserPath;
		}
		
		m_unsupportedFilesList.clear(); // Clear previous
			
		for (auto &absPath : absPaths)
		{
			if (!resManInstance.IsAssetSupported(absPath))
			{
				m_unsupportedFilesList += "- " + absPath.filename().string() + "\n";
				continue;
			}
			
			std::filesystem::path relPath;
			
			// Normalize to be case-insensitive
			std::filesystem::path normAbsPath = std::filesystem::weakly_canonical(absPath);
			
			// Check if it's already in the directory
			std::filesystem::path relPathTemp = normAbsPath.lexically_relative(normAssetRoot); // Generate relative path
			if (relPathTemp.empty() || relPathTemp.string().starts_with("..") || relPathTemp.is_absolute())
			{
				// The file is from outside the root asset dir -> Copy it
				std::filesystem::path destination = destinationBrowserDir / normAbsPath.filename();
				std::filesystem::create_directories(destination.parent_path()); // Create in case it's been deleted
				
				std::error_code ec;
				std::filesystem::copy_file(normAbsPath, destination, std::filesystem::copy_options::overwrite_existing, ec);
				if (ec)
				{
					// TODO: Log error
					continue;
				}
				
				relPath = destination.lexically_relative(normAssetRoot);
			}
			else
			{
				relPath = relPathTemp;
			}
			
			resManInstance.RegisterAsset(relPath,
				[this](bool p_succeeded, const std::vector<MetaEntry>& entries)
				{
					if (p_succeeded && !entries.empty())
					{
						m_registerPopupQueue.push(entries);
					}
				}
			);
		}
		// Show popup if any of the files were unsupported
		if (!m_unsupportedFilesList.empty())
		{
			m_showUnsupportedPopup = true;
		}
	}
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
