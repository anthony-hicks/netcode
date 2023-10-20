#pragma once

#include "SDL.h"

#include <filesystem>
#include <memory>

#define DECLARE_COPY(T, keyword)                                                    \
    T(T const&) = keyword;                                                          \
    T& operator=(T const&) = keyword

#define DEFAULT_COPY(T) DECLARE_COPY(T, default)
#define DISABLE_COPY(T) DECLARE_COPY(T, delete)

#define DECLARE_MOVE(T, keyword)                                                    \
    T(T&&) = keyword;                                                               \
    T& operator=(T&&) = keyword

#define DEFAULT_MOVE(T) DECLARE_MOVE(T, default)
#define DISABLE_MOVE(T) DECLARE_MOVE(T, delete)

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

struct WindowDeleter {
    void operator()(SDL_Window* p) { SDL_DestroyWindow(p); }
};
using Window = std::unique_ptr<SDL_Window, WindowDeleter>;

struct SurfaceDeleter {
    void operator()(SDL_Surface* p) { SDL_FreeSurface(p); }
};
using Surface_Ptr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

// No abstraction for SDL_GetWindowSurface because a simple, nullable
// pointer is accurate (non-owning) and we don't have a need for
// an actual Window class yet where we could maybe implement a member
// function: window.surface() -> SDL_Surface*
//
// CCG recommends assuming every raw pointer can be null.
SDL_Surface* GetWindowSurface(SDL_Window* window);

Surface_Ptr LoadBMP(std::filesystem::path const& path);

Surface_Ptr ConvertSurface(SDL_Surface* surface, SDL_PixelFormat const* format);

bool UpdateWindowSurface(SDL_Window* window);

bool BlitSurface(SDL_Surface* src,
                 const SDL_Rect* srcrect,
                 SDL_Surface* dest,
                 SDL_Rect* destrect);

bool BlitScaled(SDL_Surface* src,
                const SDL_Rect* srcrect,
                SDL_Surface* dest,
                SDL_Rect* destrect);

}  // namespace SDL
