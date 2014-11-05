/*
 * playsong example - Demonstrates how to decode an MP3 file with 
 *                    libaudiodecoder and play it out your soundcard
 *                    using the PortAudio library.
 *
 * libaudiodecoder - Native Portable Audio Decoder Library
 * Latest version available at: http://www.oscillicious.com/libaudiodecoder
 *
 * Copyright (c) 2010-2012 Albert Santoni, Bill Good, RJ Ryan  
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire libaudiodecoder license; however, 
 * the Oscillicious community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included aint with the 
 * license above.
 */

#ifdef _WIN32
  #include <windows.h>
  #define _USE_MATH_DEFINES
#endif
#include <iostream>
#include <math.h>
#ifdef _WIN32
  #include <portaudio.h>                 // PortAudio
  #include <audiodecoder.h> // libaudiodecoder
#else
  #include "portaudio.h"
  #include <audiodecoder/audiodecoder.h> // libaudiodecoder
#endif

#include <myo/myo.hpp>
#include "myo.h"

// All audio will be handled as stereo.
const int NUM_CHANNELS = 2;

// Parameters for flanger effect
#define FL_MAX_DELAY 0.010
#define FL_RATE 0.1
#define SAMP_RATE 44100

// Declaration for audio callback called by PortAudio.
int audioCallback(const void *input, void *output, 
                  unsigned long frameCount,
                  const PaStreamCallbackTimeInfo* timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void* userData);
// Define a circular buffer
struct CircularBuffer {
	SAMPLE data[(int)(6 * FL_MAX_DELAY * SAMP_RATE * NUM_CHANNELS)];
	int writePos;
	int readPos;
	int nElems;
	int len;
	int fl_n;
};
CircularBuffer cbuf;

// Data collector for the Myo bracelet
DataCollector collector;


int main (int argc, char * const argv[]) {

    // Initialize an AudioDecoder object and open demo.mp3
    #ifdef _WIN32
		char* filename = "dancing.m4a";
	#else
		std::string filename = "/Users/noura/myo DJ/playsong/demo.mp3";
	#endif
    AudioDecoder* pAudioDecoder = new AudioDecoder(filename);
    
    if (pAudioDecoder->open() != AUDIODECODER_OK)
    {
        std::cerr << "Failed to open " << filename << std::endl;
        return 1;
    }
    
    // Initialize the PortAudio library.
    if (Pa_Initialize() != paNoError) {
        std::cerr << "Failed to initialize PortAudio." << std::endl;
        return 1;
    };
    
    PaStream* pStream = NULL;
    
	// Create and initialize the circular buffer
	cbuf.len = (int)(6 * FL_MAX_DELAY * SAMP_RATE * NUM_CHANNELS);
	cbuf.writePos = 0;
	cbuf.nElems = 0;
	cbuf.readPos = 0;
	cbuf.fl_n = 0;

    // Open the PortAudio stream (opens the soundcard device).
    if (Pa_OpenDefaultStream(&pStream, 
                  0, // No input channels
                  2, // 2 output channel
                  paFloat32, // Sample format (see PaSampleFormat)           
                  SAMP_RATE, // Sample Rate
                  paFramesPerBufferUnspecified,  // Frames per buffer 
                  &audioCallback,
                  static_cast<void*>(&cbuf)) != paNoError)
    {
        std::cerr << "Failed to open the default PortAudio stream." << std::endl;
        return 1;
    }

    // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
    // publishing your application. The Hub provides access to one or more Myos.
    myo::Hub hub("com.example.flanger_fx");
    std::cout << "Attempting to find a Myo..." << std::endl;
        
    // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
    // immediately.
    myo::Myo* myo = hub.waitForMyo(10000);
        
    // If waitForAnyMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
    if (!myo) {
        std::cout << "Unable to find a Myo!" << std::endl;
		return -1;
    }

    // We've found a Myo.
    std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
    // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
    // Hub::run() to send events to all registered device listeners.
    hub.addListener(&collector);
 
	// A loop that runs until the song finishes. We check if the buffer needs more samples,
	// and if so, we read more from the MP3 file. We try keep at least 600ms of samples in
	// the buffer
	bool songFinished = false;
	bool songStarted = false;
	while(!songFinished)
	{
		int numSampsRead = 1;

		// If the buffer is running low, read in more samples
		if(cbuf.nElems < cbuf.len)
		{
			int sampsToRead = cbuf.len - cbuf.nElems;
			int sampsToRead1 = min(sampsToRead, cbuf.len - cbuf.writePos);

			numSampsRead = pAudioDecoder->read(sampsToRead1, static_cast<SAMPLE*>(&cbuf.data[cbuf.writePos]));
			cbuf.nElems += numSampsRead;
			cbuf.writePos = (cbuf.writePos + numSampsRead) % cbuf.len;
		}

		// Start the song, if we haven't already
		if(!songStarted)
		{
			if (Pa_StartStream(pStream) != paNoError)
			{
				std::cerr << "Failed to start the PortAudio stream." << std::endl;
				return 1;
			}        
			songStarted = true;
		}

		// Check if we have read the entire file
		if(numSampsRead == 0)
			songFinished = true;

		// Run the Myo event loop for a set number of milliseconds.
        hub.run(5);

        // After processing events, we call the print() member function we defined above to print out the values we've
        // obtained from any events that have occurred.
        collector.print();
	}
   
    // Shutdown:
    // First, stop the PortAudio stream (closes the soundcard device).
    if (Pa_StopStream(pStream) != paNoError)
    {
        std::cerr << "Failed to stop the PortAudio stream." << std::endl;
        return 1;
    }        

    // Tell the PortAudio library that we're all done with it.
    if (Pa_Terminate() != paNoError)
    {
        std::cerr << "Failed to terminate PortAudio." << std::endl;
        return 1;
    }
    
    // Close the AudioDecoder object and free it.
    delete pAudioDecoder;
	delete cbuf.data;
    
    return 0;
}

