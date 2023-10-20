#include <sdl/sdl.hpp>

#include <algorithm>
#include <ranges>


static constexpr int SCREEN_WIDTH = 640;
static constexpr int SCREEN_HEIGHT = 480;

namespace KeyPressSurface {
enum KeyPressSurfaceEnum {
    DEFAULT,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    NOE
};
}  // namespace KeyPressSurface

using KeyPressSurfaceEnum = KeyPressSurface::KeyPressSurfaceEnum;

SDL::Surface_Ptr load_optimized_bmp(std::filesystem::path const& path,
                                    SDL_PixelFormat const* format)
{
    return SDL::ConvertSurface(SDL::LoadBMP(path).get(), format);
}

// TODO: Things to learn before I can begin
//  - draw to screen
//  - keyboard input
//  - how to "move" the circle left or right (translation?)
//      - maybe this is just changing the x/y before blitting?
//      - or possibly changing velocity based on ch. 26
//      - it looks like both are possible approaches
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
      "ext/04_key_presses/press.bmp", screen_surface->format);
    surfaces[KeyPressSurface::UP] = load_optimized_bmp(
      "ext/04_key_presses/up.bmp", screen_surface->format);
    surfaces[KeyPressSurface::DOWN] = load_optimized_bmp(
      "ext/04_key_presses/down.bmp", screen_surface->format);
    surfaces[KeyPressSurface::LEFT] = load_optimized_bmp(
      "ext/04_key_presses/left.bmp", screen_surface->format);
    surfaces[KeyPressSurface::RIGHT] = load_optimized_bmp(
      "ext/04_key_presses/right.bmp", screen_surface->format);

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
