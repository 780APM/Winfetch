# Winfetch

A Windows system information tool inspired by fastfetch, written in C++ with Windows API integration.

<img src="Screenshots/Screenshot 2025-09-09 201609.png" width="49%" />

## Features

- System Information: OS version, architecture, uptime
- Hardware Details: CPU, memory, GPU information
- Storage Info: Drive sizes and free space
- Network Info: Username, domain
- Customizable Display: Colors, logos, layout options
- ASCII Art Logos: Multiple logo styles available

## Building

### Prerequisites

- Visual Studio 2019 or later (with C++ support)
- Windows 10/11

### Build Instructions

1. Open Developer Command Prompt for Visual Studio
2. Navigate to the project directory
3. Run the build script:
   ```powershell
   .\build.ps1
   ```

## Usage

```batch
winfetch [options]

Options:
  -h, --help     Show help message
  -c, --config   Specify config file path
  -v, --version  Show version information
  --no-logo      Hide ASCII logo
  --no-colors    Disable colored output
  --no-title     Hide window title
```

## Configuration

Create a `winfetch.conf` file to customize the display:

```ini
# Winfetch Configuration File
use_colors=true
show_logo=true
show_title=true
clear_screen=false
logo_style=default
logo_color=36
label_color=37
value_color=37
section_color=33
title_color=36
separator_color=90
```

## Logo Styles

- `default` - Full ASCII art logo
- `windows` - Windows-style logo
- `minimal` - Simple text logo
- `custom` - Custom logo (currently same as default)

## License

This project is open source and available under the MIT License.
