#include "file_monitor.hpp"
#include <imgui/imgui.h>
#include <iostream>
#include <chrono>
#include <cstring>
#include <algorithm> // For std::sort

FileMonitor::FileMonitor(const fs::path& source, const fs::path& dest)
    : source_dir(source), dest_dir(dest), pimpl(std::make_unique<Impl>()) {
    if (!fs::exists(dest_dir)) fs::create_directories(dest_dir);
    strncpy(pimpl->source_buffer, source.string().c_str(), sizeof(pimpl->source_buffer) - 1);
    strncpy(pimpl->dest_buffer, dest.string().c_str(), sizeof(pimpl->dest_buffer) - 1);
    initSystems(); // Initialize systems list
}

FileMonitor::~FileMonitor() { stop(); }

void FileMonitor::initSystems() {
    pimpl->systems.assign(std::begin(SYSTEM_NAMES), std::end(SYSTEM_NAMES));
    std::sort(pimpl->systems.begin(), pimpl->systems.end()); // Ensure alphabetical order
}

void FileMonitor::copyAndMoveFile(const fs::path& src, const fs::path& dest) {
    std::string filename = src.filename().string();
    size_t ext_pos = filename.rfind(".png");
    if (ext_pos != std::string::npos) {
        size_t dash_pos = filename.rfind("--", ext_pos);
        if (dash_pos != std::string::npos)
            filename = filename.substr(0, dash_pos) + ".png";
    }
    fs::path final_dest = dest / filename;
    fs::path thumb_dest = dest.parent_path().parent_path() / "thumbnails" / filename;
    if (!fs::exists(thumb_dest.parent_path())) fs::create_directories(thumb_dest.parent_path());
    try {
        fs::copy_file(src, thumb_dest, fs::copy_options::overwrite_existing);
        fs::rename(src, final_dest);
        std::lock_guard<std::mutex> lock(mutex);
        copied_files.push_back(filename + " to " + pimpl->systems[pimpl->selected_system] + "/" + (pimpl->use_named_titles ? "Named_Titles" : "Named_Snaps"));
        pimpl->use_named_titles = !pimpl->use_named_titles;
    } catch (const fs::filesystem_error& e) {
        pimpl->error_message = "Error processing " + filename + ": " + e.what();
    }
}

void FileMonitor::monitorFiles() {
    while (running) {
        fs::path src, dest_base;
        std::string sys;
        bool use_titles;
        {
            std::lock_guard<std::mutex> lock(mutex);
            src = source_dir;
            dest_base = dest_dir;
            sys = pimpl->systems[pimpl->selected_system];
            use_titles = pimpl->use_named_titles;
        }
        if (!fs::is_directory(src)) {
            pimpl->error_message = "Invalid source: " + src.string();
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }
        fs::path sub_dir = use_titles ? "Named_Titles" : "Named_Snaps";
        fs::path dest = dest_base / sys / sub_dir;
        if (!fs::exists(dest)) fs::create_directories(dest);
        try {
            for (const auto& entry : fs::directory_iterator(src)) {
                if (fs::is_regular_file(entry) && !fs::exists(dest / entry.path().filename()))
                    copyAndMoveFile(entry.path(), dest);
            }
        } catch (const fs::filesystem_error& e) {
            pimpl->error_message = "Filesystem error: " + std::string(e.what());
        }
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void FileMonitor::start() {
    if (!running) {
        running = true;
        monitor_thread = std::thread(&FileMonitor::monitorFiles, this);
    }
}

void FileMonitor::stop() {
    if (running) {
        running = false;
        if (monitor_thread.joinable()) monitor_thread.join();
    }
}

bool FileMonitor::isRunning() const { return running; }

void FileMonitor::renderUI() {
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("Overlay", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
    {
        std::lock_guard<std::mutex> lock(mutex);
        ImGui::Text("Next: %s", pimpl->use_named_titles ? "Title" : "Snap");
    }
    ImGui::End();

    ImGui::Begin("File Monitor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Source:");
    if (ImGui::InputText("##Source", pimpl->source_buffer, sizeof(pimpl->source_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        fs::path new_src = pimpl->source_buffer;
        if (fs::is_directory(new_src)) {
            std::lock_guard<std::mutex> lock(mutex);
            source_dir = new_src;
            pimpl->error_message.clear();
        } else {
            pimpl->error_message = "Invalid source: " + std::string(pimpl->source_buffer);
            strncpy(pimpl->source_buffer, source_dir.string().c_str(), sizeof(pimpl->source_buffer) - 1);
        }
    }
    ImGui::Text("Destination:");
    if (ImGui::InputText("##Dest", pimpl->dest_buffer, sizeof(pimpl->dest_buffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
        fs::path new_dest = pimpl->dest_buffer;
        if (!fs::exists(new_dest)) fs::create_directories(new_dest);
        if (fs::is_directory(new_dest)) {
            std::lock_guard<std::mutex> lock(mutex);
            dest_dir = new_dest;
            pimpl->error_message.clear();
        } else {
            pimpl->error_message = "Invalid destination: " + std::string(pimpl->dest_buffer);
            strncpy(pimpl->dest_buffer, dest_dir.string().c_str(), sizeof(pimpl->dest_buffer) - 1);
        }
    }
    ImGui::Text("System:");
    ImGui::PushItemWidth(300.0f); // Make dropdown wider
    if (ImGui::BeginCombo("##System", pimpl->systems[pimpl->selected_system].c_str(), ImGuiComboFlags_HeightLarge)) {
        for (size_t i = 0; i < pimpl->systems.size(); ++i) {
            bool is_selected = (pimpl->selected_system == static_cast<int>(i));
            if (ImGui::Selectable(pimpl->systems[i].c_str(), is_selected)) {
                std::lock_guard<std::mutex> lock(mutex);
                pimpl->selected_system = static_cast<int>(i);
                pimpl->error_message.clear();
            }
            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopItemWidth();
    if (!pimpl->error_message.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::TextWrapped("Error: %s", pimpl->error_message.c_str());
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
    ImGui::Text("Copied Files:");
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (copied_files.empty()) ImGui::Text("(No files copied)");
        else for (const auto& file : copied_files) ImGui::Text("%s", file.c_str());
    }
    ImGui::End();
}