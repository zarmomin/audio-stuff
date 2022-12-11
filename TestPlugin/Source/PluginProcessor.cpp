/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <memory>

//==============================================================================
TestPluginAudioProcessor::TestPluginAudioProcessor()
: AudioProcessor (BusesProperties()
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                  ), position(-1), treeState (*this, nullptr, "Parameters" ,createParameters()), previousNoiseSample(-1)
{
    transportSource.setLooping(true);
    formatManager.registerBasicFormats();
}

void TestPluginAudioProcessor::loadNoiseFromFile() {
    
    juce::String filename = dynamic_cast<juce::AudioParameterChoice*>(treeState.getParameter("noiseType"))->getCurrentChoiceName();
    DBG("loading..");
    DBG(filename);
    
    std::stringstream file_ss;
    file_ss <<"/Users/nico/Downloads/crackle_samples/";
    file_ss << filename;
    file_ss << ".wav";
    
    auto file = juce::File(file_ss.str());
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file)); // [2]
    
    if (reader.get() != nullptr)
    {
        std::lock_guard<std::mutex> input_file_mutex(input_file_mutex_);
        fileBuffer.clear();
        // Todo(nscheidt): figure out the memory limit here.
        fileBuffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);  // [4]
        reader->read (&fileBuffer,                                                      // [5]
                      0,                                                                //  [5.1]
                      (int) reader->lengthInSamples,                                    //  [5.2]
                      0,                                                                //  [5.3]
                      true,                                                             //  [5.4]
                      true);                                                            //  [5.5]
        position = 0;
    }
}

TestPluginAudioProcessor::~TestPluginAudioProcessor()
{
    transportSource.setSource(nullptr);
}

//==============================================================================
const juce::String TestPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TestPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TestPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TestPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TestPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TestPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int TestPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TestPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TestPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void TestPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TestPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay (samplesPerBlock, sampleRate);
}

void TestPluginAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TestPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void TestPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    const bool shouldApplyNoise = treeState.getRawParameterValue("applyNoise")->load();
    const bool duck = treeState.getRawParameterValue("duck")->load();
    int currentChoice = std::round(treeState.getRawParameterValue("noiseType")->load());

    if (previousNoiseSample != currentChoice) {
        loadNoiseFromFile();
        previousNoiseSample = currentChoice;
    }
    // Apply noise per channel
    if (position < 0 || !shouldApplyNoise) {
        return;
    }
    
    std::lock_guard<std::mutex> inputFileMutex(input_file_mutex_);
    int noise_idx = position;
    
    const float currentGain = (treeState.getRawParameterValue("gain"))->load();
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        float duckFactor = buffer.getMagnitude(channel, 0, buffer.getNumSamples());
        if (duck) duckFactor *= -1;
        int noiseChannel = channel;
        if (fileBuffer.getNumChannels() == 1)
            noiseChannel = 0;
        
        auto* channelData = buffer.getWritePointer (channel);
        auto* noiseData = fileBuffer.getReadPointer(noiseChannel);
        for (int i=0;i<buffer.getNumSamples();++i) {
            ++noise_idx;
            if (noise_idx == fileBuffer.getNumSamples()) noise_idx = 0;
            channelData[i] = channelData[i] + (1 + duckFactor) * currentGain * noiseData[position];
        }
    }
    position = noise_idx;
}

//==============================================================================
bool TestPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TestPluginAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void TestPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TestPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(treeState.state.getType()))
        {
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
    
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TestPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout TestPluginAudioProcessor::createParameters() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "gain", 1 },
                                                                  "Dry/Wet",
                                                                  0.0f,
                                                                  10.0f,
                                                                  1.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "applyNoise", 1 },
                                                                 "Apply Noise",
                                                                 true));
    params.push_back(std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "duck", 1 },
                                                                 "Duck",
                                                                 true));
    juce::StringArray choices ({"CRACKLE_LOUD", "CRACKLE_NASTY", "CRACKLE", "CRACKLE2"});
    
    params.push_back(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"noiseType", 1}, "Noise Type", choices, 0));
    return {params.begin(), params.end()};
}
