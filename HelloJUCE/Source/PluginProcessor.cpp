/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a Juce application.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
HelloJuceAudioProcessor::HelloJuceAudioProcessor()
{
}

HelloJuceAudioProcessor::~HelloJuceAudioProcessor()
{
}

//==============================================================================
const String HelloJuceAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

int HelloJuceAudioProcessor::getNumParameters()
{
    return 0;
}

float HelloJuceAudioProcessor::getParameter (int index)
{
    return 0.0f;
}

void HelloJuceAudioProcessor::setParameter (int index, float newValue)
{
}

const String HelloJuceAudioProcessor::getParameterName (int index)
{
    return String::empty;
}

const String HelloJuceAudioProcessor::getParameterText (int index)
{
    return String::empty;
}

const String HelloJuceAudioProcessor::getInputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

const String HelloJuceAudioProcessor::getOutputChannelName (int channelIndex) const
{
    return String (channelIndex + 1);
}

bool HelloJuceAudioProcessor::isInputChannelStereoPair (int index) const
{
    return true;
}

bool HelloJuceAudioProcessor::isOutputChannelStereoPair (int index) const
{
    return true;
}

bool HelloJuceAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HelloJuceAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HelloJuceAudioProcessor::silenceInProducesSilenceOut() const
{
    return false;
}

double HelloJuceAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HelloJuceAudioProcessor::getNumPrograms()
{
    return 0;
}

int HelloJuceAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HelloJuceAudioProcessor::setCurrentProgram (int index)
{
}

const String HelloJuceAudioProcessor::getProgramName (int index)
{
    return String::empty;
}

void HelloJuceAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void HelloJuceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void HelloJuceAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void HelloJuceAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        float* channelData = buffer.getSampleData (channel);

        // ..do something to the data...
    }

    // In case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
}

//==============================================================================
bool HelloJuceAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* HelloJuceAudioProcessor::createEditor()
{
    return new HelloJuceAudioProcessorEditor (this);
}

//==============================================================================
void HelloJuceAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HelloJuceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HelloJuceAudioProcessor();
}
