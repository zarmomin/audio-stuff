#include "NoiseOverlay.hpp"

static const juce::StringArray noiseTypes ({"CRACKLE_LOUD", "CRACKLE_NASTY", "CRACKLE", "CRACKLE2"});

NoiseOverlay::NoiseOverlay(juce::AudioProcessorValueTreeState& _treeState, size_t instance_idx) : treeState(_treeState),
noiseBufferPosition(-1),
previousFileIndex(-1),
instanceIndex(instance_idx) {
    // Loop the noise sample.
    noiseInputSource.setLooping(true);
    formatManager.registerBasicFormats();
    addParameters();
}


void NoiseOverlay::loadNoiseFromFile() {
    juce::String filename = dynamic_cast<juce::AudioParameterChoice*>(treeState.getParameter(fileToPlayParamName))->getCurrentChoiceName();
    
    std::stringstream fileSS;
    fileSS <<"/Users/nico/Downloads/crackle_samples/";
    fileSS << filename;
    fileSS << ".wav";
    
    auto file = juce::File(fileSS.str());
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));
    
    if (reader.get() != nullptr)
    {
        noiseFileBuffer.clear();
        // Todo(nscheidt): figure out the memory limit here.
        noiseFileBuffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);
        reader->read (&noiseFileBuffer,
                      /*startSample*/ 0,
                      (int) reader->lengthInSamples,
                      /*readerStartSample*/ 0,
                      /*useLeftChannel*/ true,
                      /*useRightChannel*/ true);
        noiseBufferPosition = 0;
    }
}

void NoiseOverlay::prepareToPlay(double sampleRate , int samplesPerBlock){
    noiseInputSource.prepareToPlay (samplesPerBlock, sampleRate);
}

void NoiseOverlay::processBlock(juce::AudioBuffer<float> & buffer) {
    const bool shouldApplyNoise = treeState.getRawParameterValue(enabledParamName)->load();
    
    const int selectedFileIndex = std::round(treeState.getRawParameterValue(fileToPlayParamName)->load());
    
    if (previousFileIndex != selectedFileIndex) {
        loadNoiseFromFile();
        previousFileIndex = selectedFileIndex;
    }
    
    // Early exit if we don't have a noise file or want to skip it anyways
    if (noiseBufferPosition < 0 || !shouldApplyNoise) {
        return;
    }
    
    const float currentGain = (treeState.getRawParameterValue(gainParamName))->load();
    const bool duck = treeState.getRawParameterValue(duckParamName)->load();
    
    // Since the noise file is most likely not exactly the lenght of the buffer, we need to keep track of the read position in that buffer.
    int noiseIndex = noiseBufferPosition;
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        noiseIndex = noiseBufferPosition;
        float duckFactor = buffer.getMagnitude(channel, 0, buffer.getNumSamples());
        
        if (duck) {
            duckFactor *= -1;
        }
        // If the noise is single-channel and our track dual, we apply the same noise channel to all input channels.
        int noiseChannel = channel;
        if (noiseFileBuffer.getNumChannels() == 1) {
            noiseChannel = 0;
        }
        
        auto* channelData = buffer.getWritePointer (channel);
        auto* noiseData = noiseFileBuffer.getReadPointer(noiseChannel);
        for (int i=0;i<buffer.getNumSamples();++i) {
            ++noiseIndex;
            if (noiseIndex == noiseFileBuffer.getNumSamples()) {
                // We read the entire noise sample, time to loop back to the start
                noiseIndex = 0;
            }
            // Actually apply the noise to the signal here. Include duck/follow and gain
            channelData[i] = channelData[i] + (1 + duckFactor) * currentGain * noiseData[noiseBufferPosition];
        }
    }
    noiseBufferPosition = noiseIndex;
    
}

std::string NoiseOverlay::assembleParamName(const std::string name) {
    std::stringstream paramNameSs;
    paramNameSs << instanceIndex << "_" << name;
    return paramNameSs.str();
}

void NoiseOverlay::addParameters() {
    gainParamName = assembleParamName("drywet");
    enabledParamName = assembleParamName("enabled");
    duckParamName = assembleParamName("duck");
    fileToPlayParamName = assembleParamName("fileToPlay");
    
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { gainParamName, 1 },
                                                                                 "Dry/Wet",
                                                                                 /*min*/0.0f,
                                                                                 /*max*/10.0f,
                                                                                 /*default*/1.0f));
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool> (juce::ParameterID { enabledParamName, 1 },
                                                                                "Apply Noise",
                                                                                /*default*/true));
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterBool> (juce::ParameterID { duckParamName, 1 },
                                                                                "Duck",
                                                                                /*default*/true));
    
    treeState.createAndAddParameter(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {fileToPlayParamName, 1}, "Noise Type", noiseTypes, 0));
    
}
