/// @file FontLibrary.hpp
/// @brief Vulkan::FontLibrary: the fonts the renderer holds (glyph tables), by GUID.
#pragma once

#include <unordered_map>

#include "Core/Guid.hpp"
#include "Assets/AssetTransfer.hpp"

namespace Vulkan
{
    /// @brief Glyph tables from Assets::FontUpload. Small CPU data: never evicted. The atlas is a texture of
    ///        GpuResourceManager.
    class FontLibrary
    {
    public:
        /// @brief Keeps the font unless the same or a newer revision is held.
        void Upload(const Assets::FontUpload& upload)
        {
            Entry& entry = m_fonts[upload.id];
            if (entry.revision >= upload.revision && entry.revision != 0)
                return;
            entry = { .revision = upload.revision, .font = upload.font };
        }

        /// @brief The font for `id`, or nullptr
        [[nodiscard]] const Assets::FontAsset* Find(Guid id) const
        {
            const auto fontIt = m_fonts.find(id);
            return fontIt != m_fonts.end() ? &fontIt->second.font : nullptr;
        }

        /// @brief Adds the fonts held, with their revisions, to `resident`.
        void AppendResident(Assets::ResidentAssets& resident) const
        {
            for (const auto& [id, entry] : m_fonts)
                resident.fonts.push_back({ .id = id, .revision = entry.revision });
        }

    private:
        struct Entry
        {
            std::uint64_t revision{ 0 };
            Assets::FontAsset font;
        };
        std::unordered_map<Guid, Entry> m_fonts;
    };
}
