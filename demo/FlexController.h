#pragma once

#include "../include/NvFlex.h"
#include "../include/NvFlexExt.h"
#include "../include/NvFlexDevice.h"

#include <iostream>

extern int g_device;
extern bool g_extensions;
extern NvFlexLibrary* g_flexLib;
extern char g_deviceName[256];
extern bool g_Error;

// необходимо объ€вить вне класса, иначе Flex не может выполнить преобразование во внутреннюю ErrorCallback функцию
void ErrorCallback(NvFlexErrorSeverity, const char* msg, const char* file, int line)
{
	printf("Flex: %s - %s:%d\n", msg, file, line);
	g_Error = true;
	//assert(0); asserts are bad for TeamCity
}

class FlexController {
public:
	//конструктор и инициализаторы
	FlexController() {}

	void InitFlex() {
		// use the PhysX GPU selected from the NVIDIA control panel
		if (g_device == -1)
			g_device = NvFlexDeviceGetSuggestedOrdinal();

		// Create an optimized CUDA context for Flex and set it on the 
		// calling thread. This is an optional call, it is fine to use 
		// a regular CUDA context, although creating one through this API
		// is recommended for best performance.
		bool success = NvFlexDeviceCreateCudaContext(g_device);

		if (!success)
		{
			printf("Error creating CUDA context.\n");
			exit(-1);
		}

		NvFlexInitDesc desc;
		desc.deviceIndex = g_device;
		desc.enableExtensions = g_extensions;
		desc.renderDevice = 0;
		desc.renderContext = 0;
		desc.computeType = eNvFlexCUDA;

		// Init Flex library, note that no CUDA methods should be called before this 
		// point to ensure we get the device context we want
		g_flexLib = NvFlexInit(NV_FLEX_VERSION, ErrorCallback, &desc);

		if (g_Error || g_flexLib == NULL)
		{
			printf("Could not initialize Flex, exiting.\n");
			exit(-1);
		}

		// store device name
		strcpy(g_deviceName, NvFlexGetDeviceName(g_flexLib));
		printf("Compute Device: %s\n\n", g_deviceName);
	}
};



