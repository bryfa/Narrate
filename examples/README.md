# Narrate Sample Import Files

This directory contains sample files demonstrating the various subtitle and transcript formats that Narrate can import.

## Supported Formats

### 1. SubRip (SRT) - `sample.srt`
Standard subtitle format used widely in video editing.
- Each subtitle has a numeric index
- Timecodes in format: `HH:MM:SS,mmm --> HH:MM:SS,mmm`
- Subtitle text follows the timecode
- Blank line separates entries

**Use Case:** Importing existing video subtitles

### 2. WebVTT - `sample.vtt`
Web Video Text Tracks format, commonly used for web videos.
- Must start with `WEBVTT` header
- Optional cue identifiers
- Timecodes in format: `HH:MM:SS.mmm --> HH:MM:SS.mmm`
- Text follows the timecode

**Use Case:** Importing web video captions

### 3. JSON - `sample.json`
Narrate's native format with full fidelity.
- Contains project name, clips, and word-level timing
- Each word has precise relative timing within its clip
- Preserves all Narrate project data

**Use Case:** Exchanging projects between Narrate instances

### 4. Plain Text - `sample.txt`
Simple text file with automatic timing.
- Each paragraph becomes a clip
- Words are automatically timed based on reading speed (~2.5 words/sec)
- No manual timing required

**Use Case:** Quick import of scripts and teleprompter text

## How to Import

1. Open Narrate Standalone application
2. Click the **Import** button in the toolbar
3. Select one of these sample files
4. The project will be created with timed clips

## Testing

These files are perfect for:
- Testing the import functionality
- Learning the different file formats
- Creating template files for your own projects
- Demonstrating Narrate's capabilities
