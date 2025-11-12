# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Narrate is a JUCE 8.0.10 audio plugin built with CMake. It builds both a VST3 plugin and a standalone application.

**Company:** MulhacenLabs
**Plugin Codes:** Manufacturer `Mlhn`, Plugin `Nrrt`
**Format:** VST3 Effects plugin with MIDI input support

## Build System

### Initial Setup

JUCE is included as a Git submodule at version 8.0.10:
```bash
git submodule update --init --recursive
```

### System Dependencies (Linux)

Required packages for building:
```bash
sudo apt install cmake build-essential libasound2-dev libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev libxcomposite-dev mesa-common-dev freeglut3-dev libcurl4-openssl-dev
```

### Build Commands

```bash
# Configure
mkdir build && cd build
cmake ..

# Build
cmake --build .

# Build with specific configuration
cmake --build . --config Release

# Parallel build (faster)
cmake --build . -j$(nproc)
```

### Build Outputs

- **VST3 Plugin:** `build/Narrate_artefacts/VST3/Narrate.vst3`
- **Standalone App:** `build/Narrate_artefacts/Standalone/Narrate`

### Running

```bash
# Run standalone app
./build/Narrate_artefacts/Standalone/Narrate

# Install VST3 plugin (Linux)
cp -r build/Narrate_artefacts/VST3/Narrate.vst3 ~/.vst3/
```

## Architecture

### Audio Processing Chain

The plugin uses JUCE's standard AudioProcessor architecture:

- **NarrateAudioProcessor** (`Source/PluginProcessor.{h,cpp}`) - Core audio processing
  - Stereo I/O configuration (supports mono or stereo)
  - MIDI input enabled
  - Currently implements pass-through processing (add DSP in `processBlock()`)

- **NarrateAudioProcessorEditor** (`Source/PluginEditor.{h,cpp}`) - GUI
  - Default size: 400x300
  - Displays "Narrate Audio Plugin" text
  - Add UI components in `resized()` and `paint()`

### JUCE Modules Used

Core audio: `juce_audio_basics`, `juce_audio_devices`, `juce_audio_formats`, `juce_audio_processors`, `juce_audio_utils`
GUI: `juce_graphics`, `juce_gui_basics`, `juce_gui_extra`
Foundation: `juce_core`, `juce_data_structures`, `juce_events`

### Key Configuration

- C++ Standard: C++20
- Web browser disabled (`JUCE_WEB_BROWSER=0`)
- CURL disabled (`JUCE_USE_CURL=0`)
- VST2 compatibility disabled

## Development Workflow

### Testing Changes

**IMPORTANT:** After applying code changes, always run the build script to verify the changes compile successfully:

**Linux/WSL:**
```bash
./build.sh
```

**Windows PowerShell:**
```powershell
.\build.ps1
```

This ensures your changes don't break the build. The build scripts handle CMake configuration and compilation automatically.

### Available Scripts

- `build.sh` / `build.ps1` - Build only (no run)
- `run.sh` / `run.ps1` - Build and run the standalone application

### Adding New Source Files

Edit `CMakeLists.txt` and add files to `target_sources(Narrate PRIVATE ...)` section, then reconfigure CMake.

### Adding JUCE Modules

Add module to `target_link_libraries()` in CMakeLists.txt:
```cmake
juce::juce_dsp  # Example: for DSP utilities
```

### Plugin Identifiers

If copying this project as a template, change these in CMakeLists.txt:
- `PLUGIN_MANUFACTURER_CODE` (4 characters, at least 1 uppercase)
- `PLUGIN_CODE` (4 characters, at least 1 uppercase)
- Codes must be unique to avoid conflicts with other plugins
