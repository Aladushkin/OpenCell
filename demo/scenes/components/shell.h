#pragma once

#include "../../component.h"

class Shell : public Component {
public:
	Shell() {};

	Shell(int group) {
		this->group = group;
	}

	~Shell() {
		NvFlexExtDestroyTearingCloth(asset);
	}

	void AddInflatable(const Mesh* mesh, float overPressure, float invMass, int phase) {
		// create a cloth mesh using the global positions / indices
		const int numParticles = int(mesh->m_positions.size());
		
		// add particles to system
		for (size_t i = 0; i < mesh->GetNumVertices(); ++i)
		{
			const Vec3 p = Vec3(mesh->m_positions[i]);

			g_buffers->positions.push_back(Vec4(p.x, p.y, p.z, invMass));
			g_buffers->restPositions.push_back(Vec4(p.x, p.y, p.z, invMass));

			g_buffers->velocities.push_back(0.0f);
			g_buffers->phases.push_back(phase);
		}
		
		// create asset
		NvFlexExtAsset* cloth = NvFlexExtCreateClothFromMesh((float*)&g_buffers->positions[0], numParticles, (int*)&mesh->m_indices[0], mesh->GetNumFaces(), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

		this->asset = cloth;
		this->splitThreshold = 4.0f;

		// set buffers for flex
		for (int i = 0; i < asset->numTriangles; ++i)
		{
			g_buffers->triangles.push_back(asset->triangleIndices[i * 3 + 0]);
			g_buffers->triangles.push_back(asset->triangleIndices[i * 3 + 1]);
			g_buffers->triangles.push_back(asset->triangleIndices[i * 3 + 2]);
		}

		for (int i = 0; i < asset->numSprings * 2; ++i) {
			g_buffers->springIndices.push_back(asset->springIndices[i]);
		}

		for (int i = 0; i < asset->numSprings; ++i)
		{
			g_buffers->springStiffness.push_back(asset->springCoefficients[i]);
			g_buffers->springLengths.push_back(asset->springRestLengths[i]);
		}
	}

	void Initialize() {


		Mesh* mesh = ImportMesh(GetFilePathByPlatform("../../data/sphere_high.ply").c_str());
		Vec3 lower = Vec3(2.0f + 0 * 2.0f, 0.4f + 0 * 1.2f, 1.0f);

		mesh->Normalize();
		mesh->Transform(TranslationMatrix(Point3(lower)));

		AddInflatable(mesh, 1.0f, 0.25f, NvFlexMakePhase(group, eNvFlexPhaseSelfCollide | eNvFlexPhaseSelfCollideFilter));

		g_numSolidParticles = g_buffers->positions.size();
		g_numExtraParticles = g_buffers->positions.size();

		delete mesh;
	}
	
	void PostInitialize() {}

	void Update() {}

	void Sync() {}

	void Draw() {
		if (!g_drawMesh)
			return;

		DrawCloth(&g_buffers->positions[0], &g_buffers->normals[0], NULL, &g_buffers->triangles[0], asset->numTriangles, g_buffers->positions.size(), (0 + 2) % 6);
	}

	NvFlexExtAsset* asset;

	float splitThreshold;

	int group;
};