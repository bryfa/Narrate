#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Features/StandaloneAudioPlayback.h"

NarrateAudioProcessor::NarrateAudioProcessor()
    : AudioProcessor(BusesProperties()
                      .withInput("Input", juce::AudioChannelSet::stereo(), true)
                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state("NarrateState"),
      audioPlayback(FeatureFactory::createAudioPlayback()),
      exportFeature(FeatureFactory::createExportFeature()),
      dawSync(FeatureFactory::createDawSyncFeature())
{
    // Initialize settings file (for global preferences like theme)
    auto options = getSettingsOptions();
    settings.reset(new juce::PropertiesFile(options));

    // Initialize state with default text
    state.setProperty("editorText", "", nullptr);
}

NarrateAudioProcessor::~NarrateAudioProcessor()
{
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

void NarrateAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String NarrateAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void NarrateAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void NarrateAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Delegate to audio playback feature if available
    if (audioPlayback->isAvailable())
    {
#if NARRATE_ENABLE_AUDIO_PLAYBACK
        auto* standaloneAudio = static_cast<StandaloneAudioPlayback*>(audioPlayback.get());
        standaloneAudio->prepareToPlay(sampleRate, samplesPerBlock);
#endif
    }
}

void NarrateAudioProcessor::releaseResources()
{
    // Delegate to audio playback feature if available
    if (audioPlayback->isAvailable())
    {
#if NARRATE_ENABLE_AUDIO_PLAYBACK
        auto* standaloneAudio = static_cast<StandaloneAudioPlayback*>(audioPlayback.get());
        standaloneAudio->releaseResources();
#endif
    }
}

bool NarrateAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void NarrateAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't have input
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Delegate to audio playback feature if available
    if (audioPlayback->isAvailable())
    {
#if NARRATE_ENABLE_AUDIO_PLAYBACK
        auto* standaloneAudio = static_cast<StandaloneAudioPlayback*>(audioPlayback.get());
        juce::AudioSourceChannelInfo channelInfo(buffer);
        standaloneAudio->getNextAudioBlock(channelInfo);
#endif
    }
}

bool NarrateAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NarrateAudioProcessor::createEditor()
{
    return new NarrateAudioProcessorEditor(*this);
}

void NarrateAudioProcessor::setEditorText(const juce::String& text)
{
    state.setProperty("editorText", text, nullptr);
}

juce::String NarrateAudioProcessor::getEditorText() const
{
    return state.getProperty("editorText").toString();
}

void NarrateAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // Save the state as XML to the memory block
    auto xml = state.createXml();
    if (xml != nullptr)
        copyXmlToBinary(*xml, destData);
}

void NarrateAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Restore state from the memory block
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName(state.getType()))
    {
        state = juce::ValueTree::fromXml(*xml);
    }
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NarrateAudioProcessor();
}
