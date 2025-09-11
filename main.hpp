#ifndef MAIN_HPP
#define MAIN_HPP

#include <SDL3/SDL.h>
#include <memory>

class FileMonitor;

class Application {
public:
    Application();
    ~Application();
    int run();

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<FileMonitor> file_monitor;
    bool running;

    bool initSDL();
    void cleanupSDL();
    void renderUI();
};

#endif // MAIN_HPP