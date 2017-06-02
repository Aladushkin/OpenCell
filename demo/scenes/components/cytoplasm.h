#pragma once

#include "component.h"

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

	void Initialize(SimBuffers *buffers) {
		this->buffers = buffers;

		Mesh* mesh = ImportMesh(GetFilePathByPlatform("../../data/sphere_high.ply").c_str());
		Vec3 lower = Vec3(2.0f + 0 * 2.0f, 0.4f + 0 * 1.2f, 1.0f);

		mesh->Normalize();
		mesh->Transform(TranslationMatrix(Point3(lower)));

		std::vector<Vec3> positions(10000);
<<<<<<< HEAD
		int n = PoissonSample3D(0.45f, g_params.radius*0.42f, &positions[0], positions.size(), 10000);
=======
		int n = MyPoissonSample3D(0.45f, g_params.radius*0.42f, &positions[0], positions.size(), 10000);
		//int n = TightPack3D(0.45f, g_params.radius*0.42f, &positions[0], positions.size());
>>>>>>> origin/master

		const int vertStart = 0 * mesh->GetNumVertices();
		const int vertEnd = vertStart + mesh->GetNumVertices();

		const int phase = NvFlexMakePhase(group++, eNvFlexPhaseSelfCollide | eNvFlexPhaseFluid);

		Vec3 center;
		for (int v = vertStart; v < vertEnd; ++v)
			center += Vec3(this->buffers->positions[v]);

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

	// Poisson sample the volume of a sphere with given separation
	inline int MyPoissonSample3D(float radius, float separation, Vec3* points, int maxPoints, int maxAttempts)
	{
		// naive O(n^2) dart throwing algorithm to generate a Poisson distribution
		int c = 0;
		while (c < maxPoints)
		{
			int a = 0;
			while (a < maxAttempts)
			{
				const Vec3 p = MyUniformSampleSphereVolume()*radius;

				// test against points already generated
				int i = 0;
				for (; i < c; ++i)
				{
					Vec3 d = p - points[i];

					// reject if closer than separation
					if (LengthSq(d) < separation*separation)
						break;
				}

				// sample passed all tests, accept
				if (i == c)
				{
					points[c] = p;
					++c;
					break;
				}

				++a;
			}

			// exit if we reached the max attempts and didn't manage to add a point
			if (a == maxAttempts)
				break;
		}

		return c;
	}

	inline Vec3 MyUniformSampleSphereVolume()
	{
		for (;;)
		{
			Vec3 v = RandVec3();
			if (Dot(v, v) < 1.0f && Dot(v,v) > 0.0f)
				return v;
		}
	}

	int group;

	int numberOfParticles;
};