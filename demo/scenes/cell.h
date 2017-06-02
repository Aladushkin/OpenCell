
#include "components\cytoplasm.h"
#include "components\kernel.h"
#include "components\shell.h"

#include "BiologyObject.h"

class Cell : public BiologyObject
{
public:
	int mNumFluidParticles;

	Cytoplasm*  cytoplasm;
	Kernel*		kernel;
	Shell*		shell;

	Cell() {}

	~Cell() {
		delete shell;
	}

	void Initialize(FlexController *flexController, SimBuffers *buffers, FlexParams *flexParams, RenderParam *renderParam) {
		this->flexController = flexController;
		this->buffers    = buffers;
		this->flexParams = flexParams;

		this->renderParam = renderParam;

		float minSize = 0.25f;
		float maxSize = 0.5f;
		float spacing = 4.0f;

		float radius = 0.1f;
		int group = 0;

		g_buffers->numExtraParticles = 20000;
		flexParams->numSubsteps = 3;

		g_params.radius = radius;
		g_params.dynamicFriction = 0.125f;
		g_params.dissipation = 0.0f;
		g_params.numIterations = 5;
		g_params.particleCollisionMargin = g_params.radius*0.05f;
		g_params.relaxationFactor = 1.0f;
		g_params.drag = 0.0f;
		g_params.anisotropyScale = 25.0f;
		g_params.smoothing = 1.f;
		g_params.maxSpeed = 0.5f * flexParams->numSubsteps * radius * 60.0;
		g_params.gravity[1] *= 1.0f;
		g_params.collisionDistance = 0.001f;
		g_params.solidPressure = 0.0f;
		g_params.solidRestDistance = 0.060f;
		g_params.fluid = true;

		g_params.fluidRestDistance = radius*0.65f;
		g_params.viscosity = 0.0;
		g_params.adhesion = 0.0f;
		g_params.cohesion = 0.02f;

		clearBuffers();

		shell = new Shell(group++);
		shell->Initialize(buffers);

		cytoplasm = new Cytoplasm(group++);
		cytoplasm->Initialize(buffers);


		kernel = new Kernel(group++);
		kernel->Initialize();


		mNumFluidParticles = cytoplasm->numberOfParticles;

<<<<<<< HEAD
		renderParam->drawCloth =  true;
		flexParams->warmup = true;
=======
		g_drawPoints = false;
		g_drawEllipsoids = true;
		g_drawSprings = 0;
		g_drawCloth = false;
		g_drawMesh = true;
		g_warmup = true;

>>>>>>> origin/master
	}
	
	void clearBuffers() {
		buffers->triangles.resize(0);
		buffers->springIndices.resize(0);
		buffers->springStiffness.resize(0);
		buffers->springLengths.resize(0);
	}

	void Sync()
	{
		// send new particle data to the GPU
		NvFlexSetRestParticles(flexController->GetSolver(), g_buffers->restPositions.buffer, g_buffers->restPositions.size());

		// update solver
		NvFlexSetSprings(flexController->GetSolver(), g_buffers->springIndices.buffer, g_buffers->springLengths.buffer, g_buffers->springStiffness.buffer, g_buffers->springLengths.size());
		NvFlexSetDynamicTriangles(flexController->GetSolver(), g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);
	}
	
	void Update() {}

	void Draw() {
		shell->Draw();
		cytoplasm->Draw();
	}
};
