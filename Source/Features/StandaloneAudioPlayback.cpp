#include "StandaloneAudioPlayback.h"

#if NARRATE_ENABLE_AUDIO_PLAYBACK

StandaloneAudioPlayback::StandaloneAudioPlayback()
{
    formatManager.registerBasicFormats();
}

StandaloneAudioPlayback::~StandaloneAudioPlayback()
{
    transportSource.setSource(nullptr);
}

bool StandaloneAudioPlayback::loadAudioFile(const juce::File& file)
{
    if (!file.existsAsFile())
        return false;

    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
        return false;

    std::unique_ptr<juce::AudioFormatReaderSource> newSource(
        new juce::AudioFormatReaderSource(reader, true));

    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
    readerSource.reset(newSource.release());
    loadedAudioFile = file;

    return true;
}

bool StandaloneAudioPlayback::hasAudioLoaded() const
{
    return readerSource != nullptr;
}

void StandaloneAudioPlayback::startPlayback()
{
    transportSource.start();
}

void StandaloneAudioPlayback::stopPlayback()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void StandaloneAudioPlayback::pausePlayback()
{
    transportSource.stop();
}

bool StandaloneAudioPlayback::isPlaying() const
{
    return transportSource.isPlaying();
}

double StandaloneAudioPlayback::getPosition() const
{
    return transportSource.getCurrentPosition();
}

void StandaloneAudioPlayback::setPosition(double positionInSeconds)
{
    transportSource.setPosition(positionInSeconds);
}

double StandaloneAudioPlayback::getDuration() const
{
    if (readerSource == nullptr)
        return 0.0;

    return transportSource.getLengthInSeconds();
}

void StandaloneAudioPlayback::getThumbnailData(int channel, double startTime, double endTime,
                                                float* samples, int numSamples)
{
    juce::ignoreUnused(channel, startTime, endTime);

    // This would require maintaining an AudioThumbnail
    // For now, just clear the buffer
    juce::FloatVectorOperations::clear(samples, numSamples);
}

void StandaloneAudioPlayback::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void StandaloneAudioPlayback::releaseResources()
{
    transportSource.releaseResources();
}

void StandaloneAudioPlayback::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
}

#endif // NARRATE_ENABLE_AUDIO_PLAYBACK
