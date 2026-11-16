
#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace Droplet {
/// Generational handle: a stale handle (slot reused) fails lookup instead of
/// dangling
template <class Tag> 
struct Handle {
  static constexpr std::uint32_t kInvalid = 0xFFFFFFFFu;

  std::uint32_t index = kInvalid; ///< position in m_slots
  std::uint32_t generation = 0; ///< when object is replaced

  /// generates a key by packing index and generation into a single 64 bit uint
  constexpr std::uint64_t Key() const noexcept 
  {
    return (std::uint64_t{generation} << 32 | index);
  }
  
  /// rebuilds a handle from a value returned by Key()
  static constexpr Handle FromKey(std::uint64_t key) noexcept {
    return {
        .index = static_cast<std::uint32_t>(key & 0xFFFFFFFFu),
        .generation = static_cast<std::uint32_t>(key >> 32)
        };
        
  }

  constexpr bool IsValid() const noexcept { return index != kInvalid; }
  friend constexpr bool operator==(Handle, Handle) = default;
};

/// Slot pool. Remove() hands the object back so the caller can *retire* it (GPU
/// may still be using it) instead of destroying it here.
template <class Tag, class T> class HandlePool {
public:
  using HandleType = Handle<Tag>;

  HandleType Add(T value) 
  {
    std::uint32_t index;

    if (!m_free.empty()) 
    {
      index = m_free.back();
      m_free.pop_back();
    }
    else 
    {
      index = static_cast<std::uint32_t>(m_slots.size());
      m_slots.emplace_back();
    }
    m_slots[index].value.emplace(std::move(value));
 
    return {index, m_slots[index].generation};
  }

  T *Get(HandleType handleType) noexcept 
  {
    return IsAlive(handleType) 
    ? &*m_slots[handleType.index].value 
    : nullptr;
  }

  const T *Get(HandleType handleType) const noexcept 
  {
    return IsAlive(handleType) ? &*m_slots[handleType.index].value : nullptr;
  }

  std::optional<T> Remove(HandleType handle) 
  {
    if (!IsAlive(handle))
    {     
        return std::nullopt;
    }

    std::optional<T> out = std::move(m_slots[handle.index].value);
    m_slots[handle.index].value.reset();
    ++m_slots[handle.index].generation;
    m_free.push_back(handle.index);

    return out;
  }

  bool IsAlive(HandleType handleType) const noexcept {
    return handleType.index < m_slots.size() &&
           m_slots[handleType.index].generation == handleType.generation &&
           m_slots[handleType.index].value.has_value();
  }

private:
  struct Slot 
  {
    std::optional<T> value;
    std::uint32_t generation = 1; // iterates when the slot updates
  };
  std::vector<Slot> m_slots;
  std::vector<std::uint32_t> m_free;
};
} // namespace Droplet
