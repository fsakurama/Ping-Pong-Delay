/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PingpongDelayAudioProcessor::PingpongDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet",
                                                                "Dry Wet",
                                                                0.0,
                                                                1.0,
                                                                0.5
                                                                ));
        
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback",
                                                                  "Feedback",
                                                                  0,
                                                                  0.98,
                                                                  0.5));
        
    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat("delaytime",
                                                                   "Delay Time",
                                                                   0.01,
                                                                   MAX_DELAY_TIME,
                                                                   0.5));
    
    mCircularBufferL = nullptr;
    mCircularBufferR = nullptr;
    
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    
    mDelayTimeInSamples = 0;
    mDelayReadHead = 0;

}

PingpongDelayAudioProcessor::~PingpongDelayAudioProcessor()
{
    if (mCircularBufferL != nullptr){
            delete [] mCircularBufferL;
            mCircularBufferL = nullptr;
        }
    if (mCircularBufferR != nullptr){
            delete [] mCircularBufferR;
            mCircularBufferR = nullptr;
        }
}

//==============================================================================
const juce::String PingpongDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PingpongDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PingpongDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PingpongDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PingpongDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PingpongDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PingpongDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PingpongDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PingpongDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void PingpongDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PingpongDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;

    mDelayTimeInSamples = sampleRate * *mDelayTimeParameter;
    
    mDelayReadHead = 0;
    
    if (mCircularBufferL == nullptr) {
            mCircularBufferL = new float[(int) (sampleRate * MAX_DELAY_TIME)];
        }
    
    juce::zeromem(mCircularBufferL, mCircularBufferLength * sizeof(float));
    
    mCircularBufferWriteHead = 0;
        
    mDelayTimeSmooth = *mDelayTimeParameter;
    
    if (mCircularBufferR == nullptr) {
            mCircularBufferR = new float[(int) (sampleRate * MAX_DELAY_TIME)];
        }
    
    juce::zeromem(mCircularBufferL, mCircularBufferLength * sizeof(float));
    
    mCircularBufferWriteHead = 0;
}

void PingpongDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PingpongDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PingpongDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    
    mDelayTimeInSamples = getSampleRate() * *mDelayTimeParameter;
    
    float* leftChannel = buffer.getWritePointer(0); // Get the data from the main buffer as a pointer
    float* rightChannel = buffer.getWritePointer(1);
        
    for (int i = 0; i < buffer.getNumSamples(); i++) {
        
        /*mDelayTimeSmooth = mDelayTimeSmooth - 0.001 * (mDelayTimeSmooth - *mDelayTimeParameter);
        mDelayTimeInSamples = getSampleRate() * mDelayTimeSmooth;
        
        mCircularBufferL[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferR[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
            
        mDelayReadHead =  mCircularBufferWriteHead - mDelayTimeInSamples;
        
        if (mDelayReadHead < 0) {
                mDelayReadHead += mCircularBufferLength;
                }
        
        int readHead_x = (int) mDelayReadHead;
        float readHeadFloat = mDelayReadHead - readHead_x;
                
        int readHead_x1 = readHead_x + 1;
        if (readHead_x1 >= mCircularBufferLength){
            readHead_x1 -= mCircularBufferLength;
        }
        
        float delay_sample_left = lin_interp(mCircularBufferL[readHead_x], mCircularBufferL[readHead_x1], readHeadFloat);//change
        float delay_sample_right = lin_interp(mCircularBufferR[readHead_x], mCircularBufferR[readHead_x1], readHeadFloat);//change
        
        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;
        
        buffer.addSample(0, i, buffer.getSample(0, i) * *mDryWetParameter + delay_sample_left * (1 - *mDryWetParameter));//change
        buffer.addSample(1, i, buffer.getSample(1, i) * *mDryWetParameter + delay_sample_right * (1 - *mDryWetParameter));//change
            
        mCircularBufferWriteHead++; //update the write position
            
        if(mCircularBufferWriteHead >= mCircularBufferLength){
                mCircularBufferWriteHead = 0; // fold back to 0 if our position is bigger than buffer length
                }*/
        
        //PingPong
        if (i % 2 == 0){
            mDelayTimeSmooth = mDelayTimeSmooth - 0.001 * (mDelayTimeSmooth - *mDelayTimeParameter);
            mDelayTimeInSamples = getSampleRate() * mDelayTimeSmooth;
            
            mCircularBufferL[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
            mCircularBufferR[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
                
            mDelayReadHead =  mCircularBufferWriteHead - mDelayTimeInSamples;
            
            if (mDelayReadHead < 0) {
                    mDelayReadHead += mCircularBufferLength;
                    }
            
            int readHead_x = (int) mDelayReadHead;
            float readHeadFloat = mDelayReadHead - readHead_x;
                    
            int readHead_x1 = readHead_x + 1;
            if (readHead_x1 >= mCircularBufferLength){
                readHead_x1 -= mCircularBufferLength;
            }
            
            float delay_sample_left = lin_interp(mCircularBufferR[readHead_x], mCircularBufferR[readHead_x1], readHeadFloat);
            float delay_sample_right = lin_interp(mCircularBufferL[readHead_x], mCircularBufferL[readHead_x1], readHeadFloat);
            
            mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
            mFeedbackRight = delay_sample_right * *mFeedbackParameter;
            
            buffer.addSample(0, i, buffer.getSample(0, i) * *mDryWetParameter + delay_sample_left * (1 - *mDryWetParameter));
            //buffer.addSample(1, i, buffer.getSample(1, i) * *mDryWetParameter + delay_sample_right * (1 - *mDryWetParameter));
                
            mCircularBufferWriteHead++;
                
            if(mCircularBufferWriteHead >= mCircularBufferLength){
                    mCircularBufferWriteHead = 0; // fold back to 0 if our position is bigger than buffer length
                    }
        }
        
        if (i % 2 == 1){
            mDelayTimeSmooth = mDelayTimeSmooth - 0.001 * (mDelayTimeSmooth - *mDelayTimeParameter);
            mDelayTimeInSamples = getSampleRate() * mDelayTimeSmooth;
            
            mCircularBufferL[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
            mCircularBufferR[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
                
            mDelayReadHead =  mCircularBufferWriteHead - mDelayTimeInSamples;
            
            if (mDelayReadHead < 0) {
                    mDelayReadHead += mCircularBufferLength;
                    }
            
            int readHead_x = (int) mDelayReadHead;
            float readHeadFloat = mDelayReadHead - readHead_x;
                    
            int readHead_x1 = readHead_x + 1;
            if (readHead_x1 >= mCircularBufferLength){
                readHead_x1 -= mCircularBufferLength;
            }
            
            float delay_sample_left = lin_interp(mCircularBufferR[readHead_x], mCircularBufferR[readHead_x1], readHeadFloat);
            float delay_sample_right = lin_interp(mCircularBufferL[readHead_x], mCircularBufferL[readHead_x1], readHeadFloat);
            
            mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
            mFeedbackRight = delay_sample_right * *mFeedbackParameter;
            
            //buffer.addSample(0, i, buffer.getSample(0, i) * *mDryWetParameter + delay_sample_left * (1 - *mDryWetParameter));
            buffer.addSample(1, i, buffer.getSample(1, i) * *mDryWetParameter + delay_sample_right * (1 - *mDryWetParameter));
                
            mCircularBufferWriteHead++; //update the write position
                
            if(mCircularBufferWriteHead >= mCircularBufferLength){
                    mCircularBufferWriteHead = 0; // fold back to 0 if our position is bigger than buffer length
                    }
        }
        
        }
    
}

//==============================================================================
bool PingpongDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PingpongDelayAudioProcessor::createEditor()
{
    return new PingpongDelayAudioProcessorEditor (*this);
}

//==============================================================================
void PingpongDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void PingpongDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PingpongDelayAudioProcessor();
}

float PingpongDelayAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}
