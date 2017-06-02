#include "../core/types.h"
#include "../core/maths.h"
#include "../core/platform.h"
#include "../core/mesh.h"
#include "../core/voxelize.h"
#include "../core/sdf.h"
#include "../core/pfm.h"
#include "../core/tga.h"
#include "../core/perlin.h"
#include "../core/convex.h"
#include "../core/cloth.h"

#include "../external/SDL2-2.0.4/include/SDL.h"

#include "../include/NvFlex.h"
#include "../include/NvFlexExt.h"
#include "../include/NvFlexDevice.h"

#include <iostream>
#include <map>

#include "shaders.h"
#include "controller/imgui_controller/imgui.h"

// buffer
#include "SimBuffer.h"
#include "controller/render_controller/RenderBuffer.h"

// controller
#include "controller/SDLController.h"
#include "controller/ConsoleController.h"
#include "controller/compute_controller/FlexController.h"
#include "controller/render_controller/RenderController.h"
#include "controller/render_controller/Camera.h"
#include "controller/imgui_controller/IMGUIController.h"

// params
#include "controller/render_controller/RenderParam.h"
#include "controller/compute_controller/FlexParams.h"

#include "Timer.h"

using namespace std;

//timer
Timer timer;

// controllers
RenderController renderController;
SDLController sdlController;
FlexController flexController;
IMGUIController imguiController;

Camera camera;

// buffers
SimBuffers* g_buffers;
RenderBuffers* renderBuffers;

// parameters
RenderParam *renderParam;
FlexParams *flexParams;
NvFlexParams g_params;

// param of main control //////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_reset = false;  //if the user clicks the reset button or presses the reset key this is set to true;
bool g_pause = false;
bool g_step = false;
bool g_debug = false;

bool g_benchmark = false;

// count of frames
int g_frame = 0;

// пока здесь, потом переставлю 
int g_scene = 0;

class Scene;
extern std::vector<Scene*> g_scenes;
///////////////////////////////////

bool g_extensions = true;

// size of scene
Vec3 g_sceneLower = FLT_MAX;
Vec3 g_sceneUpper = -FLT_MAX;

// logging
bool g_teamCity = false;

////for class MPEG
bool capture = false;
FILE* g_ffmpeg;
////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "helpers.h"
#include "scenes.h"
#include "scenes\SceneCell.h"
#include "benchmark.h"

