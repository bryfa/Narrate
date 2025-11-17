#include "PluginProcessor.h"
#include "PluginEditor.h"

NarrateAudioProcessor::NarrateAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     ),
      state("NarrateState")
{
    // Initialize settings file (for global preferences like theme)
    auto options = getSettingsOptions();
    settings.reset(new juce::PropertiesFile(options));

    // Initialize state with default text
    state.setProperty("editorText", "", nullptr);

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Initialize audio format manager
    formatManager.registerBasicFormats();  // WAV, AIFF, MP3, etc.

    // Initialize mixer and add transport source to it
    mixerSource.addInputSource (&transportSource, false);
#endif
}

NarrateAudioProcessor::~NarrateAudioProcessor()
{
#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Clean up audio playback
    transportSource.setSource (nullptr);
    mixerSource.removeAllInputs();
#endif

    // Save settings when app closes
    if (settings != nullptr)
        settings->saveIfNeeded();
}

juce::PropertiesFile::Options NarrateAudioProcessor::getSettingsOptions()
{
    juce::PropertiesFile::Options options;
    options.applicationName = "Narrate";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    options.folderName = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("MulhacenLabs")
                            .getChildFile("Narrate")
                            .getFullPathName();
    options.storageFormat = juce::PropertiesFile::storeAsXML;
    return options;
}

const juce::String NarrateAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NarrateAudioProcessor::acceptsMidi() const
{
    return true;
}

bool NarrateAudioProcessor::producesMidi() const
{
    return false;
}

bool NarrateAudioProcessor::isMidiEffect() const
{
    return false;
}

double NarrateAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NarrateAudioProcessor::getNumPrograms()
{
    return 1;
}

int NarrateAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NarrateAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String NarrateAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void NarrateAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void NarrateAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Prepare audio playback sources
    transportSource.prepareToPlay (samplesPerBlock, sampleRate);
    mixerSource.prepareToPlay (samplesPerBlock, sampleRate);
#else
    juce::ignoreUnused (sampleRate, samplesPerBlock);
#endif
}

void NarrateAudioProcessor::releaseResources()
{
#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Release audio playback resources
    transportSource.releaseResources();
    mixerSource.releaseResources();
#endif
}

bool NarrateAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void NarrateAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

#if NARRATE_ENABLE_AUDIO_PLAYBACK
    // Standalone-only: Mix audio playback into output buffer
    juce::AudioSourceChannelInfo channelInfo (buffer);
    mixerSource.getNextAudioBlock (channelInfo);
#endif
}

bool NarrateAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NarrateAudioProcessor::createEditor()
{
    return new NarrateAudioProcessorEditor (*this);
}

void NarrateAudioProcessor::setEditorText(const juce::String& text)
{
    state.setProperty("editorText", text, nullptr);
}

juce::String NarrateAudioProcessor::getEditorText() const
{
    return state.getProperty("editorText").toString();
}

void NarrateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the state as XML to the memory block
    auto xml = state.createXml();
    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void NarrateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore state from the memory block
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName(state.getType()))
    {
        state = juce::ValueTree::fromXml(*xml);
    }
}

#if NARRATE_ENABLE_AUDIO_PLAYBACK
// Standalone-only: Audio playback implementation

bool NarrateAudioProcessor::loadAudioFile (const juce::File& file)
{
    // Stop any current playback
    transportSource.stop();
    transportSource.setSource (nullptr);
    readerSource.reset();

    // Try to create a reader for the file
    auto* reader = formatManager.createReaderFor (file);
    if (reader == nullptr)
        return false;

    // Create a new reader source and set it as the transport source
    readerSource.reset (new juce::AudioFormatReaderSource (reader, true));
    transportSource.setSource (readerSource.get(), 0, nullptr, reader->sampleRate);

    // Store the loaded file
    loadedAudioFile = file;

    return true;
}

void NarrateAudioProcessor::startAudioPlayback()
{
    if (hasAudioLoaded())
        transportSource.start();
}

void NarrateAudioProcessor::stopAudioPlayback()
{
    transportSource.stop();
    transportSource.setPosition (0.0);
}

void NarrateAudioProcessor::pauseAudioPlayback()
{
    transportSource.stop();
}

bool NarrateAudioProcessor::isAudioPlaying() const
{
    return transportSource.isPlaying();
}

double NarrateAudioProcessor::getAudioPosition() const
{
    return transportSource.getCurrentPosition();
}

void NarrateAudioProcessor::setAudioPosition (double positionInSeconds)
{
    transportSource.setPosition (positionInSeconds);
}

double NarrateAudioProcessor::getAudioDuration() const
{
    if (readerSource != nullptr && readerSource->getAudioFormatReader() != nullptr)
    {
        auto* reader = readerSource->getAudioFormatReader();
        return reader->lengthInSamples / reader->sampleRate;
    }
    return 0.0;
}

bool NarrateAudioProcessor::hasAudioLoaded() const
{
    return readerSource != nullptr;
}
#endif

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NarrateAudioProcessor();
}
