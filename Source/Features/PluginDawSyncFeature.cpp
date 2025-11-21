#include "PluginDawSyncFeature.h"

#if NARRATE_ENABLE_DAW_TRANSPORT_SYNC

PluginDawSyncFeature::PluginDawSyncFeature()
    : syncEnabled(false)
{
}

void PluginDawSyncFeature::setSyncEnabled(bool enabled)
{
    syncEnabled = enabled;
}

double PluginDawSyncFeature::getCurrentPosition(juce::AudioPlayHead* playHead)
{
    if (!syncEnabled || playHead == nullptr)
        return 0.0;

    juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo = playHead->getPosition();

    if (posInfo.hasValue())
    {
        auto timeInSeconds = posInfo->getTimeInSeconds();
        if (timeInSeconds.hasValue())
            return *timeInSeconds;
    }

    return 0.0;
}

bool PluginDawSyncFeature::isPlaying(juce::AudioPlayHead* playHead)
{
    if (!syncEnabled || playHead == nullptr)
        return false;

    juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo = playHead->getPosition();

    if (posInfo.hasValue())
    {
        return posInfo->getIsPlaying();
    }

    return false;
}

double PluginDawSyncFeature::getBPM(juce::AudioPlayHead* playHead)
{
    if (playHead == nullptr)
        return 120.0; // Default BPM

    juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo = playHead->getPosition();

    if (posInfo.hasValue())
    {
        auto bpm = posInfo->getBpm();
        if (bpm.hasValue())
            return *bpm;
    }

    return 120.0;
}

int PluginDawSyncFeature::getTimeSignatureNumerator(juce::AudioPlayHead* playHead)
{
    if (playHead == nullptr)
        return 4;

    juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo = playHead->getPosition();

    if (posInfo.hasValue())
    {
        auto timeSig = posInfo->getTimeSignature();
        if (timeSig.hasValue())
            return timeSig->numerator;
    }

    return 4;
}

int PluginDawSyncFeature::getTimeSignatureDenominator(juce::AudioPlayHead* playHead)
{
    if (playHead == nullptr)
        return 4;

    juce::Optional<juce::AudioPlayHead::PositionInfo> posInfo = playHead->getPosition();

    if (posInfo.hasValue())
    {
        auto timeSig = posInfo->getTimeSignature();
        if (timeSig.hasValue())
            return timeSig->denominator;
    }

    return 4;
}

#endif // NARRATE_ENABLE_DAW_TRANSPORT_SYNC
