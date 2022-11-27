/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ff_meters.h"

//==============================================================================
/**
*/
class TestPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    TestPluginAudioProcessorEditor (TestPluginAudioProcessor&);
    ~TestPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TestPluginAudioProcessor& audioProcessor;
    juce::Slider volumedBSlider;
    juce::Label volumeLabel;
    juce::Label levelLabel;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestPluginAudioProcessorEditor)
};
