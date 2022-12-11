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
    volumedBSlider.setRange (0.0, 10.0, 0.01);
    volumedBSlider.setValue(audioProcessor.level.load(), juce::dontSendNotification);
    volumedBSlider.onValueChange = [this] {audioProcessor.level.store(volumedBSlider.getValue());};
    volumedBSlider.setSkewFactorFromMidPoint(1.0);
    volumedBSlider.setTextBoxIsEditable(false);
    addAndMakeVisible (&volumedBSlider);
    
    auto file = juce::File("/Users/nico/Downloads/crackle_samples/CRACKLE_LOUD.wav");
    std::unique_ptr<juce::AudioFormatReader> reader (audioProcessor.formatManager.createReaderFor (file)); // [2]

    if (reader.get() != nullptr)
    {
        // Todo(nscheidt): figure out the memory limit here.
        audioProcessor.fileBuffer.setSize ((int) reader->numChannels, (int) reader->lengthInSamples);  // [4]
        reader->read (&audioProcessor.fileBuffer,                                                      // [5]
                      0,                                                                //  [5.1]
                      (int) reader->lengthInSamples,                                    //  [5.2]
                      0,                                                                //  [5.3]
                      true,                                                             //  [5.4]
                      true);                                                            //  [5.5]
        audioProcessor.position = 0;
    }
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
    const int border = 20;
    const int width = getWidth() / 3 - border;
    const int height = width;
    volumedBSlider.setBounds(border, border, width, height);
}
