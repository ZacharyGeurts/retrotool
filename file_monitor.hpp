#ifndef FILE_MONITOR_HPP
#define FILE_MONITOR_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>
#include <SDL3/SDL.h>
#include "imgui/imgui.h"

namespace fs = std::filesystem;

class FileMonitor {
public:
    FileMonitor(const fs::path& source, const fs::path& dest);
    ~FileMonitor();
    void start();
    void stop();
    void run();
    void renderUI();
    bool isRunning() const;

private:
    // Implementation class for PIMPL idiom
    class Impl {
    public:
        char source_buffer[1024];
        char dest_buffer[1024];
        std::string error_message;
        std::vector<std::string> systems; // Dropdown system names
        int selected_system; // Index of selected system
        bool use_named_titles; // Flip-flop state: true for Named_Titles, false for Named_Snaps
    };

    fs::path source_dir;
    fs::path dest_dir;
    std::vector<std::string> copied_files;
    std::thread monitor_thread;
    std::mutex mutex;
    bool running;
    SDL_Window* window;
    SDL_Renderer* renderer;
    std::unique_ptr<Impl> pimpl;

    bool initSDL();
    void cleanupSDL();
    void monitorFiles();
};

#endif // FILE_MONITOR_HPP