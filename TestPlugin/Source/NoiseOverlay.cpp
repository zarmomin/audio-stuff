#include "NoiseOverlay.hpp"

NoiseOverlay::NoiseOverlay(juce::AudioProcessorValueTreeState& _treeState, size_t instance_idx) : treeState(_treeState),
position(-1),
previousNoiseSample(-1),
instance_index(instance_idx) {
    transportSource.setLooping(true);
    formatManager.registerBasicFormats();
    addParameters();
}


void NoiseOverlay::loadNoiseFromFile() {
    juce::String filename = dynamic_cast<juce::AudioParameterChoice*>(treeState.getParameter(fileToPlayParamName))->getCurrentChoiceName();
    
    std::stringstream file_ss;
    file_ss <<"/Users/nico/Downloads/crackle_samples/";
    file_ss << filename;
    file_ss << ".wav";
    
    auto file = juce::File(file_ss.str());
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    
    if (reader.get() != nullptr)
    {
        fileBuffer.clear();
        // Todo(nscheidt): figure out the memory limit here.
        fileBuffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read (&fileBuffer,
                      0,
                      (int) reader->lengthInSamples,
                      0,
                      true,
                      true);
        position = 0;
    }
}

void NoiseOverlay::prepareToPlay(double sampleRate , int samplesPerBlock){
    transportSource.prepareToPlay (samplesPerBlock, sampleRate);
}

void NoiseOverlay::processBlock(juce::AudioBuffer<float> & buffer) {
    const bool shouldApplyNoise = treeState.getRawParameterValue(enabledParamName)->load();
    const bool duck = treeState.getRawParameterValue(duckParamName)->load();
    int currentChoice = std::round(treeState.getRawParameterValue(fileToPlayParamName)->load());
    
    if (previousNoiseSample != currentChoice) {
        loadNoiseFromFile();
        previousNoiseSample = currentChoice;
    }
    // Apply noise per channel
    if (position < 0 || !shouldApplyNoise) {
        return;
    }
    
    int noise_idx = position;
    
    const float currentGain = (treeState.getRawParameterValue(gainParamName))->load();
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
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

std::string NoiseOverlay::assembleParamName(const std::string name) {
    std::stringstream param_name_ss;
    param_name_ss << instance_index << "_" << name;
    return param_name_ss.str();
}

void NoiseOverlay::addParameters() {
    gainParamName = assembleParamName("drywet");
    enabledParamName = assembleParamName("enabled");
    duckParamName = assembleParamName("duck");
    fileToPlayParamName = assembleParamName("fileToPlay");
    
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { gainParamName, 1 },
                                                                                 "Dry/Wet",
                                                                                 0.0f,
                                                                                 10.0f,
                                                                                 1.0f));
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool> (juce::ParameterID { enabledParamName, 1 },
                                                                                "Apply Noise",
                                                                                true));
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool> (juce::ParameterID { duckParamName, 1 },
                                                                                "Duck",
                                                                                true));
    juce::StringArray choices ({"CRACKLE_LOUD", "CRACKLE_NASTY", "CRACKLE", "CRACKLE2"});
    
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {fileToPlayParamName, 1}, "Noise Type", choices, 0));
    
}
