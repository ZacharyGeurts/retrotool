#include "main.hpp"
#include "file_monitor.hpp"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"
#include <filesystem>

Application::Application() : window(nullptr), renderer(nullptr), running(false) {
    file_monitor = std::make_unique<FileMonitor>("/mnt/ffdf60e6-db3c-45e1-a454-8897263729af/Screenshots",
                                                "/mnt/ffdf60e6-db3c-45e1-a454-8897263729af");
}

Application::~Application() { cleanupSDL(); }

bool Application::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) return false;
    window = SDL_CreateWindow("Application", 1920, 768, SDL_WINDOW_RESIZABLE);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) { SDL_DestroyWindow(window); SDL_Quit(); return false; }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.FontGlobalScale = 40.0f / 13.0f;
    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer) || !ImGui_ImplSDLRenderer3_Init(renderer)) {
        SDL_DestroyRenderer(renderer); SDL_DestroyWindow(window); SDL_Quit(); return false;
    }
    return true;
}

void Application::cleanupSDL() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

void Application::renderUI() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT) { running = false; return; }
    }
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    file_monitor->renderUI(); // Render FileMonitor window
    // Add more windows here in the future
    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

int Application::run() {
    if (!initSDL()) return 1;
    running = true;
    file_monitor->start();
    while (running) {
        renderUI();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60fps
    }
    file_monitor->stop();
    return 0;
}

int main() {
    Application app;
    return app.run();
}