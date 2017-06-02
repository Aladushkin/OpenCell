#pragma once

#include "controller/render_controller/RenderController.h"

#include "controller/render_controller/Camera.h"

// выпилится, когда появится Compute Controller
extern NvFlexParams g_params;

// глобальные параметры исполнения
extern bool g_pause;
extern bool g_debug;
extern bool g_reset;
extern bool g_step;

class SDLController {
private:
	// указатель на контроллер рендера
	RenderController *renderController;

	// указатель на камеру
	Camera *camera;

	unsigned int windowId;

	// mouse
	int lastx;
	int lasty;
	int lastb = -1;

	// methods of control
	// нажатие клавиши
	bool InputKeyboardDown(unsigned char key, int x, int y)
	{
		float kSpeed = camera->GetCamSpeed();

		switch (key)
		{
		case 'w':
		{
			camera->SetCamVelZ(kSpeed);
			break;
		}
		case 's':
		{
			camera->SetCamVelZ(-kSpeed);
			break;
		}
		case 'a':
		{
			camera->SetCamVelX(-kSpeed);
			break;
		}
		case 'd':
		{
			camera->SetCamVelX(kSpeed);
			break;
		}
		case 'q':
		{
			camera->SetCamVelY(kSpeed);
			break;
		}
		case 'z':
		{
			camera->SetCamVelY(-kSpeed);
			break;
		}

		case 'u':
		{
			if (renderController->GetFullscreen())
			{
				SDL_SetWindowFullscreen(renderController->GetWindow(), 0);
				renderController->ReshapeWindow(1280, 720);
				renderController->SetFullscreen(false);
			}
			else
			{
				SDL_SetWindowFullscreen(renderController->GetWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);
				renderController->SetFullscreen(true);
			}
			break;
		}
		case 'r':
		{
			g_reset = true;
			break;
		}
		case 'p':
		{
			g_pause = !g_pause;
			break;
		}
		case 'o':
		{
			g_step = true;
			break;
		}
		case 'h':
		{
			// переделать возможно
			renderController->SetShowGUI(!renderController->GetShowGUI());
			break;
		}
		case 'e':
		{
			renderController->SetDrawEllipsoids(!renderController->GetDrawEllipsoids());
			break;
		}
		case 't':
		{
			renderController->SetDrawOpaque(!renderController->GetDrawOpaque());
			break;
		}
		case 'v':
		{
			renderController->SetDrawPoints(!renderController->GetDrawPoints());
			break;
		}
		case 'f':
		{
			renderController->SetDrawSprings((renderController->GetDrawSprings() + 1) % 3);
			break;
		}
		case 'm':
		{
			renderController->SetDrawMesh(!renderController->GetDrawMesh());
			break;
		}
		case '.':
		{
			renderController->SetProfile(!renderController->GetProfile());
			break;
		}
		case 'g':
		{
			if (g_params.gravity[1] != 0.0f)
				g_params.gravity[1] = 0.0f;
			else
				g_params.gravity[1] = -9.8f;

			break;
		}
		case ';':
		{
			g_debug = !g_debug;
			break;
		}
		};

		return false;
	}

	// отпуск клавиши
	void InputKeyboardUp(unsigned char key, int x, int y)
	{
		switch (key)
		{
		case 'w':
		case 's':
		{
			camera->SetCamVelZ(0.0f);
			break;
		}
		case 'a':
		case 'd':
		{
			camera->SetCamVelX(0.0f);
			break;
		}
		case 'q':
		case 'z':
		{
			camera->SetCamVelY(0.0f);
			break;
		}
		};
	}

	void MouseFunc(int b, int state, int x, int y)
	{
		switch (state)
		{
		case SDL_RELEASED: {
			lastx = x;
			lasty = y;
			lastb = -1;
			break;
		}
		case SDL_PRESSED: {
			lastx = x;
			lasty = y;
			lastb = b;
			break;
		}
		};
	}

	void MousePassiveMotionFunc(int x, int y) {
		lastx = x;
		lasty = y;
	}

	void MouseMotionFunc(unsigned state, int x, int y) {
		float dx = float(x - lastx);
		float dy = float(y - lasty);

		lastx = x;
		lasty = y;

		if (state & SDL_BUTTON_RMASK)
		{
			const float kSensitivity = DegToRad(0.1f);
			const float kMaxDelta = FLT_MAX;

			Vec3 camAngle = camera->GetCamAngle();
			camAngle.x -= Clamp(dx*kSensitivity, -kMaxDelta, kMaxDelta);
			camAngle.y -= Clamp(dy*kSensitivity, -kMaxDelta, kMaxDelta);
			camera->SetCamAngle(camAngle);
		}
	}

public:
	// инкапсуляция
	unsigned int GetWindowId() {
		return windowId;
	}

	int GetLastX() {
		return lastx;
	}
	void SetLastX(int lastx) {
		this->lastx = lastx;
	}

	int GetLastY() {
		return lasty;
	}
	void SetLastY(int lasty) {
		this->lasty = lasty;
	}

	int GetLastB() {
		return lastb;
	}
	void SetLastB(int lastb) {
		this->lastb = lastb;
	}

	//конструктор и инициализация
	SDLController() {}

	void SDLInit(Camera *camera, RenderController *renderController, const char* title)
	{
		this->camera = camera;
		this->renderController = renderController;

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)	// Initialize SDL's Video subsystem and game controllers
			printf("Unable to initialize SDL");

		// Create our window centered
		renderController->SetWindow(SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			renderController->GetWidth(), renderController->GetHeight(), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));

		windowId = SDL_GetWindowID(renderController->GetWindow());
	}

	// return true, if program is running
	bool SDLMainLoop() {
		bool quit = false;
		SDL_Event e;

		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
				if (e.key.keysym.sym < 256 && (e.key.keysym.mod == KMOD_NONE || (e.key.keysym.mod & KMOD_NUM)))
					quit = InputKeyboardDown(e.key.keysym.sym, 0, 0);
				break;

			case SDL_KEYUP:
				if (e.key.keysym.sym < 256 && (e.key.keysym.mod == 0 || (e.key.keysym.mod & KMOD_NUM)))
					InputKeyboardUp(e.key.keysym.sym, 0, 0);
				break;

			case SDL_MOUSEMOTION:
				if (e.motion.state)
					MouseMotionFunc(e.motion.state, e.motion.x, e.motion.y);
				else
					MousePassiveMotionFunc(e.motion.x, e.motion.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				MouseFunc(e.button.button, e.button.state, e.motion.x, e.motion.y);
				break;

			case SDL_WINDOWEVENT:
				if (e.window.windowID == windowId)
				{
					if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
						renderController->ReshapeWindow(e.window.data1, e.window.data2);
				}
				break;

			case SDL_WINDOWEVENT_LEAVE:
				camera->SetCamVel(Vec3(0.0f, 0.0f, 0.0f));
				break;
			}
		}

		// TODO: swap to running
		return !quit;
	}

};