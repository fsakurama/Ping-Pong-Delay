/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PingpongDelayAudioProcessorEditor::PingpongDelayAudioProcessorEditor (PingpongDelayAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    mDefaultButton.addListener(this);
    
    setSize (200, 400);
    
    auto& params = processor.getParameters();
        
    juce::AudioParameterFloat* mDryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    
    mDryWetSlider.setBounds(100, 0, 100, 100);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setRange(mDryWetParameter->range.start, mDryWetParameter->range.end);
    mDryWetSlider.setValue(*mDryWetParameter);
    addAndMakeVisible(mDryWetSlider);
    
    mDryWetLabel.setText("DryWet", juce::dontSendNotification);
    mDryWetLabel.attachToComponent(&mDryWetSlider, true);
    addAndMakeVisible(mDryWetLabel);
    mDryWetLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    mDryWetLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    mDryWetLabel.setJustificationType(juce::Justification::centred);
    
    mDryWetSlider.onValueChange = [this, mDryWetParameter]{
            *mDryWetParameter = mDryWetSlider.getValue();
        };
        
    mDryWetSlider.onDragStart = [mDryWetParameter]{
            mDryWetParameter->beginChangeGesture();
        };
        
    mDryWetSlider.onDragEnd = [mDryWetParameter]{
            mDryWetParameter->endChangeGesture();
        };
    
    juce::AudioParameterFloat* mFeedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);
    
    mFeedbackSlider.setBounds(100, 200, 100, 100);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(mFeedbackParameter->range.start, mFeedbackParameter->range.end);
    mFeedbackSlider.setValue(*mFeedbackParameter);
    addAndMakeVisible(mFeedbackSlider);
    
    mFeedbackLabel.setText("Feedback", juce::dontSendNotification);
    mFeedbackLabel.attachToComponent(&mFeedbackSlider, true);
    addAndMakeVisible(mFeedbackLabel);
    mFeedbackLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    mFeedbackLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    mFeedbackLabel.setJustificationType(juce::Justification::centred);
    
    mFeedbackSlider.onValueChange = [this, mFeedbackParameter]{
            *mFeedbackParameter = mFeedbackSlider.getValue();
        };
        
    mFeedbackSlider.onDragStart = [mFeedbackParameter]{
        mFeedbackParameter->beginChangeGesture();
        };
        
    mFeedbackSlider.onDragEnd = [mFeedbackParameter]{
        mFeedbackParameter->endChangeGesture();
        };
    
    juce::AudioParameterFloat* mDelayTimeParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);
    
    mDelayTimeSlider.setBounds(100, 100, 100, 100);
    mDelayTimeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mDelayTimeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDelayTimeSlider.setRange(mDelayTimeParameter->range.start, mDelayTimeParameter->range.end);
    mDelayTimeSlider.setValue(*mDelayTimeParameter);
    addAndMakeVisible(mDelayTimeSlider);
    
    mDelayTimeLabel.setText("DelayTime", juce::dontSendNotification);
    mDelayTimeLabel.attachToComponent(&mDelayTimeSlider, true);
    addAndMakeVisible(mDelayTimeLabel);
    mDelayTimeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    mDelayTimeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
    mDelayTimeLabel.setJustificationType(juce::Justification::centred);
    
    mDelayTimeSlider.onValueChange = [this, mDelayTimeParameter]{
            *mDelayTimeParameter = mDelayTimeSlider.getValue();
        };
        
    mDelayTimeSlider.onDragStart = [mDelayTimeParameter]{
        mDelayTimeParameter->beginChangeGesture();
        };
        
    mDelayTimeSlider.onDragEnd = [mDelayTimeParameter]{
        mDelayTimeParameter->endChangeGesture();
        };
    
    mDefaultButton.setBounds(0, 300, 200, 80);
    mDefaultButton.setButtonText("Default Value");
    addAndMakeVisible(mDefaultButton);
    
    addAndMakeVisible(mDefaultButtonLabel);
    mDefaultButtonLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
    mDefaultButtonLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    mDefaultButtonLabel.setJustificationType(juce::Justification::centred);
}

PingpongDelayAudioProcessorEditor::~PingpongDelayAudioProcessorEditor()
{
}

//==============================================================================
void PingpongDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    /*g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);*/
}

void PingpongDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

/*void buttonClicked (juce::Button* button)
{
    if (button == *mOnOffButton)
}*/

void PingpongDelayAudioProcessorEditor::buttonClicked(juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == &mDefaultButton)
    {
        mDryWetSlider.setValue(0.5);
        mFeedbackSlider.setValue(0.5);
        mDelayTimeSlider.setValue(0.5);
    }
}
