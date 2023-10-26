#ifndef NoiseOverlay_hpp
#define NoiseOverlay_hpp

#include <memory>
#include <JuceHeader.h>

class NoiseOverlay {
public:
    explicit NoiseOverlay(juce::AudioProcessorValueTreeState& treeState, size_t instance_idx);
    void processBlock (juce::AudioBuffer<float>& buffer) ;
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources() {noiseInputSource.releaseResources();}
    ~NoiseOverlay() { noiseInputSource.setSource(nullptr); }
private:
    void loadNoiseFromFile();
    void addParameters();
    std::string assembleParamName(const std::string name);
private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource noiseInputSource;
    juce::AudioSampleBuffer noiseFileBuffer;
    juce::AudioProcessorValueTreeState& treeState;
    int noiseBufferPosition;
    int previousFileIndex;
    const size_t instanceIndex;
    std::string gainParamName, enabledParamName, duckParamName, fileToPlayParamName;
};
#endif /* NoiseOverlay_hpp */
