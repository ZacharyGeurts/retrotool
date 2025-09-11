#ifndef FILE_MONITOR_HPP
#define FILE_MONITOR_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <memory>

namespace fs = std::filesystem;

// List of systems (sorted, maintainable, easy to extend)
constexpr const char* SYSTEM_NAMES[] = {
    "Acorn - BBC Micro",
    "Amstrad - CPC",
    "Amstrad - GX4000",
    "Arduboy Inc - Arduboy",
    "Atari - 2600",
    "Atari - 5200",
    "Atari - 7800",
    "Atari - 8-bit Family",
    "Atari - Jaguar",
    "Atari - Lynx",
    "Atari - ST",
    "Atomiswave",
    "Bandai - WonderSwan",
    "Bandai - WonderSwan Color",
    "Cannonball",
    "Casio - Loopy",
    "Casio - PV-1000",
    "Cave Story",
    "ChaiLove",
    "CHIP-8",
    "Coleco - ColecoVision",
    "Commodore - 64",
    "Commodore - Amiga",
    "Commodore - CD32",
    "Commodore - CDTV",
    "Commodore - PET",
    "Commodore - Plus-4",
    "Commodore - VIC-20",
    "Dinothawr",
    "DOOM",
    "DOS",
    "Elektronika - BK-0010",
    "Elektronika - BK-0010-0011M",
    "Elektronika - BK-001-411",
    "Emerson - Arcadia 2001",
    "Entex - Adventure Vision",
    "Epoch - Super Cassette Vision",
    "Fairchild - Channel F",
    "FBNeo - Arcade Games",
    "Flashback",
    "Funtech - Super Acan",
    "GamePark - GP32",
    "GCE - Vectrex",
    "Handheld Electronic Game",
    "Hartung - Game Master",
    "Jump 'n Bump",
    "LeapFrog - Leapster Learning Game System",
    "LowRes NX",
    "Lutro",
    "Magnavox - Odyssey2",
    "MAME",
    "Mattel - Intellivision",
    "Microsoft - MSX",
    "Microsoft - MSX2",
    "Microsoft - Xbox",
    "Microsoft - Xbox 360",
    "MrBoom",
    "NEC - PC-8001 - PC-8801",
    "NEC - PC-98",
    "NEC - PC Engine - TurboGrafx 16",
    "NEC - PC Engine CD - TurboGrafx-CD",
    "NEC - PC Engine SuperGrafx",
    "NEC - PC-FX",
    "Nintendo - Family Computer Disk System",
    "Nintendo - Game Boy",
    "Nintendo - Game Boy Advance",
    "Nintendo - Game Boy Color",
    "Nintendo - GameCube",
    "Nintendo - Nintendo 3DS",
    "Nintendo - Nintendo 64",
    "Nintendo - Nintendo 64DD",
    "Nintendo - Nintendo DS",
    "Nintendo - Nintendo DSi",
    "Nintendo - Nintendo Entertainment System",
    "Nintendo - Pokemon Mini",
    "Nintendo - Satellaview",
    "Nintendo - Sufami Turbo",
    "Nintendo - Super Nintendo Entertainment System",
    "Nintendo - Virtual Boy",
    "Nintendo - Wii",
    "Nintendo - Wii U",
    "Philips - CD-i",
    "Philips - Videopac+",
    "Quake",
    "Quake II",
    "Quake III",
    "RCA - Studio II",
    "Rick Dangerous",
    "RPG Maker",
    "ScummVM",
    "Sega - 32X",
    "Sega - Dreamcast",
    "Sega - Game Gear",
    "Sega - Master System - Mark III",
    "Sega - Mega-CD - Sega CD",
    "Sega - Mega Drive - Genesis",
    "Sega - Naomi",
    "Sega - Naomi 2",
    "Sega - PICO",
    "Sega - Saturn",
    "Sega - SG-1000",
    "Sega - ST-V",
    "Sharp - X1",
    "Sharp - X68000",
    "Sinclair - ZX 81",
    "Sinclair - ZX Spectrum",
    "SNK - Neo Geo",
    "SNK - Neo Geo CD",
    "SNK - Neo Geo Pocket",
    "SNK - Neo Geo Pocket Color",
    "Sony - PlayStation",
    "Sony - PlayStation 2",
    "Sony - PlayStation 3",
    "Sony - PlayStation 4",
    "Sony - PlayStation Portable",
    "Sony - PlayStation Portable (PSN)",
    "Sony - PlayStation Vita",
    "Spectravideo - SVI-318 - SVI-328",
    "The 3DO Company - 3DO",
    "Thomson - MOTO",
    "TIC-80",
    "Tiger - Game.com",
    "Tomb Raider",
    "Uzebox",
    "Vircon32",
    "VTech - CreatiVision",
    "VTech - V.Smile",
    "WASM-4",
    "Watara - Supervision",
    "Welback - Mega Duck",
    "Wolfenstein 3D"
};

class FileMonitor {
public:
    FileMonitor(const fs::path& source, const fs::path& dest);
    ~FileMonitor();
    void start();
    void stop();
    void renderUI();
    bool isRunning() const;

private:
    struct Impl {
        char source_buffer[1024];
        char dest_buffer[1024];
        std::string error_message;
        std::vector<std::string> systems;
        int selected_system{0};
        bool use_named_titles{true};
    };

    fs::path source_dir, dest_dir;
    std::vector<std::string> copied_files;
    std::thread monitor_thread;
    std::mutex mutex;
    bool running{false};
    std::unique_ptr<Impl> pimpl;

    void monitorFiles();
    void copyAndMoveFile(const fs::path& src, const fs::path& dest);
    void initSystems(); // New function to initialize systems vector
};

#endif // FILE_MONITOR_HPP