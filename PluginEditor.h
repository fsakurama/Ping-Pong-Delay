/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PingpongDelayAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    PingpongDelayAudioProcessorEditor (PingpongDelayAudioProcessor&);
    ~PingpongDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;
    //void setGate(bool gate);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PingpongDelayAudioProcessor& audioProcessor;
    
    juce::Slider mDryWetSlider;
    juce::Label mDryWetLabel;
    juce::Slider mFeedbackSlider;
    juce::Label mFeedbackLabel;
    juce::Slider mDelayTimeSlider;
    juce::Label mDelayTimeLabel;
    
    juce::TextButton mDefaultButton;
    juce::Label mDefaultButtonLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PingpongDelayAudioProcessorEditor)
};
