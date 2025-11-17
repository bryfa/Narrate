#include "WaveformDisplay.h"

WaveformDisplay::WaveformDisplay()
    : thumbnailCache(5),  // Cache 5 thumbnails
      thumbnail(512, formatManager, thumbnailCache)  // 512 samples per thumbnail point
{
    // Register basic audio formats
    formatManager.registerBasicFormats();

    // Listen for thumbnail changes
    thumbnail.addChangeListener(this);
}

WaveformDisplay::~WaveformDisplay()
{
    thumbnail.removeChangeListener(this);
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    // Draw background
    g.fillAll (juce::Colour (0xff1a1a1a));

    // Draw border
    g.setColour (juce::Colours::black);
    g.drawRect (bounds, 1);

    if (thumbnail.getTotalLength() > 0.0)
    {
        // Draw waveform
        g.setColour (juce::Colour (0xff4a9eff));  // Blue waveform
        thumbnail.drawChannels (g, bounds.reduced(2),
                               0.0, thumbnail.getTotalLength(),
                               1.0f);

        // Draw playback position indicator
        if (relativePosition > 0.0)
        {
            auto playheadX = static_cast<int>(bounds.getWidth() * relativePosition);
            g.setColour (juce::Colours::yellow);
            g.drawLine (static_cast<float>(playheadX), 0.0f,
                       static_cast<float>(playheadX), static_cast<float>(bounds.getHeight()), 2.0f);
        }
    }
    else
    {
        // No waveform loaded - show placeholder text
        g.setColour (juce::Colours::grey);
        g.setFont (14.0f);
        g.drawText ("No audio loaded", bounds, juce::Justification::centred);
    }
}

void WaveformDisplay::resized()
{
    // Nothing to resize currently
}

void WaveformDisplay::loadURL (const juce::File& file)
{
    thumbnail.clear();

    auto* reader = formatManager.createReaderFor (file);

    if (reader != nullptr)
    {
        auto duration = reader->lengthInSamples / reader->sampleRate;

        if (duration < 2.0)
        {
            thumbnail.reset (static_cast<int>(reader->numChannels), reader->sampleRate, static_cast<int>(reader->lengthInSamples));

            // For short files, read into a buffer and add to thumbnail
            juce::AudioBuffer<float> buffer (static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
            reader->read (&buffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, true);
            thumbnail.addBlock (0, buffer, 0, static_cast<int>(reader->lengthInSamples));
        }
        else
        {
            thumbnail.setSource (new juce::FileInputSource (file));
        }

        delete reader;
    }
}

void WaveformDisplay::setRelativePosition (double position)
{
    if (std::abs(relativePosition - position) > 0.001)
    {
        relativePosition = position;
        repaint();
    }
}

void WaveformDisplay::changeListenerCallback (juce::ChangeBroadcaster*)
{
    // Thumbnail has changed, repaint
    repaint();
}
