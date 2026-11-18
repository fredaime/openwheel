# OpenWheel Build System

This document describes the build system, testing infrastructure, and installation targets for the OpenWheel project.

## Build System Overview

OpenWheel uses CMake as its build system with support for:
- **Multi-component builds**: Daemon and gadget can be built independently
- **Flexible configuration**: Build options to customize what gets built
- **Testing infrastructure**: CTest integration for automated testing
- **Comprehensive installation**: Install targets for binaries, data, and development files

## Prerequisites

### Required
- CMake 3.16 or newer
- C11 compiler (for openwheel-daemon)
- C++20 compiler (for openwheel-gadget)
- pkg-config
- D-Bus development libraries (for openwheel-daemon)

### For Gadget Component
- Qt6 with Core, Gui, Widgets, Qml, Quick, QuickControls2 and DBus
- Qt6 Test module when `BUILD_TESTS=ON`

### Optional
- KDE Frameworks 6 (for enhanced features)
- X11 with XTest (for keyboard/mouse simulation)

## Build Options

The following CMake options control the build:

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_DAEMON` | ON | Build the openwheel-daemon component |
| `BUILD_GADGET` | ON | Build the openwheel-gadget component |
| `BUILD_TESTS` | ON | Build and enable tests |
| `ENABLE_INSTALL` | ON | Enable installation targets |
| `CMAKE_BUILD_TYPE` | Release | Build type (Debug, Release, RelWithDebInfo, MinSizeRel) |

## Building

### Standard Build

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
make -j$(nproc)
```

### Build with Custom Options

```bash
# Build only the daemon
cmake -DBUILD_GADGET=OFF ..
make

# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Build without tests
cmake -DBUILD_TESTS=OFF ..
make

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/usr/local ..
make
```

### Out-of-source Build (Recommended)

```bash
# From project root
mkdir -p build/release
cd build/release
cmake ../..
make -j$(nproc)
```

## Testing

The project uses CTest for testing infrastructure.

### Running Tests

```bash
# From build directory
ctest

# Verbose output
ctest -V

# Run specific test
ctest -R daemon_executable_exists

# Run tests in parallel
ctest -j$(nproc)
```

### Available Tests

- `hid_report` tests HID decoding and button/rotation ordering (daemon builds).
- `test_rotation` tests the Qt rotation handler (gadget builds).
- `daemon_executable_exists` and `gadget_executable_exists` check the requested build artifacts.

The daemon-only build and tests require no Qt dependencies.
Configure with `cmake -S . -B build-daemon -DBUILD_GADGET=OFF`.

## Installation

### Installation Targets

The project provides component-based installation:

| Component | Description | Files |
|-----------|-------------|-------|
| `daemon` | Daemon executable | `asus_wheel` binary |
| `daemon-dev` | Daemon headers | Development headers |
| `gadget` | Gadget executable | `openwheel-gadget` binary |
| `gadget-dev` | Gadget headers | Development headers |
| `data` | Data files | Profiles, desktop files, icons |

### Install Commands

```bash
# Install everything
sudo make install

# Install specific component
sudo cmake --install . --component daemon
sudo cmake --install . --component gadget
sudo cmake --install . --component data

# Install to custom prefix
cmake -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make install  # No sudo needed
```

### Installation Paths

Default installation paths (with `/usr/local` prefix):

- **Binaries**: `/usr/local/bin/`
  - `asus_wheel` (daemon)
  - `openwheel-gadget` (gadget)
- **Headers**: `/usr/local/include/openwheel/`
- **Data files**: `/usr/local/share/openwheel/`
- **Desktop files**: `/usr/local/share/applications/`
- **Icons**: `/usr/local/share/icons/hicolor/128x128/apps/`

## Build Artifacts

Build outputs are organized in the build directory:

```
build/
├── bin/              # Executables
│   ├── asus_wheel
│   └── openwheel-gadget
├── lib/              # Libraries (if any)
└── Testing/          # CTest output
```

## Common Build Tasks

### Clean Build

```bash
# Remove build directory
rm -rf build

# Recreate and build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Rebuild After Changes

```bash
# From build directory
make clean
make -j$(nproc)
```

### Update CMake Configuration

```bash
# From build directory
cmake ..
make -j$(nproc)
```

## Troubleshooting

### Missing Dependencies

If CMake reports missing dependencies:

```bash
# Ubuntu/Debian
sudo apt-get install cmake build-essential pkg-config libdbus-1-dev
sudo apt-get install qt6-base-dev qt6-declarative-dev  # For Qt6
sudo apt-get install libx11-dev libxtst-dev  # For X11 support

# Fedora
sudo dnf install cmake gcc-c++ pkg-config dbus-devel
sudo dnf install qt6-qtbase-devel qt6-qtdeclarative-devel
sudo dnf install libX11-devel libXtst-devel

# Arch
sudo pacman -S cmake base-devel pkg-config dbus
sudo pacman -S qt6-base qt6-declarative
sudo pacman -S libx11 libxtst
```

### Build Warnings

To enable additional compiler warnings:

```bash
cmake -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic" ..
make
```

## Development Workflow

### Making Changes

1. Make code changes
2. Rebuild: `make -j$(nproc)`
3. Run tests: `ctest`
4. Install locally: `make install DESTDIR=/tmp/openwheel-test`

### Adding Tests

1. Add test to `tests/CMakeLists.txt`
2. Reconfigure: `cmake ..`
3. Build and test: `make && ctest`

## Build System Maintenance

### Updating Version

Edit `CMakeLists.txt` and update the version:

```cmake
project(openwheel VERSION 0.2.0 LANGUAGES C CXX)
```

### Adding Build Options

Add to root `CMakeLists.txt`:

```cmake
option(NEW_FEATURE "Enable new feature" OFF)
```

### Adding Dependencies

Add to component's `CMakeLists.txt`:

```cmake
find_package(NewLib REQUIRED)
target_link_libraries(target_name PRIVATE NewLib::NewLib)
```

## Performance Considerations

- Use parallel builds: `make -j$(nproc)`
- Use Ninja for faster builds: `cmake -G Ninja ..`
- Enable ccache: `cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..`
- Use LTO for smaller binaries: `cmake -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..`

## Continuous Integration

For CI/CD pipelines:

```bash
# Configure with all checks
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..

# Build with verbose output
make VERBOSE=1

# Run tests with output
ctest --output-on-failure

# Install to staging directory
make install DESTDIR=staging
```

---

For more information, see the main [README.md](README.md) or visit the [project repository](https://github.com/fredaime/openwheel).
