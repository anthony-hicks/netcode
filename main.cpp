#include <SDL2/SDL.h>

#include <cstdio>
#include <format>
#include <iostream>
#include <memory>

#define DISABLE_COPY(T) \
    T(T const&) = delete; \
    T& operator=(T const&) = delete

#define DISABLE_MOVE(T) \
    T(T&&) = delete; \
    T& operator=(T&&) = delete

static constexpr int SCREEN_WIDTH = 640;
static constexpr int SCREEN_HEIGHT = 480;

namespace SDL {
    namespace detail {

        class Library {
        public:
            // Initializes the library
            static bool initialize(uint32_t flags)
            {
                if (int err = SDL_Init(flags); err < 0) {
                    std::clog << std::format("ERROR: SDL_Init ({}): {}", err, SDL_GetError());
                    return false;
                }

                static Library library;

                return true;
            }

            // Shuts down the library
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
    }

    bool initialize(uint32_t flags)
    {
        return detail::Library::initialize(flags);
    }

    struct WindowDeleter {
        void operator()(SDL_Window* p) { SDL_DestroyWindow(p); }
    };

    using Window = std::unique_ptr<SDL_Window, WindowDeleter>;
}

int main(int argc, char* argv[])
{

    // TODO: .clang-format / .clang-tidy

    if (!SDL::initialize(SDL_INIT_VIDEO)) {
        return 1;
    }

    // Create window
    SDL::Window window(SDL_CreateWindow("Title",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    ));

    if (window == nullptr) {
        printf("Failed to create window: %s\n", SDL_GetError());
        return 1;
    }

    // Surface contained by the window
    SDL_Surface* screen_surface = SDL_GetWindowSurface(window.get());
    SDL_Surface* bmp_surface = SDL_LoadBMP("hello_world.bmp");
    if (bmp_surface == nullptr) {
        printf("Unable to load bmp: %s\n", SDL_GetError());
        return 1;
    }

    // Apply the image
    SDL_BlitSurface(bmp_surface, nullptr, screen_surface, nullptr);

    // Update surface
    SDL_UpdateWindowSurface(window.get());

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

    SDL_FreeSurface(bmp_surface);
    bmp_surface = nullptr;

    SDL_DestroyWindow(window.get());
    SDL_Quit();

    return 0;
}
