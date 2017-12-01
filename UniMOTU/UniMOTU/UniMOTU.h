#pragma once

//Plays a matrix that corresponds to the phoneme code
extern "C" __declspec(dllexport) int play(int phonemeCode);

//Plays a matrix that corresponds to the phoneme code synchronously
extern "C" __declspec(dllexport) int playSynchronous(int phonemeCode);

//Play a simple sine wave to test the connection
extern "C" __declspec(dllexport) int testPlay();

//Returns log code
extern "C" __declspec(dllexport) int getLogCode();

//Returns playing flag
extern "C" __declspec(dllexport) int isPlaying();

//Returns dll path
extern "C" __declspec(dllexport) void setDllPathAsMessage();

//Get the general message
extern "C" __declspec(dllexport) char getMsg();