void InitScene(int scene, bool centerCamera = true)
{
	RandInit();

	if (flexController.GetSolver())
	{
		if (g_buffers)
			delete g_buffers;

		DestroyFluidRenderBuffers(renderBuffers->fluidRenderBuffers);


		for (auto& iter : renderBuffers->meshes) {
			NvFlexDestroyTriangleMesh(flexController.GetLib(), iter.first);
			DestroyGpuMesh(iter.second);
		}

		for (auto& iter : renderBuffers->fields) {
			NvFlexDestroyDistanceField(flexController.GetLib(), iter.first);
			DestroyGpuMesh(iter.second);
		}

		for (auto& iter : renderBuffers->convexes) {
			NvFlexDestroyConvexMesh(flexController.GetLib(), iter.first);
			DestroyGpuMesh(iter.second);
		}

		renderBuffers->fields.clear();
		renderBuffers->meshes.clear();
		renderBuffers->convexes.clear();

		NvFlexDestroySolver(flexController.GetSolver());
		flexController.SetSolver(NULL);
	}

	// alloc buffers
	g_buffers	  = new SimBuffers(flexController.GetLib());
	renderBuffers = new RenderBuffers();
	renderController.SetRenderBuffers(renderBuffers);

	// initialize params
	flexParams = new FlexParams();

	// map during initialization
	g_buffers->MapBuffers();
	
	// initialize buffers of particles
	g_buffers->ResizePositionVelocityPhases();
	g_buffers->ResizeRigid();
	g_buffers->ResizeSpring();
	g_buffers->ResizeShape();

	// i don't know what it is
	//g_ropes.resize(0);
<<<<<<< HEAD
	renderBuffers->meshSkinIndices.resize(0);
	renderBuffers->meshSkinWeights.resize(0);

	flexParams->InitFlexParams(g_params);
=======
	g_meshSkinIndices.resize(0);
	g_meshSkinWeights.resize(0);

	// remove collision shapes
	delete g_mesh; g_mesh = NULL;

	g_frame = 0;
	g_pause = false;

	g_dt = 1.0f / 60.0f;
	//g_waveTime = 0.0f;

	g_blur = 1.0f;
	g_fluidColor = Vec4(0.1f, 0.4f, 0.8f, 1.0f);
	g_meshColor = Vec3(0.0f, 0.0f, 0.0f);
	g_drawEllipsoids = false;
	g_drawPoints = true;
	g_drawCloth = true;
	g_expandCloth = 0.0f;

	g_drawOpaque = false;
	g_drawSprings = false;
	g_drawDiffuse = false;
	g_drawMesh = true;
	g_drawRopes = true;
	g_drawDensity = false;
	g_ior = 1.0f;
	g_lightDistance = 2.0f;
	g_fogDistance = 0.005f;

	g_camSpeed = 0.075f;
	g_camNear = 0.01f;
	g_camFar = 1000.0f;

	g_pointScale = 1.0f;
	g_ropeScale = 1.0f;
	g_drawPlaneBias = 0.0f;

	InitFlexParams(g_params);

	g_numSubsteps = 2;

	g_diffuseScale = 0.5f;
	g_diffuseColor = 1.0f;
	g_diffuseMotionScale = 1.0f;
	g_diffuseShadow = false;
	g_diffuseInscatter = 0.8f;
	g_diffuseOutscatter = 0.53f;
>>>>>>> origin/master

	// reset phase 0 particle color to blue
	extern Colour gColors[];
	gColors[0] = Colour(0.0f, 0.5f, 1.0f);

	// create scene
	g_scenes[g_scene]->Initialize(&flexController, g_buffers, flexParams, renderParam);

	g_buffers->numParticles = g_buffers->positions.size();
	g_buffers->maxParticles = g_buffers->numParticles + g_buffers->numExtraParticles * g_buffers->numExtraMultiplier;


	// calculate particle bounds
	Vec3 particleLower, particleUpper;
	GetParticleBounds(particleLower, particleUpper);

	//TODO: refactoring

	// accommodate shapes
	Vec3 shapeLower, shapeUpper;
	GetShapeBounds(shapeLower, shapeUpper);

	// update bounds
	g_sceneLower = Min(Min(g_sceneLower, particleLower), shapeLower);
	g_sceneUpper = Max(Max(g_sceneUpper, particleUpper), shapeUpper); 

	g_sceneLower -= g_params.collisionDistance;
	g_sceneUpper += g_params.collisionDistance;


	// initialize Diffuse and Smooth
	g_buffers->ResizeDiffuseSmooth();

	// initialize Normals and Triangles
	g_buffers->ResizeNormalsTriangles();

	// save mesh positions for skinning
	if (renderBuffers->mesh) 
		renderBuffers->meshRestPositions = renderBuffers->mesh->m_positions;
	else
		renderBuffers->meshRestPositions.resize(0);

	// main create method for the Flex solver
	NvFlexSolver *solver = NvFlexCreateSolver(flexController.GetLib(), g_buffers->maxParticles, g_buffers->maxDiffuseParticles, flexParams->maxNeighborsPerParticle);
	flexController.SetSolver(solver);

	// give scene a chance to do some post solver initialization
	g_scenes[g_scene]->PostInitialize();

	// center camera on particles
	if (centerCamera)
	{
		camera.SetCamPos(Vec3((g_sceneLower.x + g_sceneUpper.x)*0.5f, 
							   std::min(g_sceneUpper.y*1.25f, 6.0f), g_sceneUpper.z + std::min(g_sceneUpper.y, 6.0f)*2.0f));
		camera.SetCamAngle(Vec3(0.0f, -DegToRad(15.0f), 0.0f));

		// give scene a chance to modify camera position
		g_scenes[g_scene]->CenterCamera();
	}

	// create active indices (just a contiguous block for the demo)
	g_buffers->ResizeActiveIndices();

	// resize particle buffers to fit
	g_buffers->ResizeFitCell();

	// unmap so we can start transferring data to GPU
	g_buffers->UnmapBuffers();

	//-----------------------------
	// Send data to Flex

	// params of Flex
	NvFlexSetParams(solver, &g_params);

	// build constraints
	g_buffers->BuildConstraints();

	// send buffers
	g_buffers->SendBuffers(solver);

	// create render buffers
	renderBuffers->fluidRenderBuffers = CreateFluidRenderBuffers(g_buffers->maxParticles, renderParam->interop);

	// perform initial sim warm up
	if (flexParams->warmup)
	{
		printf("Warming up sim..\n");

		// warm it up (relax positions to reach rest density without affecting velocity)
		NvFlexParams copy = g_params;
		copy.numIterations = 4;

		NvFlexSetParams(solver, &copy);

		const int kWarmupIterations = 100;

		for (int i = 0; i < kWarmupIterations; ++i)
		{
			NvFlexUpdateSolver(solver, 0.0001f, 1, false);
			NvFlexSetVelocities(solver, g_buffers->velocities.buffer, g_buffers->maxParticles);
		}

		// udpate host copy
		NvFlexGetParticles(solver, g_buffers->positions.buffer, g_buffers->positions.size());
		NvFlexGetSmoothParticles(solver, g_buffers->smoothPositions.buffer, g_buffers->smoothPositions.size());
		NvFlexGetAnisotropy(solver, g_buffers->anisotropy1.buffer, g_buffers->anisotropy2.buffer, g_buffers->anisotropy3.buffer);

		printf("Finished warm up.\n");
	}

	imguiController.Initialize(&flexController, flexParams, g_buffers, &renderController, renderParam, &sdlController);
}

