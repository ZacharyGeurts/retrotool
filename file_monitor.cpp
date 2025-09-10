// file_monitor.cpp
#include "file_monitor.hpp"
#include "imgui/backends/imgui_impl_sdl3.h"
#include "imgui/backends/imgui_impl_sdlrenderer3.h"
#include <iostream>
#include <chrono>
#include <cstring>

FileMonitor::FileMonitor(const fs::path& source, const fs::path& dest)
    : source_dir(source), dest_dir(dest), running(false), window(nullptr), renderer(nullptr), pimpl(std::make_unique<Impl>()) {
    // Initialize destination directory
    try {
        if (!fs::exists(dest_dir)) {
            fs::create_directories(dest_dir);
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating destination directory: " << e.what() << '\n';
        pimpl->error_message = "Failed to create destination directory: " + std::string(e.what());
    }
    // Initialize input buffers
    strncpy(pimpl->source_buffer, source_dir.string().c_str(), sizeof(pimpl->source_buffer) - 1);
    pimpl->source_buffer[sizeof(pimpl->source_buffer) - 1] = '\0';
    strncpy(pimpl->dest_buffer, dest_dir.string().c_str(), sizeof(pimpl->dest_buffer) - 1);
    pimpl->dest_buffer[sizeof(pimpl->dest_buffer) - 1] = '\0';
    // Initialize systems for dropdown
    pimpl->systems = {"System1", "System2", "System3"}; // Add more systems here
    pimpl->selected_system = 0; // Default to first system
    pimpl->use_named_titles = true; // Start with Named_Titles
}

FileMonitor::~FileMonitor() {
    stop();
    cleanupSDL();
}

bool FileMonitor::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) == 0) { // 0 is success
        std::cerr << "SDL_Init Error: " << SDL_GetError() << '\n';
        pimpl->error_message = "SDL initialization failed: " + std::string(SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("File Monitor", 1920, 768, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << '\n';
        pimpl->error_message = "Window creation failed: " + std::string(SDL_GetError());
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << '\n';
        pimpl->error_message = "Renderer creation failed: " + std::string(SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Set font scale for ~30 points (1 point ≈ 1.333 pixels, 30 points ≈ 40 pixels)
    io.FontGlobalScale = 40.0f / 13.0f;

    if (!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer)) {
        std::cerr << "ImGui_ImplSDL3_InitForSDLRenderer failed\n";
        pimpl->error_message = "ImGui SDL3 backend initialization failed";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    if (!ImGui_ImplSDLRenderer3_Init(renderer)) {
        std::cerr << "ImGui_ImplSDLRenderer3_Init failed\n";
        pimpl->error_message = "ImGui SDL Renderer backend initialization failed";
        ImGui_ImplSDL3_Shutdown();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    return true;
}

void FileMonitor::cleanupSDL() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    SDL_Quit();
}

void FileMonitor::monitorFiles() {
    while (running) {
        try {
            fs::path current_source, current_dest_base;
            std::string current_system;
            bool use_named_titles;
            {
                std::lock_guard<std::mutex> lock(mutex);
                current_source = source_dir;
                current_dest_base = dest_dir;
                current_system = pimpl->systems[pimpl->selected_system];
                use_named_titles = pimpl->use_named_titles;
            }
            if (!fs::exists(current_source) || !fs::is_directory(current_source)) {
                std::cerr << "Source directory invalid: " << current_source << '\n';
                pimpl->error_message = "Source directory invalid: " + current_source.string();
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
            // Construct destination path: dest_dir/system_name/[Named_Titles|Named_Snaps]
            fs::path sub_dir = use_named_titles ? "Named_Titles" : "Named_Snaps";
            fs::path current_dest = current_dest_base / current_system / sub_dir;
            try {
                if (!fs::exists(current_dest)) {
                    fs::create_directories(current_dest);
                }
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error creating destination subdirectory: " << e.what() << '\n';
                pimpl->error_message = "Error creating subdirectory: " + std::string(e.what());
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;
            }
            for (const auto& entry : fs::directory_iterator(current_source)) {
                if (fs::is_regular_file(entry)) {
                    fs::path dest_file = current_dest / entry.path().filename();
                    if (!fs::exists(dest_file)) {
                        try {
    							// Get the original filename
    							std::string filename = entry.path().filename().string();
    
    							// Remove the last two dashes and date-time portion, keeping the extension
    							size_t extension_pos = filename.rfind(".png");
    							if (extension_pos != std::string::npos) {
        							size_t last_two_dashes = filename.rfind("--", extension_pos);
        							if (last_two_dashes != std::string::npos) {
            							filename = filename.substr(0, last_two_dashes) + ".png";
        							}
    							}

    							// Construct the new destination path with the modified filename
    							fs::path dest_file = current_dest / filename;

								// Copy to ../thumbnails
								//fs::copy_file(entry, dest_file);
    							// Rename/move the file to thumbies
    							fs::rename(entry, dest_file);
    
    							std::lock_guard<std::mutex> lock(mutex);
    							copied_files.push_back(filename + " to " + current_system + "/" + sub_dir.string());
    							std::cout << "Moved: " << filename << " to " << current_dest << '\n';
    
    							// Flip the state for the next file
    							pimpl->use_named_titles = !pimpl->use_named_titles;
						} catch (const fs::filesystem_error& e) {
    						std::cerr << "Error moving file " << entry.path().filename() << ": " << e.what() << '\n';
    						pimpl->error_message = "Error moving " + entry.path().filename().string() + ": " + e.what();
						}
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << '\n';
            pimpl->error_message = "Filesystem error: " + std::string(e.what());
        } catch (const std::exception& e) {
            std::cerr << "Unexpected error: " << e.what() << '\n';
            pimpl->error_message = "Unexpected error: " + std::string(e.what());
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void FileMonitor::start() {
    if (!running) {
        if (!initSDL()) {
            std::cerr << "Failed to initialize SDL. Monitoring will not start.\n";
            return;
        }
        running = true;
        monitor_thread = std::thread(&FileMonitor::monitorFiles, this);
    }
}

void FileMonitor::stop() {
    if (running) {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }
}

bool FileMonitor::isRunning() const {
    return running;
}

void FileMonitor::renderUI() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT) {
            stop();
            return;
        }
    }

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // Overlay text in top-left corner
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize);
    {
        std::lock_guard<std::mutex> lock(mutex);
        ImGui::Text("Next: %s", pimpl->use_named_titles ? "Title" : "Snap");
    }
    ImGui::End();

    ImGui::Begin("File Monitor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // Source directory input
    ImGui::Text("Source Directory:");
    if (ImGui::InputText("##Source", pimpl->source_buffer, sizeof(pimpl->source_buffer)+200, ImGuiInputTextFlags_EnterReturnsTrue)) {
        fs::path new_source = pimpl->source_buffer;
        try {
            if (fs::exists(new_source) && fs::is_directory(new_source)) {
                std::lock_guard<std::mutex> lock(mutex);
                source_dir = new_source;
                pimpl->error_message.clear();
                std::cout << "Updated source directory to: " << source_dir << '\n';
            } else {
                pimpl->error_message = "Invalid source directory: " + std::string(pimpl->source_buffer);
                std::cerr << pimpl->error_message << '\n';
                strncpy(pimpl->source_buffer, source_dir.string().c_str(), sizeof(pimpl->source_buffer) - 1);
                pimpl->source_buffer[sizeof(pimpl->source_buffer) - 1] = '\0';
            }
        } catch (const fs::filesystem_error& e) {
            pimpl->error_message = "Error accessing source directory: " + std::string(e.what());
            std::cerr << pimpl->error_message << '\n';
            strncpy(pimpl->source_buffer, source_dir.string().c_str(), sizeof(pimpl->source_buffer) - 1);
            pimpl->source_buffer[sizeof(pimpl->source_buffer) - 1] = '\0';
        }
    }

    // Destination directory input
    ImGui::Text("Destination Directory:");
    if (ImGui::InputText("##Dest", pimpl->dest_buffer, sizeof(pimpl->dest_buffer)+200, ImGuiInputTextFlags_EnterReturnsTrue)) {
        fs::path new_dest = pimpl->dest_buffer;
        try {
            if (!fs::exists(new_dest)) {
                fs::create_directories(new_dest);
            }
            if (fs::is_directory(new_dest)) {
                std::lock_guard<std::mutex> lock(mutex);
                dest_dir = new_dest;
                pimpl->error_message.clear();
                std::cout << "Updated destination directory to: " << dest_dir << '\n';
            } else {
                pimpl->error_message = "Invalid destination directory: " + std::string(pimpl->dest_buffer);
                std::cerr << pimpl->error_message << '\n';
                strncpy(pimpl->dest_buffer, dest_dir.string().c_str(), sizeof(pimpl->dest_buffer) - 1);
                pimpl->dest_buffer[sizeof(pimpl->dest_buffer) - 1] = '\0';
            }
        } catch (const fs::filesystem_error& e) {
            pimpl->error_message = "Error updating destination directory: " + std::string(e.what());
            std::cerr << pimpl->error_message << '\n';
            strncpy(pimpl->dest_buffer, dest_dir.string().c_str(), sizeof(pimpl->dest_buffer) - 1);
            pimpl->dest_buffer[sizeof(pimpl->dest_buffer) - 1] = '\0';
        }
    }

    // System selection dropdown
    ImGui::Text("System:");
    if (ImGui::BeginCombo("##System", pimpl->systems[pimpl->selected_system].c_str())) {
        for (size_t i = 0; i < pimpl->systems.size(); ++i) {
            bool is_selected = (pimpl->selected_system == static_cast<int>(i));
            if (ImGui::Selectable(pimpl->systems[i].c_str(), is_selected)) {
                std::lock_guard<std::mutex> lock(mutex);
                pimpl->selected_system = static_cast<int>(i);
                pimpl->error_message.clear();
                std::cout << "Selected system: " << pimpl->systems[i] << '\n';
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Display error message if any
    if (!pimpl->error_message.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f)); // Red text
        ImGui::TextWrapped("Error: %s", pimpl->error_message.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Separator();
    ImGui::Text("Copied Files:");
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (copied_files.empty()) {
            ImGui::Text("(No files copied yet)");
        } else {
            for (const auto& file : copied_files) {
                ImGui::Text("%s", file.c_str());
            }
        }
    }

    ImGui::End();

    ImGui::Render();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void FileMonitor::run() {
    start();
    while (isRunning()) {
        renderUI();
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

int main() {
    const fs::path source_dir = "/mnt/ffdf60e6-db3c-45e1-a454-8897263729af/Screenshots";
    const fs::path dest_dir = "/mnt/ffdf60e6-db3c-45e1-a454-8897263729af/thumbies";

    FileMonitor monitor(source_dir, dest_dir);
    monitor.run();

    return 0;
}