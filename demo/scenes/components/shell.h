#pragma once

#include "../../component.h"

class Shell : public Component {
public:
	Shell() {};

	Shell(int group) {
		this->group = group;
	}

	void AddInflatable(const Mesh* mesh, float overPressure, float invMass, int phase)
	{
		// create a cloth mesh using the global positions / indices
		const int numParticles = int(mesh->m_positions.size());
		const int maxParticles = numParticles * 2;

		// add particles to system
		for (size_t i = 0; i < mesh->GetNumVertices(); ++i)
		{
			const Vec3 p = Vec3(mesh->m_positions[i]);

			g_buffers->positions.push_back(Vec4(p.x, p.y, p.z, invMass));
			g_buffers->restPositions.push_back(Vec4(p.x, p.y, p.z, invMass));

			g_buffers->velocities.push_back(0.0f);
			g_buffers->phases.push_back(phase);
		}
		/*
		for (size_t i = 0; i < mesh->m_indices.size(); i += 3)
		{
			int a = mesh->m_indices[i + 0];
			int b = mesh->m_indices[i + 1];
			int c = mesh->m_indices[i + 2];

			Vec3 n = -Normalize(Cross(mesh->m_positions[b] - mesh->m_positions[a], mesh->m_positions[c] - mesh->m_positions[a]));
			g_buffers->triangleNormals.push_back(n);

			g_buffers->triangles.push_back(a);
			g_buffers->triangles.push_back(b);
			g_buffers->triangles.push_back(c);
		}
		*/

		// create asset
		NvFlexExtAsset* cloth = NvFlexExtCreateClothFromMesh((float*)&g_buffers->positions[0], numParticles, (int*)&mesh->m_indices[0], mesh->GetNumFaces(), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);

		this->asset = cloth;
		this->splitThreshold = 4.0f;
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

		DrawCloth(&g_buffers->positions[0], &g_buffers->normals[0], NULL, &g_buffers->triangles[0], asset->numTriangles, g_buffers->positions.size(), (0 + 2) % 6);//, g_params.radius*0.25f);	
	}

	NvFlexExtAsset* asset;

	float splitThreshold;

	int group;
};