#include "PluginProcessor.h"
#include "PluginEditor.h"

NarrateAudioProcessor::NarrateAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     )
{
}

NarrateAudioProcessor::~NarrateAudioProcessor()
{
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
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void NarrateAudioProcessor::releaseResources()
{
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

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Your audio processing code goes here
}

bool NarrateAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* NarrateAudioProcessor::createEditor()
{
    return new NarrateAudioProcessorEditor (*this);
}

void NarrateAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void NarrateAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NarrateAudioProcessor();
}
