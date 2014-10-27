// playWave.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <iostream>
#include <fstream>
#include "portaudio.h"

using namespace std;

#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)

#ifndef M_PI
#define M_PI  (3.14159265)
#endif


// Structure to hold the wavefile
struct WaveFile
{
public:

    static const unsigned short NUM_CHARS = 4;

public:

    WaveFile() : Data(nullptr) {}
    ~WaveFile() { delete[] Data; }

    char ChunkID[NUM_CHARS];
    unsigned int ChunkSize;
    char Format[NUM_CHARS];
    char SubChunkID[NUM_CHARS];
    unsigned int SubChunkSize;
    unsigned short AudioFormat;
    unsigned short NumChannels;
    unsigned int SampleRate;
    unsigned int ByteRate;
    unsigned short BlockAlign;
    unsigned short BitsPerSample;
    char SubChunk2ID[NUM_CHARS];
    unsigned int SubChunk2Size;
    unsigned char* Data; 
};


class Sine
{
public:
    PaStream *stream;
    WaveFile waveFile;
    unsigned int sample_count;
    char message[20];

	Sine()
    {
		stream = 0;
		sample_count = 0;

		cout << "Loading wave file" << endl;
		ifstream file("D:\\Programming\\dj_gestures\\myoDJ\\media\\this_is_dj_simon.wav", ios::binary);

		if(file.good())
			cout << "File loaded." << endl;
		else
			cout << "File load FAILED." << endl;

		if (file.good())
		{
			file.read(waveFile.ChunkID, WaveFile::NUM_CHARS);
			file.read(reinterpret_cast<char*>(&waveFile.ChunkSize), sizeof(unsigned int));
			file.read(waveFile.Format, WaveFile::NUM_CHARS);
			file.read(waveFile.SubChunkID, WaveFile::NUM_CHARS);
			file.read(reinterpret_cast<char*>(&waveFile.SubChunkSize), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&waveFile.AudioFormat), sizeof(unsigned short));
			file.read(reinterpret_cast<char*>(&waveFile.NumChannels), sizeof(unsigned short));
			file.read(reinterpret_cast<char*>(&waveFile.SampleRate), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&waveFile.ByteRate), sizeof(unsigned int));
			file.read(reinterpret_cast<char*>(&waveFile.BlockAlign), sizeof(unsigned short));
			file.read(reinterpret_cast<char*>(&waveFile.BitsPerSample), sizeof(unsigned short));
			file.read(waveFile.SubChunk2ID, WaveFile::NUM_CHARS);
			file.read(reinterpret_cast<char*>(&waveFile.SubChunk2Size), sizeof(unsigned int));
			waveFile.Data = new unsigned char[waveFile.SubChunk2Size];
			file.read(reinterpret_cast<char*>(waveFile.Data), sizeof(waveFile.SubChunk2Size));
			file.close();
		}

		// Print size of audio data
	    cout << "Num chan: " << waveFile.NumChannels << "Samp Rate: " << waveFile.SampleRate << endl;
		cout << "Data section is " << waveFile.SubChunk2Size << "bytes." << endl;

		for(int i = 0; i < 1000; i++)
			cout << reinterpret_cast<short*>(waveFile.Data)[i] << endl;

    }

    bool open(PaDeviceIndex index)
    {
        PaStreamParameters outputParameters;

        outputParameters.device = index;
        if (outputParameters.device == paNoDevice) {
            return false;
        }

        outputParameters.channelCount = 2;       /* stereo output */
        outputParameters.sampleFormat = paFloat32; /* 16 bit int output */
        outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;

        PaError err = Pa_OpenStream(
            &stream,
            NULL, /* no input */
            &outputParameters,
            SAMPLE_RATE,
            FRAMES_PER_BUFFER,
            paClipOff,      /* we won't output out of range samples so don't bother clipping them */
            &Sine::paCallback,
            this            /* Using 'this' for userData so we can cast to Sine* in paCallback method */
            );

        if (err != paNoError)
        {
            /* Failed to open stream to device !!! */
            return false;
        }

        err = Pa_SetStreamFinishedCallback( stream, &Sine::paStreamFinished );

        if (err != paNoError)
        {
            Pa_CloseStream( stream );
            stream = 0;

            return false;
        }

        return true;
    }

    bool close()
    {
        if (stream == 0)
            return false;

        PaError err = Pa_CloseStream( stream );
        stream = 0;

        return (err == paNoError);
    }


    bool start()
    {
        if (stream == 0)
            return false;

        PaError err = Pa_StartStream( stream );

        return (err == paNoError);
    }

    bool stop()
    {
        if (stream == 0)
            return false;

        PaError err = Pa_StopStream( stream );

        return (err == paNoError);
    }

private:

    /* The instance callback, where we have access to every method/variable in object of class Sine */
    int paCallbackMethod(const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags)
    {
        float *out = (float*)outputBuffer;
        unsigned long i;

        (void) timeInfo; /* Prevent unused variable warnings. */
        (void) statusFlags;
        (void) inputBuffer;

        for( i=0; i<framesPerBuffer; i++ )
        {
            //*out++ = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); //((short*)waveFile.Data)[sample_count];
            //*out++ = static_cast <float> (rand()) / static_cast <float> (RAND_MAX); //((short*)waveFile.Data)[sample_count];

            *out++ = (float) ( ((short*)waveFile.Data)[sample_count] );
            *out++ = (float) ( ((short*)waveFile.Data)[sample_count] );

			sample_count++;
            if( sample_count >= waveFile.SubChunk2Size/2 ) sample_count = 0;
        }

        return paContinue;

    }

    /* This routine will be called by the PortAudio engine when audio is needed.
    ** It may called at interrupt level on some machines so don't do anything
    ** that could mess up the system like calling malloc() or free().
    */
    static int paCallback( const void *inputBuffer, void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData )
    {
        /* Here we cast userData to Sine* type so we can call the instance method paCallbackMethod, we can do that since 
           we called Pa_OpenStream with 'this' for userData */
        return ((Sine*)userData)->paCallbackMethod(inputBuffer, outputBuffer,
            framesPerBuffer,
            timeInfo,
            statusFlags);
    }


    void paStreamFinishedMethod()
    {
        printf( "Stream Completed: %s\n", message );
    }

    /*
     * This routine is called by portaudio when playback is done.
     */
    static void paStreamFinished(void* userData)
    {
        return ((Sine*)userData)->paStreamFinishedMethod();
    }

};


/*******************************************************************/
int main(void)
{
    PaError err;
    Sine sine;

    printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);
    
    err = Pa_Initialize();
    if( err != paNoError ) goto error;

    if (sine.open(Pa_GetDefaultOutputDevice()))
    {
        if (sine.start())
        {
            printf("Play for %d seconds.\n", NUM_SECONDS );
            Pa_Sleep( NUM_SECONDS * 1000 );

            sine.stop();
        }

        sine.close();
    }

    Pa_Terminate();
    printf("Test finished.\n");
    
    return err;

error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return err;
}

