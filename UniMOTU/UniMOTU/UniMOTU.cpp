// UniMOTU.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER (64)
#define TABLE_SIZE (21168)
#define SMALL_TABLE_SIZE (200)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

typedef struct {
	float sineData[SMALL_TABLE_SIZE];
	int duration;
	int j;
}
simpleSineData;

typedef struct {
	float data[TABLE_SIZE][24];
	int duration;
}
staticPhonemeData;

PaStream *stream;
int logCode =0;
int playing = 0;
int phoneme_index_table = 0;
int dynamic_table_size = 0;
int phoneme_index = -1;
const char* general_message;

/*Search for the 24 channel motu output*/
PaDeviceIndex findMOTU() {
	const PaDeviceInfo* info;
	int numDevices = Pa_GetDeviceCount();
	for (int i = 0; i < numDevices; i++) {
		info = Pa_GetDeviceInfo(i);
		if (info->maxOutputChannels == 24 && strstr(info->name, "MOTU"))
			return i;
	}
	return paNoDevice;
}

std::string escapeStr(std::string str) {
	std::string result = "";
	int pos = 0;
	for (int i = 0; i < str.length(); ++i) {
		if (str[i] == '\\') {
			std::string sub = str.substr(pos, i);
			result = result + sub + '\\';
			pos = i+1;
		}
	}
	return str;
}

/*Read a phoneme CSV file
TODO: Testing just for the phoneme OY
*/
static void readOYCSVPhoneme(staticPhonemeData* phoneme) {

	std::vector<std::string> vector;
	std::string line;
	std::ifstream fileStream;
	std::vector<wchar_t> pathBuf;
	DWORD copied = 0;
	do {
		pathBuf.resize(pathBuf.size() + MAX_PATH);
		copied = GetModuleFileName(0, &pathBuf.at(0), pathBuf.size());
	} while (copied >= pathBuf.size());

	pathBuf.resize(copied);
	std::string path(pathBuf.begin(), pathBuf.end());
	std::string::size_type pos = std::string(path).find_last_of("\\/");
	std::string dir = (std::string(path).substr(0, pos));

	WCHAR   DllPath[MAX_PATH] = { 0 };
	GetModuleFileNameW((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));
	std::wstring out = std::wstring(DllPath);
	//fileStream.open(dir + "\\OY.csv");
	fileStream.open( out + L"\\OY.csv");
	//fileStream.open("C:\\Users\\Juan Sebastian\\Documents\\Unity Projects\\MOTUTests\\Assets\\Plugins\\OY.csv");
	
	while (fileStream.good()) {
		std::getline(fileStream, line);
		vector.push_back(line);
	}
	fileStream.close();

	int j;
	size_t rows = vector.size();

	for (int i = 0; i < rows; i++) {
		std::string row = vector[i];
		size_t pos = row.find(",");
		std::string value;
		j = 0;
		while (pos != std::string::npos) {
			value = row.substr(0, pos);
			row.erase(0, pos + 1);
			phoneme->data[i][j] = atof(value.c_str());
			j++;
			pos = row.find(",");

		}
		phoneme->data[i][j] = atof(row.c_str());
		j = 0;
	}
	phoneme->duration = 480;

}

/*PortAudio callback method for a simple sine test*/
static int simpleSineTestCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData) {

	simpleSineData *myData = (simpleSineData*)userData;
	float *out = (float*)outputBuffer;
	unsigned long i;

	(void)timeInfo; /* Prevent unused variable warnings. */
	(void)statusFlags;
	(void)inputBuffer;
	int k;
	for (i = 0; i<framesPerBuffer; i++)
	{
		for(k = 0; k < 24; k++)
			*out++ = myData->sineData[myData->j];
		myData->j += 1;
		if (myData->j >= SMALL_TABLE_SIZE) myData->j -= SMALL_TABLE_SIZE;
	}
	return paContinue;
}

/*PortAudio callback method for a phoneme play*/
static int phonemePlayCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData) {

	Phoneme *myData = (Phoneme*)userData;
	float *out = (float*)outputBuffer;
	unsigned long i;

	(void)timeInfo; /* Prevent unused variable warnings. */
	(void)statusFlags;
	(void)inputBuffer;

	int k;
	for (i = 0; i<framesPerBuffer; i++)
	{
		for (k = 0; k < 24; k++)
			*out++ = myData->valueAt(phoneme_index_table,k);
		phoneme_index_table += 1;
		if (phoneme_index_table >= dynamic_table_size) {
			return paComplete;
		}
	}
	return paContinue;
}

/*
* This routine is called by portaudio when playback is done.
*/
static void StreamFinished(void* userData)
{
	playing = 0;
	logCode = 0;
	phoneme_index_table = 0;
	dynamic_table_size = 0;
	phoneme_index = -1;

}

