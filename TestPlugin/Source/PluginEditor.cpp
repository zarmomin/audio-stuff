/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TestPluginAudioProcessorEditor::TestPluginAudioProcessorEditor (TestPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // editor's size to whatever you need it to be.
    setSize (600, 600);
 
    // these define the parameters of our slider object
    volumedBSlider.setRange (-100.0, 6.0, 1.0);
    volumedBSlider.setTextValueSuffix (" dB");
    volumedBSlider.setTextBoxStyle (juce::Slider::TextBoxLeft, false, 160, volumedBSlider.getTextBoxHeight());
    volumedBSlider.setValue(juce::Decibels::gainToDecibels (audioProcessor.level.load(), -100.f), juce::dontSendNotification);
    volumedBSlider.onValueChange = [this] {audioProcessor.level.store(juce::Decibels::decibelsToGain ((float) volumedBSlider.getValue()));};
    volumedBSlider.setSkewFactorFromMidPoint(0.0);
    addAndMakeVisible (&volumedBSlider);
    
    addAndMakeVisible (volumeLabel);
    volumeLabel.setText ("Volume", juce::dontSendNotification);
    volumeLabel.attachToComponent (&volumedBSlider, true);
}

TestPluginAudioProcessorEditor::~TestPluginAudioProcessorEditor()
{
}

//==============================================================================
void TestPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hey AL!",0,0, getWidth(), 30, juce::Justification::centred, 1);
}

void TestPluginAudioProcessorEditor::resized()
{
    auto sliderLeft = 120;
    volumedBSlider.setBounds (sliderLeft, 50, getWidth() - sliderLeft - 10, 50);
}
