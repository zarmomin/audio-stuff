//
//  NoiseOverlay.hpp
//  NicosFirstVolumePlugin
//
//  Created by Nicolas Scheidt on 11/12/2022.
//

#ifndef NoiseOverlay_hpp
#define NoiseOverlay_hpp

#include <memory>
#include <JuceHeader.h>

class NoiseOverlay {
public:
    NoiseOverlay(juce::AudioProcessorValueTreeState& treeState, size_t instance_idx);
    void processBlock (juce::AudioBuffer<float>& buffer) ;
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources() {transportSource.releaseResources();}
    ~NoiseOverlay(){transportSource.setSource(nullptr);}
private:
    void loadNoiseFromFile();
    void addParameters();
    std::string assembleParamName(const std::string name);
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioSampleBuffer fileBuffer;
    juce::AudioProcessorValueTreeState& treeState;
    int position;
    int previousNoiseSample;
    const size_t instance_index;
    std::string gainParamName, enabledParamName, duckParamName, fileToPlayParamName;
};
#endif /* NoiseOverlay_hpp */
