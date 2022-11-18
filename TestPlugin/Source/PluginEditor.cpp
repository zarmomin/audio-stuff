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
    
    addAndMakeVisible (levelLabel);
    levelLabel.setTitle("Level");
    levelLabel.attachToComponent (&volumedBSlider, false);
    
    addAndMakeVisible (&openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] { openButtonClicked(); };
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
    auto& ms =audioProcessor.getMeterSource();
    float level0 = ms.getRMSLevel(0);
    float level1 = ms.getRMSLevel(1);
    std::stringstream ss;
    ss << "Levels: ";
    ss << std::setprecision(2) << std::fixed << level0 << " - " << level1 << std::endl;
    ss << "Upper:  " << ms.getMaxLevel(0) << " - " << ms.getMaxLevel(1);
    std::string levels = ss.str();
    levelLabel.setText(levels, juce::dontSendNotification);
}

void TestPluginAudioProcessorEditor::resized()
{
    auto sliderLeft = 120;
    volumedBSlider.setBounds (sliderLeft, 50, getWidth() - sliderLeft - 10, 50);
}

void TestPluginAudioProcessorEditor::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                   juce::File{},
                                                   "*.wav");
    auto chooserFlags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})
        {
            auto* reader = audioProcessor.formatManager.createReaderFor (file);
            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
                audioProcessor.transportSource.setSource (newSource.get(), 0, nullptr, reader->sampleRate);
                audioProcessor.readerSource.reset (newSource.release());
            }
        }
    });
}
