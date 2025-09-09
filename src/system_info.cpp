#include "system_info.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <psapi.h>
#include <powrprof.h>
#include <wbemidl.h>
#include <comdef.h>
#include <winternl.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "wbemuuid.lib")

SystemInfo::SystemInfo() {
    gatherAllInfo();
}

void SystemInfo::gatherAllInfo() {
    gatherOSInfo();
    gatherCPUInfo();
    gatherMemoryInfo();
    gatherGPUInfo();
    gatherStorageInfo();
    gatherNetworkInfo();
    gatherUptimeInfo();
    gatherWindowsInfo();
}

void SystemInfo::gatherOSInfo() {
    // Get OS version information
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        osName = "Windows";
        osVersion = std::to_string(osvi.dwMajorVersion) + "." + std::to_string(osvi.dwMinorVersion);
        osBuild = std::to_string(osvi.dwBuildNumber);
        
        // Determine Windows edition
        if (osvi.wProductType == VER_NT_WORKSTATION) {
            if (osvi.dwMajorVersion == 10) {
                if (osvi.dwBuildNumber >= 22000) {
                    windowsEdition = "Windows 11";
                } else {
                    windowsEdition = "Windows 10";
                }
            } else if (osvi.dwMajorVersion == 6) {
                if (osvi.dwMinorVersion == 3) {
                    windowsEdition = "Windows 8.1";
                } else if (osvi.dwMinorVersion == 2) {
                    windowsEdition = "Windows 8";
                } else if (osvi.dwMinorVersion == 1) {
                    windowsEdition = "Windows 7";
                }
            }
        }
        
        // Try to get more accurate Windows version from registry
        std::string productName = getRegistryValue(HKEY_LOCAL_MACHINE, 
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
            "ProductName");
        if (!productName.empty()) {
            windowsEdition = productName;
        }
        
        // Additional check for Windows 11
        std::string displayVersion = getRegistryValue(HKEY_LOCAL_MACHINE, 
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
            "DisplayVersion");
        if (!displayVersion.empty() && osvi.dwBuildNumber >= 22000) {
            windowsEdition = "Windows 11 " + displayVersion;
        }
    }
    
    // Get architecture
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            architecture = "x64";
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            architecture = "ARM64";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            architecture = "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            architecture = "IA64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            architecture = "x86";
            break;
        default:
            architecture = "Unknown";
            break;
    }
}