// This is the function that gets called when we need to generate sound! 
// In this example, we're going to decode some audio using libaudiodecoder
// and fill the "output" buffer with that. In other words, we're going to
// decode demo.mp3 and send that audio to the soundcard. Easy!
int audioCallback(const void *input, void *output, 
                  unsigned long frameCount,
                  const PaStreamCallbackTimeInfo* timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void* userData)
{
    
    CircularBuffer* cbuf = static_cast<CircularBuffer*>(userData);

	// If the buffer is empty, don't start playing
	if(cbuf->nElems < FL_MAX_DELAY * SAMP_RATE * NUM_CHANNELS || cbuf->nElems < (int)(frameCount) * NUM_CHANNELS)
	{
		std::cout << "Too few samples in buffer!. Have " << cbuf->nElems << std::endl;
		return paContinue;
	}

	// Do some audio processing on the samples. 
	// The result of the processing is written directly to the output buffer
	for(int i = 0; i < frameCount * NUM_CHANNELS; i++)
	{
		// Calculate the audio fx params from the Myo params
		float fl_mix = min( max(collector.my_yaw / 40.0, 0.0), 1.0);
		float vol = max(collector.my_pitch / 40.0, 0.0);

		// Compute the flanger delay
		// Delay = MAX_DELAY/2 * (1 - cos(2pi * deviation_rate * n)
		float delay = (FL_MAX_DELAY * SAMP_RATE) / 2.0 * (1.0 - cos(2.0*M_PI * FL_RATE / SAMP_RATE * cbuf->fl_n));
		float frac_delay = delay - floor(delay);
		cbuf->fl_n++;
		if(cbuf->fl_n == (int)(SAMP_RATE / FL_RATE))
			cbuf->fl_n = 0;

		// Compute the indices
		int base_idx = (cbuf->readPos + (int)(FL_MAX_DELAY * SAMP_RATE * NUM_CHANNELS)) % cbuf->len;
		int delay_idx1 = (base_idx - (int)(floor(delay)) * NUM_CHANNELS);
		if(delay_idx1 < 0) delay_idx1 += cbuf->len;
		int delay_idx2 = (base_idx - (int)(ceil (delay)) * NUM_CHANNELS);
		if(delay_idx2 < 0) delay_idx2 += cbuf->len;

		// Compute the original and delayed sample, using interpolation
		float this_samp = cbuf->data[base_idx] * (1 - fl_mix);
		float delayed_samp = ( cbuf->data[delay_idx1]*(1-frac_delay) + cbuf->data[delay_idx2]*frac_delay ) * fl_mix;

		// Adjust the volume
		SAMPLE final_samp = (this_samp + delayed_samp) * vol;

		// Store the sample
		(static_cast<SAMPLE*>(output))[i] = final_samp;
		cbuf->readPos = (cbuf->readPos + 1) % cbuf->len;
	}
   
	cbuf->nElems -= frameCount * NUM_CHANNELS;
	return paContinue;
}
