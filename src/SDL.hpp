#pragma once

#include "common.hpp"

#include "SDL.h"

#include <filesystem>
#include <memory>

#define LOG_SDL_ERROR(fn, ret) spdlog::error(#fn": ({}) {}", ret, SDL_GetError())
#define RETURN_IF_SDL_ERROR(fn, ...) \
    if (auto ret = (fn)(__VA_ARGS__); ret < 0) LOG_SDL_ERROR((fn), ret)

namespace SDL {

namespace detail {

class Library {
public:
    /// @brief Initializes the library
    /// @throws std::runtime_error on failure
    static void initialize(uint32_t flags);

    /// @brief Shuts down the library
    ~Library();

    DISABLE_COPY(Library);
    DISABLE_MOVE(Library);

private:
    explicit Library(uint32_t flags);
};
}  // namespace detail

/// @brief Initializes the SDL library
/// @throws std::runtime_error on failure
void initialize(uint32_t flags);

struct Window_deleter {
    void operator()(SDL_Window* p) { SDL_DestroyWindow(p); }
};

using Window_ptr = std::unique_ptr<SDL_Window, Window_deleter>;

struct Surface_deleter {
    void operator()(SDL_Surface* p) { SDL_FreeSurface(p); }
};

using Surface_ptr = std::unique_ptr<SDL_Surface, Surface_deleter>;

struct Renderer_deleter {
    void operator()(SDL_Renderer* p) { SDL_DestroyRenderer(p); }
};

using Renderer_ptr = std::unique_ptr<SDL_Renderer, Renderer_deleter>;

// No abstraction for SDL_GetWindowSurface because a simple, nullable
// pointer is accurate (non-owning) and we don't have a need for
// an actual Window_ptr class yet where we could maybe implement a member
// function: window.surface() -> SDL_Surface*
//
// CCG recommends assuming every raw pointer can be null.
SDL_Surface* GetWindowSurface(SDL_Window* window);

Surface_ptr LoadBMP(std::filesystem::path const& path);

Surface_ptr ConvertSurface(SDL_Surface* surface, SDL_PixelFormat const* format);
}  // namespace SDL