void SystemInfo::gatherCPUInfo() {
    // Get CPU information from registry
    cpuName = getRegistryValue(HKEY_LOCAL_MACHINE, 
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
        "ProcessorNameString");
    
    if (cpuName.empty()) {
        cpuName = "Unknown CPU";
    }
    
    // Get CPU cores and threads
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    
    // Get logical processors (threads) and physical cores
    DWORD logicalProcessors = 0;
    DWORD physicalCores = 0;
    DWORD size = 0;
    
    if (GetLogicalProcessorInformation(nullptr, &size)) {
        std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
        if (GetLogicalProcessorInformation(buffer.data(), &size)) {
            for (const auto& info : buffer) {
                if (info.Relationship == RelationProcessorCore) {
                    physicalCores++;
                    // Count the number of logical processors (threads) for this core
                    DWORD mask = info.ProcessorMask;
                    DWORD threads = 0;
                    while (mask) {
                        if (mask & 1) threads++;
                        mask >>= 1;
                    }
                    logicalProcessors += threads;
                }
            }
        }
    }
    
    // Fallback if the above method fails
    if (physicalCores == 0) {
        // Try alternative method using WMI
        std::string wmiResult = executeCommand("wmic cpu get NumberOfCores,NumberOfLogicalProcessors /value");
        if (!wmiResult.empty()) {
            size_t coresPos = wmiResult.find("NumberOfCores=");
            size_t threadsPos = wmiResult.find("NumberOfLogicalProcessors=");
            
            if (coresPos != std::string::npos) {
                size_t start = coresPos + 14;
                size_t end = wmiResult.find('\n', start);
                if (end == std::string::npos) end = wmiResult.length();
                std::string cores = wmiResult.substr(start, end - start);
                cores.erase(0, cores.find_first_not_of(" \t\r\n"));
                cores.erase(cores.find_last_not_of(" \t\r\n") + 1);
                if (!cores.empty()) {
                    physicalCores = std::stoi(cores);
                }
            }
            
            if (threadsPos != std::string::npos) {
                size_t start = threadsPos + 26;
                size_t end = wmiResult.find('\n', start);
                if (end == std::string::npos) end = wmiResult.length();
                std::string threads = wmiResult.substr(start, end - start);
                threads.erase(0, threads.find_first_not_of(" \t\r\n"));
                threads.erase(threads.find_last_not_of(" \t\r\n") + 1);
                if (!threads.empty()) {
                    logicalProcessors = std::stoi(threads);
                }
            }
        }
        
        // Final fallback
        if (physicalCores == 0) {
            physicalCores = si.dwNumberOfProcessors;
            logicalProcessors = si.dwNumberOfProcessors;
        }
    }
    
    cpuCores = std::to_string(physicalCores);
    cpuThreads = std::to_string(logicalProcessors);
    
    // Get CPU frequency
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        
        DWORD frequency = 0;
        DWORD size = sizeof(frequency);
        if (RegQueryValueEx(hKey, "~MHz", nullptr, nullptr, 
            reinterpret_cast<LPBYTE>(&frequency), &size) == ERROR_SUCCESS) {
            cpuFrequency = std::to_string(frequency) + " MHz";
        }
        RegCloseKey(hKey);
    }
    
    // Try to get actual RAM speed from WMI
    std::string ramSpeed = executeCommand("wmic memorychip get speed /value");
    if (!ramSpeed.empty()) {
        size_t pos = ramSpeed.find("Speed=");
        if (pos != std::string::npos) {
            size_t start = pos + 6;
            size_t end = ramSpeed.find('\n', start);
            if (end == std::string::npos) end = ramSpeed.length();
            std::string speed = ramSpeed.substr(start, end - start);
            speed.erase(0, speed.find_first_not_of(" \t\r\n"));
            speed.erase(speed.find_last_not_of(" \t\r\n") + 1);
            if (!speed.empty() && speed != "0") {
                // Update CPU frequency to show RAM speed instead
                cpuFrequency = "RAM: " + speed + " MHz";
            }
        }
    }
}

void SystemInfo::gatherMemoryInfo() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    
    if (GlobalMemoryStatusEx(&memStatus)) {
        totalMemory = formatBytes(memStatus.ullTotalPhys);
        availableMemory = formatBytes(memStatus.ullAvailPhys);
        
        // Calculate memory usage percentage
        double usagePercent = ((double)(memStatus.ullTotalPhys - memStatus.ullAvailPhys) / memStatus.ullTotalPhys) * 100.0;
        memoryUsage = std::to_string(static_cast<int>(usagePercent)) + "%";
    }
}

