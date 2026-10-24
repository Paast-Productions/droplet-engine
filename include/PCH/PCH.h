// Any files that are not frequently changed and are used across multiple source files can be included in a precompiled header (PCH) to speed up compilation times. 
// This file serves as a precompiled header for the project, including commonly used headers and external libraries.

#pragma once

// ALWAYS FIRST INCLUDE
#include <Debug/TracyTrackedAlloc.hpp>
#include <Core/GlobalDefs.hpp>

// C++ library headers
#include <algorithm>
#include <bitset>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
#include <valarray>
#include <vector>

// C++11
#include <array>
#include <atomic>
#include <chrono>
#include <codecvt>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

// C++14
#include <shared_mutex>

// C++17
#include <any>
#include <charconv>
#include <execution>
#include <filesystem>
#include <memory_resource>
#include <optional>
#include <string_view>
#include <variant>

// C++20
#include <barrier>
#include <bit>
#include <compare>
#include <concepts>
#include <coroutine>
#include <format>
#include <latch>
#include <numbers>
#include <ranges>
#include <semaphore>
#include <source_location>
#include <span>
#include <stop_token>
#include <syncstream>
#include <version>

// C++23
#include <expected>
#include <generator>
#include <mdspan>
#include <print>
#include <spanstream>
#include <stacktrace>
#include <stdfloat>

// "C++ headers for C library facilities"
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>

// "C++ headers for C library facilities" - C++11
#include <cfenv>
#include <cinttypes>
#include <cstdint>
#include <cuchar>

// Externals
#include <json/json.hpp>
#include <glm/glm.hpp>
#include <tracy/public/tracy/Tracy.hpp>

// Engine
#include <Core/IoManager.hpp>
#include <Core/StringUtils.hpp>
#include <Core/ThreadPool.hpp>
#include <Core/ThreadSafeQueue.hpp>
#include <Debug/Logger.hpp>
#include <Editor/DragDropFields.hpp>
#include <Engine/EngineFlags.hpp>