void Reset()
{
	InitScene(g_scene, false);
}

void Shutdown()
{
	// free buffers
	delete g_buffers;

	for (auto& iter : renderBuffers->meshes)
	{
		NvFlexDestroyTriangleMesh(flexController.GetLib(), iter.first);
		DestroyGpuMesh(iter.second);
	}

	for (auto& iter : renderBuffers->fields)
	{
		NvFlexDestroyDistanceField(flexController.GetLib(), iter.first);
		DestroyGpuMesh(iter.second);
	}

	for (auto& iter : renderBuffers->convexes)
	{
		NvFlexDestroyConvexMesh(flexController.GetLib(), iter.first);
		DestroyGpuMesh(iter.second);
	}

	renderBuffers->fields.clear();
	renderBuffers->meshes.clear();

	NvFlexDestroySolver(flexController.GetSolver());
	NvFlexShutdown(flexController.GetLib());
}

void SyncScene()
{
	// let the scene send updates to flex directly
	g_scenes[g_scene]->Sync();
}

void UpdateScene()
{
	// give scene a chance to make changes to particle buffers
	g_scenes[g_scene]->Update();
}

void UpdateFrame()
{
	static double lastTime;

	// real elapsed frame time
	double frameBeginTime = GetSeconds();

	timer.realdt = float(frameBeginTime - lastTime);
	lastTime = frameBeginTime;

	//-------------------------------------------------------------------
	// Scene Update

	double waitBeginTime = GetSeconds();
	g_buffers->MapBuffers();
	double waitEndTime = GetSeconds();

	camera.UpdateCamera();

	if (!g_pause || g_step)	{	
		UpdateScene();
	}

	//-------------------------------------------------------------------
	// Render

	double renderBeginTime = GetSeconds();

	if (renderController.GetProfile() && (!g_pause || g_step)) {
		if (g_benchmark) {
			NvFlexDetailTimer *detailTimers = flexController.GetDetailTimers();
			timer.numDetailTimers = NvFlexGetDetailTimers(flexController.GetSolver(), &detailTimers);
		}
		else {
			memset(&flexController.GetTimers(), 0, sizeof(flexController.GetTimers()));
			NvFlexGetTimers(flexController.GetSolver(), &flexController.GetTimers());
		}
	}

	float newSimLatency = NvFlexGetDeviceLatency(flexController.GetSolver());

	StartFrame(Vec4(renderParam->clearColor, 1.0f));

	// main scene render
	int numParticles = NvFlexGetActiveCount(flexController.GetSolver());
	int numDiffuse   = NvFlexGetDiffuseParticles(flexController.GetSolver(), NULL, NULL, NULL);

	renderController.RenderScene(numParticles, numDiffuse);
	renderController.RenderDebug();
	
	EndFrame();

	const int newScene = imguiController.DoUI(NvFlexGetActiveCount(flexController.GetSolver()), 
											  NvFlexGetDiffuseParticles(flexController.GetSolver(), NULL, NULL, NULL));

	g_buffers->UnmapBuffers();

	if (capture)
	{
		TgaImage img;
		img.m_width = renderController.GetWidth();
		img.m_height = renderController.GetHeight();
		img.m_data = new uint32_t[renderController.GetWidth()*renderController.GetHeight()];

		ReadFrame((int*)img.m_data, renderController.GetWidth(), renderController.GetHeight());

		fwrite(img.m_data, sizeof(uint32_t)*renderController.GetWidth()*renderController.GetHeight(), 1, g_ffmpeg);

		delete[] img.m_data;
	}

	double renderEndTime = GetSeconds();

	// if user requested a scene reset process it now
	if (g_reset) 
	{
		Reset();
		g_reset = false;
	}

	//-------------------------------------------------------------------
	// Flex Update

	double updateBeginTime = GetSeconds();

	// send any particle updates to the solver
	NvFlexSetParticles(flexController.GetSolver(), g_buffers->positions.buffer, g_buffers->positions.size());
	NvFlexSetVelocities(flexController.GetSolver(), g_buffers->velocities.buffer, g_buffers->velocities.size());
	NvFlexSetPhases(flexController.GetSolver(), g_buffers->phases.buffer, g_buffers->phases.size());
	NvFlexSetActive(flexController.GetSolver(), g_buffers->activeIndices.buffer, g_buffers->activeIndices.size());

	// allow scene to update constraints etc
	SyncScene();

	if (flexParams->shapesChanged)
	{
		NvFlexSetShapes(
			flexController.GetSolver(),
			g_buffers->shapeGeometry.buffer,
			g_buffers->shapePositions.buffer,
			g_buffers->shapeRotations.buffer,
			g_buffers->shapePrevPositions.buffer,
			g_buffers->shapePrevRotations.buffer,
			g_buffers->shapeFlags.buffer,
			int(g_buffers->shapeFlags.size()));

		flexParams->shapesChanged = false;
	}

	if (!g_pause || g_step)
	{
		// tick solver
		NvFlexSetParams(flexController.GetSolver(), &g_params);
		NvFlexUpdateSolver(flexController.GetSolver(), timer.dt, flexParams->numSubsteps, renderController.GetProfile());

		g_frame++;
		g_step = false;
	}

	// read back base particle data
	// Note that flexGet calls don't wait for the GPU, they just queue a GPU copy 
	// to be executed later.
	// When we're ready to read the fetched buffers we'll Map them, and that's when
	// the CPU will wait for the GPU flex update and GPU copy to finish.
	NvFlexGetParticles(flexController.GetSolver(), g_buffers->positions.buffer, g_buffers->positions.size());
	NvFlexGetVelocities(flexController.GetSolver(), g_buffers->velocities.buffer, g_buffers->velocities.size());
	NvFlexGetNormals(flexController.GetSolver(), g_buffers->normals.buffer, g_buffers->normals.size());

	// readback triangle normals
	if (g_buffers->triangles.size())
		NvFlexGetDynamicTriangles(flexController.GetSolver(), g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);

	// readback rigid transforms
	if (g_buffers->rigidOffsets.size())
		NvFlexGetRigidTransforms(flexController.GetSolver(), g_buffers->rigidRotations.buffer, g_buffers->rigidTranslations.buffer);

	if (!renderParam->interop)
	{
		// if not using interop then we read back fluid data to host
		if (renderController.GetDrawEllipsoids())
		{
			NvFlexGetSmoothParticles(flexController.GetSolver(), g_buffers->smoothPositions.buffer, g_buffers->smoothPositions.size());
			NvFlexGetAnisotropy(flexController.GetSolver(), g_buffers->anisotropy1.buffer, g_buffers->anisotropy2.buffer, g_buffers->anisotropy3.buffer);
		}

		// read back diffuse data to host
		if (renderParam->drawDensity)
			NvFlexGetDensities(flexController.GetSolver(), g_buffers->densities.buffer, g_buffers->positions.size());
	}

	double updateEndTime = GetSeconds();

	//-------------------------------------------------------
	// Update the on-screen timers

	float newUpdateTime = float(updateEndTime - updateBeginTime);
	float newRenderTime = float(renderEndTime - renderBeginTime);
	float newWaitTime = float(waitBeginTime - waitEndTime);

	// Exponential filter to make the display easier to read
	const float timerSmoothing = 0.05f;

	timer.updateTime = (timer.updateTime == 0.0f) ? newUpdateTime : Lerp(timer.updateTime, newUpdateTime, timerSmoothing);
	timer.renderTime = (timer.renderTime == 0.0f) ? newRenderTime : Lerp(timer.renderTime, newRenderTime, timerSmoothing);
	timer.waitTime = (timer.waitTime == 0.0f) ? newWaitTime : Lerp(timer.waitTime, newWaitTime, timerSmoothing);
	timer.simLatency = (timer.simLatency == 0.0f) ? newSimLatency : Lerp(timer.simLatency, newSimLatency, timerSmoothing);
	
	PresentFrame(renderParam->vsync);
}

