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

#include "delay.h"

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
    
    // delay effect setup
    //Delay_Init(85.6,0.7,0.7,1);
    //Delay_Init(500.0, 0.9, 0.7, 1);
    //Delay_Init(2500.0, 0.3, 0.3, 0.3);
    Delay_Init(10000, 0.5, 0.2, .3);
    
    // Start the audio stream. PortAudio will then start calling our callback function
    // every time the soundcard needs audio.
    // Note that this is non-blocking by default!
    if (Pa_StartStream(pStream) != paNoError)
    {
        std::cerr << "Failed to start the PortAudio stream." << std::endl;
        return 1;
    }
    
#ifdef _WIN32
    Sleep(200000);
#else
    sleep(200);
#endif
    
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
    float vol = 2.0;

	for(int i = 0; i <  samplesRead; i++)
	{
        static_cast<SAMPLE*>(output)[i] = Delay_task(sampBuf[i]);
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
