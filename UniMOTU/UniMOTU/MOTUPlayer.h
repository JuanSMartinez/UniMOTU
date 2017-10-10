#pragma once

//Player object that handles the communication with PortAudio

#include <process.h>
#include "portaudio.h"

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)
#ifndef M_PI
#define M_PI  (3.14159265)
#endif

class MOTUPlayer {
	private:
		PaStream *stream;
		PaStreamParameters outputParameters;
		int streamFinishedFlag;
		const char* logText;
		float duration;
		char* LogError();
	public:
		MOTUPlayer();
		int Initialize();
		int Play(float** matrix, int rows, int cols);
		int AsyncHandleStream(void*);
		int Terminate();
		static int PortAudioCallback(const void *inputBuffer,
			void *outputBuffer,
			unsigned long frameCount,
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags,
			void *userData);
		static void StreamFinished(void* userData);
		static struct {
			int dataRows;
			int dataCols;
			float** matrixToPlay;
		}tableData;
};