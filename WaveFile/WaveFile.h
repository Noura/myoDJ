#ifndef WAVE_FILE_H
#define WAVE_FILE_H

#include <iostream>
#include <fstream>

class WaveFile
{
private:

	enum class WavChunks
	{
		RiffHeader = 0x46464952,
		WavRiff = 0x54651475,
		Format = 0x020746d66,
		LabeledText = 0x478747C6,
		Instrumentation = 0x478747C6,
		Sample = 0x6C706D73,
		Fact = 0x47361666,
		Data = 0x61746164,
		Junk = 0x4b4e554a,
	};

	enum class WavFormat
	{
		PulseCodeModulation = 0x01,
		IEEEFloatingPoint = 0x03,
		ALaw = 0x06,
		MuLaw = 0x07,
		IMAADPCM = 0x11,
		YamahaITUG723ADPCM = 0x16,
		GSM610 = 0x31,
		ITUG721ADPCM = 0x40,
		MPEG = 0x50,
		Extensible = 0xFFFE
	};

	// Attributes
	std::ifstream infile;
	unsigned int chunkid;
	bool datachunk;
	unsigned int formatsize;
	unsigned short format;
	unsigned short channels;
	unsigned int samplerate;
	unsigned int bitspersecond;
	unsigned short formatblockalign;
	unsigned short bitdepth;
	unsigned int headerid;
	unsigned int memsize;
	unsigned int riffstyle;
	unsigned int datasize;

	int num_wave_samps;
	int samps_read;

public:

	WaveFile(std::string filename);

	int getChannels();
	int getSampleRate();
	int getNumSamps();

	void getSamples(int nSampsToRead, float* dest);
};

#endif
