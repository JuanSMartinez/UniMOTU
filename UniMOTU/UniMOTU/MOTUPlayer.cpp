
#include "MOTUPlayer.h"
#include <math.h>
#include <string.h>

/*Stream callback*/
static int PortAudioCallback(const void *inputBuffer,
	void *outputBuffer,
	unsigned long frameCount,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData) {

	float *out = (float*)outputBuffer;
	unsigned long i,j,k;

	(void)timeInfo; /* Prevent unused variable warnings. */
	(void)statusFlags;
	(void)inputBuffer;

	for (i = 0; i<frameCount; i++)
	{

	}

	return paContinue;
}

/*Stream finished callback*/
static void StreamFinished(void* userData)
{

}

/*
Creates a new MOTUPlayer with a sine wave of 60 Hz, 2 seconds and 24 channels as default data
*/
MOTUPlayer::MOTUPlayer() {
	streamFinishedFlag = 0;
	duration = 2;
	tableData.dataRows = duration*1000;
	tableData.dataCols = 24;
	logText = "No error";
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
		logText = Pa_GetErrorText(err);
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

	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		MOTUPlayer::PortAudioCallback,
		&tableData);

	if (err != paNoError) {
		logText = "Error in opening the stream";
		return -1;
	}

	err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
	if (err != paNoError) {
		logText = "Error in creating stream finished callback";
		return -1;
	}

	return 0;
}
