#pragma once

#include "RenderParam.h"



class SDLController {
private:
	RenderConfig *renderConfig;
	
	unsigned int windowId;

public:
	// инкапсул€ци€
	unsigned int GetWindowId() {
		return windowId;
	}

	//конструктор и инициализаци€
	SDLController() {}

	void SDLInit(RenderConfig *renderConfig, const char* title)
	{
		this->renderConfig = renderConfig;

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)	// Initialize SDL's Video subsystem and game controllers
			printf("Unable to initialize SDL");

		// Create our window centered
		renderConfig->SetWindow(SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			renderConfig->GetWidth(), renderConfig->GetHeight(), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));

		windowId = SDL_GetWindowID(renderConfig->GetWindow());
	}
};