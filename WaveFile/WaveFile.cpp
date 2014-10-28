#include <iostream>
#include <fstream>
#include "WaveFile.h"

using namespace std;

// Constructor
WaveFile::WaveFile(string filename)
{
	unsigned int extradata;
	unsigned int skipsize;
	samps_read = 0;
	chunkid = 0;
	datachunk = false;

	infile.open(filename, ios::binary);

	if(infile.good())
		cout << "File loaded." << endl;
	else
		cout << "File load FAILED." << endl;

	// Wait until we find the data chunk
	while ( !datachunk )
	{
		infile.read(reinterpret_cast<char *>(&chunkid), sizeof(chunkid));

		switch(chunkid) {

		case WavChunks::Format:
			cout << "Format chunk" << endl;
			infile.read(reinterpret_cast<char *>(&formatsize), sizeof(formatsize));
			infile.read(reinterpret_cast<char *>(&format), sizeof(format));
			infile.read(reinterpret_cast<char *>(&channels), sizeof(channels));
			infile.read(reinterpret_cast<char *>(&samplerate), sizeof(samplerate));
			infile.read(reinterpret_cast<char *>(&bitspersecond), sizeof(bitspersecond));
			infile.read(reinterpret_cast<char *>(&formatblockalign), sizeof(formatblockalign));
			infile.read(reinterpret_cast<char *>(&bitdepth), sizeof(bitdepth));
			if (formatsize == 18) {
				infile.read(reinterpret_cast<char *>(&extradata), sizeof(extradata));
				infile.seekg(extradata, infile.cur);
			}
			break;

		case WavChunks::RiffHeader:
			cout << "Riff header" << endl;
			headerid = chunkid;
			infile.read(reinterpret_cast<char *>(&memsize), sizeof(memsize));
			infile.read(reinterpret_cast<char *>(&riffstyle), sizeof(riffstyle));
			break;

		case WavChunks::Data:
			cout << "Data chunk" << endl;
			datachunk = true;
			infile.read(reinterpret_cast<char *>(&datasize), sizeof(datasize));
			break;

		default:
			infile.read(reinterpret_cast<char *>(&skipsize), sizeof(skipsize));
			infile.seekg(skipsize, infile.cur);
			break;
		}
	}

	num_wave_samps = datasize/2;

	// Print size of audio data
	cout << "Num chan: " << channels << " Samp Rate: " << samplerate << endl;
	cout << "Data section is " << num_wave_samps << " bytes long." << endl;
}

// Methods to get wave file parameters
int WaveFile::getChannels() {
	return channels;
}

int WaveFile::getSampleRate() {
	return samplerate;
}

int WaveFile::getNumSamps() {
	return num_wave_samps;
}

// Actually read the samples from the wavefile
// The audio channels are interleaved.
// Samples are returned as floating point numbers between -1 and +1
void WaveFile::getSamples(int nSampsToRead, float* dest)
{
	// Read in the 16-bit samples
	short* wavedata = new short[nSampsToRead];

	for(int i = 0; (i < nSampsToRead) && (i + samps_read < num_wave_samps); i++)
	{
		infile.read(reinterpret_cast<char *>(&wavedata[i]), sizeof(wavedata[i]));
		samps_read++;
	}

	// Convert them to floats
	for(int i = 0; i < nSampsToRead; i++)
		dest[i] = (float)(wavedata[i] / 32768.0);

	delete wavedata;
}