#pragma once

//Initialize PortAudio and the audio device and verifies the connection to the 24 channel motu output
extern "C" __declspec(dllexport) int Initialize();

//Tests a stereo sinusoidal sound 
extern "C" __declspec(dllexport) int TestPlay();

//Plays a matrix sent from the game
extern "C" __declspec(dllexport) int Play(int rows, int cols, float** matrix);

//Close and terminate PortAudio
extern "C" __declspec(dllexport) int Terminate();