#pragma once

//Player object that handles the communication with PortAudio

#include <process.h>
#include "portaudio.h"

#define SAMPLE_RATE   (44100)
#define FRAMES_PER_BUFFER  (64)
#define TABLE_SIZE (200)
#ifndef M_PI
#define M_PI  (3.14159265)
#endif

class MOTUPlayer {
	private:
		PaStreamParameters outputParameters;
		char* LogError();
		struct {
			int dataRows;
			int dataCols;
			float matrixToPlay[TABLE_SIZE][24];
		}tableData;
	public:
		static float duration;
		static PaStream *stream;
		static int streamFinishedFlag;
		static char* logText;
		MOTUPlayer();
		int Initialize();
		int Play(float matrix[TABLE_SIZE][24], int rows, int cols, float duration);
		int PlayerCallback(const void *inputBuffer, void *outputBuffer,
			unsigned long framesPerBuffer,
			const PaStreamCallbackTimeInfo* timeInfo,
			PaStreamCallbackFlags statusFlags);
		
};