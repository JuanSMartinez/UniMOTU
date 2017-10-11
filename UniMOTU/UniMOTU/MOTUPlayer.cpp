
#include "MOTUPlayer.h"
#include <math.h>

float MOTUPlayer::duration;
PaStream* MOTUPlayer::stream;
int MOTUPlayer::streamFinishedFlag;
char* MOTUPlayer::logText;

/*Stream callback*/
static int PortAudioCallback(const void *inputBuffer,
	void *outputBuffer,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData) {

	return ((MOTUPlayer*)userData)->PlayerCallback(inputBuffer, outputBuffer,
		frameCount, timeInfo, statusFlags);
}

/*Stream finished callback*/
static void StreamFinished(void* userData)
{

}

/*
Creates a new MOTUPlayer with a sine wave of 60 Hz, 2 seconds and 24 channels as default data
*/
MOTUPlayer::MOTUPlayer() {
	MOTUPlayer::streamFinishedFlag = 0;
	MOTUPlayer::duration = 2;
	MOTUPlayer::logText = "No error";
	tableData.dataRows = duration*1000;
	tableData.dataCols = 24;
	int i, j;
	for (i = 0; i < tableData.dataRows; i++)
		for (j = 0; j < tableData.dataCols; j++)
			tableData.matrixToPlay[i][j] = (float)sin((double)(2.0*M_PI * 60 * i / 1000.0));
}

/*
Initializes the output parameters
*/
int MOTUPlayer::Initialize() {
	PaError err;

	err = Pa_Initialize();
	if (err != paNoError) {
		logText = (char*)Pa_GetErrorText(err);
		return -1;
	}

	//MOTU has to be selected as an output audio device
	outputParameters.device = Pa_GetDefaultOutputDevice();
	outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = NULL;
	if (outputParameters.channelCount != 24) {
		logText = "MOTU is not the default output device";
		return -1;
	}

	return 0;
}

/*Object callback*/
int  MOTUPlayer::PlayerCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags) {
	
	float *out = (float*)outputBuffer;
	unsigned long i,j,k;
	(void)timeInfo; /* Prevent unused variable warnings. */
    (void)statusFlags;
	(void)inputBuffer;
	for (i = 0; i<framesPerBuffer; i++){
		k = 0;
		for (j = 0; j < tableData.dataCols; j++)
			*out++ = tableData.matrixToPlay[k][j];
		k += 1;
		if (k == tableData.dataRows)
			k = 0;
	}
	return paContinue;
}

/*Error message log*/
char* MOTUPlayer::LogError() {
	return logText;
}

/*Start and play stream in a separate thread*/
void AsyncHandleStream(void*) {
	PaError err;
	MOTUPlayer::streamFinishedFlag = 1;
	err = Pa_StartStream(MOTUPlayer::stream);
	if (err != paNoError) {
		MOTUPlayer::logText = "Error in starting stream thread";
		MOTUPlayer::streamFinishedFlag = 0;
		Pa_Terminate();
	}

	Pa_Sleep(MOTUPlayer::duration * 1000);

	err = Pa_StopStream(MOTUPlayer::stream);
	if (err != paNoError) {
		MOTUPlayer::logText = "Error in stopping stream thread";
		MOTUPlayer::streamFinishedFlag = 0;
		Pa_Terminate();
	}

	err = Pa_CloseStream(MOTUPlayer::stream);
	if (err != paNoError) {
		MOTUPlayer::logText = "Error in closing stream thread";
		MOTUPlayer::streamFinishedFlag = 0;
		Pa_Terminate();
	}

	Pa_Terminate();
	_endthread();
}

/*Play a matrix*/
int MOTUPlayer::Play(float matrix[TABLE_SIZE][24], int rows, int cols, float duration) {
	PaError err;
	tableData.dataCols = cols;
	tableData.dataRows = rows;
	for (int i = 0; i < TABLE_SIZE; i++)
		for (int j = 0; j < 24; j++)
			tableData.matrixToPlay[i][j] = matrix[i][j];

	MOTUPlayer::duration = duration;

	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		PortAudioCallback,
		this);

	if (err != paNoError) {
		logText = "Error in opening the stream";
		return -1;
	}

	err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
	if (err != paNoError) {
		logText = "Error in creating stream finished callback";
		return -1;
	}
	_beginthread(AsyncHandleStream, 0, NULL);
	return 0;
}