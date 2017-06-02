#pragma once

#include <vector>
#include "../scenes.h"
#include "cell.h"

class SceneCell : public Scene {

public:

	Cell cell;

	SceneCell() {}

	SceneCell(const char* name) : Scene(name) {}

	~SceneCell() {
		//delete cell;
	}

	void Initialize(FlexController *flexController, SimBuffers *buffers, FlexParams *flexParams, RenderParam *renderParam) {
		this->flexController = flexController;
		this->buffers = buffers;
		this->flexParams = flexParams;

		this->renderParam = renderParam;

		cell.Initialize(flexController, buffers, flexParams, renderParam);
	}

	void clearBuffers() {
		cell.clearBuffers();
	}

	void Sync() {
		cell.Sync();
	}

	void Update() {}

	void Draw() {
		cell.Draw();
	}

};