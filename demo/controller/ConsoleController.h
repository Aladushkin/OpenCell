#include <iostream>

#include "render_controller/RenderController.h"
#include "compute_controller/FlexController.h"
#include "imgui_controller/IMGUIController.h"
#include "../SimBuffer.h"

extern FlexController flexController;
extern SimBuffers *g_buffers;

extern RenderController renderController;
extern IMGUIController imguiController;

#include "render_controller/RenderParam.h"

extern RenderParam *renderParam;

extern bool g_extensions;
extern bool g_benchmark;
extern bool g_teamCity;



void ConsoleController(int argc, char* argv[]) {
	// process command line args
	for (int i = 1; i < argc; ++i)
	{
		int d;
		if (sscanf(argv[i], "-device=%d", &d))
			flexController.SetDevice(d);

		if (sscanf(argv[i], "-extensions=%d", &d))
			g_extensions = d != 0;

		if (strstr(argv[i], "-benchmark")) {
			g_benchmark = true;
			renderController.SetProfile(true);
		}

		if (strstr(argv[i], "-tc"))
			g_teamCity = true;

		if (sscanf(argv[i], "-msaa=%d", &d))
			renderController.SetMSAASamples(d);

		if (strstr(argv[i], "-fullscreen"))
			renderController.SetFullscreen(true);

		if (sscanf(argv[i], "-vsync=%d", &d))
			renderParam->vsync = d != 0;

		if (sscanf(argv[i], "-multiplier=%d", &d) == 1)
		{
			g_buffers->numExtraMultiplier = d;
		}

		if (strstr(argv[i], "-disabletweak"))
		{
			imguiController.SetTweakPanel(false);
		}

		if (strstr(argv[i], "-disableinterop"))
		{
			renderParam->interop = false;
		}
	}
}