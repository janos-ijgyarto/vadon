#ifndef VADON_PRIVATE_PCH_GRAPHICSAPI_HPP
#define VADON_PRIVATE_PCH_GRAPHICSAPI_HPP
#include <memory>
#include <vector>
#include <unordered_map>
#include <array>

#include <string>
#include <format>

#include <cstdint>
#include <limits>
#include <algorithm>

#include <cassert>

#ifdef VADON_GRAPHICS_API_DIRECTX
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif
#endif