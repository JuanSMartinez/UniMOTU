// UniMOTU.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
MOTUPlayer player;

__declspec(dllexport) int Initialize() {
	player = MOTUPlayer();
	return player.Initialize();	
}

__declspec(dllexport) int Play(float matrix[200][24], float duration) {
	return player.Play(matrix, 200, 24, duration);
}

