# Narrate - Architecture & Design Document

**Version:** 1.1
**Last Updated:** 2025-11-19
**Status:** Living Document

---

## Table of Contents

1. [Overview](#overview)
2. [Build Targets & Feature Configuration](#build-targets--feature-configuration)
3. [System Architecture](#system-architecture)
4. [Core Components](#core-components)
5. [Timing System Deep Dive](#timing-system-deep-dive)
6. [Data Model](#data-model)
7. [Render Strategies](#render-strategies)
8. [Current Features](#current-features)
9. [Development Roadmap](#development-roadmap)
10. [Design Decisions](#design-decisions)
11. [Extension Points](#extension-points)

---

## Overview

**Narrate** is a JUCE-based audio plugin (VST3/Standalone) for synchronized text display and highlighting. It enables creators to display lyrics, narration, or teleprompter text with precise word-level timing synchronized to audio playback.

### Key Capabilities

- **Multiple render strategies**: Scrolling, Karaoke, Teleprompter
- **Flexible timing modes**: Natural, Rhythmic (quantized), Fixed duration
- **Word-level formatting**: Colors, bold, italic, font size per word
- **Project management**: Save/load projects in XML format (.narrate)
- **Timeline recalculation**: Automatic gap removal between clips

### Technology Stack

- **Framework**: JUCE 8.0.10
- **Language**: C++20
- **Build System**: CMake 3.15+
- **Plugin Formats**: VST3, Standalone
- **Data Format**: XML-based project files

---

## Build Targets & Feature Configuration

Narrate builds as **two distinct products** with different features optimized for their use cases:

### Target Audiences

**1. Standalone Application**
- **Primary Users**: Video creators, lyric video producers, content creators
- **Use Case**: Creating synchronized lyric videos for YouTube/social media
- **Workflow**: Load audio file → Time lyrics → Export subtitles → Import to video editor
- **Key Need**: Audio playback and visual timing tools

**2. VST3 Plugin**
- **Primary Users**: Music producers, audio engineers, DAW users
- **Use Case**: Display lyrics/text during music production
- **Workflow**: Load plugin in DAW → Sync to DAW transport → Display during mixing
- **Key Need**: DAW integration and transport synchronization

### Feature Matrix

| Feature | Standalone | VST3 Plugin | Rationale |
|---------|-----------|-------------|-----------|
| **Audio File Loading** | ✅ YES | ❌ NO | Standalone needs own audio; DAW already has audio tracks |
| **Audio Playback** | ✅ YES | ❌ NO | Must hear audio to time lyrics; DAW handles playback |
| **Waveform Display** | ✅ YES | ❌ NO | Visual aid for timing; use DAW's waveform instead |
| **Transport Controls** | ✅ YES | ❌ NO | Play/pause/seek needed; DAW controls transport |
| **Export SRT/WebVTT** | ✅ YES | ❌ NO | Export for video editors; use DAW export instead |
| **DAW Transport Sync** | ❌ NO | ✅ YES | N/A for standalone; essential for plugin |
| **DAW Automation** | ❌ NO | ✅ YES | N/A for standalone; professional workflow |
| **Project Management** | ✅ YES | ✅ YES | Both need save/load |
| **Clip/Word Editing** | ✅ YES | ✅ YES | Core functionality |
| **Render Strategies** | ✅ YES | ✅ YES | Core functionality |
| **Formatting** | ✅ YES | ✅ YES | Core functionality |

### Configuration System

**File:** `Source/NarrateConfig.h`

Central configuration file that defines which features are available in each build:

```cpp
// Automatic build target detection
#if JUCE_STANDALONE_APPLICATION
    #define NARRATE_STANDALONE 1
    #define NARRATE_PLUGIN 0
#else
    #define NARRATE_STANDALONE 0
    #define NARRATE_PLUGIN 1
#endif

// Feature flags - Standalone
#if NARRATE_STANDALONE
    #define NARRATE_ENABLE_AUDIO_PLAYBACK 1
    #define NARRATE_ENABLE_AUDIO_FILE_LOADING 1
    #define NARRATE_ENABLE_WAVEFORM_DISPLAY 1
    #define NARRATE_ENABLE_TRANSPORT_CONTROLS 1
    #define NARRATE_ENABLE_SUBTITLE_EXPORT 1

    #define NARRATE_SHOW_LOAD_AUDIO_BUTTON 1
    #define NARRATE_SHOW_EXPORT_MENU 1

    #define NARRATE_ENABLE_DAW_TRANSPORT_SYNC 0
    #define NARRATE_ENABLE_DAW_AUTOMATION 0
#endif

// Feature flags - Plugin
#if NARRATE_PLUGIN
    #define NARRATE_ENABLE_AUDIO_PLAYBACK 0
    #define NARRATE_ENABLE_AUDIO_FILE_LOADING 0
    #define NARRATE_ENABLE_WAVEFORM_DISPLAY 0
    #define NARRATE_ENABLE_TRANSPORT_CONTROLS 0
    #define NARRATE_ENABLE_SUBTITLE_EXPORT 0

    #define NARRATE_SHOW_LOAD_AUDIO_BUTTON 0
    #define NARRATE_SHOW_EXPORT_MENU 0

    #define NARRATE_ENABLE_DAW_TRANSPORT_SYNC 1
    #define NARRATE_ENABLE_DAW_AUTOMATION 1
    #define NARRATE_SHOW_DAW_SYNC_INDICATOR 1
#endif
```

### Runtime Detection

Helper functions to query capabilities:

```cpp
namespace NarrateConfig
{
    inline const char* getBuildTarget()
    {
        #if NARRATE_STANDALONE
            return "Standalone";
        #else
            return "VST3 Plugin";
        #endif
    }

    inline bool hasAudioPlayback() { return NARRATE_ENABLE_AUDIO_PLAYBACK; }
    inline bool hasDawSync() { return NARRATE_ENABLE_DAW_TRANSPORT_SYNC; }
    inline bool canExportSubtitles() { return NARRATE_ENABLE_SUBTITLE_EXPORT; }
}
```

### UI Differences

**Standalone Toolbar:**
```
┌─────────────────────────────────────────────────────────────────────┐
│ [New] [Load] [Save]  Render:[▼]  [Load Audio]  [Preview]  [Export  │
│                                                            SRT] [VTT]│
│ Audio: my_song.wav                                                  │
└─────────────────────────────────────────────────────────────────────┘
```

**VST3 Plugin Toolbar:**
```
┌─────────────────────────────────────────────────────────────────────┐
│ [New] [Load] [Save]  Render:[▼]  DAW Sync: Disabled    [Preview]   │
└─────────────────────────────────────────────────────────────────────┘
```

### Conditional Compilation Example

**In `EditorView.h`:**
```cpp
#include "NarrateConfig.h"

class EditorView : public juce::Component
{
    // Common components
    juce::TextButton previewButton {"Preview"};

#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    // Standalone-only components
    juce::TextButton loadAudioButton {"Load Audio"};
    juce::Label audioFileLabel {"", "No audio loaded"};
    void loadAudioClicked();
#endif

#if NARRATE_SHOW_EXPORT_MENU
    // Standalone-only export
    juce::TextButton exportSrtButton {"Export SRT"};
    juce::TextButton exportVttButton {"Export WebVTT"};
#endif

#if NARRATE_SHOW_DAW_SYNC_INDICATOR
    // Plugin-only indicator
    juce::Label dawSyncLabel {"", "DAW Sync: Disabled"};
#endif
};
```

**In `EditorView.cpp`:**
```cpp
EditorView::EditorView(NarrateAudioProcessor* processor)
{
    // Common initialization
    addAndMakeVisible(previewButton);

#if NARRATE_SHOW_LOAD_AUDIO_BUTTON
    // Standalone-only initialization
    loadAudioButton.onClick = [this] { loadAudioClicked(); };
    addAndMakeVisible(loadAudioButton);
    addAndMakeVisible(audioFileLabel);
#endif

#if NARRATE_SHOW_DAW_SYNC_INDICATOR
    // Plugin-only initialization
    addAndMakeVisible(dawSyncLabel);
#endif
}
```

### Benefits

1. **Optimized Binaries**: Smaller plugin (no audio engine), richer standalone app
2. **Clear Development**: Know exactly which features belong where
3. **User-Focused**: Each product has features for its specific use case
4. **Maintainable**: Shared core code, separate feature sets
5. **Testable**: Build both to verify differences work correctly

### Current Implementation Status

**Standalone Features:**
- ✅ UI components added (Load Audio button, Export buttons, Audio file label)
- ⏳ Audio playback (Phase 1 - TODO)
- ⏳ Waveform display (Phase 1 - TODO)
- ⏳ SRT/WebVTT export (Phase 1 - TODO)

**VST3 Plugin Features:**
- ✅ UI component added (DAW Sync indicator)
- ⏳ DAW transport sync (Phase 2 - TODO)
- ⏳ Automation support (Phase 2 - TODO)

---

## System Architecture

### High-Level Component Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                     NarrateAudioProcessor                     │
│                    (Plugin Entry Point)                       │
└────────────────┬─────────────────────────────────┬───────────┘
                 │                                 │
        ┌────────▼────────┐              ┌────────▼────────┐
        │   EditorView    │              │   RunningView   │
        │  (Edit Mode)    │              │  (Play Mode)    │
        └────────┬────────┘              └────────┬────────┘
                 │                                 │
                 │                        ┌────────▼──────────┐
                 │                        │ TimelineEvent     │
                 │                        │    Manager        │
                 │                        │ (Event Engine)    │
                 │                        └────────┬──────────┘
                 │                                 │
        ┌────────▼──────────────────────────┬─────▼──────┐
        │     NarrateDataModel              │            │
        │  ┌──────────────────┐             │            │
        │  │ NarrateProject   │             │            │
        │  │  ├─ Clips        │             │            │
        │  │  │  └─ Words     │             │            │
        │  │  └─ Settings     │             │            │
        │  └──────────────────┘             │            │
        └───────────────────────────────────┘            │
                                                          │
                                            ┌─────────────▼──────────┐
                                            │   Render Strategies    │
                                            │  ┌──────────────────┐  │
                                            │  │ Scrolling        │  │
                                            │  │ Karaoke          │  │
                                            │  │ Teleprompter     │  │
                                            │  └──────────────────┘  │
                                            └────────────────────────┘
```

### Data Flow During Playback

```
User clicks "Play"
    ↓
RunningView starts timer (60fps)
    ↓
timerCallback() advances currentTime (+16ms)
    ↓
TimelineEventManager.processEvents(prevTime, currentTime + lookAhead)
    ↓
Fire event callbacks:
  - onClipStart → update currentClipIndex
  - onWordStart → update currentWordIndex
  - onHighlightEnd → trigger repaint
    ↓
repaint() called
    ↓
RenderStrategy.render() draws text with highlighting
    ↓
Display updated on screen
```

### Feature Components Architecture

**New in v1.1** - Narrate uses a **Feature Components pattern** to cleanly separate build-target-specific functionality without scattering `#if` directives throughout the codebase.

#### Design Pattern

```
┌─────────────────────────────────────────────────────────┐
│              PluginProcessor                             │
│  ┌──────────────────────────────────────────────────┐   │
│  │  std::unique_ptr<AudioPlaybackFeature>           │   │
│  │  std::unique_ptr<ExportFeature>                  │   │
│  │  std::unique_ptr<DawSyncFeature>                 │   │
│  └──────────────────────────────────────────────────┘   │
└────────────────┬────────────────────────────────────────┘
                 │
        ┌────────▼─────────┐
        │  FeatureFactory  │  ← Only place with #if directives
        └────────┬─────────┘
                 │
    ┌────────────┼────────────┐
    │            │            │
┌───▼────┐  ┌───▼─────┐  ┌──▼──────┐
│Standalone│ │ No-Op   │ │ Plugin  │
│Audio     │ │Export   │ │DawSync  │
│Playback  │ │Feature  │ │Feature  │
└──────────┘ └─────────┘ └─────────┘
```

#### Layer Structure

**1. Feature Interfaces** (`Source/Features/*Feature.h`)
- Pure virtual base classes
- Define common API for all implementations
- Examples: `AudioPlaybackFeature`, `ExportFeature`, `DawSyncFeature`

**2. Full Implementations** (`Source/Features/Standalone*.cpp`, `Plugin*.cpp`)
- Complete functionality for specific build targets
- Examples:
  - `StandaloneAudioPlayback` - Full audio player (Standalone only)
  - `StandaloneExportFeature` - SRT/WebVTT export (Standalone only)
  - `PluginDawSyncFeature` - DAW transport sync (Plugin only)

**3. No-Op Implementations** (`Source/Features/NoOp*.h`)
- Empty stubs for disabled features
- Null Object pattern - no null checks needed
- Zero runtime overhead
- Examples:
  - `NoOpAudioPlayback` - Used in Plugin builds
  - `NoOpExportFeature` - Used in Plugin builds
  - `NoOpDawSyncFeature` - Used in Standalone builds

**4. Feature Factory** (`Source/Features/FeatureFactory.h`)
- **The ONLY file with `#if` directives for features**
- Creates appropriate implementation based on build target
- Clean, centralized feature selection

**5. UI Panels** (`Source/UI/*Panel.cpp`)
- Self-contained UI for each feature
- Shows/hides based on feature availability
- Examples:
  - `AudioPlaybackPanel` - Transport controls, waveform (Standalone only)
  - `ExportPanel` - Export buttons (Standalone only)
  - `DawSyncPanel` - Sync indicator (Plugin only)

#### Example Usage

```cpp
// In PluginProcessor constructor - NO #if needed!
audioPlayback = FeatureFactory::createAudioPlayback();
// Standalone: Gets StandaloneAudioPlayback
// Plugin: Gets NoOpAudioPlayback

// Later in code - runtime check, no #if
if (audioPlayback->isAvailable())
{
    audioPlayback->startPlayback();  // Works or does nothing
}

// UI visibility - automatic
audioPlaybackPanel.setVisible(audioPlayback->isAvailable());
// Standalone: Shows transport controls
// Plugin: Hidden
```

#### Benefits

✅ **Clean Code** - No scattered `#if` directives in main application code
✅ **Type Safety** - Compile-time interface checking
✅ **Testability** - Easy to create mock implementations
✅ **Maintainability** - Each feature's code in one place
✅ **Null Safety** - No null pointer checks needed
✅ **Zero Runtime Cost** - No-op implementations inline to nothing

---

## Core Components

### 1. NarrateAudioProcessor
**File:** `Source/PluginProcessor.h/cpp`

- Entry point for the plugin
- Creates and manages editor components
- Currently passes audio through unchanged (no processing)
- Future: Will integrate audio playback and DAW transport sync

**Key Responsibilities:**
- Plugin initialization and lifecycle
- Audio buffer processing (currently passthrough)
- Editor creation (EditorView or RunningView)

### 2. EditorView
**File:** `Source/EditorView.h/cpp`

- Main editing interface for creating and managing projects
- Left panel: Clip list with Add/Remove/Recalculate buttons
- Right panel: Clip editor with timing and text fields
- Top toolbar: Project management and preview

**Key Components:**
```
EditorView
├─ clipListBox (juce::ListBox)
├─ addClipButton / removeClipButton / recalculateButton
├─ startTimeEditor / endTimeEditor
├─ clipTextEditor
├─ autoSpaceButton
├─ renderStrategyCombo
└─ previewButton
```

### 3. RunningView
**File:** `Source/RunningView.h/cpp`

- Playback mode interface
- Timer-based playback at 60fps (~16ms intervals)
- Manages event callbacks from TimelineEventManager
- Delegates rendering to active RenderStrategy

**State Management:**
- `currentTime`: Playback position in seconds
- `previousTime`: Previous frame time for event detection
- `currentClipIndex`: Active clip being displayed
- `currentWordIndex`: Active word being highlighted
- `isRunning`: Playback state flag

### 4. TimelineEventManager
**File:** `Source/TimelineEventManager.h/cpp`

Core timing engine that converts project data into a timeline of events.

**Event Types:**
```cpp
enum class EventType {
    ClipStart,      // Clip becomes active
    ClipEnd,        // Clip finishes
    WordStart,      // Word starts (highlight begins)
    WordEnd,        // Word finishes
    HighlightEnd    // Highlight should fade/end (separate from WordEnd)
};
```

**Event Callbacks:**
```cpp
std::function<void(int clipIndex)> onClipStart;
std::function<void(int clipIndex)> onClipEnd;
std::function<void(int clipIndex, int wordIndex)> onWordStart;
std::function<void(int clipIndex, int wordIndex)> onWordEnd;
std::function<void(int clipIndex, int wordIndex)> onHighlightEnd;
```

**Timeline Building Process:**
1. Clear existing timeline
2. Iterate through all clips in sorted order
3. For each clip:
   - Apply quantization to clip start if enabled
   - Add ClipStart event
   - For each word:
     - Calculate absolute time = clip.startTime + word.relativeTime
     - Apply quantization to word time if enabled
     - Add WordStart event
     - Calculate highlight duration based on settings
     - Add HighlightEnd event (at wordStart + highlightDuration)
     - Add WordEnd event (at wordStart + wordDuration)
   - Add ClipEnd event
4. Sort all events by time
5. Reset event cursor to index 0

**Event Processing:**
```cpp
void processEvents(double previousTime, double currentTime)
{
    // Process all events in time range [previousTime, currentTime)
    while (nextEventIndex < timeline.size()) {
        const auto& event = timeline[nextEventIndex];

        if (event.time < previousTime) {
            nextEventIndex++; // Skip past events
            continue;
        }

        if (event.time >= currentTime) {
            break; // Future event, stop processing
        }

        // Fire appropriate callback
        switch (event.type) {
            case EventType::WordStart:
                if (onWordStart) onWordStart(event.clipIndex, event.wordIndex);
                break;
            // ... other event types
        }

        nextEventIndex++;
    }
}
```

### 5. HighlightSettings
**File:** `Source/HighlightSettings.h`

Configuration for timing behavior and highlight duration calculation.

**Duration Modes:**
- `Original`: Use word's natural duration (word spacing)
- `Minimum`: Ensure minimum readability duration
- `Fixed`: All words highlighted for same duration
- `GridBased`: Highlight until next rhythmic grid position

**Quantization:**
- BPM-based timing snap (120 BPM default)
- Subdivision: whole, half, quarter, eighth notes
- Snap interval = (60.0 / BPM) / subdivision

**Look-Ahead Compensation:**
- Default: 25ms
- Triggers visual highlighting early to compensate for rendering latency
- Applied during event processing: `currentTime + lookAheadMs`

**Built-in Presets:**
1. **Natural**: Original timing, no quantization
2. **Storytelling**: Minimum 200ms per word for readability
3. **Rhythmic**: Quantized to 120 BPM quarter notes
4. **Teleprompter**: Minimum 300ms for comfortable reading
5. **Fixed Duration**: 500ms per word

---

## Timing System Deep Dive

### Timeline Event Generation

The timing system converts hierarchical project data into a flat, sorted timeline of events.

**Input Data Structure:**
```
NarrateProject
  ├─ Clip 0: startTime=0.0, endTime=3.0
  │   ├─ Word 0: "Hello" relativeTime=0.0
  │   ├─ Word 1: "World" relativeTime=1.0
  │   └─ Word 2: "!" relativeTime=2.0
  └─ Clip 1: startTime=3.0, endTime=5.0
      ├─ Word 0: "Welcome" relativeTime=0.0
      └─ Word 1: "Here" relativeTime=1.5
```

**Generated Timeline (with Natural settings):**
```
Time    Event Type      Clip    Word    Notes
─────────────────────────────────────────────────────────────
0.000   ClipStart       0       -       Clip 0 begins
0.000   WordStart       0       0       "Hello" starts
1.000   HighlightEnd    0       0       "Hello" highlight ends
1.000   WordEnd         0       0       "Hello" fully processed
1.000   WordStart       0       1       "World" starts
2.000   HighlightEnd    0       1       "World" highlight ends
2.000   WordEnd         0       1       "World" fully processed
2.000   WordStart       0       2       "!" starts
3.000   HighlightEnd    0       2       "!" highlight ends
3.000   WordEnd         0       2       "!" fully processed
3.000   ClipEnd         0       -       Clip 0 ends
3.000   ClipStart       1       -       Clip 1 begins
3.000   WordStart       1       0       "Welcome" starts
4.500   HighlightEnd    1       0       "Welcome" highlight ends
4.500   WordEnd         1       0       "Welcome" fully processed
4.500   WordStart       1       1       "Here" starts
5.000   HighlightEnd    1       1       "Here" highlight ends
5.000   WordEnd         1       1       "Here" fully processed
5.000   ClipEnd         1       -       Clip 1 ends
```

### Quantization Algorithm

When quantization is enabled, word and clip times snap to musical grid positions.

**Snap Interval Calculation:**
```cpp
double snapInterval = (60.0 / bpm) / subdivision;

// Examples:
// 120 BPM, quarter notes: (60/120)/4 = 0.125s = 125ms
// 120 BPM, eighth notes:  (60/120)/8 = 0.0625s = 62.5ms
// 90 BPM, quarter notes:  (60/90)/4 = 0.167s = 167ms
```

**Quantization Process:**
```cpp
double quantizeTime(double time) const {
    if (!quantizeEnabled) return time;
    double interval = getSnapInterval();
    return std::round(time / interval) * interval;
}
```

**Application in Timeline Building:**
```cpp
// Quantize clip start time
double clipStartTime = quantizeTime(clip.getStartTime());

// Quantize word time
double wordTime = clipStartTime + word.relativeTime;
wordTime = quantizeTime(wordTime);

// Ensure word doesn't start before previous word ends
if (wordTime < previousWordEndTime) {
    wordTime = previousWordEndTime;
}
```

### Highlight Duration Calculation

**File:** `TimelineEventManager.cpp` (Lines 94-134)

```cpp
double calculateHighlightDuration(double wordDuration,
                                  double wordStartTime,
                                  const HighlightSettings& settings)
{
    switch (settings.durationMode) {
        case DurationMode::Original:
            return wordDuration;

        case DurationMode::Minimum:
            return std::max(wordDuration, settings.minimumDuration);

        case DurationMode::Fixed:
            return settings.fixedDuration;

        case DurationMode::GridBased: {
            // Highlight until next grid position
            double snapInterval = settings.getSnapInterval();
            double nextGridTime = std::ceil(wordStartTime / snapInterval) * snapInterval;
            if (nextGridTime <= wordStartTime)
                nextGridTime += snapInterval;
            return nextGridTime - wordStartTime;
        }
    }
}
```

### Look-Ahead Compensation

To compensate for rendering latency and human perception delay, events are processed with look-ahead.

**Implementation (RunningView.cpp:130):**
```cpp
double lookAheadTime = currentTime + (highlightSettings.lookAheadMs / 1000.0);
eventManager.processEvents(previousTime, lookAheadTime);
```

**Effect:**
- Default 25ms look-ahead means highlights appear 25ms before the word's actual time
- Compensates for screen refresh delay and perceived lag
- User-adjustable in future versions

### Current Limitations

1. **No DAW Transport Sync**: Timer-based, not synced to audio host playhead
2. **No Audio Playback**: Background audio file supported in data model but not played
3. **Timer Precision**: 60fps (~16ms) limits timing accuracy
4. **No Sample-Accurate Events**: Not synced to audio buffer processing
5. **Fixed Look-Ahead**: Not adjustable at runtime (requires preset change)

---

## Data Model

### Hierarchy

```
NarrateProject
  ├─ projectName (String)
  ├─ backgroundAudioFile (File) [not yet used for playback]
  ├─ defaultFontSize (float)
  ├─ defaultTextColour (Colour)
  ├─ highlightColour (Colour)
  ├─ renderStrategy (enum: Scrolling/Karaoke/Teleprompter)
  └─ clips (Array<NarrateClip>)
       └─ NarrateClip
            ├─ id (String)
            ├─ startTime (double, absolute seconds)
            ├─ endTime (double, absolute seconds)
            ├─ defaultFormatting (TextFormatting)
            └─ words (Array<NarrateWord>)
                 └─ NarrateWord
                      ├─ text (String)
                      ├─ relativeTime (double, seconds from clip start)
                      └─ formatting (optional<TextFormatting>)
```

### TextFormatting
**File:** `Source/NarrateDataModel.h` (Lines 10-25)

```cpp
struct TextFormatting {
    juce::Colour colour = juce::Colours::white;
    bool bold = false;
    bool italic = false;
    float fontSizeMultiplier = 1.0f;  // Relative to base font
};
```

### NarrateWord
**File:** `Source/NarrateDataModel.h` (Lines 28-52)

```cpp
struct NarrateWord {
    juce::String text;
    double relativeTime = 0.0;  // Relative to clip start
    std::optional<TextFormatting> formatting;  // Overrides clip default

    TextFormatting getEffectiveFormatting(const TextFormatting& clipDefault) const {
        return formatting.has_value() ? formatting.value() : clipDefault;
    }
};
```

**Key Insight:** Words store *relative* time, not absolute time. This allows clips to be moved in the timeline without recalculating all word times.

### NarrateClip
**File:** `Source/NarrateDataModel.h` (Lines 55-124)

```cpp
class NarrateClip {
    juce::String id;
    double startTime = 0.0;    // Absolute timeline position
    double endTime = 0.0;      // Absolute timeline position
    juce::Array<NarrateWord> words;
    TextFormatting defaultFormatting;

public:
    double getDuration() const { return endTime - startTime; }

    double getWordAbsoluteTime(int wordIndex) const {
        return startTime + words[wordIndex].relativeTime;
    }

    void setText(const juce::String& text) {
        words.clear();
        auto tokens = juce::StringArray::fromTokens(text, " \t\n", "");
        for (const auto& token : tokens)
            words.add(NarrateWord(token, 0.0));
    }
};
```

### NarrateProject
**File:** `Source/NarrateDataModel.h` (Lines 127-241)

```cpp
class NarrateProject {
    juce::String projectName;
    juce::Array<NarrateClip> clips;
    juce::File backgroundAudioFile;
    float defaultFontSize = 24.0f;
    juce::Colour defaultTextColour = juce::Colours::white;
    juce::Colour highlightColour = juce::Colours::yellow;
    RenderStrategy renderStrategy = RenderStrategy::Scrolling;

public:
    enum class RenderStrategy {
        Scrolling = 0,
        Karaoke = 1,
        Teleprompter = 2
    };

    // Clip management with auto-sorting
    void addClip(const NarrateClip& clip) {
        clips.add(clip);
        sortClips();
    }

    // Timeline recalculation
    void recalculateTimeline();

    // Validation
    bool hasOverlappingClips() const;

    // Serialization
    bool saveToFile(const juce::File& file);
    bool loadFromFile(const juce::File& file);
};
```

### Timeline Recalculation

**Feature:** Removes gaps between clips after deletion.

**Implementation (NarrateDataModel.cpp:206-226):**
```cpp
void NarrateProject::recalculateTimeline() {
    if (clips.isEmpty()) return;

    double currentTime = 0.0;

    for (int i = 0; i < clips.size(); ++i) {
        auto& clip = clips.getReference(i);
        double duration = clip.getEndTime() - clip.getStartTime();

        clip.setStartTime(currentTime);
        clip.setEndTime(currentTime + duration);

        currentTime += duration;
    }

    sortClips();
}
```

**Example:**
```
Before deletion:
  Clip 0: [0.0 - 3.0]
  Clip 1: [3.0 - 5.0]  ← Delete this
  Clip 2: [5.0 - 8.0]

After recalculation:
  Clip 0: [0.0 - 3.0]
  Clip 2: [3.0 - 6.0]  ← Moved up, duration preserved
```

### XML Serialization

**File Format:** `.narrate` files are XML with this structure:

```xml
<NarrateProject version="1.0" projectName="My Project">
  <Settings defaultFontSize="24"
            defaultTextColour="FFFFFFFF"
            highlightColour="FFFFFF00"
            renderStrategy="0"/>
  <BackgroundAudio path="/path/to/audio.wav"/>
  <Clips>
    <Clip id="clip_1" startTime="0.0" endTime="3.0">
      <DefaultFormatting colour="FFFFFFFF" bold="false" italic="false" fontSizeMultiplier="1.0"/>
      <Words>
        <Word text="Hello" relativeTime="0.0"/>
        <Word text="World" relativeTime="1.0">
          <Formatting colour="FFFF0000" bold="true"/>
        </Word>
      </Words>
    </Clip>
  </Clips>
</NarrateProject>
```

---

## Render Strategies

All render strategies implement the `RenderStrategy` interface and receive a `RenderContext` with current playback state.

### RenderContext Structure
**File:** `Source/RenderStrategy.h` (Lines 14-23)

```cpp
struct RenderContext {
    const Narrate::NarrateProject& project;
    double currentTime;
    int currentClipIndex;
    bool isRunning;
    juce::Rectangle<int> bounds;
    int clipIndex;      // From event callbacks
    int wordIndex;      // From event callbacks
};
```

### 1. ScrollingRenderStrategy
**File:** `Source/ScrollingRenderStrategy.h/cpp`

**Behavior:**
- Displays all clips vertically
- Current clip centered in viewport
- Previous clips above, future clips below
- Highlights current word with background and brighter color

**Key Features:**
- Smooth scrolling animation
- Alpha fade for non-current clips (0.5 opacity)
- Word wrapping to fit viewport width
- Configurable line spacing and word spacing

**Visual Layout:**
```
┌─────────────────────────────────┐
│  [Previous Clip - dimmed]       │
│                                 │
├═════════════════════════════════┤
│  Current Clip - Full Brightness │
│  Hello [World] Welcome          │  ← "World" highlighted
│                                 │
├═════════════════════════════════┤
│  [Next Clip - dimmed]           │
│                                 │
└─────────────────────────────────┘
```

### 2. KaraokeRenderStrategy
**File:** `Source/KaraokeRenderStrategy.h/cpp`

**Behavior:**
- Shows current line centered horizontally
- Previous line above (dimmed)
- Next line below (dimmed)
- Current word highlighted

**Line Breaking:**
- Calculates line breaks based on word widths
- Respects viewport width
- Centers lines horizontally

**Visual Layout:**
```
┌─────────────────────────────────┐
│                                 │
│    Previous line (dimmed)       │
│                                 │
│ Current [Word] Being Highlighted│  ← Centered
│                                 │
│    Next line coming up          │
│                                 │
└─────────────────────────────────┘
```

### 3. TeleprompterRenderStrategy
**File:** `Source/TeleprompterRenderStrategy.h/cpp`

**Behavior:**
- Shows all text continuously scrolling upward
- Read line at 1/3 from top (configurable)
- Current word kept at read line via scroll offset
- Larger font size (1.3x multiplier)

**Configuration:**
```cpp
float readLinePosition = 0.33f;    // 33% from top
float lineSpacing = 1.5f;          // Line height multiplier
float wordSpacing = 15.0f;         // Pixels between words
bool showReadLine = true;          // Draw horizontal guide line
```

**Scroll Calculation:**
```cpp
float calculateScrollOffset(const std::vector<LineInfo>& allLines,
                           int currentClipIndex,
                           int currentWordIndex,
                           float lineHeight,
                           float areaY,
                           float readLineY) const
{
    // Find line containing current word from current clip
    for (int i = 0; i < allLines.size(); ++i) {
        const auto& line = allLines[i];

        if (line.clipIndex != currentClipIndex) continue;

        if (currentWordIndex >= line.startWordIndex &&
            currentWordIndex <= line.endWordIndex) {
            // Position this line at read line
            float targetY = i * lineHeight;
            float readLineOffset = readLineY - areaY;
            return targetY - readLineOffset;
        }
    }
    return 0.0f;
}
```

**Visual Layout:**
```
┌─────────────────────────────────┐
│  Earlier text...                │
│  More text above...             │
├─────────────────────────────────┤ ← Read line (33%)
│  Current [word] right here      │
├─────────────────────────────────┤
│  Future text coming up...       │
│  Even more text below...        │
└─────────────────────────────────┘
```

### Adding New Render Strategies

To create a custom render strategy:

1. Inherit from `RenderStrategy` interface
2. Implement `render(Graphics& g, const RenderContext& context)`
3. Implement `getName()` for UI display
4. Register in `PluginEditor.cpp` strategy initialization

**Example:**
```cpp
class CustomRenderStrategy : public RenderStrategy {
public:
    void render(juce::Graphics& g, const RenderContext& context) override {
        // Your rendering code here
        // Use context.currentClipIndex and context.wordIndex
        // to determine what to highlight
    }

    juce::String getName() const override {
        return "Custom";
    }
};
```

---

## Current Features

### Implemented (v1.0)

✅ **Project Management**
- Create/load/save projects (.narrate XML format)
- Project-level settings (font, colors, render strategy)
- Background audio file reference (not yet played)

✅ **Clip Management**
- Add/remove clips with timing
- Timeline recalculation (remove gaps)
- Auto-sorting by start time
- Overlap detection

✅ **Word Management**
- Text editing with auto-word-splitting
- Manual word timing (relative to clip)
- Auto-space words evenly across clip duration
- Per-word formatting (color, bold, italic, size)

✅ **Timing System**
- Event-driven timeline with 5 event types
- Multiple duration modes (Original, Minimum, Fixed, GridBased)
- BPM-based quantization with subdivisions
- Look-ahead compensation for rendering latency
- 5 built-in timing presets

✅ **Render Strategies**
- Scrolling: Vertical scroll with centered current clip
- Karaoke: Line-by-line with dimmed context
- Teleprompter: Continuous scroll with read line

✅ **Playback**
- Timer-based playback at 60fps
- Preview mode from editor
- Play/stop controls
- Time display

✅ **Data Model**
- Hierarchical project structure
- XML serialization/deserialization
- Validation (overlap detection)
- Clip sorting and timeline management

### Known Limitations

⚠️ **No Audio Integration**
- No background audio playback
- No DAW transport synchronization
- No audio buffer processing

⚠️ **Manual Timing Only**
- All word times must be set manually
- No audio analysis or auto-timing
- No visual waveform display

⚠️ **No Timeline UI**
- No graphical timeline editor
- No drag-and-drop timing adjustment
- No visual feedback during editing

⚠️ **Fixed Presets**
- Can't create custom timing presets
- Can't adjust settings during playback
- Look-ahead not user-adjustable at runtime

---

## Development Roadmap

### Phase 1: Enhanced Manual Timing Workflow
**Priority:** High | **Effort:** 1-2 weeks

**Goal:** Improve editing experience without audio integration

**Features:**
- [ ] Visual timeline editor with draggable word markers
- [ ] Waveform display (static, from loaded audio file)
- [ ] Real-time timing adjustment sliders
  - Global timing offset (shift all words)
  - Per-clip timing offset
  - Look-ahead adjustment
- [ ] Keyboard shortcuts for timing nudge (±10ms, ±100ms)
- [ ] Improved auto-spacing algorithms
  - Syllable-aware distribution
  - Ease-in/out curves
- [ ] Undo/redo for timing changes

**Benefits:**
- Immediate UX improvement
- Lays foundation for audio integration
- Addresses top user pain point (manual timing)

---

### Phase 2: Audio Playback Integration
**Priority:** High | **Effort:** 2-4 weeks

**Goal:** Sync timing to actual audio playback

**Features:**
- [ ] Background audio playback
  - Load audio file into plugin
  - Audio transport controls (play, pause, seek)
  - Sync timer to audio position
- [ ] Waveform visualization
  - Zoomable waveform display
  - Click to place word timing
  - Visual onset detection markers
- [ ] Audio-assisted timing
  - "Tap to mark" mode (tap key during playback)
  - Onset detection to suggest word boundaries
  - Beat detection for BPM calculation
- [ ] Scrubbing and preview
  - Drag playhead to scrub audio
  - Loop region playback
  - Preview word timing in context

**Technical Changes:**
```cpp
// Add to PluginProcessor
juce::AudioFormatManager formatManager;
std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
juce::AudioTransportSource transportSource;

void prepareToPlay(double sampleRate, int samplesPerBlock) {
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    transportSource.getNextAudioBlock(bufferInfo);

    // Sync timing to audio position
    double audioPos = transportSource.getCurrentPosition();
    // Pass to RunningView for event processing
}
```

**Benefits:**
- Transforms workflow from purely manual to audio-assisted
- Natural synchronization between audio and text
- Foundation for advanced features

---

### Phase 3: DAW Integration & Professional Features
**Priority:** Medium | **Effort:** 4-8 weeks

**Goal:** Professional production workflow integration

**Features:**
- [ ] DAW transport synchronization
  - Read host playhead position via `AudioPlayHead`
  - Start/stop follows DAW transport
  - Support for DAW loop regions
- [ ] Timeline tempo sync
  - Read session BPM from DAW
  - Support tempo changes
  - Time signature awareness
- [ ] Automation support
  - Map look-ahead parameter to DAW automation
  - Automate highlight duration per section
  - Automate render strategy switching
- [ ] Export formats
  - SRT subtitle export
  - WebVTT export
  - Final Cut Pro XML
  - MIDI markers for word timings
- [ ] Project organization
  - Multiple tracks/layers
  - Color-coded clips
  - Markers and regions

**Technical Changes:**
```cpp
void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    if (auto* playHead = getPlayHead()) {
        juce::AudioPlayHead::CurrentPositionInfo info;
        if (playHead->getCurrentPosition(info)) {
            double daw_position = info.timeInSeconds;
            double daw_bpm = info.bpm;
            bool is_playing = info.isPlaying;

            // Sync to DAW instead of internal timer
            eventManager.processEvents(prev, daw_position);
        }
    }
}
```

**Benefits:**
- Professional video production integration
- Seamless music production workflow
- Export to standard formats

---

### Phase 4: AI/ML & Advanced Features
**Priority:** Low | **Effort:** 8+ weeks

**Goal:** Automated timing and intelligent features

**Features:**
- [ ] AI auto-timing
  - Whisper API integration for word-level timestamps
  - Confidence scores and manual correction UI
  - Custom model training from user corrections
- [ ] Natural language processing
  - Emphasis detection (louder words = longer highlight)
  - Pause detection and insertion
  - Punctuation-aware timing
  - Sentiment-based color formatting
- [ ] Advanced analysis
  - Syllable counting for smart spacing
  - Prosody analysis for natural timing
  - Multi-language support
- [ ] Cloud features
  - Cloud project storage
  - Collaborative editing
  - Template marketplace
  - Version history

**Benefits:**
- Revolutionary workflow improvement
- Differentiation from competitors
- Potential revenue streams

---

### Phase 5: Mobile & Web Expansion
**Priority:** Low | **Effort:** 12+ weeks

**Goal:** Cross-platform reach

**Features:**
- [ ] Web-based editor
  - WebAssembly JUCE build
  - Browser-based editing
  - Cloud project integration
- [ ] Mobile apps
  - iOS/Android viewers
  - Remote control for presentations
  - Offline playback
- [ ] Live streaming integration
  - OBS plugin
  - RTMP overlay
  - Twitch/YouTube integration

---

## Design Decisions

### Why Timer-Based Playback Instead of Audio Buffer Sync?

**Decision:** Use JUCE Timer at 60fps for playback advancement

**Rationale:**
- Simpler initial implementation for text-only display
- Decouples text rendering from audio processing
- Easier to debug and test timing logic
- Allows standalone preview without audio

**Trade-offs:**
- ❌ Limited precision (~16ms resolution)
- ❌ Not sample-accurate
- ❌ Can drift from audio over time
- ✅ Simple and reliable for prototype
- ✅ Easy to switch to audio sync later

**Future:** Will migrate to audio buffer-driven timing in Phase 2.

---

### Why Separate HighlightEnd from WordEnd Events?

**Decision:** Create distinct event types for word ending vs highlight ending

**Rationale:**
- Allows highlight duration to differ from word spacing
- Supports overlapping highlights (next word starts while previous still highlighted)
- Enables fade-out effects and transitions
- Flexibility for different render strategies

**Example:**
```
Time: 0.0     1.0     1.5     2.0
      |-------|-------|-------|
      Word1   Word2   Word3
      [====]           [Highlight duration = 1.0s]
      Word1   [====]   [Highlight duration = 1.0s]
              Word2    [====]
                       Word3
```

---

### Why Relative Time for Words Instead of Absolute?

**Decision:** Store word times relative to clip start, not absolute timeline

**Rationale:**
- Clips can be moved without recalculating word times
- Easier to understand and edit (word is "0.5s into the clip")
- Supports clip reuse and templates
- Simplifies copy/paste and duplication

**Trade-off:**
- Requires conversion to absolute time for event generation
- Handled automatically by TimelineEventManager

---

### Why Strategy Pattern for Renderers?

**Decision:** Use Strategy pattern with `RenderStrategy` interface

**Rationale:**
- Easy to add new visualization modes
- Clean separation of rendering from logic
- Can switch strategies at runtime
- Users can create custom strategies

**Implementation:**
```cpp
class RenderStrategy {
public:
    virtual ~RenderStrategy() = default;
    virtual void render(juce::Graphics&, const RenderContext&) = 0;
    virtual juce::String getName() const = 0;
};
```

**Benefits:**
- 3 strategies implemented without modifying core code
- Future strategies can be plugins/extensions

---

### Why XML Instead of JSON or Binary?

**Decision:** Use XML for project file format

**Rationale:**
- Native JUCE support (`XmlElement`)
- Human-readable for debugging
- Supports nested structures naturally
- Handles attributes and elements
- Can be edited manually if needed

**Trade-offs:**
- ❌ Larger file size than binary
- ❌ Slower parsing than binary
- ✅ Easy to debug and version control
- ✅ No external dependencies
- ✅ Platform-independent

---

### Why Separate Standalone and VST3 Feature Sets?

**Decision:** Build two distinct products with different features using conditional compilation

**Rationale:**
- **Different User Needs**: Video creators need audio playback; DAW users already have audio
- **Optimized Binaries**: VST3 plugin stays small without audio engine code
- **Focused UX**: Each product has exactly the features its users need
- **No Redundancy**: VST3 doesn't duplicate DAW functionality (transport, audio, export)

**Target Markets:**
```
Standalone → Video Creators (YouTube lyric videos, social media)
VST3 Plugin → Music Producers (DAW-based workflow)
```

**Implementation Approach:**
- **Single Codebase**: Shared core functionality (timing, rendering, data model)
- **Conditional Compilation**: `#if NARRATE_STANDALONE` / `#if NARRATE_PLUGIN`
- **Feature Flags**: Centralized in `NarrateConfig.h`
- **UI Adaptation**: Different toolbars and controls per build target

**Benefits:**
- ✅ Smaller plugin size (no audio engine, no file I/O for audio)
- ✅ Richer standalone app (audio playback, waveform, export)
- ✅ Clear development direction (know what belongs where)
- ✅ Better user experience (no confusing unused features)
- ✅ Easier testing (can verify both builds independently)

**Trade-offs:**
- ❌ More complex build configuration
- ❌ Need to maintain two UI variants
- ✅ Worth it for user-focused products
- ✅ Shared core reduces maintenance burden

**Future Consideration:**
As features mature, may add AU (Audio Unit) format for macOS with similar conditional approach.

---

## Extension Points

### Adding Custom Render Strategies

**Interface:** `Source/RenderStrategy.h`

```cpp
class MyCustomStrategy : public RenderStrategy {
public:
    void render(juce::Graphics& g, const RenderContext& context) override {
        // Access project data
        const auto& project = context.project;

        // Check playback state
        if (!context.isRunning) {
            // Draw preview state
            return;
        }

        // Get current word
        if (context.clipIndex >= 0) {
            const auto& clip = project.getClip(context.clipIndex);
            const auto& word = clip.getWord(context.wordIndex);

            // Your rendering logic here
        }
    }

    juce::String getName() const override {
        return "My Custom";
    }
};
```

**Registration in PluginEditor.cpp:**
```cpp
renderStrategies.add(std::make_unique<MyCustomStrategy>());
```

---

### Adding Custom Timing Presets

**Location:** `Source/HighlightSettings.h`

```cpp
static HighlightSettings myCustomPreset() {
    HighlightSettings settings;
    settings.quantizeEnabled = true;
    settings.bpm = 140.0;
    settings.subdivision = 8;  // Eighth notes
    settings.durationMode = DurationMode::GridBased;
    settings.lookAheadMs = 30.0;
    return settings;
}
```

---

### Extending the Data Model

**Adding new word properties:**

```cpp
// In NarrateDataModel.h
struct NarrateWord {
    juce::String text;
    double relativeTime = 0.0;
    std::optional<TextFormatting> formatting;

    // ADD YOUR PROPERTIES HERE
    float customVolume = 1.0f;
    juce::String phonetic;  // Pronunciation guide
};

// Update XML serialization in NarrateDataModel.cpp
std::unique_ptr<juce::XmlElement> NarrateWord::toXml() const {
    auto xml = std::make_unique<juce::XmlElement>("Word");
    xml->setAttribute("text", text);
    xml->setAttribute("relativeTime", relativeTime);
    xml->setAttribute("customVolume", customVolume);  // ADD THIS
    xml->setAttribute("phonetic", phonetic);          // AND THIS
    // ...
    return xml;
}
```

---

### Adding New Event Types

**In TimelineEventManager.h:**

```cpp
enum class EventType {
    ClipStart,
    ClipEnd,
    WordStart,
    WordEnd,
    HighlightEnd,

    // ADD YOUR EVENT TYPES HERE
    SectionMarker,
    BreakPoint,
    CustomTrigger
};
```

**Register callback:**
```cpp
std::function<void(int clipIndex)> onSectionMarker;
```

**Fire event in processEvents():**
```cpp
case EventType::SectionMarker:
    if (onSectionMarker) onSectionMarker(event.clipIndex);
    break;
```

---

## Quick Reference

### Key Files Cheat Sheet

| Component | Header | Implementation |
|-----------|--------|----------------|
| Plugin Entry | `PluginProcessor.h` | `PluginProcessor.cpp` |
| Editor UI | `EditorView.h` | `EditorView.cpp` |
| Playback UI | `RunningView.h` | `RunningView.cpp` |
| Timing Engine | `TimelineEventManager.h` | `TimelineEventManager.cpp` |
| Data Model | `NarrateDataModel.h` | `NarrateDataModel.cpp` |
| Timing Config | `HighlightSettings.h` | (header-only) |
| **Build Config** | **`NarrateConfig.h`** | **(header-only)** |
| Render Interface | `RenderStrategy.h` | (interface) |
| Scrolling | `ScrollingRenderStrategy.h` | `.cpp` |
| Karaoke | `KaraokeRenderStrategy.h` | `.cpp` |
| Teleprompter | `TeleprompterRenderStrategy.h` | `.cpp` |

### Code References by Feature

| Feature | Primary Location |
|---------|-----------------|
| Event timeline building | `TimelineEventManager.cpp:13-92` |
| Event processing | `TimelineEventManager.cpp:136-188` |
| Quantization | `HighlightSettings.h:107-126` |
| Highlight duration calc | `TimelineEventManager.cpp:94-134` |
| Timer playback | `RunningView.cpp:118-143` |
| Timeline recalculation | `NarrateDataModel.cpp:206-226` |
| Word absolute time | `NarrateDataModel.h:103-108` |
| XML save/load | `NarrateDataModel.cpp:118-204` |
| Teleprompter scroll | `TeleprompterRenderStrategy.cpp:162-197` |

---

## Contributing

See [README.md](README.md) for build instructions and contribution guidelines.

For questions about architecture decisions, open a discussion on GitHub.

---

**Document Status:** Active Development
**Maintained By:** Mulhacen Labs
**Last Review:** 2025-11-17
