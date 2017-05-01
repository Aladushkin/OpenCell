
#include "components\cytoplasm.h"
#include "components\kernel.h"
#include "components\shell.h"

class Cell : public Scene
{
public:

	Cell(const char* name) : Scene(name) {}

	~Cell() {
		delete shell;
		//NvFlexExtDestroyTearingCloth(shell->asset);
	}

	void Initialize() {
		float minSize = 0.25f;
		float maxSize = 0.5f;
		float spacing = 4.0f;

		// convex rocks
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 1; j++)
				AddRandomConvex(10, Vec3(i*maxSize*spacing, 0.0f, j*maxSize*spacing), minSize, maxSize, Vec3(0.0f, 1.0f, 0.0f), Randf(0.0f, k2Pi));

		float radius = 0.1f;
		int group = 0;

		g_numExtraParticles = 20000;
		g_numSubsteps = 3;

		g_params.radius = radius;
		g_params.dynamicFriction = 0.125f;
		g_params.dissipation = 0.0f;
		g_params.numIterations = 5;
		g_params.particleCollisionMargin = g_params.radius*0.05f;
		g_params.relaxationFactor = 1.0f;
		g_params.drag = 0.0f;
		g_params.anisotropyScale = 25.0f;
		g_params.smoothing = 1.f;
		g_params.maxSpeed = 0.5f*g_numSubsteps*radius / g_dt;
		g_params.gravity[1] *= 1.0f;
		g_params.collisionDistance = 0.01f;
		g_params.solidPressure = 0.0f;

		g_params.fluid = true;

		g_params.fluidRestDistance = radius*0.65f;
		g_params.viscosity = 0.0;
		g_params.adhesion = 0.0f;
		g_params.cohesion = 0.02f;

		clearBuffers();

		delete shell;
		delete cytoplasm;

		shell = new Shell(group++);
		shell->Initialize();

		cytoplasm = new Cytoplasm(group++);
		cytoplasm->Initialize();

		mNumFluidParticles = cytoplasm->numberOfParticles;

		g_drawPoints = false;
		g_drawEllipsoids = true;
		g_drawSprings = 0;
		g_drawCloth = false;
		g_warmup = true;

	}
	
	void clearBuffers() {
		g_buffers->triangles.resize(0);
		g_buffers->springIndices.resize(0);
		g_buffers->springStiffness.resize(0);
		g_buffers->springLengths.resize(0);
	}

	void Sync()
	{
		// send new particle data to the GPU
		NvFlexSetRestParticles(g_flex, g_buffers->restPositions.buffer, g_buffers->restPositions.size());

		// update solver
		NvFlexSetSprings(g_flex, g_buffers->springIndices.buffer, g_buffers->springLengths.buffer, g_buffers->springStiffness.buffer, g_buffers->springLengths.size());
		NvFlexSetDynamicTriangles(g_flex, g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);
	}
	
	void Update() {
	}

	void Draw() {
		shell->Draw();
		cytoplasm->Draw();
	}

	int mNumFluidParticles;

	Cytoplasm* cytoplasm;
	Kernel* kernel;
	Shell* shell;
};
