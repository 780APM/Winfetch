#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "system_info.h"
#include "display.h"
#include "config.h"
#include "ascii_art.h"

void printUsage() {
    std::cout << "Winfetch - Windows System Information Tool\n";
    std::cout << "Usage: winfetch [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -c, --config   Specify config file path\n";
    std::cout << "  -v, --version  Show version information\n";
    std::cout << "  --no-logo      Hide ASCII logo\n";
    std::cout << "  --no-colors    Disable colored output\n";
    std::cout << "  --no-title     Hide window title\n";
}

void printVersion() {
    std::cout << "Winfetch v1.0.0\n";
    std::cout << "A Windows system information tool inspired by fastfetch\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    bool showLogo = true;
    bool useColors = true;
    bool showTitle = true;
    std::string configPath = "";
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage();
            return 0;
        }
        else if (arg == "-v" || arg == "--version") {
            printVersion();
            return 0;
        }
        else if (arg == "--no-logo") {
            showLogo = false;
        }
        else if (arg == "--no-colors") {
            useColors = false;
        }
        else if (arg == "--no-title") {
            showTitle = false;
        }
        else if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                configPath = argv[++i];
            } else {
                std::cerr << "Error: --config requires a file path\n";
                return 1;
            }
        }
        else {
            std::cerr << "Unknown option: " << arg << "\n";
            printUsage();
            return 1;
        }
    }
    
    try {
        // Initialize configuration
        Config config;
        if (!configPath.empty()) {
            config.loadFromFile(configPath);
        }
        
        // Override config with command line options
        if (!useColors) config.setUseColors(false);
        if (!showLogo) config.setShowLogo(false);
        if (!showTitle) config.setShowTitle(false);
        
        // Initialize display system
        Display display(config);
        
        // Gather system information
        SystemInfo sysInfo;
        
        // Display the information
        display.showSystemInfo(sysInfo);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << "Press Enter to exit...";
        std::cin.get();
        return 1;
    }
    
    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
