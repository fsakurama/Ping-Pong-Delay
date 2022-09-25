/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#define MAX_DELAY_TIME 2

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PingpongDelayAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PingpongDelayAudioProcessor();
    ~PingpongDelayAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    float lin_interp(float sample_x, float sample_x1, float inPhase);

private:
    
    float mDelaytimeSamples;
    float mDelayTimeSmooth;
    
    juce::AudioParameterFloat* mDryWetParameter;
    juce::AudioParameterFloat* mFeedbackParameter;
    juce::AudioParameterFloat* mDelayTimeParameter;
    
    float mDryWet;
    
    float mFeedbackLeft;
    float mFeedbackRight;
    
    float* mCircularBufferL;
    float* mCircularBufferR;
    
    int mCircularBufferWriteHead;
    int mCircularBufferLength;
    
    float mDelayTimeInSamples; //self explanatory
    float mDelayReadHead; //Notice that these are floats, there is a reason for that which will be clear in a few more steps
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PingpongDelayAudioProcessor)
};
