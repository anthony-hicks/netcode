#include <SDL2/SDL.h>

#include <filesystem>
#include <format>
#include <iostream>
#include <memory>

#define DISABLE_COPY(T)                                                             \
    T(T const&) = delete;                                                           \
    T& operator=(T const&) = delete

#define DISABLE_MOVE(T)                                                             \
    T(T&&) = delete;                                                                \
    T& operator=(T&&) = delete

static constexpr int SCREEN_WIDTH = 640;
static constexpr int SCREEN_HEIGHT = 480;

namespace SDL {
namespace detail {

class Library {
public:
    /// @brief Initializes the library
    /// @throws std::runtime_error on failure
    static void initialize(uint32_t flags)
    {
        if (int err = SDL_Init(flags); err < 0) {
            throw std::runtime_error(
              std::format("ERROR: SDL_Init ({}): {}", err, SDL_GetError()));
        }

        static Library library;
    }

    /// @brief Shuts down the library
    ~Library()
    {
        // Clean up all initialized subsystems
        SDL_Quit();
    }

    DISABLE_COPY(Library);
    DISABLE_MOVE(Library);

private:
    Library() = default;
};
}  // namespace detail

void initialize(uint32_t flags) { return detail::Library::initialize(flags); }

struct WindowDeleter {
    void operator()(SDL_Window* p) { SDL_DestroyWindow(p); }
};

using Window = std::unique_ptr<SDL_Window, WindowDeleter>;

struct SurfaceDeleter {
    void operator()(SDL_Surface* p) { SDL_FreeSurface(p); }
};

using Surface = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

// No abstraction for SDL_GetWindowSurface because a simple, nullable
// pointer is accurate (non-owning) and we don't have a need for
// an actual Window class yet where we could maybe implement a member
// function: window.surface() -> SDL_Surface*
//
// CCG recommends assuming every raw pointer can be null.
SDL_Surface* GetWindowSurface(SDL_Window* window)
{
    // Returns NULL on failure
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    if (surface == nullptr) {
        std::clog << std::format("ERROR: SDL_GetWindowSurface (NULL): {}\n",
                                 SDL_GetError());
    }

    return surface;
}

Surface LoadBMP(std::filesystem::path const& path)
{
    // Returns NULL on failure
    std::clog << path.string() << '\n';
    Surface surface(SDL_LoadBMP(path.string().c_str()));

    if (!surface) {
        std::clog << std::format("ERROR: SDL_LoadBMP (NULL): {}\n", SDL_GetError());
    }

    return surface;
}

// TODO: Doc design of naming with lib UpperCase
// TODO: Doc design choice:
// Since SDL doesn't give any API for using the error code,
// propagating it is useless. We only need it to determine success. In
// many cases, we should terminate on failure, but sometimes we may
// be able to recover. So a bool return makes sense.
bool UpdateWindowSurface(SDL_Window* window)
{
    if (int err = SDL_UpdateWindowSurface(window); err < 0) {
        std::clog << std::format(
          "ERROR: SDL_UpdateWindowSurface ({}): {}", err, SDL_GetError());
        return false;
    }

    return true;
}

bool BlitSurface(SDL_Surface* src,
                 const SDL_Rect* srcrect,
                 SDL_Surface* dest,
                 SDL_Rect* destrect)
{
    if (int err = SDL_BlitSurface(src, srcrect, dest, destrect); err < 0) {
        std::clog << std::format("ERROR: SDL_BlitSurface (-1): {}", SDL_GetError());
        return false;
    }

    return true;
}

}  // namespace SDL

int main(int argc, char* argv[])
{
    // TODO: .clang-format / .clang-tidy

    SDL::initialize(SDL_INIT_VIDEO);

    // Create window
    SDL::Window window(SDL_CreateWindow("Title",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SCREEN_WIDTH,
                                        SCREEN_HEIGHT,
                                        SDL_WINDOW_SHOWN));

    if (!window) {
        return 1;
    }

    // Surface contained by the window
    SDL_Surface* screen_surface = SDL::GetWindowSurface(window.get());
    if (!screen_surface) {
        return 1;
    }

    SDL::Surface bmp_surface = SDL::LoadBMP("helloworld.bmp");
    if (!bmp_surface) {
        return 1;
    }

    // Apply the image
    if (!SDL::BlitSurface(bmp_surface.get(), nullptr, screen_surface, nullptr)) {
        return 1;
    }

    if (!SDL::UpdateWindowSurface(window.get())) {
        return 1;
    }

    // Hack to get window to stay up
    SDL_Event event;
    bool quit = false;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    return 0;
}
