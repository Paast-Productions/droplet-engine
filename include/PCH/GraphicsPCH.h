#pragma once
#include <PCH.h>

// Vulkan & VMA
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

// Slang
#ifdef _WIN32
#include <slang/slang.h>
#include <slang/slang-com-ptr.h>
#elifdef __linux__
#include <shader-slang/slang.h>
#include <shader-slang/slang-com-ptr.h>
#endif

// SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>

// ImGui
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_sdl3.h>