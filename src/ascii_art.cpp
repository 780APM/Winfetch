#include "ascii_art.h"
#include <vector>
#include <string>

AsciiArt::AsciiArt(const Config& config) : config(config) {
}

std::vector<std::string> AsciiArt::getLogo() {
    if (config.getLogoStyle() == "windows") {
        return getWindowsLogo();
    } else if (config.getLogoStyle() == "custom") {
        return getCustomLogo();
    } else if (config.getLogoStyle() == "minimal") {
        return getMinimalLogo();
    } else {
        return getDefaultLogo();
    }
}

std::vector<std::string> AsciiArt::getWindowsLogo() {
    return {
        " __      __.__        _____       __         .__     ",
        "/  \\    /  \\__| _____/ ____\\_____/  |_  ____ |  |__  ",
        "\\   \\/\\/   /  |/    \\   __\\/ __ \\   __\\/ ___\\|  |  \\ ",
        " \\        /|  |   |  \\  | \\  ___/|  | \\  \\___|   Y  \\",
        "  \\__/\\  / |__|___|  /__|  \\___  >__|  \\___  >___|  /",
        "       \\/          \\/          \\/          \\/     \\/ "
    };
}

std::vector<std::string> AsciiArt::getMinimalLogo() {
    return {
        "    Winfetch",
        "    ========"
    };
}

std::vector<std::string> AsciiArt::getDefaultLogo() {
    return {
        " __      __.__        _____       __         .__     ",
        "/  \\    /  \\__| _____/ ____\\_____/  |_  ____ |  |__  ",
        "\\   \\/\\/   /  |/    \\   __\\/ __ \\   __\\/ ___\\|  |  \\ ",
        " \\        /|  |   |  \\  | \\  ___/|  | \\  \\___|   Y  \\",
        "  \\__/\\  / |__|___|  /__|  \\___  >__|  \\___  >___|  /",
        "       \\/          \\/          \\/          \\/     \\/ "
    };
}

std::vector<std::string> AsciiArt::getCustomLogo() {
    return getDefaultLogo();
}
