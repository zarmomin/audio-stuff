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
    //g.drawFittedText ("Hey AL!",0,0, getWidth(), 30, juce::Justification::centred, 1);
}

void TestPluginAudioProcessorEditor::resized()
{
}