void MainLoop()
{
	bool running = true;
	while (running) {
		UpdateFrame();
		running = sdlController.SDLMainLoop();
	}
}

int main(int argc, char* argv[])
{
	// потом переделать
	renderParam = new RenderParam();

	// Read argument from console
	ConsoleController(argc, argv);

	// init controller
	sdlController.SDLInit(&camera, &renderController, "Flex Demo (CUDA)");
	if (renderController.GetFullscreen())
		SDL_SetWindowFullscreen(renderController.GetWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);

	// init gl
	renderController.InitRender(renderParam);

	// init flex
	flexController.InitFlex();
	
	// init benchmark (возможно стоит выпилить)
	if (g_benchmark)
		BenchmarkInit();

	// init default scene
	g_scenes.push_back(new SceneCell("Water Balloons"));
	InitScene(g_scene);

	// create shadow maps
	renderController.SetShadowMap(renderController.shadows.ShadowCreate());

	MainLoop();
		
	if (renderController.GetFluidRenderer())
		DestroyFluidRenderer(renderController.GetFluidRenderer());

	DestroyFluidRenderBuffers(renderBuffers->fluidRenderBuffers);

	ShadowDestroy(renderController.GetShadowMap());
	DestroyRender();

	Shutdown();

	SDL_DestroyWindow(renderController.GetWindow());
	SDL_Quit();

	return 0;
}
