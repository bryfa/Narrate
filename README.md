# Narrate

**Synchronized text display and highlighting plugin for audio/video production**

A JUCE-based audio plugin (VST3/Standalone) that displays lyrics, narration, or teleprompter text with precise word-level timing. Perfect for music videos, karaoke, live performances, and video production.

[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-lightgrey)]()
[![JUCE](https://img.shields.io/badge/JUCE-8.0.10-blue)]()
[![License](https://img.shields.io/badge/license-Proprietary-red)]()

---

## Features

> **Note:** Narrate builds as two products - **Standalone** (for video creators) and **VST3 Plugin** (for DAW users). Each has features optimized for its use case. See [Feature Matrix](#feature-availability) below.

✨ **Multiple Render Strategies**
- **Scrolling**: Vertical scroll with centered current clip
- **Karaoke**: Line-by-line display with dimmed context
- **Teleprompter**: Continuous scroll with configurable read line

⏱️ **Flexible Timing System**
- Natural word timing (original spacing)
- Rhythmic timing with BPM quantization
- Fixed duration mode
- Minimum duration for readability
- Look-ahead compensation for rendering latency

🎨 **Rich Text Formatting**
- Per-word colors, bold, italic
- Font size multipliers
- Highlight colors and styles
- Customizable default formatting per clip

💾 **Project Management**
- Save/load projects (.narrate XML format)
- Timeline recalculation (remove gaps)
- Clip management with auto-sorting
- Overlap detection and validation

🎵 **Audio Integration** (Planned)
- Background audio playback (coming soon)
- DAW transport sync (coming soon)
- Waveform display (coming soon)

---

## Screenshots

*Coming soon - add screenshots of EditorView and RunningView*

---

## Installation

### Requirements

- **Windows**: Windows 10 or later
- **macOS**: macOS 10.13 or later
- **Linux**: Ubuntu 20.04+ or equivalent

### Quick Install

**Option 1: Download Prebuilt Binaries** (Coming soon)
- Download the latest release from the [Releases](../../releases) page
- Extract the archive
- Copy the VST3 plugin to your DAW's plugin folder

**Option 2: Build from Source** (See [Building](#building) below)

### Plugin Locations

After building or installing, place the plugin in these standard locations:

**Windows:**
```
VST3: C:\Program Files\Common Files\VST3\Narrate.vst3
```

**macOS:**
```
VST3: /Library/Audio/Plug-Ins/VST3/Narrate.vst3
Standalone: /Applications/Narrate.app
```

**Linux:**
```
VST3: ~/.vst3/Narrate.vst3
```

---

## Usage

### Quick Start

1. **Open the plugin** in your DAW or run the standalone app
2. **Click "New"** to create a new project
3. **Add a clip** with the "Add Clip" button
4. **Set timing**: Enter start time (0.0s) and end time (5.0s)
5. **Enter text**: Type or paste your lyrics/narration
6. **Auto-space words**: Click "Auto-Space Words" to distribute timing evenly
7. **Preview**: Click "Preview" to see the result
8. **Save**: Click "Save" to save your project

### Editor Interface

```
┌────────────────────────────────────────────────────────────┐
│  [New] [Load] [Save]    Render: [Scrolling ▼]  [Preview]  │
├──────────────────┬─────────────────────────────────────────┤
│  Clip List       │  Clip Editor                            │
│  ─────────────   │  Start Time: [0.000] s                  │
│  □ Clip 1        │  End Time:   [5.000] s                  │
│  ■ Clip 2 (sel)  │                                         │
│  □ Clip 3        │  Text:                                  │
│                  │  ┌─────────────────────────────────┐    │
│  [Add Clip]      │  │ Hello world welcome to Narrate  │    │
│  [Remove Clip]   │  │                                 │    │
│                  │  └─────────────────────────────────┘    │
│  [Recalculate]   │  [Auto-Space Words]                     │
│                  │  Words will be evenly spaced            │
└──────────────────┴─────────────────────────────────────────┘
```

### Workflow Examples

#### Simple Lyrics Timing

1. Load background audio (reference only, not yet played)
2. Create clips for verses/choruses
3. Paste lyrics into each clip
4. Set start/end times based on audio
5. Click "Auto-Space Words" for even distribution
6. Preview and adjust timing manually if needed

#### Teleprompter Setup

1. Set render strategy to "Teleprompter"
2. Create one long clip with all text
3. Set duration to match presentation length
4. Auto-space words for consistent reading pace
5. Preview with appropriate font size

#### Karaoke Creation

1. Set render strategy to "Karaoke"
2. Create clips for each line
3. Use rhythmic timing preset (120 BPM)
4. Quantize word timings to beat grid
5. Preview to verify synchronization

---

## Building

### Prerequisites

Install these tools before building:

#### Windows (WSL2 or Native)

**WSL2/Ubuntu:**
```bash
sudo apt update
sudo apt install build-essential cmake git
```

**Native Windows:**
- Visual Studio 2019 or later with C++ development tools
- CMake 3.15+ ([download](https://cmake.org/download/))
- Git ([download](https://git-scm.com/download/win))

#### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install CMake
brew install cmake
```

#### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install build-essential cmake git \
    libasound2-dev libjack-jackd2-dev \
    ladspa-sdk libcurl4-openssl-dev \
    libfreetype6-dev libx11-dev libxcomposite-dev \
    libxcursor-dev libxcursor-dev libxext-dev \
    libxinerama-dev libxrandr-dev libxrender-dev \
    libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev
```

### Clone the Repository

```bash
# Clone with submodules (includes JUCE)
git clone --recurse-submodules https://github.com/yourusername/Narrate.git
cd Narrate

# Or if already cloned without submodules:
git submodule update --init --recursive
```

### Build Instructions

#### Using the build script (Recommended)

```bash
# Make script executable (first time only)
chmod +x build.sh

# Build the project
./build.sh
```

The script will:
1. Create a `build/` directory
2. Run CMake configuration
3. Build both VST3 and Standalone targets
4. Output binaries to `build/Narrate_artefacts/`

#### Manual CMake build

```bash
# Create build directory
mkdir build && cd build

# Configure (Unix Makefiles)
cmake -G "Unix Makefiles" ..

# Or on Windows with Visual Studio:
cmake -G "Visual Studio 17 2022" ..

# Build
cmake --build . --config Release

# Or with parallel jobs:
cmake --build . --config Release -j8
```

### Build Output Locations

After a successful build, you'll find:

```
build/Narrate_artefacts/
├── VST3/
│   └── Narrate.vst3/          # VST3 plugin
└── Standalone/
    └── Narrate                 # Standalone application (.app on macOS)
```

### Build Troubleshooting

**Error: "Unknown CMake command 'juce_add_plugin'"**
- JUCE submodule not initialized
- Run: `git submodule update --init --recursive`

**Error: "nproc: command not found" (macOS)**
- Old version of build.sh
- Update from repository or manually use: `sysctl -n hw.ncpu`

**Error: "No rule to make target 'Makefile'"**
- Wrong CMake generator or corrupted cache
- Delete `build/` directory and rebuild with `-G "Unix Makefiles"`

**Compiler warnings about unused parameters**
- These are non-critical and documented in CLAUDE.md
- Will be cleaned up in future versions

---

## Project Structure

```
Narrate/
├── Source/                      # C++ source files
│   ├── PluginProcessor.h/cpp    # Plugin entry point
│   ├── PluginEditor.h/cpp       # Main editor window
│   ├── EditorView.h/cpp         # Edit mode interface
│   ├── RunningView.h/cpp        # Playback mode interface
│   ├── NarrateDataModel.h/cpp   # Data structures
│   ├── TimelineEventManager.h/cpp  # Timing engine
│   ├── HighlightSettings.h      # Timing configuration
│   ├── NarrateConfig.h          # Build target feature flags
│   ├── RenderStrategy.h         # Render interface
│   ├── ScrollingRenderStrategy.h/cpp
│   ├── KaraokeRenderStrategy.h/cpp
│   └── TeleprompterRenderStrategy.h/cpp
├── JUCE/                        # JUCE framework (submodule)
├── CMakeLists.txt               # CMake configuration
├── build.sh                     # Build script (cross-platform)
├── README.md                    # This file
├── ARCHITECTURE.md              # System design and roadmap
└── CLAUDE.md                    # Development guidelines
```

---

## Configuration

### Timing Presets

Narrate includes 5 built-in timing presets:

1. **Natural** - Original word spacing, no quantization
2. **Storytelling** - Minimum 200ms per word for readability
3. **Rhythmic** - Quantized to 120 BPM quarter notes
4. **Teleprompter** - Minimum 300ms for comfortable reading
5. **Fixed Duration** - All words highlighted for 500ms

*Note: Custom presets are planned for future versions*

### Render Strategies

- **Scrolling**: Best for song lyrics with multiple verses
- **Karaoke**: Best for sing-along applications
- **Teleprompter**: Best for presentations and speeches

Switch strategies at any time from the dropdown menu.

###  Feature Availability

Narrate builds as **two products** with different feature sets:

| Feature | Standalone App | VST3 Plugin | Notes |
|---------|---------------|-------------|-------|
| Project Management | ✅ | ✅ | Save/load projects |
| Clip/Word Editing | ✅ | ✅ | Core functionality |
| Render Strategies | ✅ | ✅ | All three modes |
| Timing Presets | ✅ | ✅ | Natural, Rhythmic, etc. |
| **Audio File Loading** | ✅ | ❌ | Standalone only |
| **Audio Playback** | ⏳ Phase 1 | ❌ | Standalone only |
| **Waveform Display** | ⏳ Phase 1 | ❌ | Standalone only |
| **Export SRT/WebVTT** | ⏳ Phase 1 | ❌ | Standalone only |
| **DAW Transport Sync** | ❌ | ⏳ Phase 2 | Plugin only |
| **DAW Automation** | ❌ | ⏳ Phase 2 | Plugin only |

**Why Different Features?**
- **Standalone**: Designed for video creators who need audio playback and subtitle export
- **VST3 Plugin**: Designed for DAW users who already have audio and transport control

For technical details, see [ARCHITECTURE.md - Build Targets](ARCHITECTURE.md#build-targets--feature-configuration).

---

## Development

### Architecture Overview

Narrate uses an event-driven timing system:

1. **Data Model** stores clips and words with relative timing
2. **TimelineEventManager** builds a sorted timeline of events
3. **RunningView** advances time with a 60fps timer
4. **Events fire callbacks** updating current clip/word indices
5. **RenderStrategy** draws text based on current state

For detailed architecture documentation, see [ARCHITECTURE.md](ARCHITECTURE.md).

### Key Concepts

**Relative vs Absolute Time:**
- Words store time *relative to clip start* (e.g., 0.5s into the clip)
- Events use *absolute timeline time* (e.g., 10.5s from project start)
- Allows clips to be moved without recalculating word times

**Event Types:**
- `ClipStart` / `ClipEnd` - Clip lifecycle
- `WordStart` - Word becomes active (highlight begins)
- `HighlightEnd` - Highlight should fade/end (may differ from WordEnd)
- `WordEnd` - Word fully processed

**Look-Ahead Compensation:**
- Events trigger 25ms early by default
- Compensates for rendering latency and perceived delay
- Configurable in HighlightSettings

---

## Contributing

We welcome contributions! Here's how to get started:

### Setting Up Development Environment

1. **Fork and clone** the repository
2. **Create a feature branch**: `git checkout -b feature/my-feature`
3. **Make changes** and follow code style (see CLAUDE.md)
4. **Build and test**: `./build.sh` and verify no new warnings
5. **Commit changes**: Use descriptive commit messages
6. **Push and create PR**: Submit a pull request with description

### Code Style Guidelines

- Follow existing code formatting
- Use JUCE naming conventions (`PascalCase` for classes, `camelCase` for methods)
- Add comments for non-obvious logic
- Resolve compiler warnings when possible (see CLAUDE.md)
- Use `juce::ignoreUnused()` or `[[maybe_unused]]` for intentionally unused parameters

### What to Contribute

We're especially interested in:

- 🐛 **Bug fixes** - Report or fix issues
- ✨ **New render strategies** - Implement custom visualization modes
- 🎨 **UI improvements** - Enhance the editor interface
- 📚 **Documentation** - Improve README, add tutorials
- 🧪 **Tests** - Add unit tests for core components
- 🌐 **Translations** - Internationalization support

### Testing Your Changes

Before submitting:

1. **Build successfully** on your platform
2. **Test basic functionality**:
   - Create a new project
   - Add clips and words
   - Preview with different render strategies
   - Save and reload project
3. **Check for regressions**: Ensure existing features still work
4. **Verify no new warnings**: Check build output

### Reporting Issues

When reporting bugs, please include:

- Narrate version (commit hash or release version)
- Operating system and version
- DAW (if applicable) and version
- Steps to reproduce the issue
- Expected vs actual behavior
- Screenshots or screen recordings (if relevant)
- Project file (if the issue is project-specific)

Use the issue template on GitHub if available.

---

## Roadmap

See [ARCHITECTURE.md - Development Roadmap](ARCHITECTURE.md#development-roadmap) for detailed plans.

### Upcoming Features

**Phase 1: Enhanced Manual Timing** (Next)
- Visual timeline editor
- Waveform display
- Real-time timing adjustments
- Keyboard shortcuts

**Phase 2: Audio Integration**
- Background audio playback
- Audio-synced timing
- Onset detection
- Tap-to-mark timing

**Phase 3: Professional Features**
- DAW transport sync
- Automation support
- SRT/WebVTT export
- Multi-track support

**Phase 4: AI/ML Features**
- Automatic word timing from audio
- Speech-to-text integration
- Emphasis detection
- Multi-language support

---

## FAQ

**Q: Does Narrate play audio?**
A: Not yet. Currently, timing is manual or preset-based. Audio playback is planned for Phase 2 (see roadmap).

**Q: Can I use this with my DAW?**
A: The plugin loads in DAWs as a VST3, but doesn't sync to transport yet. This is planned for Phase 3.

**Q: What file format does Narrate use?**
A: Projects are saved as `.narrate` files (XML format). You can edit them manually if needed.

**Q: Can I create custom render strategies?**
A: Yes! See [ARCHITECTURE.md - Extension Points](ARCHITECTURE.md#adding-custom-render-strategies) for details.

**Q: Is there a limit on project size?**
A: No hard limit, but very large projects (1000+ clips) may impact performance. This will be optimized in future versions.

**Q: Can I export subtitles?**
A: Not yet, but SRT and WebVTT export are planned for Phase 3.

**Q: Does Narrate support MIDI?**
A: Not currently, but MIDI timecode sync is under consideration for future versions.

---

## License

Copyright © 2025 Mulhacen Labs. All rights reserved.

This software is proprietary. Contact us for licensing inquiries.

---

## Support

- **Documentation**: See [ARCHITECTURE.md](ARCHITECTURE.md) for technical details
- **Issues**: Report bugs on [GitHub Issues](../../issues)
- **Discussions**: Ask questions on [GitHub Discussions](../../discussions)
- **Email**: [support@mulhacenlabs.com](mailto:support@mulhacenlabs.com)

---

## Credits

**Development:** Mulhacen Labs
**Framework:** [JUCE](https://juce.com/) by Roli Ltd.
**Contributors:** See [CONTRIBUTORS.md](CONTRIBUTORS.md)

---

## Acknowledgments

Special thanks to:
- The JUCE community for excellent framework and support
- Contributors who helped test and improve Narrate
- Our users for valuable feedback and feature suggestions

---

**Built with ❤️ using JUCE**
