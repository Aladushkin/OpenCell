#pragma once

#include "component.h"

class Kernel : public Component {
public:
	Kernel() {}

	Kernel(int group) {
		this->group = group;
	}

	void Initialize() {
		float radius = 0.1f;
		float restDistance = radius*0.5f;

		int n = 1;
		float spacing = 64 * restDistance*0.9f / (2.0f*n);
		float sampling = restDistance*0.8f;
		Vec3 size = sampling*12.0f;

		const float mass[] = { 1.0f, 0.25f, 0.005f };

		CreateParticleShape(GetFilePathByPlatform("../../data/sphere.ply").c_str(), Vec3(2.5f, 0.9f ,1.5f), size, 0.0f, sampling, Vec3(0.0f, 0.0f, 0.0f), mass[0], true, 1.0f, NvFlexMakePhase(group++, 0), true, 0.0001f);
	};

	void PostInitialize() {}

	void Update() {}

	void Sync() {}

	void Draw() {}

	int group;
};