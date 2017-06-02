#pragma once

#include "../controller/compute_controller/FlexParams.h"
#include "../controller/compute_controller/FlexController.h"

#include "../SimBuffer.h"

class BiologyObject
{
protected:
	FlexController *flexController;
	FlexParams *flexParams;
	SimBuffers *buffers;

	RenderParam *renderParam;

public:

	BiologyObject() {}

	virtual void Initialize(FlexController *flexController, SimBuffers *buffers, FlexParams *flexParams, RenderParam *renderParam) = 0;
	virtual void PostInitialize() {}

	// update any buffers (all guaranteed to be mapped here)
	virtual void Update() {}

	// send any changes to flex (all buffers guaranteed to be unmapped here)
	virtual void Sync() {}

	virtual void Draw(int pass) {}
};