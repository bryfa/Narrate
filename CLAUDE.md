# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System design, component architecture, timing system deep dive, and development roadmap
- **[README.md](README.md)** - User-facing documentation, installation, usage, and contribution guidelines
- **CLAUDE.md** (this file) - Build instructions, development workflow, and technical setup

## Project Overview

Narrate is a JUCE 8.0.10 audio plugin built with CMake. It builds both a VST3 plugin and a standalone application.

**Company:** MulhacenLabs
**Plugin Codes:** Manufacturer `Mlhn`, Plugin `Nrrt`
**Format:** VST3 Effects plugin with MIDI input support
**Purpose:** Synchronized text display and highlighting for audio/video production

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

**For detailed architecture documentation, see [ARCHITECTURE.md](ARCHITECTURE.md)**

### Quick Reference

**Main Components:**
- `PluginProcessor.{h,cpp}` - Plugin entry point, audio passthrough, audio playback (Standalone)
- `PluginEditor.{h,cpp}` - Main window, view switching, theme management
- `EditorView.{h,cpp}` - Edit mode interface with clip/word management, transport controls, waveform display
- `RunningView.{h,cpp}` - Playback mode with timer-driven rendering, audio sync
- `WaveformDisplay.{h,cpp}` - Audio waveform visualization with playback indicator
- `NarrateDataModel.{h,cpp}` - Data structures (Project, Clip, Word)
- `TimelineEventManager.{h,cpp}` - Core timing engine
- `HighlightSettings.h` - Timing configuration and presets
- `RenderStrategy.h` - Interface for visualization modes
- `ScrollingRenderStrategy.cpp` / `KaraokeRenderStrategy.cpp` / `TeleprompterRenderStrategy.cpp` - Render implementations
- `NarrateLookAndFeel.{h,cpp}` - Custom UI theme (Dark/Light modes)
- `NarrateConfig.h` - Conditional compilation flags for Standalone vs VST3

**Key Design Patterns:**
- Strategy Pattern for render modes
- Event-driven timing system
- Relative time storage (words relative to clip start)
- Callback-based state management
- Conditional compilation for Standalone vs Plugin features

**Audio Features (Standalone Only):**
- Audio file loading (WAV, MP3, AIFF, FLAC)
- Transport controls (Play/Pause, Stop, Seek)
- Real-time waveform visualization with playback indicator
- Audio sync with lyrics display in RunningView
- Position display with time counter

### JUCE Modules Used

Core audio: `juce_audio_basics`, `juce_audio_devices`, `juce_audio_formats`, `juce_audio_processors`, `juce_audio_utils`
GUI: `juce_graphics`, `juce_gui_basics`, `juce_gui_extra`
Foundation: `juce_core`, `juce_data_structures`, `juce_events`

### Key Configuration

- C++ Standard: C++20
- Web browser disabled (`JUCE_WEB_BROWSER=0`)
- CURL disabled (`JUCE_USE_CURL=0`)
- VST2 compatibility disabled

### Conditional Compilation Flags (NarrateConfig.h)

The project uses conditional compilation to enable different features for Standalone vs VST3 builds:

- `NARRATE_ENABLE_AUDIO_PLAYBACK` - Enables audio playback system (Standalone only)
- `NARRATE_ENABLE_AUDIO_FILE_LOADING` - Enables audio file loading (Standalone only)
- `NARRATE_SHOW_LOAD_AUDIO_BUTTON` - Shows Load Audio button and transport controls (Standalone only)
- `NARRATE_SHOW_EXPORT_MENU` - Shows Export SRT/WebVTT buttons (Standalone only)
- `NARRATE_SHOW_DAW_SYNC_INDICATOR` - Shows DAW sync status (VST3 only)

These flags automatically set based on `JucePlugin_Build_Standalone` and `JucePlugin_Build_VST3` macros.

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

### Code Quality Standards

**Compiler and Linker Warnings:**

When making code changes, all compiler and linker warnings should be resolved if possible:

- **Unused variables/parameters:** Remove or mark with `juce::ignoreUnused()` or `[[maybe_unused]]`
- **Sign comparison warnings:** Use appropriate types or explicit casts
- **Variable shadowing:** Rename local variables to avoid shadowing members
- **Unused parameters:** If required by interface, use `[[maybe_unused]]` attribute

Warnings should not be ignored unless there's a documented reason. Clean builds with zero warnings improve code maintainability and help catch real issues.

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