void SystemInfo::gatherGPUInfo() {
    gpuName = "Unknown GPU";
    gpuDriver = "Unknown";
    gpuMemory = "Unknown";
    
    // Try multiple registry locations for GPU info
    std::vector<std::string> gpuPaths = {
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000",
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0001",
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0002"
    };
    
    for (const auto& path : gpuPaths) {
        std::string gpu = getRegistryValue(HKEY_LOCAL_MACHINE, path, "DriverDesc");
        if (!gpu.empty() && gpu != "Unknown GPU") {
            gpuName = gpu;
            // Get proper GPU driver version from different registry keys
            std::string driverVersion = getRegistryValue(HKEY_LOCAL_MACHINE, path, "DriverVersion");
            if (!driverVersion.empty()) {
                gpuDriver = driverVersion;
            }
            
            // Try to get AMD-specific driver version
            std::string amdDriver = getRegistryValue(HKEY_LOCAL_MACHINE, path, "UserModeDriverVersion");
            if (!amdDriver.empty()) {
                gpuDriver = amdDriver;
            }
            
            // Try alternative AMD driver version location
            std::string amdDriver2 = getRegistryValue(HKEY_LOCAL_MACHINE, path, "DriverDate");
            if (!amdDriver2.empty() && gpuDriver.empty()) {
                gpuDriver = amdDriver2;
            }
            break;
        }
    }
    
    // Try to get GPU from WMI using command
    if (gpuName == "Unknown GPU") {
        std::string wmiResult = executeCommand("wmic path win32_VideoController get name /format:list");
        if (!wmiResult.empty()) {
            // Parse WMI result - look for all GPU entries
            std::vector<std::string> lines = splitString(wmiResult, '\n');
            for (const auto& line : lines) {
                if (line.find("Name=") != std::string::npos) {
                    std::string gpu = line.substr(5); // Remove "Name="
                    // Clean up the string
                    gpu.erase(0, gpu.find_first_not_of(" \t\r\n"));
                    gpu.erase(gpu.find_last_not_of(" \t\r\n") + 1);
                    
                    // Skip basic display adapters and unknown entries
                    if (!gpu.empty() && 
                        gpu != "Microsoft Basic Display Adapter" && 
                        gpu.find("Unknown") == std::string::npos &&
                        gpu.find("Standard") == std::string::npos) {
                        gpuName = gpu;
                        break;
                    }
                }
            }
        }
    }
    
    // Try to get GPU driver version from WMI if we haven't found it yet
    if (gpuDriver == "Unknown") {
        std::string wmiDriver = executeCommand("wmic path win32_VideoController get DriverVersion /value");
        if (!wmiDriver.empty()) {
            size_t pos = wmiDriver.find("DriverVersion=");
            if (pos != std::string::npos) {
                size_t start = pos + 14;
                size_t end = wmiDriver.find('\n', start);
                if (end == std::string::npos) end = wmiDriver.length();
                std::string version = wmiDriver.substr(start, end - start);
                version.erase(0, version.find_first_not_of(" \t\r\n"));
                version.erase(version.find_last_not_of(" \t\r\n") + 1);
                if (!version.empty()) {
                    gpuDriver = version;
                }
            }
        }
    }
    
    // Try alternative WMI command for driver version
    if (gpuDriver == "Unknown") {
        std::string wmiDriver2 = executeCommand("wmic path win32_PnPSignedDriver where DeviceName like '%" + gpuName + "%' get DriverVersion /value");
        if (!wmiDriver2.empty()) {
            size_t pos = wmiDriver2.find("DriverVersion=");
            if (pos != std::string::npos) {
                size_t start = pos + 14;
                size_t end = wmiDriver2.find('\n', start);
                if (end == std::string::npos) end = wmiDriver2.length();
                std::string version = wmiDriver2.substr(start, end - start);
                version.erase(0, version.find_first_not_of(" \t\r\n"));
                version.erase(version.find_last_not_of(" \t\r\n") + 1);
                if (!version.empty()) {
                    gpuDriver = version;
                }
            }
        }
    }
    
    // Simplified - removed complex AMD registry detection
    
    // Simplified - removed complex dxdiag detection that was causing issues
    
    // Simplified GPU driver detection - just use the Windows driver version
    // The complex AMD detection was causing issues
    
    // Try alternative WMI command
    if (gpuName == "Unknown GPU") {
        std::string wmiResult2 = executeCommand("wmic path win32_VideoController get name /value");
        if (!wmiResult2.empty()) {
            std::vector<std::string> lines = splitString(wmiResult2, '\n');
            for (const auto& line : lines) {
                if (line.find("Name=") != std::string::npos) {
                    std::string gpu = line.substr(5);
                    gpu.erase(0, gpu.find_first_not_of(" \t\r\n"));
                    gpu.erase(gpu.find_last_not_of(" \t\r\n") + 1);
                    
                    if (!gpu.empty() && 
                        gpu != "Microsoft Basic Display Adapter" && 
                        gpu.find("Unknown") == std::string::npos &&
                        gpu.find("Standard") == std::string::npos) {
                        gpuName = gpu;
                        break;
                    }
                }
            }
        }
    }
    
    if (gpuName.empty()) {
        gpuName = "Unknown GPU";
    }
}

