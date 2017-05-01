#pragma once

#include "../external/SDL2-2.0.4/include/SDL.h"

#include "opengl/imguiRenderGL.h"
#include "opengl/shader.h"

extern bool g_benchmark;

extern GLuint g_msaaFbo;
extern GLuint g_msaaColorBuf;
extern GLuint g_msaaDepthBuf;

namespace {
	FluidRenderer* g_fluidRenderer;

}

class RenderConfig {

private:
	SDL_Window* window;			// window handle
	unsigned int windowId;		// window id

	bool fullscreen = false;

	int screenWidth = 1280;
	int screenHeight = 720;

	int msaaSamples = 8;

	//Работа с окном 
	// Изменение размеров окна (кишки OpenGL)
	void ReshapeRender(SDL_Window* window)
	{
		int width, height;
		SDL_GetWindowSize(window, &width, &height);

		if (msaaSamples)
		{
			glVerify(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			if (g_msaaFbo)
			{
				glVerify(glDeleteFramebuffers(1, &g_msaaFbo));
				glVerify(glDeleteRenderbuffers(1, &g_msaaColorBuf));
				glVerify(glDeleteRenderbuffers(1, &g_msaaDepthBuf));
			}

			int samples;
			glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);

			// clamp samples to 4 to avoid problems with point sprite scaling
			samples = Min(samples, Min(msaaSamples, 4));

			glVerify(glGenFramebuffers(1, &g_msaaFbo));
			glVerify(glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFbo));

			glVerify(glGenRenderbuffers(1, &g_msaaColorBuf));
			glVerify(glBindRenderbuffer(GL_RENDERBUFFER, g_msaaColorBuf));
			glVerify(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height));

			glVerify(glGenRenderbuffers(1, &g_msaaDepthBuf));
			glVerify(glBindRenderbuffer(GL_RENDERBUFFER, g_msaaDepthBuf));
			glVerify(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height));
			glVerify(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_msaaDepthBuf));

			glVerify(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, g_msaaColorBuf));

			glVerify(glCheckFramebufferStatus(GL_FRAMEBUFFER));

			glEnable(GL_MULTISAMPLE);
		}
	}

public:
	//Конструктор и инициализация
	RenderConfig() {}
	
	SDL_Window* InitRender(bool fullscreen)
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);

		// Turn on double buffering with a 24bit Z buffer.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		SDL_GL_CreateContext(window);

		// This makes our buffer swap syncronized with the monitor's vertical refresh
		SDL_GL_SetSwapInterval(1);

		glewExperimental = GL_TRUE;
		glewInit();

		imguiRenderGLInit(GetFilePathByPlatform("../../data/DroidSans.ttf").c_str());

		//g_msaaSamples = msaaSamples;
		return window;
	}

	//Инкапсуляция
	SDL_Window* GetWindow() {
		return window;
	}
	void SetWindow(SDL_Window* window) {
		this->window = window;
	}

	bool GetFullscreen() {
		return this->fullscreen;
	}
	void SetFullscreen(bool fullscreen) {
		this->fullscreen = fullscreen;
	}

	int GetMSAASamples() {
		return this->msaaSamples;
	}
	void SetMSAASamples(int msaaSamples) {
		this->msaaSamples = msaaSamples;
	}

	int GetWidth() {
		return this->screenWidth;
	}
	void SetWidth(int width) {
		this->screenWidth = width;
	}

	int GetHeight() {
		return this->screenHeight;
	}
	void SetHeight(int height) {
		this->screenHeight = height;
	}

	unsigned int GetWindowID() {
		return this->windowId;
	}
	void SetWindowID(unsigned int windowID) {
		this->windowId = windowID;
	}

	//Работа с окном
	//Изменение размеров окна
	void ReshapeWindow(int width, int height)
	{
		if (!g_benchmark)
			printf("Reshaping\n");

		ReshapeRender(window);

		if (!g_fluidRenderer || (width != this->screenWidth || height != this->screenHeight))
		{
			if (g_fluidRenderer)
				DestroyFluidRenderer(g_fluidRenderer);
			g_fluidRenderer = CreateFluidRenderer(width, height);
		}

		screenWidth = width;
		screenHeight = height;
	}
	void ReshapeWindow() {
		if (!g_benchmark)
			printf("Reshaping\n");

		ReshapeRender(window);

		if (!g_fluidRenderer)
		{
			if (g_fluidRenderer)
				DestroyFluidRenderer(g_fluidRenderer);
			g_fluidRenderer = CreateFluidRenderer(screenWidth, screenHeight);
		}
	}
};

