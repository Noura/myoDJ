// playWave.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <iostream>
#include <fstream>

using namespace std;

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

WaveFile waveFile;

int main(int argc, char** argv)
{
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
	cout << "Data section is " << waveFile.SubChunk2Size << "bytes." << endl;

	system("pause");
	return 0;
}

