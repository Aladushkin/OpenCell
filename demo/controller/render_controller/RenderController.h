#pragma once

#include "../../../external/SDL2-2.0.4/include/SDL.h"

#include "../../opengl/imguiRenderGL.h"
#include "../../opengl/shader.h"

#include "../../shaders.h"

#include "Shadows.h"

#include "../../../core/maths.h"
#include "../../../core/platform.h"

extern bool g_benchmark;

#include "../../SimBuffer.h"
extern SimBuffers *g_buffers;

extern Vec3 g_sceneLower;
extern Vec3 g_sceneUpper;

// пока так, но потом впилить в класс
#include "RenderParam.h"

#include "RenderBuffer.h"
extern RenderBuffers *renderBuffers;

#include "Camera.h"
extern Camera camera;

extern NvFlexParams g_params;

#include "../compute_controller/FlexController.h"
extern FlexController flexController;

extern int g_scene;

class Scene;
extern std::vector<Scene*> g_scenes;
#include "../../scenes.h"

#include "Fluid/FluidRenderer.h"

class RenderController {

private:
	SDL_Window* window;			// window handle
	bool fullscreen = false;

	int screenWidth = 1280;
	int screenHeight = 720;

	int msaaSamples = 8;

	// отрисовка GUI
	bool showGUI = true;

	// параметры рендера
	bool drawEllipsoids = true;
	bool drawOpaque = false;
	bool drawPoints = false;
	int  drawSprings = 0;		// 0: no draw, 1: draw stretch 2: draw tether

	// пока ничего не меняет, пофиксить 
	bool drawMesh = true;

	// вывод дебага для карты
	bool profile = false;

	//Работа с окном 
	// Изменение размеров окна (кишки OpenGL)
	void ReshapeRender(SDL_Window* window)
	{
		int width, height;
		SDL_GetWindowSize(window, &width, &height);

		if (msaaSamples)
		{
			glVerify(glBindFramebuffer(GL_FRAMEBUFFER, 0));

			if (renderParam->msaaFbo)
			{
				glVerify(glDeleteFramebuffers(1, &renderParam->msaaFbo));
				glVerify(glDeleteRenderbuffers(1, &renderParam->msaaColorBuf));
				glVerify(glDeleteRenderbuffers(1, &renderParam->msaaDepthBuf));
			}

			int samples;
			glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);

			// clamp samples to 4 to avoid problems with point sprite scaling
			samples = Min(samples, Min(msaaSamples, 4));

			glVerify(glGenFramebuffers(1, &renderParam->msaaFbo));
			glVerify(glBindFramebuffer(GL_FRAMEBUFFER, renderParam->msaaFbo));

			glVerify(glGenRenderbuffers(1, &renderParam->msaaColorBuf));
			glVerify(glBindRenderbuffer(GL_RENDERBUFFER, renderParam->msaaColorBuf));
			glVerify(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width, height));

			glVerify(glGenRenderbuffers(1, &renderParam->msaaDepthBuf));
			glVerify(glBindRenderbuffer(GL_RENDERBUFFER, renderParam->msaaDepthBuf));
			glVerify(glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height));
			glVerify(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderParam->msaaDepthBuf));

			glVerify(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderParam->msaaColorBuf));

			glVerify(glCheckFramebufferStatus(GL_FRAMEBUFFER));

			glEnable(GL_MULTISAMPLE);
		}
	}

	//Render buffer
	RenderBuffers *renderBuffers;

	//Параметры рендера
	RenderParam *renderParam;

	//functions for rendering
	void SkinMesh();

	//renderers
	FluidRenderer *fluidRenderer;

	//shadow map
	ShadowMap *shadowMap;

public:

	FluidRenderer* GetFluidRenderer() {
		return fluidRenderer;
	}

	void DrawShapes();

	//публичные объекты рендера
	//Тени
	Shadows shadows = Shadows();

	//Конструктор и инициализация
	RenderController() {}
	
	SDL_Window* InitRender(RenderParam *renderParam)
	{
		//fluidRenderer = new FluidRenderer();

		this->renderParam = renderParam;

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
		ReshapeWindow();

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

	bool GetShowGUI() {
		return showGUI;
	}
	void SetShowGUI(bool showGUI) {
		this->showGUI = showGUI;
	}

	bool GetDrawEllipsoids() {
		return drawEllipsoids;
	}
	void SetDrawEllipsoids(bool drawEllipsoids) {
		this->drawEllipsoids = drawEllipsoids;
	}

	bool GetDrawOpaque() {
		return drawOpaque;
	}
	void SetDrawOpaque(bool drawOpaque) {
		this->drawOpaque = drawOpaque;
	}

	bool GetDrawPoints() {
		return drawPoints;
	}
	void SetDrawPoints(bool drawPoints) {
		this->drawPoints = drawPoints;
	}

	int GetDrawSprings() {
		return drawSprings;
	}
	void SetDrawSprings(int drawSprings) {
		this->drawSprings = drawSprings;
	}

	bool GetDrawMesh() {
		return drawMesh;
	}
	void SetDrawMesh(bool drawMesh) {
		this->drawMesh = drawMesh;
	}

	bool GetProfile() {
		return profile;
	}
	void SetProfile(bool profile) {
		this->profile = profile;
	}

	ShadowMap* GetShadowMap() {
		return shadowMap;
	}
	void SetShadowMap(ShadowMap *shadowMap) {
		this->shadowMap = shadowMap;
	}

	RenderBuffers* GetRenderBuffers() {
		return renderBuffers;
	}
	void SetRenderBuffers(RenderBuffers *renderBuffers) {
		this->renderBuffers = renderBuffers;
	}

	//Работа с окном
	//Изменение размеров окна
	void ReshapeWindow(int width, int height)
	{
		if (!g_benchmark)
			printf("Reshaping\n");

		ReshapeRender(window);

		if (!fluidRenderer || (width != this->screenWidth || height != this->screenHeight))
		{
			if (fluidRenderer)
				DestroyFluidRenderer(fluidRenderer);
			fluidRenderer = CreateFluidRenderer(width, height);
		}

		screenWidth = width;
		screenHeight = height;
	}
	void ReshapeWindow() {
		if (!g_benchmark)
			printf("Reshaping\n");

		ReshapeRender(window);

		if (!fluidRenderer)
		{
			if (fluidRenderer)
				DestroyFluidRenderer(fluidRenderer);
			fluidRenderer = CreateFluidRenderer(screenWidth, screenHeight);
		}
	}

	// main render
	void RenderScene(int numParticles, int numDiffuse);

	void RenderDebug();
};

