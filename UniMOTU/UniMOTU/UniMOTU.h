#pragma once

//Initialize PortAudio and the audio device and verifies the connection to the 24 channel motu output
extern "C" __declspec(dllexport) int Initialize();

//Plays a matrix sent from the game
extern "C" __declspec(dllexport) int Play(float matrix[200][24], float duration);
