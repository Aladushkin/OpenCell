#include <iostream>
#include "RenderParam.h"

extern RenderConfig renderConfig;

extern int g_device;
extern bool g_extensions;
extern bool g_benchmark;
extern bool g_profile;
extern bool g_teamCity;
//extern int g_msaaSamples;
extern bool g_fullscreen;
extern bool g_vsync;
extern int g_numExtraMultiplier;
extern bool g_tweakPanel;
extern bool g_interop;

void ConsoleController(int argc, char* argv[]) {
	// process command line args
	for (int i = 1; i < argc; ++i)
	{
		int d;
		if (sscanf(argv[i], "-device=%d", &d))
			g_device = d;

		if (sscanf(argv[i], "-extensions=%d", &d))
			g_extensions = d != 0;

		if (strstr(argv[i], "-benchmark")) {
			g_benchmark = true;
			g_profile = true;
		}

		if (strstr(argv[i], "-tc"))
			g_teamCity = true;

		if (sscanf(argv[i], "-msaa=%d", &d))
			renderConfig.SetMSAASamples(d);

		if (strstr(argv[i], "-fullscreen"))
			g_fullscreen = true;

		if (sscanf(argv[i], "-vsync=%d", &d))
			g_vsync = d != 0;

		if (sscanf(argv[i], "-multiplier=%d", &d) == 1)
		{
			g_numExtraMultiplier = d;
		}

		if (strstr(argv[i], "-disabletweak"))
		{
			g_tweakPanel = false;
		}

		if (strstr(argv[i], "-disableinterop"))
		{
			g_interop = false;
		}
	}
}