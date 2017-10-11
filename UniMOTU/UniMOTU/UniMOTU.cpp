// UniMOTU.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER (64)
#define TABLE_SIZE (200)

#ifndef M_PI
#define M_PI (3.14159265)
#endif

typedef struct {
	float sineData[TABLE_SIZE];
	int duration;
}
simpleSineData;

typedef struct {
	float data[TABLE_SIZE][24];
	int duration;
}
phonemeData;

PaStream *stream;
int logCode =0;
int playing = 0;

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
	int j = 0;
	int k;
	for (i = 0; i<framesPerBuffer; i++)
	{
		for(k = 0; k < 24; k++)
			*out++ = myData->sineData[j];
		j += 1;
		if (j >= TABLE_SIZE) j -= TABLE_SIZE;
	}
	return paContinue;
}

/*
* This routine is called by portaudio when playback is done.
*/
static void StreamFinished(void* userData)
{
	//TODO
}

void AsyncSimpleSinePlay(void*) {
	playing = 1;
	//Error
	PaError err;

	PaStreamParameters outputParameters;

	//Device information
	const PaDeviceInfo* info;

	//Create sine data
	int i;
	simpleSineData sineData;
	for (i = 0; i<TABLE_SIZE; i++)
	{
		sineData.sineData[i] = (float)0.5*sin(((double)i / (double)TABLE_SIZE) * M_PI * 2.);
	}
	sineData.duration = 2;

	//Initialize
	err = Pa_Initialize();
	if (err != paNoError) {
		logCode = 1;
		Pa_Terminate();
	}

	//Verify that MOTU is the default output device
	info = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
	outputParameters.device = Pa_GetDefaultOutputDevice();
	if (outputParameters.device == paNoDevice || (info->maxOutputChannels != 24 && strstr(info->name, "MOTU"))) {
		logCode = 2;
		Pa_Terminate();
	}

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

/*Plays a phoneme*/
__declspec(dllexport) int play(int phonemeCode) {
	return 0;
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






