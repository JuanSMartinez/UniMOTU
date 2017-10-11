#pragma once

//Plays a matrix that corresponds to the phoneme code
extern "C" __declspec(dllexport) int play(int phonemeCode);

//Play a simple sine wave to test the connection
extern "C" __declspec(dllexport) int testPlay();

//Returns log code
extern "C" __declspec(dllexport) int getLogcode();



