#include <SDL2/SDL.h>

#include <algorithm>
#include <filesystem>
#include <format>
#include <iostream>
#include <memory>
#include <ranges>
#include <unordered_map>

#define DECLARE_COPY(T, keyword)                                                    \
    T(T const&) = keyword;                                                          \
    T& operator=(T const&) = keyword

#define DEFAULT_COPY(T) DECLARE_COPY(T, default);
#define DISABLE_COPY(T) DECLARE_COPY(T, delete);

#define DECLARE_MOVE(T, keyword)                                                    \
    T(T&&) = keyword;                                                               \
    T& operator=(T&&) = keyword

#define DEFAULT_MOVE(T) DECLARE_MOVE(T, default);
#define DISABLE_MOVE(T) DECLARE_MOVE(T, delete);

static constexpr int SCREEN_WIDTH = 640;
static constexpr int SCREEN_HEIGHT = 480;

// TODO: Set up intellisense and clang-format here

namespace SDL {

namespace detail {

class Library {
public:
    /// @brief Initializes the library
    /// @throws std::runtime_error on failure
    static void initialize(uint32_t flags)
    {
        if (int result = SDL_Init(flags); result < 0) {
            throw std::runtime_error(
              std::format("ERROR: SDL_Init ({}): {}", result, SDL_GetError()));
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

using Surface_Ptr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

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

Surface_Ptr LoadBMP(std::filesystem::path const& path)
{
    // Returns NULL on failure
    Surface_Ptr surface(SDL_LoadBMP(path.string().c_str()));

    if (!surface) {
        std::clog << std::format("ERROR: SDL_LoadBMP (NULL): {}\n", SDL_GetError());
    }

    return surface;
}

Surface_Ptr ConvertSurface(SDL_Surface* surface, SDL_PixelFormat const* format)
{
    // Returns NULL on failure
    Surface_Ptr converted_surface(SDL_ConvertSurface(surface, format, 0));

    if (!converted_surface) {
        std::clog << std::format("ERROR: SDL_ConvertSurface (NULL): {}\n",
                                 SDL_GetError());
    }

    return converted_surface;
}

// TODO: Things to learn before I can begin
//  - draw to screen
//  - keyboard input
//  - how to "move" the circle left or right (translation?)
//      - maybe this is just changing the x/y before blitting?
//      - or possibly changing velocity based on ch. 26
//      - it looks like both are possible approaches

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

bool BlitScaled(SDL_Surface* src,
                const SDL_Rect* srcrect,
                SDL_Surface* dest,
                SDL_Rect* destrect)
{
    if (int result = SDL_BlitScaled(src, srcrect, dest, destrect); result < 0) {
        std::clog << std::format("ERROR: SDL_BlitScaled (-1): {}", SDL_GetError());
        return false;
    }

    return true;
}

}  // namespace SDL

namespace KeyPressSurface {
enum KeyPressSurfaceEnum {
    DEFAULT,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NOE
};
};  // namespace KeyPressSurface

using KeyPressSurfaceEnum = KeyPressSurface::KeyPressSurfaceEnum;

SDL::Surface_Ptr load_optimized_bmp(std::filesystem::path const& path,
                                    SDL_PixelFormat const* format)
{
    return SDL::ConvertSurface(SDL::LoadBMP(path).get(), format);
}

int main(int argc, char* argv[])
{
    // TODO: .clang-format / .clang-tidy

    SDL::initialize(SDL_INIT_VIDEO);

    // Create window
    SDL::Window window(SDL_CreateWindow("Keyboard Input!!!",
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

    std::array<SDL::Surface_Ptr, KeyPressSurface::NOE> surfaces{};
    surfaces[KeyPressSurface::DEFAULT] = load_optimized_bmp(
      "ext/tutorial/04_key_presses/press.bmp", screen_surface->format);
    surfaces[KeyPressSurface::UP] = load_optimized_bmp(
      "ext/tutorial/04_key_presses/up.bmp", screen_surface->format);
    surfaces[KeyPressSurface::DOWN] = load_optimized_bmp(
      "ext/tutorial/04_key_presses/down.bmp", screen_surface->format);
    surfaces[KeyPressSurface::LEFT] = load_optimized_bmp(
      "ext/tutorial/04_key_presses/left.bmp", screen_surface->format);
    surfaces[KeyPressSurface::RIGHT] = load_optimized_bmp(
      "ext/tutorial/04_key_presses/right.bmp", screen_surface->format);

    if (std::ranges::any_of(surfaces, [](auto const& surface) {
            return !surface;
        })) {
        return 1;
    }

    SDL_Event event;
    SDL_Surface* current_surface = surfaces[KeyPressSurface::DEFAULT].get();

    // Game loop
    while (true) {
        // Poll event queue. When the queue is empty, this function returns 0.
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                return 0;
            }

            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP:
                        current_surface = surfaces[KeyPressSurface::UP].get();
                        break;
                    case SDLK_DOWN:
                        current_surface = surfaces[KeyPressSurface::DOWN].get();
                        break;
                    case SDLK_LEFT:
                        current_surface = surfaces[KeyPressSurface::LEFT].get();
                        break;
                    case SDLK_RIGHT:
                        current_surface = surfaces[KeyPressSurface::RIGHT].get();
                        break;
                    default:
                        current_surface = surfaces[KeyPressSurface::DEFAULT].get();
                        break;
                }
            }

            SDL_Rect stretched_rect;
            stretched_rect.x = 0;
            stretched_rect.y = 0;
            stretched_rect.w = SCREEN_WIDTH;
            stretched_rect.h = SCREEN_HEIGHT;

            if (!SDL::BlitScaled(
                  current_surface, nullptr, screen_surface, &stretched_rect)) {
                return 1;
            }

            if (!SDL::UpdateWindowSurface(window.get())) {
                return 1;
            }
        }
    }

    return 0;
}