void SystemInfo::gatherStorageInfo() {
    drives.clear();
    driveSizes.clear();
    driveFree.clear();
    
    DWORD drives = GetLogicalDrives();
    char driveLetter = 'A';
    
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            std::string drive = std::string(1, driveLetter) + ":";
            
            ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
            
            if (GetDiskFreeSpaceEx(drive.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
                this->drives.push_back(drive);
                driveSizes.push_back(formatBytes(totalNumberOfBytes.QuadPart));
                driveFree.push_back(formatBytes(freeBytesAvailable.QuadPart));
            }
        }
        driveLetter++;
    }
}

void SystemInfo::gatherNetworkInfo() {
    // Get hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        this->hostname = hostname;
    } else {
        this->hostname = "Unknown";
    }
    
    // Get username
    char username[256];
    DWORD size = sizeof(username);
    if (GetUserName(username, &size)) {
        this->username = username;
    } else {
        this->username = "Unknown";
    }
    
    // Get domain
    char domain[256];
    DWORD domainSize = sizeof(domain);
    if (GetComputerName(domain, &domainSize)) {
        this->domain = domain;
    } else {
        this->domain = "Unknown";
    }
}

void SystemInfo::gatherUptimeInfo() {
    DWORD uptime = GetTickCount();
    this->uptime = formatUptime(uptime);
    
    // Get timezone
    TIME_ZONE_INFORMATION tzi;
    if (GetTimeZoneInformation(&tzi) != TIME_ZONE_ID_INVALID) {
        // Convert to readable format (simplified)
        this->timezone = "UTC" + std::to_string(-tzi.Bias / 60);
    } else {
        this->timezone = "Unknown";
    }
    
    // Get language
    char locale[256];
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLANGUAGE, locale, sizeof(locale))) {
        this->language = locale;
    } else {
        this->language = "Unknown";
    }
}

void SystemInfo::gatherWindowsInfo() {
    // Windows activation status (simplified)
    windowsActivation = "Unknown";
    
    // Windows Defender status (simplified)
    windowsDefender = "Unknown";
    
    // Windows Update status (simplified)
    windowsUpdate = "Unknown";
}

std::string SystemInfo::executeCommand(const std::string& command) {
    std::string result;
    char buffer[128];
    
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        return "";
    }
    
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    
    _pclose(pipe);
    return result;
}

std::string SystemInfo::getRegistryValue(HKEY hKey, const std::string& subKey, const std::string& valueName) {
    HKEY hSubKey;
    if (RegOpenKeyEx(hKey, subKey.c_str(), 0, KEY_READ, &hSubKey) != ERROR_SUCCESS) {
        return "";
    }
    
    char buffer[1024];
    DWORD bufferSize = sizeof(buffer);
    DWORD type;
    
    if (RegQueryValueEx(hSubKey, valueName.c_str(), nullptr, &type, 
        reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
        RegCloseKey(hSubKey);
        return std::string(buffer);
    }
    
    RegCloseKey(hSubKey);
    return "";
}

std::string SystemInfo::formatBytes(DWORDLONG bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024 && unit < 4) {
        size /= 1024;
        unit++;
    }
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << size << " " << units[unit];
    return oss.str();
}

std::string SystemInfo::formatUptime(DWORD uptimeMs) {
    DWORD days = uptimeMs / (1000 * 60 * 60 * 24);
    DWORD hours = (uptimeMs % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60);
    DWORD minutes = (uptimeMs % (1000 * 60 * 60)) / (1000 * 60);
    
    std::ostringstream oss;
    if (days > 0) {
        oss << days << "d " << hours << "h " << minutes << "m";
    } else if (hours > 0) {
        oss << hours << "h " << minutes << "m";
    } else {
        oss << minutes << "m";
    }
    
    return oss.str();
}

std::vector<std::string> SystemInfo::splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}