void AsyncSimpleSinePlay(void*) {
	playing = 1;
	logCode = 0;
	//Error
	PaError err;

	PaStreamParameters outputParameters;

	//Device information
	const PaDeviceInfo* info;

	//Create sine data
	int i;
	simpleSineData sineData;
	for (i = 0; i<SMALL_TABLE_SIZE; i++)
	{
		sineData.sineData[i] = (float)(0.1075*sin(((double)i / (double)SMALL_TABLE_SIZE) * M_PI * 2.));
	}
	sineData.duration = 2;
	sineData.j = 0;

	//Initialize
	err = Pa_Initialize();
	if (err != paNoError) {
		logCode = 1;
		Pa_Terminate();
		return;
	}

	//Find MOTU
	PaDeviceIndex device = findMOTU();
	if (device == paNoDevice) {
		logCode = 2;
		Pa_Terminate();
		return;
	}
	info = Pa_GetDeviceInfo(device);
	outputParameters.device = device;
	outputParameters.channelCount = 24;       /* stereo output */
	outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
	outputParameters.suggestedLatency = info->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;

	
	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		simpleSineTestCallback,
		&sineData);

	if (err != paNoError) {
		logCode = 3;
		Pa_Terminate();
		return;
	}

	err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
	if (err != paNoError) {
		logCode = 4;
		Pa_Terminate();
		return;
	}

	err = Pa_StartStream(stream);
	if (err != paNoError) {
		logCode = 5;
		Pa_Terminate();
		return;
	}

	Pa_Sleep(sineData.duration * 1000);

	err = Pa_StopStream(stream);
	if (err != paNoError) {
		logCode = 6;
		Pa_Terminate();
		return;
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError) {
		logCode = 7;
		Pa_Terminate();
		return;
	}

	Pa_Terminate();
	logCode = 0;
	playing = 0;
	_endthread();
}

void AsyncPlayPhoneme(void*) {
	playing = 1;
	logCode = 0;
	phoneme_index_table = 0;
	dynamic_table_size = 0;

	Phoneme phoneme(phoneme_index);
	dynamic_table_size = phoneme.getNumberOfRows();
	
	//Error
	PaError err;

	PaStreamParameters outputParameters;

	//Device information
	const PaDeviceInfo* info;


	//Initialize
	err = Pa_Initialize();
	if (err != paNoError) {
		logCode = 1;
		Pa_Terminate();
		return;
	}

	//Find MOTU
	PaDeviceIndex device = findMOTU();
	if (device == paNoDevice) {
		logCode = 2;
		Pa_Terminate();
		return;
	}
	info = Pa_GetDeviceInfo(device);
	outputParameters.device = device;
	outputParameters.channelCount = 24;       /* stereo output */
	outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
	outputParameters.suggestedLatency = info->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;


	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		phonemePlayCallback,
		&phoneme);

	if (err != paNoError) {
		logCode = 3;
		Pa_Terminate();
		return;
	}

	err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
	if (err != paNoError) {
		logCode = 4;
		Pa_Terminate();
		return;
	}

	err = Pa_StartStream(stream);
	if (err != paNoError) {
		logCode = 5;
		Pa_Terminate();
		return;
	}

	while (Pa_IsStreamActive(stream));

	err = Pa_StopStream(stream);
	if (err != paNoError) {
		logCode = 6;
		Pa_Terminate();
		return;
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError) {
		logCode = 7;
		Pa_Terminate();
		return;
	}

	Pa_Terminate();
	logCode = 0;
	playing = 0;
	phoneme_index_table = 0;
	dynamic_table_size = 0;
	phoneme_index = -1;
	_endthread();
}

/*Plays a phoneme*/
__declspec(dllexport) int play(int phonemeCode) {
	
	if (playing == 0) {
		phoneme_index = phonemeCode;
		_beginthread(AsyncPlayPhoneme, 0, NULL);
		return 0;
	}
	else return -1;
}

/*Play a simple sine wave*/
__declspec(dllexport) int testPlay() {
	if (playing == 0) {
		_beginthread(AsyncSimpleSinePlay, 0, NULL);
		return 0;
	}
	else return -1;
		
}

/*Return the log code*/
extern "C" __declspec(dllexport) int getLogCode() {
	return logCode;
}

/*Return the playing flag*/
__declspec(dllexport) int isPlaying() {
	return playing;
}

/*reset the general message buffer*/
void resetMessageBuffer() {
	general_message = "\0";
}

__declspec(dllexport) void setDllPathAsMessage() {

	std::string basePath = "";
	std::vector<wchar_t> pathBuf;
	DWORD copied = 0;
	do {
		pathBuf.resize(pathBuf.size() + MAX_PATH);
		copied = GetModuleFileName(0, &pathBuf.at(0), pathBuf.size());
	} while (copied >= pathBuf.size());

	pathBuf.resize(copied);
	std::string path(pathBuf.begin(), pathBuf.end());
	std::string::size_type pos = std::string(path).find_last_of("\\/");
	basePath = std::string(path).substr(0, pos) + "\\";

	resetMessageBuffer();
	general_message = basePath.c_str();
}

__declspec(dllexport) char getMsg() {
	return general_message++[0];
}







