// PTG_GPU_DX12.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

const char* WND_TITLE = "Procedural Terrain Generation with DirectX 12";
const unsigned int WND_WIDTH	= 800;
const unsigned int WND_HEIGHT	= 800;

SDL_Window * window = NULL;
SDL_Surface  * screenSurface = NULL;

int main()
{

	if (SDL_Init(SDL_INIT_VIDEO) == 0)
	{
		window = SDL_CreateWindow(WND_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WND_WIDTH, WND_HEIGHT, SDL_WINDOW_SHOWN);
		if (window)
		{
			screenSurface = SDL_GetWindowSurface(window);
			SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 255, 0, 0));
			SDL_UpdateWindowSurface(window);
			SDL_Delay(2000);
		}
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
	else
		SDL_Log("Unable to intialize SDL: %s", SDL_GetError());

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
