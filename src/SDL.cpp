#include "SDL.hpp"

#include <spdlog/spdlog.h>

#include <format>
#include <iostream>

// TODO: Doc design of naming with lib UpperCase
// TODO: Doc design choice:
// Since SDL doesn't give any API for using the error code,
// propagating it is useless. We only need it to determine success. In
// many cases, we should terminate on failure, but sometimes we may
// be able to recover. So a bool return makes sense.

namespace SDL {
namespace detail {

Library::~Library()
{
    // Clean up all initialized subsystems
    SDL_Quit();
}

Library::Library(uint32_t flags)
{
    if (int result = SDL_Init(flags); result < 0) {
        spdlog::error("SDL_Init ({}): {}", result, SDL_GetError());
        throw std::runtime_error(
          std::format("ERROR: SDL_Init ({}): {}", result, SDL_GetError())
        );
    }

    spdlog::debug("SDL initialized");
}

void Library::initialize(uint32_t flags)
{
    static Library const library(flags);
}

}  // namespace detail

void initialize(uint32_t flags)
{
    return detail::Library::initialize(flags);
}

SDL_Surface* GetWindowSurface(SDL_Window* window)
{
    // Returns NULL on failure
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    if (surface == nullptr) {
        spdlog::error("SDL_GetWindowSurface (nullptr): {}", SDL_GetError());
    }

    return surface;
}

// TODO: Ideally std::expected or even just std::optional
Surface_ptr LoadBMP(std::filesystem::path const& path)
{
    // Returns NULL on failure
    Surface_ptr surface(SDL_LoadBMP(path.string().c_str()));

    if (!surface) {
        spdlog::error("SDL_LoadBMP (nullptr): {}", SDL_GetError());
    }

    return surface;
}

Surface_ptr ConvertSurface(SDL_Surface* surface, SDL_PixelFormat const* format)
{
    // Returns NULL on failure
    Surface_ptr converted_surface(SDL_ConvertSurface(surface, format, 0));

    if (!converted_surface) {
        spdlog::error("SDL_ConvertSurface (nullptr): {}", SDL_GetError());
    }

    return converted_surface;
}

bool UpdateWindowSurface(SDL_Window* window)
{
    if (int err = SDL_UpdateWindowSurface(window); err < 0) {
        spdlog::error("SDL_UpdateWindowSurface ({}): {}", err, SDL_GetError());
        return false;
    }

    return true;
}

bool BlitSurface(
  SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect
)
{
    if (int err = SDL_BlitSurface(src, srcrect, dest, destrect); err < 0) {
        spdlog::error("SDL_BlitSurface (-1): {}", SDL_GetError());
        return false;
    }

    return true;
}

bool BlitScaled(
  SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dest, SDL_Rect* destrect
)
{
    if (int result = SDL_BlitScaled(src, srcrect, dest, destrect); result < 0) {
        spdlog::error("SDL_BlitScaled (-1): {}", SDL_GetError());
        return false;
    }

    return true;
}
}  // namespace SDL
