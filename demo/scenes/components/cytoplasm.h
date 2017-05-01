#pragma once

#include "../../component.h"

class Cytoplasm : public Component {
public:
	Cytoplasm() {
		numberOfParticles = 0;
		group = 0;
	}

	Cytoplasm(int group) {
		this->group = group;
		numberOfParticles = 0;
	}

	void Initialize() {
		Mesh* mesh = ImportMesh(GetFilePathByPlatform("../../data/sphere_high.ply").c_str());
		Vec3 lower = Vec3(2.0f + 0 * 2.0f, 0.4f + 0 * 1.2f, 1.0f);

		mesh->Normalize();
		mesh->Transform(TranslationMatrix(Point3(lower)));

		std::vector<Vec3> positions(10000);
		int n = PoissonSample3D(0.45f, g_params.radius*0.42f, &positions[0], positions.size(), 10000);
		//int n = TightPack3D(0.45f, g_params.radius*0.42f, &positions[0], positions.size());

		const int vertStart = 0 * mesh->GetNumVertices();
		const int vertEnd = vertStart + mesh->GetNumVertices();

		const int phase = NvFlexMakePhase(group++, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid);

		Vec3 center;
		for (int v = vertStart; v < vertEnd; ++v)
			center += Vec3(g_buffers->positions[v]);

		center /= float(vertEnd - vertStart);

		printf("%d, %d - %f %f %f\n", vertStart, vertEnd, center.x, center.y, center.z);

		for (int i = 0; i < n; ++i)
		{
			g_buffers->positions.push_back(Vec4(center + positions[i], 1.0f));
			g_buffers->restPositions.push_back(Vec4());
			g_buffers->velocities.push_back(0.0f);
			g_buffers->phases.push_back(phase);
		}

		numberOfParticles = n;

		delete mesh;
	};

	void PostInitialize() {}

	void Update() {}

	void Sync() {}

	void Draw() {}

	int group;

	int numberOfParticles;
};