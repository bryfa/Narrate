# Narrate Glossary

This document defines key terminology used throughout the Narrate application codebase.

## Core Concepts

### Playhead
The current time position marker in the timeline, similar to a DAW's vertical cursor. The playhead indicates where in the timeline playback or display is currently occurring. When navigating using controls (Previous/Next/Jump buttons), the playhead moves to the new time position, and all visual elements (word highlight, waveform position, etc.) follow it.

**Related Code:**
- `RunningView::currentTime` - The playhead position in seconds
- `AudioPlaybackPanel::updateUI()` - Updates waveform display to show playhead position

### Clip
A time-bounded segment of text/lyrics with a defined start time, end time, and associated words. Each clip represents a section of the narration that will be displayed during a specific time range.

**Related Code:**
- `Narrate::NarrateClip` class in `NarrateDataModel.h`
- `NarrateProject::getClip()` - Access clips by index

### Word
An individual word within a clip, with associated timing information. Each word has a relative time (offset from the clip's start time) and text content. Words are highlighted individually during playback.

**Related Code:**
- `Narrate::TimedWord` structure in `NarrateDataModel.h`
- `NarrateClip::getWords()` - Access words within a clip

### Project
A complete Narrate project containing all clips, timing information, render strategy settings, and metadata. Projects can be saved to/loaded from JSON files.

**Related Code:**
- `Narrate::NarrateProject` class in `NarrateDataModel.h`
- Save/load methods in `StandaloneImportFeature` and `StandaloneExportFeature`

## UI Components

### Editor View
The main editing interface where users create and modify projects. Provides controls for adding/removing clips, editing text, adjusting timing, and configuring project settings.

**Related Code:**
- `EditorView` class in `EditorView.h/cpp`
- Panels: `AudioPlaybackPanel`, `ExportPanel`, `DawSyncPanel`

### Running View
The playback/preview mode that displays the narration text with highlighting as it plays. This is the view users see when presenting or recording their narration.

**Related Code:**
- `RunningView` class in `RunningView.h/cpp`
- Navigation controls: Previous (<<), Next (>>), Jump Back (|<), Jump Forward (>|)

### Word Highlight
The visual indication (typically a yellow box) that shows the currently active word during playback. The highlight appears at the playhead time and follows the timeline as playback progresses.

**Related Code:**
- `currentWordIndex` in `RunningView`
- `RenderStrategy::render()` - Draws the highlight
- `HighlightSettings` - Configures highlight behavior

### Waveform Display
The audio waveform visualization shown in the editor view. Displays the loaded audio file's amplitude over time, with a vertical line indicating the playhead position.

**Related Code:**
- `WaveformDisplay` class in `WaveformDisplay.h/cpp`
- `AudioPlaybackPanel` - Contains the waveform display

## Timeline System

### Timeline Events
Time-based events that trigger during playback, including:
- **Clip Start/End** - When a clip begins or finishes
- **Word Start/End** - When a word becomes active or inactive
- **Highlight End** - When the word highlight should disappear

**Related Code:**
- `TimelineEventManager` class in `TimelineEventManager.h/cpp`
- `TimelineEventManager::buildTimeline()` - Creates sorted event list

### Event Processing
The system that fires timeline events as the playhead advances. Events are processed between `previousTime` and `currentTime` to ensure no events are missed.

**Related Code:**
- `TimelineEventManager::processEvents()` - Fires events in time range
- `TimelineEventManager::seekToTime()` - Resets event index for navigation

### Quantization
Optional time snapping that aligns events to a grid (e.g., 1/16th notes at a given BPM). Used to synchronize narration with musical timing.

**Related Code:**
- `HighlightSettings::quantizeEnabled` - Enable/disable quantization
- `HighlightSettings::quantizeTime()` - Snap time to grid

## Render Strategies

### Render Strategy
A pluggable algorithm that determines how text is displayed in Running View. Different strategies provide different visual presentations.

**Related Code:**
- `RenderStrategy` base class in `RenderStrategy.h`
- `RenderStrategy::render()` - Called every frame to draw content

### Scrolling Strategy
Displays text in a vertically scrolling format, with the current word highlighted. Text scrolls upward as playback progresses.

**Related Code:**
- `ScrollingRenderStrategy` class in `ScrollingRenderStrategy.h/cpp`

### Karaoke Strategy
Displays text in a static format with word-by-word highlighting, similar to karaoke lyrics display.

**Related Code:**
- `KaraokeRenderStrategy` class in `KaraokeRenderStrategy.h/cpp`

### Teleprompter Strategy
Displays text in a teleprompter style, designed for smooth reading during presentation.

**Related Code:**
- `TeleprompterRenderStrategy` class in `TeleprompterRenderStrategy.h/cpp`

## Import/Export

### SRT (SubRip Subtitle)
A subtitle file format with numbered entries containing timecodes and text. Narrate can import SRT files to create projects.

**Format:**
```
1
00:00:01,000 --> 00:00:03,500
This is the first subtitle

2
00:00:04,000 --> 00:00:06,500
This is the second subtitle
```

**Related Code:**
- `StandaloneImportFeature::importSRT()` - Parse SRT files

### WebVTT (Web Video Text Tracks)
A web-standard subtitle format similar to SRT but with a WEBVTT header. Supported for import.

**Format:**
```
WEBVTT

00:00:01.000 --> 00:00:03.500
First subtitle text

00:00:04.000 --> 00:00:06.500
Second subtitle text
```

**Related Code:**
- `StandaloneImportFeature::importWebVTT()` - Parse WebVTT files

### JSON Project Format
Narrate's native project file format. Contains complete project data including clips, words, timing, and settings.

**Related Code:**
- `StandaloneImportFeature::importJSON()` - Load projects
- `StandaloneExportFeature::exportJSON()` - Save projects

## Build Targets

### Standalone Mode
A standalone application that includes audio playback capabilities and full import/export features. Built with `NARRATE_ENABLE_AUDIO_PLAYBACK=1`.

**Build Command:**
```bash
./build.sh --target STANDALONE
```

### Plugin Mode
An audio plugin (VST3) that runs within a DAW. Synchronizes with DAW playback but does not include audio loading features. Built with `NARRATE_ENABLE_AUDIO_PLAYBACK=0`.

**Build Command:**
```bash
./build.sh --target PLUGIN
```

### Console Mode
A command-line tool for batch processing and testing. No GUI components included.

**Build Command:**
```bash
./build.sh --target CONSOLE
```

## Feature Components

### Feature Component Architecture
A flexible system that allows different implementations of features based on build target. Features are injected at runtime through the `FeatureRegistry`.

**Related Code:**
- `FeatureRegistry` class in `FeatureRegistry.h`
- Feature interfaces: `ImportFeature`, `ExportFeature`, `AudioPlaybackFeature`, `DawSyncFeature`

### Audio Playback Feature
Handles loading and playing audio files. Implementation varies by build target:
- **Standalone:** Full audio playback with `StandaloneAudioPlayback`
- **Plugin:** No-op implementation since DAW handles audio

**Related Code:**
- `AudioPlaybackFeature` interface in `AudioPlaybackFeature.h`
- `StandaloneAudioPlayback` class in `StandaloneAudioPlayback.h/cpp`

### DAW Sync Feature
Synchronizes playback with a Digital Audio Workstation when running as a plugin.

**Related Code:**
- `DawSyncFeature` interface in `DawSyncFeature.h`
- `PluginDawSyncFeature` class in `PluginDawSyncFeature.h/cpp`

## Settings and Configuration

### Highlight Settings
Configurable parameters that control word highlighting behavior during playback.

**Parameters:**
- `durationMode` - How long highlights last (Original/Minimum/Fixed/GridBased)
- `minimumDuration` - Minimum highlight duration in seconds
- `fixedDuration` - Fixed highlight duration for all words
- `lookAheadMs` - Render latency compensation in milliseconds
- `quantizeEnabled` - Enable time quantization

**Related Code:**
- `HighlightSettings` class in `HighlightSettings.h`

### Theme
Visual appearance settings (Dark/Light mode) that affect the entire UI.

**Related Code:**
- `NarrateLookAndFeel::Theme` enum
- `NarrateLookAndFeel::setTheme()` - Switch themes
- Saved to processor settings via `PluginProcessor::getSettings()`

## Navigation Controls

### Previous Clip (<<)
Jumps to the start of the previous clip in the project.

**Related Code:**
- `RunningView::previousClipClicked()` - line 207

### Next Clip (>>)
Jumps to the start of the next clip in the project.

**Related Code:**
- `RunningView::nextClipClicked()` - line 245

### Jump Back (|<)
Jumps backward 5 seconds in the timeline.

**Related Code:**
- `RunningView::jumpBackClicked()` - line 283

### Jump Forward (>|)
Jumps forward 5 seconds in the timeline.

**Related Code:**
- `RunningView::jumpForwardClicked()` - line 326

## Threading and Synchronization

### Message Thread
JUCE's main UI thread where all GUI updates must occur. Background operations must use `MessageManager::callAsync()` to update UI.

**Related Code:**
- `juce::MessageManager::callAsync()` - Post operation to message thread
- Used in `EditorView::importProjectClicked()` for thread-safe import

### Component::SafePointer
A thread-safe smart pointer for components that can detect if a component has been deleted. Used when accessing UI components from background threads.

**Related Code:**
- Used in `EditorView::importProjectClicked()` for progress window
- `juce::Component::SafePointer<ProgressWindow>`

### Background Thread
Worker threads for long-running operations like file import. Must not directly access UI components.

**Related Code:**
- `juce::Thread::launch()` - Start background task
- Used for SRT import to prevent UI blocking
