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
#endif
#include <iostream>
#ifdef _WIN32
  #include <portaudio.h>                 // PortAudio
  #include <audiodecoder.h> // libaudiodecoder
#else
  #include "portaudio.h"
  #include <audiodecoder/audiodecoder.h> // libaudiodecoder
#endif

#include <myo/myo.hpp>

#include "myo.h"

#define MAX(a, b) ( (a) > (b) ? a : b)
#define MIN(a, b) ( (a) < (b) ? a : b)

// All audio will be handled as stereo.
const int NUM_CHANNELS = 2;

// Declaration for audio callback called by PortAudio.
int audioCallback(const void *input, void *output, 
                  unsigned long frameCount,
                  const PaStreamCallbackTimeInfo* timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void* userData);

// Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
DataCollector collector;

int main (int argc, char * const argv[]) {

    // Initialize an AudioDecoder object and open demo.mp3
    #ifdef _WIN32
		char* filename = "demo.mp3";
	#else
		std::string filename = "../../../robyn_dancing.m4a";
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
    
    // Open the PortAudio stream (opens the soundcard device).
    if (Pa_OpenDefaultStream(&pStream, 
                  0, // No input channels
                  2, // 2 output channel
                  paFloat32, // Sample format (see PaSampleFormat)           
                  44100, // Sample Rate
                  paFramesPerBufferUnspecified,  // Frames per buffer 
                  &audioCallback,
                  static_cast<void*>(pAudioDecoder)) != paNoError)
    {
        std::cerr << "Failed to open the default PortAudio stream." << std::endl;
        return 1;
    }
    
    // Start the audio stream. PortAudio will then start calling our callback function
    // every time the soundcard needs audio.
    // Note that this is non-blocking by default!
    if (Pa_StartStream(pStream) != paNoError)
    {
        std::cerr << "Failed to start the PortAudio stream." << std::endl;
        return 1;
    }
    
    // START OF MYO CODE //////////////////////////////////////////////////////////
    std::cout << "hey this is main" << std::endl;
    // We catch any exceptions that might occur below -- see the catch statement for more details.
    try {
        
        // First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
        // publishing your application. The Hub provides access to one or more Myos.
        myo::Hub hub("com.example.hello-myo");
        
        std::cout << "Attempting to find a Myo..." << std::endl;
        
        // Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
        // immediately.
        // waitForAnyMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
        // if that fails, the function will return a null pointer.
        myo::Myo* myo = hub.waitForMyo(10000);
        
        // If waitForAnyMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
        if (!myo) {
            throw std::runtime_error("Unable to find a Myo!");
        }
        
        // We've found a Myo.
        std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
        
        // Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
        // Hub::run() to send events to all registered device listeners.
        hub.addListener(&collector);
        
        // Finally we enter our main loop.
        while (1) {
            // In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
            // In this case, we wish to update our display 20 times a second, so we run for 1000/20 milliseconds.
            hub.run(1000/20);
            // After processing events, we call the print() member function we defined above to print out the values we've
            // obtained from any events that have occurred.
            collector.print();
        }
        
        // If a standard exception occurred, we print out its message and exit.
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Press enter to continue.";
        std::cin.ignore();
        return 1;
    } // END OF MYO CODE //////////////////////////////////////////
    
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
    
    AudioDecoder* pAudioDecoder = static_cast<AudioDecoder*>(userData);
    
	// Temporary buffer to store samples
	SAMPLE* sampBuf = new SAMPLE[frameCount * NUM_CHANNELS];

    // Play it safe when debugging and coding, protect your ears by clearing
    // the output buffer.
    memset(sampBuf, 0, frameCount * NUM_CHANNELS * sizeof(float));
    
    // Decode the number of samples that PortAudio said it needs and
	// write it to the temporary buffer
    int samplesRead = pAudioDecoder->read(frameCount * NUM_CHANNELS, static_cast<SAMPLE*>(sampBuf));

	// Do some audio processing on the samples. In this case, just adjust
	// the volume.
	// The result of the processing is written directly to the output buffer
    float vol = MAX(collector.my_pitch / 50.0, 0.0);
    float distort = MAX(collector.my_yaw / 5.0, 0.0);
    float delay_vol = MAX(collector.my_roll / 20.0, 0.0);

	for(int i = 0; i <  samplesRead; i++)
	{
        if (i < 100) {
            static_cast<SAMPLE*>(output)[i] = sampBuf[i] * vol;
        } else {
            static_cast<SAMPLE*>(output)[i] = ((1.0 - delay_vol) * sampBuf[i] + delay_vol * sampBuf[i-100]) * vol;
        }
	}

    // IMPORTANT:
    // Note that in a real application, you will probably want to call read()
    // in a separate thread because it is NOT realtime safe. That means it does
    // not run in constant time because it might be allocating memory, calling
    // system functions, or doing other things that can take an 
    // unpredictably long amount of time.
    //
    // The danger is that if read() takes too long, then audioCallback() might not
    // finish quickly enough, and there will be a "dropout" or pop in the audio
    // that comes out your speakers. 
    //
    // If you want to guard against dropouts, you should either decode the entire file into
    // memory or decode it on-the-fly in a separate thread. To implement the latter, you would:
    //   - Set up a secondary thread and read() a few seconds of audio into a ringbuffer
    //   - When the callback runs, you want to fetch audio from that ringbuffer.
    //   - Have the secondary thread keep read()ing and ensuring the ringbuffer is full.
    //
    // Ross Bencina has a great introduction to realtime programming and goes into
    // more detail here:
    // http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing
    
	delete sampBuf;
    return paContinue;
}
