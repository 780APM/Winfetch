#include "config.h"
#include <fstream>
#include <iostream>
#include <sstream>

Config::Config() {
    setDefault();
}

void Config::setDefault() {
    useColors = true;
    showLogo = true;
    showTitle = true;
    clearScreen = false;
    logoStyle = "default";
    logoColor = 36; // Cyan
    labelColor = 37; // White
    valueColor = 37; // White
    sectionColor = 33; // Yellow
    titleColor = 36; // Cyan
    separatorColor = 90; // Bright Black
}

void Config::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return; // Use defaults if file doesn't exist
    }
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // Skip empty lines and comments
        }
        
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            
            // Trim whitespace
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            settings[key] = value;
        }
    }
    
    file.close();
}

void Config::saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }
    
    file << "# Winfetch Configuration File\n";
    file << "# Generated automatically\n\n";
    
    file << "use_colors=" << (useColors ? "true" : "false") << "\n";
    file << "show_logo=" << (showLogo ? "true" : "false") << "\n";
    file << "show_title=" << (showTitle ? "true" : "false") << "\n";
    file << "clear_screen=" << (clearScreen ? "true" : "false") << "\n";
    file << "logo_style=" << logoStyle << "\n";
    file << "logo_color=" << logoColor << "\n";
    file << "label_color=" << labelColor << "\n";
    file << "value_color=" << valueColor << "\n";
    file << "section_color=" << sectionColor << "\n";
    file << "title_color=" << titleColor << "\n";
    file << "separator_color=" << separatorColor << "\n";
    
    file.close();
}
