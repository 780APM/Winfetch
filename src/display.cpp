#include "display.h"
#include <iostream>
#include <iomanip>
#include <sstream>

Display::Display(const Config& config) : config(config), asciiArt(config) {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
}

Display::~Display() {
    resetColor();
}

void Display::showSystemInfo(const SystemInfo& sysInfo) {
    // Clear screen if configured
    if (config.getClearScreen()) {
        system("cls");
    }
    
    // Print title
    if (config.getShowTitle()) {
        printTitle();
        std::cout << std::endl;
    }
    
    // Print logo
    if (config.getShowLogo()) {
        printLogo();
    }
    
    // Print system information
    printSystemInfo(sysInfo);
    printHardwareInfo(sysInfo);
    printStorageInfo(sysInfo);
    printDesktopInfo(sysInfo);
    
    // Print separator at the end
    printSeparator();
}

void Display::printLogo() {
    std::vector<std::string> logo = asciiArt.getLogo();
    
    for (const auto& line : logo) {
        setColor(config.getLogoColor());
        std::cout << line << std::endl;
        resetColor();
    }
    std::cout << std::endl;
}

void Display::printInfoLine(const std::string& label, const std::string& value, int color) {
    setColor(config.getLabelColor());
    std::cout << label << ": ";
    resetColor();
    
    setColor(color);
    std::cout << value << std::endl;
    resetColor();
}

void Display::printSeparator() {
    setColor(config.getSeparatorColor());
    std::cout << std::string(50, '-') << std::endl;
    resetColor();
}

void Display::printTitle() {
    setColor(config.getTitleColor());
    printCentered("Winfetch - Windows System Information");
    resetColor();
}

void Display::setColor(int color) {
    if (!config.getUseColors()) {
        return;
    }
    
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    
    WORD winColor = WIN_COLOR_WHITE;
    
    switch (color) {
        case COLOR_BLACK: winColor = WIN_COLOR_BLACK; break;
        case COLOR_RED: winColor = WIN_COLOR_RED; break;
        case COLOR_GREEN: winColor = WIN_COLOR_GREEN; break;
        case COLOR_YELLOW: winColor = WIN_COLOR_YELLOW; break;
        case COLOR_BLUE: winColor = WIN_COLOR_BLUE; break;
        case COLOR_MAGENTA: winColor = WIN_COLOR_MAGENTA; break;
        case COLOR_CYAN: winColor = WIN_COLOR_CYAN; break;
        case COLOR_WHITE: winColor = WIN_COLOR_WHITE; break;
        case COLOR_BRIGHT_WHITE: winColor = WIN_COLOR_BRIGHT_WHITE; break;
        default: winColor = WIN_COLOR_WHITE; break;
    }
    
    SetConsoleTextAttribute(hConsole, winColor);
}

void Display::resetColor() {
    if (!config.getUseColors()) {
        return;
    }
    
    SetConsoleTextAttribute(hConsole, WIN_COLOR_WHITE);
}

void Display::printCentered(const std::string& text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    
    int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int textWidth = static_cast<int>(text.length());
    int padding = (width - textWidth) / 2;
    
    if (padding > 0) {
        std::cout << std::string(padding, ' ');
    }
    std::cout << text << std::endl;
}

void Display::printRightAligned(const std::string& text, int width) {
    int textWidth = static_cast<int>(text.length());
    int padding = width - textWidth;
    
    if (padding > 0) {
        std::cout << std::string(padding, ' ');
    }
    std::cout << text;
}

void Display::printLeftAligned(const std::string& text, int width) {
    std::cout << text;
    int textWidth = static_cast<int>(text.length());
    int padding = width - textWidth;
    
    if (padding > 0) {
        std::cout << std::string(padding, ' ');
    }
}

std::string Display::formatInfoLine(const std::string& label, const std::string& value) {
    return label + ": " + value;
}

void Display::printSystemInfo(const SystemInfo& sysInfo) {
    setColor(config.getSectionColor());
    std::cout << "System Information:" << std::endl;
    resetColor();
    
    printInfoLine("OS", sysInfo.windowsEdition + " " + sysInfo.architecture, COLOR_CYAN);
    printInfoLine("Version", sysInfo.osVersion + " Build " + sysInfo.osBuild, COLOR_WHITE);
    printInfoLine("Uptime", sysInfo.uptime, COLOR_GREEN);
    printInfoLine("Language", sysInfo.language, COLOR_YELLOW);
    printInfoLine("Timezone", sysInfo.timezone, COLOR_YELLOW);
    
    std::cout << std::endl;
}

void Display::printHardwareInfo(const SystemInfo& sysInfo) {
    setColor(config.getSectionColor());
    std::cout << "Hardware Information:" << std::endl;
    resetColor();
    
    printInfoLine("CPU", sysInfo.cpuName, COLOR_CYAN);
    printInfoLine("Cores", sysInfo.cpuCores + " cores, " + sysInfo.cpuThreads + " threads", COLOR_WHITE);
    if (!sysInfo.cpuFrequency.empty()) {
        printInfoLine("Frequency", sysInfo.cpuFrequency, COLOR_WHITE);
    }
    printInfoLine("Memory", sysInfo.totalMemory + " (" + sysInfo.memoryUsage + " used)", COLOR_GREEN);
    if (!sysInfo.gpuDriver.empty() && sysInfo.gpuDriver != "Unknown") {
        printInfoLine("GPU", sysInfo.gpuName + " (Display Driver: " + sysInfo.gpuDriver + ")", COLOR_MAGENTA);
    } else {
        printInfoLine("GPU", sysInfo.gpuName, COLOR_MAGENTA);
    }
    
    std::cout << std::endl;
}

void Display::printStorageInfo(const SystemInfo& sysInfo) {
    if (sysInfo.drives.empty()) {
        return;
    }
    
    setColor(config.getSectionColor());
    std::cout << "Storage Information:" << std::endl;
    resetColor();
    
    for (size_t i = 0; i < sysInfo.drives.size(); i++) {
        std::string driveInfo = sysInfo.drives[i] + " " + sysInfo.driveSizes[i] + " (" + sysInfo.driveFree[i] + " free)";
        printInfoLine("Drive", driveInfo, COLOR_BLUE);
    }
    
    std::cout << std::endl;
}

void Display::printDesktopInfo(const SystemInfo& sysInfo) {
    setColor(config.getSectionColor());
    std::cout << "Desktop Information:" << std::endl;
    resetColor();
    
    printInfoLine("Username", sysInfo.username, COLOR_WHITE);
    printInfoLine("PC Name", sysInfo.domain, COLOR_YELLOW);
    
    std::cout << std::endl;
}

void Display::printWindowsInfo(const SystemInfo& sysInfo) {
    setColor(config.getSectionColor());
    std::cout << "Windows Information:" << std::endl;
    resetColor();
    
    if (!sysInfo.windowsActivation.empty() && sysInfo.windowsActivation != "Unknown") {
        printInfoLine("Activation", sysInfo.windowsActivation, COLOR_GREEN);
    }
    if (!sysInfo.windowsDefender.empty() && sysInfo.windowsDefender != "Unknown") {
        printInfoLine("Defender", sysInfo.windowsDefender, COLOR_RED);
    }
    if (!sysInfo.windowsUpdate.empty() && sysInfo.windowsUpdate != "Unknown") {
        printInfoLine("Updates", sysInfo.windowsUpdate, COLOR_YELLOW);
    }
    
    std::cout << std::endl;
}
