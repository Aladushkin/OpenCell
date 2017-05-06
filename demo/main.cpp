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
#include "imgui.h"

#include "SimBuffer.h"
#include "UIController.h"

#include "ConsoleController.h"
#include "RenderParam.h"
#include "SDLController.h"
#include "FlexController.h"

using namespace std;

int g_numSubsteps;
int g_numExtraMultiplier = 1;

bool g_tweakPanel = true;

bool g_benchmark = false;
bool g_extensions = true;
bool g_teamCity = false;
bool g_interop = true;

RenderConfig renderConfig;
SDLController sdlController;
FlexController flexController;

bool g_fullscreen = false;

GLuint g_msaaFbo;
GLuint g_msaaColorBuf;
GLuint g_msaaDepthBuf;

// a setting of -1 means Flex will use the device specified in the NVIDIA control panel
int g_device = -1;
char g_deviceName[256];
bool g_vsync = true;

FluidRenderBuffers g_fluidRenderBuffers;
DiffuseRenderBuffers g_diffuseRenderBuffers;

NvFlexSolver* g_flex;
NvFlexLibrary* g_flexLib;
NvFlexParams g_params;
NvFlexTimers g_timers;
int g_numDetailTimers;
NvFlexDetailTimer * g_detailTimers;

// mesh used for deformable object rendering
Mesh* g_mesh;
vector<int> g_meshSkinIndices;
vector<float> g_meshSkinWeights;
vector<Point3> g_meshRestPositions;
const int g_numSkinWeights = 4;

// mapping of collision mesh to render mesh
std::map<NvFlexConvexMeshId, GpuMesh*> g_convexes;
std::map<NvFlexTriangleMeshId, GpuMesh*> g_meshes;
std::map<NvFlexDistanceFieldId, GpuMesh*> g_fields;

// flag to request collision shapes be updated
bool g_shapesChanged = false;

SimBuffers* g_buffers;

bool g_warmup = false;

float g_blur;
float g_ior;
bool g_drawEllipsoids;
bool g_drawPoints;
bool g_drawMesh;
bool g_drawCloth;
float g_expandCloth;	// amount to expand cloth along normal (to account for particle radius)

bool g_drawOpaque;
int g_drawSprings;		// 0: no draw, 1: draw stretch 2: draw tether
bool g_drawBases = false;
bool g_drawContacts = false;
bool g_drawNormals = false;
bool g_drawDiffuse;
bool g_drawShapeGrid = false;
bool g_drawDensity = false;
bool g_drawRopes;
float g_pointScale;
float g_ropeScale;
float g_drawPlaneBias;	// move planes along their normal for rendering

float g_diffuseScale;
float g_diffuseMotionScale;
bool g_diffuseShadow;
float g_diffuseInscatter;
float g_diffuseOutscatter;


int g_scene = 0;
int g_levelScroll;			// offset for level selection scroll area
bool g_resetScene = false;  //if the user clicks the reset button or presses the reset key this is set to true;

int g_frame = 0;
int g_numSolidParticles = 0;

float g_mouseT = 0.0f;
Vec3 g_mousePos;

// mouse
int g_lastx;
int g_lasty;
int g_lastb = -1;

bool g_profile = false;

ShadowMap* g_shadowMap;

Vec4 g_fluidColor;
Vec4 g_diffuseColor;
Vec3 g_meshColor;
Vec3  g_clearColor;
float g_lightDistance;
float g_fogDistance;

FILE* g_ffmpeg;

void DrawShapes();

class Scene;
vector<Scene*> g_scenes;

#include "helpers.h"
#include "scenes.h"
#include "benchmark.h"
#include "FlexParams.h"

void Init(int scene, bool centerCamera = true)
{
	RandInit();

	if (g_flex)
	{
		if (g_buffers)
			delete g_buffers;

		DestroyFluidRenderBuffers(g_fluidRenderBuffers);
		DestroyDiffuseRenderBuffers(g_diffuseRenderBuffers);

		for (auto& iter : g_meshes)
		{
			NvFlexDestroyTriangleMesh(g_flexLib, iter.first);
			DestroyGpuMesh(iter.second);
		}

		for (auto& iter : g_fields)
		{
			NvFlexDestroyDistanceField(g_flexLib, iter.first);
			DestroyGpuMesh(iter.second);
		}

		for (auto& iter : g_convexes)
		{
			NvFlexDestroyConvexMesh(g_flexLib, iter.first);
			DestroyGpuMesh(iter.second);
		}


		g_fields.clear();
		g_meshes.clear();
		g_convexes.clear();

		NvFlexDestroySolver(g_flex);
		g_flex = NULL;
	}

	// alloc buffers
	g_buffers = new SimBuffers(g_flexLib);

	// map during initialization
	g_buffers->MapBuffers();
	
	// initialize buffers of particles
	g_buffers->ResizePositionVelocityPhases();
	g_buffers->ResizeRigid();
	g_buffers->ResizeSpring();
	g_buffers->ResizeShape();

	// i don't know what it is
	//g_ropes.resize(0);
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

	// reset phase 0 particle color to blue
	extern Colour gColors[];
	gColors[0] = Colour(0.0f, 0.5f, 1.0f);

	g_numSolidParticles = 0;

	g_warmup = false;

	g_maxNeighborsPerParticle = 96;
	g_numExtraParticles = 0;	// number of particles allocated but not made active	

	g_sceneLower = FLT_MAX;
	g_sceneUpper = -FLT_MAX;

	// create scene
	g_scenes[g_scene]->Initialize();

	numParticles = g_buffers->positions.size();
	maxParticles = numParticles + g_numExtraParticles*g_numExtraMultiplier;


	// calculate particle bounds
	Vec3 particleLower, particleUpper;
	GetParticleBounds(particleLower, particleUpper);

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
	if (g_mesh) 
		g_meshRestPositions = g_mesh->m_positions;
	else
		g_meshRestPositions.resize(0);

	// main create method for the Flex solver
	g_flex = NvFlexCreateSolver(g_flexLib, maxParticles, g_maxDiffuseParticles, g_maxNeighborsPerParticle);

	// give scene a chance to do some post solver initialization
	g_scenes[g_scene]->PostInitialize();

	// center camera on particles
	if (centerCamera)
	{
		g_camPos = Vec3((g_sceneLower.x + g_sceneUpper.x)*0.5f, min(g_sceneUpper.y*1.25f, 6.0f), g_sceneUpper.z + min(g_sceneUpper.y, 6.0f)*2.0f);
		g_camAngle = Vec3(0.0f, -DegToRad(15.0f), 0.0f);

		// give scene a chance to modify camera position
		g_scenes[g_scene]->CenterCamera();
	}

	// create active indices (just a contiguous block for the demo)
	g_buffers->ResizeActiveIndices();

	// resize particle buffers to fit
	g_buffers->ResizeFitCell();

	// builds rigids constraints
	if (g_buffers->rigidOffsets.size())
	{
		assert(g_buffers->rigidOffsets.size() > 1);

		const int numRigids = g_buffers->rigidOffsets.size() - 1;

		// calculate local rest space positions
		g_buffers->rigidLocalPositions.resize(g_buffers->rigidOffsets.back());
		CalculateRigidLocalPositions(&g_buffers->positions[0], g_buffers->positions.size(), &g_buffers->rigidOffsets[0], &g_buffers->rigidIndices[0], numRigids, &g_buffers->rigidLocalPositions[0]);

		g_buffers->rigidRotations.resize(g_buffers->rigidOffsets.size() - 1, Quat());
		g_buffers->rigidTranslations.resize(g_buffers->rigidOffsets.size() - 1, Vec3());

	}

	// unmap so we can start transferring data to GPU
	g_buffers->UnmapBuffers();

	//-----------------------------
	// Send data to Flex

	NvFlexSetParams(g_flex, &g_params);
	NvFlexSetParticles(g_flex, g_buffers->positions.buffer, numParticles);
	NvFlexSetVelocities(g_flex, g_buffers->velocities.buffer, numParticles);
	NvFlexSetNormals(g_flex, g_buffers->normals.buffer, numParticles);
	NvFlexSetPhases(g_flex, g_buffers->phases.buffer, g_buffers->phases.size());
	NvFlexSetRestParticles(g_flex, g_buffers->restPositions.buffer, g_buffers->restPositions.size());

	NvFlexSetActive(g_flex, g_buffers->activeIndices.buffer, numParticles);

	// springs
	if (g_buffers->springIndices.size())
	{
		assert((g_buffers->springIndices.size() & 1) == 0);
		assert((g_buffers->springIndices.size() / 2) == g_buffers->springLengths.size());

		NvFlexSetSprings(g_flex, g_buffers->springIndices.buffer, g_buffers->springLengths.buffer, g_buffers->springStiffness.buffer, g_buffers->springLengths.size());
	}

	// rigids
	if (g_buffers->rigidOffsets.size())
	{
		NvFlexSetRigids(g_flex, g_buffers->rigidOffsets.buffer, g_buffers->rigidIndices.buffer, g_buffers->rigidLocalPositions.buffer, g_buffers->rigidLocalNormals.buffer, g_buffers->rigidCoefficients.buffer, g_buffers->rigidRotations.buffer, g_buffers->rigidTranslations.buffer, g_buffers->rigidOffsets.size() - 1, g_buffers->rigidIndices.size());
	}

	// inflatables
	if (g_buffers->inflatableTriOffsets.size())
	{
		NvFlexSetInflatables(g_flex, g_buffers->inflatableTriOffsets.buffer, g_buffers->inflatableTriCounts.buffer, g_buffers->inflatableVolumes.buffer, g_buffers->inflatablePressures.buffer, g_buffers->inflatableCoefficients.buffer, g_buffers->inflatableTriOffsets.size());
	}

	// dynamic triangles
	if (g_buffers->triangles.size())
	{
		NvFlexSetDynamicTriangles(g_flex, g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);
	}

	// collision shapes
	if (g_buffers->shapeFlags.size())
	{
		NvFlexSetShapes(
			g_flex,
			g_buffers->shapeGeometry.buffer,
			g_buffers->shapePositions.buffer,
			g_buffers->shapeRotations.buffer,
			g_buffers->shapePrevPositions.buffer,
			g_buffers->shapePrevRotations.buffer,
			g_buffers->shapeFlags.buffer,
			int(g_buffers->shapeFlags.size()));
	}

	// create render buffers
	g_fluidRenderBuffers = CreateFluidRenderBuffers(maxParticles, g_interop);
	g_diffuseRenderBuffers = CreateDiffuseRenderBuffers(g_maxDiffuseParticles, g_interop);

	// perform initial sim warm up
	if (g_warmup)
	{
		printf("Warming up sim..\n");

		// warm it up (relax positions to reach rest density without affecting velocity)
		NvFlexParams copy = g_params;
		copy.numIterations = 4;

		NvFlexSetParams(g_flex, &copy);

		const int kWarmupIterations = 100;

		for (int i = 0; i < kWarmupIterations; ++i)
		{
			NvFlexUpdateSolver(g_flex, 0.0001f, 1, false);
			NvFlexSetVelocities(g_flex, g_buffers->velocities.buffer, maxParticles);
		}

		// udpate host copy
		NvFlexGetParticles(g_flex, g_buffers->positions.buffer, g_buffers->positions.size());
		NvFlexGetSmoothParticles(g_flex, g_buffers->smoothPositions.buffer, g_buffers->smoothPositions.size());
		NvFlexGetAnisotropy(g_flex, g_buffers->anisotropy1.buffer, g_buffers->anisotropy2.buffer, g_buffers->anisotropy3.buffer);

		printf("Finished warm up.\n");
	}
}

void Reset()
{
	Init(g_scene, false);
}

void Shutdown()
{
	// free buffers
	delete g_buffers;

	for (auto& iter : g_meshes)
	{
		NvFlexDestroyTriangleMesh(g_flexLib, iter.first);
		DestroyGpuMesh(iter.second);
	}

	for (auto& iter : g_fields)
	{
		NvFlexDestroyDistanceField(g_flexLib, iter.first);
		DestroyGpuMesh(iter.second);
	}

	for (auto& iter : g_convexes)
	{
		NvFlexDestroyConvexMesh(g_flexLib, iter.first);
		DestroyGpuMesh(iter.second);
	}

	g_fields.clear();
	g_meshes.clear();

	NvFlexDestroySolver(g_flex);
	NvFlexShutdown(g_flexLib);
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

void RenderScene()
{
	const int numParticles = NvFlexGetActiveCount(g_flex);
	const int numDiffuse = NvFlexGetDiffuseParticles(g_flex, NULL, NULL, NULL);

	//---------------------------------------------------
	// use VBO buffer wrappers to allow Flex to write directly to the OpenGL buffers
	// Flex will take care of any CUDA interop mapping/unmapping during the get() operations

	if (numParticles)
	{

		if (g_interop)
		{
			// copy data directly from solver to the renderer buffers
			UpdateFluidRenderBuffers(g_fluidRenderBuffers, g_flex, g_drawEllipsoids, g_drawDensity);
		}
		else
		{
			// copy particle data to GPU render device

			if (g_drawEllipsoids)
			{
				// if fluid surface rendering then update with smooth positions and anisotropy
				UpdateFluidRenderBuffers(g_fluidRenderBuffers,
					&g_buffers->smoothPositions[0],
					(g_drawDensity) ? &g_buffers->densities[0] : (float*)&g_buffers->phases[0],
					&g_buffers->anisotropy1[0],
					&g_buffers->anisotropy2[0],
					&g_buffers->anisotropy3[0],
					g_buffers->positions.size(),
					&g_buffers->activeIndices[0],
					numParticles);
			}
			else
			{
				// otherwise just send regular positions and no anisotropy
				UpdateFluidRenderBuffers(g_fluidRenderBuffers,
					&g_buffers->positions[0],
					(float*)&g_buffers->phases[0],
					NULL, NULL, NULL,
					g_buffers->positions.size(),
					&g_buffers->activeIndices[0],
					numParticles);
			}
		}
	}
	
	if (numDiffuse)
	{
		if (g_interop)
		{
			// copy data directly from solver to the renderer buffers
			UpdateDiffuseRenderBuffers(g_diffuseRenderBuffers, g_flex);
		}
		else
		{
			// copy diffuse particle data from host to GPU render device 
			UpdateDiffuseRenderBuffers(g_diffuseRenderBuffers,
				&g_buffers->diffusePositions[0],
				&g_buffers->diffuseVelocities[0],
				&g_buffers->diffuseIndices[0],
				numDiffuse);
		}
	}
	
	//---------------------------------------
	// setup view and state

	float fov = kPi / 4.0f;
	float aspect = float(renderConfig.GetWidth()) / float(renderConfig.GetHeight());

	Matrix44 proj = ProjectionMatrix(RadToDeg(fov), aspect, g_camNear, g_camFar);
	Matrix44 view = RotationMatrix(-g_camAngle.x, Vec3(0.0f, 1.0f, 0.0f))*RotationMatrix(-g_camAngle.y, Vec3(cosf(-g_camAngle.x), 0.0f, sinf(-g_camAngle.x)))*TranslationMatrix(-Point3(g_camPos));

	//------------------------------------
	// lighting pass

	// expand scene bounds to fit most scenes
	g_sceneLower = Min(g_sceneLower, Vec3(-2.0f, 0.0f, -2.0f));
	g_sceneUpper = Max(g_sceneUpper, Vec3(2.0f, 2.0f, 2.0f));

	Vec3 sceneExtents = g_sceneUpper - g_sceneLower;
	Vec3 sceneCenter = 0.5f*(g_sceneUpper + g_sceneLower);

	g_lightDir = Normalize(Vec3(5.0f, 15.0f, 7.5f));
	g_lightPos = sceneCenter + g_lightDir*Length(sceneExtents)*g_lightDistance;
	g_lightTarget = sceneCenter;

	// calculate tight bounds for shadow frustum
	float lightFov = 2.0f*atanf(Length(g_sceneUpper - sceneCenter) / Length(g_lightPos - sceneCenter));

	// scale and clamp fov for aesthetics
	lightFov = Clamp(lightFov, DegToRad(25.0f), DegToRad(65.0f));

	Matrix44 lightPerspective = ProjectionMatrix(RadToDeg(lightFov), 1.0f, 1.0f, 1000.0f);
	Matrix44 lightView = LookAtMatrix(Point3(g_lightPos), Point3(g_lightTarget));
	Matrix44 lightTransform = lightPerspective*lightView;

	// non-fluid particles maintain radius distance (not 2.0f*radius) so multiply by a half
	float radius = g_params.solidRestDistance;

	// fluid particles overlap twice as much again, so half the radius again
	if (g_params.fluid)
		radius = g_params.fluidRestDistance;

	radius *= 0.5f;
	radius *= g_pointScale;

	//-------------------------------------
	// shadowing pass 

	if (g_meshSkinIndices.size())
		SkinMesh();

	// create shadow maps
	ShadowBegin(g_shadowMap);

	SetView(lightView, lightPerspective);
	SetCullMode(false);

	// give scene a chance to do custom drawing
	g_scenes[g_scene]->Draw(1);

	if (g_drawMesh)
		DrawMesh(g_mesh, g_meshColor);

	DrawShapes();

	if (g_drawCloth && g_buffers->triangles.size())
	{
		DrawCloth(&g_buffers->positions[0], &g_buffers->normals[0], g_buffers->uvs.size() ? &g_buffers->uvs[0].x : NULL, &g_buffers->triangles[0], g_buffers->triangles.size() / 3, g_buffers->positions.size(), 3, g_expandCloth);
	}

	int shadowParticles = numParticles;
	int shadowParticlesOffset = 0;

	if (!g_drawPoints)
	{
		shadowParticles = 0;

		if (g_drawEllipsoids && g_params.fluid)
		{
			shadowParticles = numParticles - g_numSolidParticles;
			shadowParticlesOffset = g_numSolidParticles;
		}
	}
	else
	{
		int offset = g_drawMesh ? g_numSolidParticles : 0;

		shadowParticles = numParticles - offset;
		shadowParticlesOffset = offset;
	}

	if (g_buffers->activeIndices.size())
		DrawPoints(g_fluidRenderBuffers.mPositionVBO, g_fluidRenderBuffers.mDensityVBO, g_fluidRenderBuffers.mIndices, shadowParticles, shadowParticlesOffset, radius, 2048, 1.0f, lightFov, g_lightPos, g_lightTarget, lightTransform, g_shadowMap, g_drawDensity);

	ShadowEnd();

	//----------------
	// lighting pass

	BindSolidShader(g_lightPos, g_lightTarget, lightTransform, g_shadowMap, 0.0f, Vec4(g_clearColor, g_fogDistance));

	SetView(view, proj);
	SetCullMode(true);

	DrawPlanes((Vec4*)g_params.planes, g_params.numPlanes, g_drawPlaneBias);

	if (g_drawMesh)
		DrawMesh(g_mesh, g_meshColor);


	DrawShapes();

	if (g_drawCloth && g_buffers->triangles.size())
		DrawCloth(&g_buffers->positions[0], &g_buffers->normals[0], g_buffers->uvs.size() ? &g_buffers->uvs[0].x : NULL, &g_buffers->triangles[0], g_buffers->triangles.size() / 3, g_buffers->positions.size(), 3, g_expandCloth);

	// give scene a chance to do custom drawing
	g_scenes[g_scene]->Draw(0);

	UnbindSolidShader();


	// first pass of diffuse particles (behind fluid surface)
	if (g_drawDiffuse)
		RenderDiffuse(g_fluidRenderer, g_diffuseRenderBuffers, numDiffuse, radius*g_diffuseScale, float(renderConfig.GetWidth()), aspect, fov, g_diffuseColor, g_lightPos, g_lightTarget, lightTransform, g_shadowMap, g_diffuseMotionScale, g_diffuseInscatter, g_diffuseOutscatter, g_diffuseShadow, false);

	if (g_drawEllipsoids && g_params.fluid)
	{
		// draw solid particles separately
		if (g_numSolidParticles && g_drawPoints)
			DrawPoints(g_fluidRenderBuffers.mPositionVBO, g_fluidRenderBuffers.mDensityVBO, g_fluidRenderBuffers.mIndices, g_numSolidParticles, 0, radius, float(renderConfig.GetWidth()), aspect, fov, g_lightPos, g_lightTarget, lightTransform, g_shadowMap, g_drawDensity);

		// render fluid surface
		RenderEllipsoids(g_fluidRenderer, g_fluidRenderBuffers, numParticles - g_numSolidParticles, g_numSolidParticles, radius, float(renderConfig.GetWidth()), aspect, fov, g_lightPos, g_lightTarget, lightTransform, g_shadowMap, g_fluidColor, g_blur, g_ior, g_drawOpaque);

		// second pass of diffuse particles for particles in front of fluid surface
		if (g_drawDiffuse)
			RenderDiffuse(g_fluidRenderer, g_diffuseRenderBuffers, numDiffuse, radius*g_diffuseScale, float(renderConfig.GetWidth()), aspect, fov, g_diffuseColor, g_lightPos, g_lightTarget, lightTransform, g_shadowMap, g_diffuseMotionScale, g_diffuseInscatter, g_diffuseOutscatter, g_diffuseShadow, true);
	}
	else
	{
		// draw all particles as spheres
		if (g_drawPoints)
		{
			int offset = g_drawMesh ? g_numSolidParticles : 0;

			if (g_buffers->activeIndices.size())
				DrawPoints(g_fluidRenderBuffers.mPositionVBO, g_fluidRenderBuffers.mDensityVBO, g_fluidRenderBuffers.mIndices, numParticles - offset, offset, radius, float(renderConfig.GetWidth()), aspect, fov, g_lightPos, g_lightTarget, lightTransform, g_shadowMap, g_drawDensity);
		}
	}

}

void RenderDebug()
{
	// springs
	if (g_drawSprings)
	{
		Vec4 color;

		if (g_drawSprings == 1)
		{
			// stretch 
			color = Vec4(0.0f, 0.0f, 1.0f, 0.8f);
		}
		if (g_drawSprings == 2)
		{
			// tether
			color = Vec4(0.0f, 1.0f, 0.0f, 0.8f);
		}

		BeginLines();

		int start = 0;

		for (int i = start; i < g_buffers->springLengths.size(); ++i)
		{
			if (g_drawSprings == 1 && g_buffers->springStiffness[i] < 0.0f)
				continue;
			if (g_drawSprings == 2 && g_buffers->springStiffness[i] > 0.0f)
				continue;

			int a = g_buffers->springIndices[i * 2];
			int b = g_buffers->springIndices[i * 2 + 1];

			DrawLine(Vec3(g_buffers->positions[a]), Vec3(g_buffers->positions[b]), color);
		}

		EndLines();
	}

	// visualize contacts against the environment
	if (g_drawContacts)
	{
		const int maxContactsPerParticle = 6;

		NvFlexVector<Vec4> contactPlanes(g_flexLib, g_buffers->positions.size()*maxContactsPerParticle);
		NvFlexVector<Vec4> contactVelocities(g_flexLib, g_buffers->positions.size()*maxContactsPerParticle);
		NvFlexVector<int> contactIndices(g_flexLib, g_buffers->positions.size());
		NvFlexVector<unsigned int> contactCounts(g_flexLib, g_buffers->positions.size());

		NvFlexGetContacts(g_flex, contactPlanes.buffer, contactVelocities.buffer, contactIndices.buffer, contactCounts.buffer);

		// ensure transfers have finished
		contactPlanes.map();
		contactVelocities.map();
		contactIndices.map();
		contactCounts.map();

		BeginLines();

		for (int i = 0; i < int(g_buffers->activeIndices.size()); ++i)
		{
			const int contactIndex = contactIndices[g_buffers->activeIndices[i]];
			const unsigned int count = contactCounts[contactIndex];

			const float scale = 0.1f;

			for (unsigned int c = 0; c < count; ++c)
			{
				Vec4 plane = contactPlanes[contactIndex*maxContactsPerParticle + c];

				DrawLine(Vec3(g_buffers->positions[g_buffers->activeIndices[i]]), 
						 Vec3(g_buffers->positions[g_buffers->activeIndices[i]]) + Vec3(plane)*scale,
						 Vec4(0.0f, 1.0f, 0.0f, 0.0f));
			}
		}

		EndLines();
	}
	
	if (g_drawBases)
	{
		for (int i = 0; i < int(g_buffers->rigidRotations.size()); ++i)
		{
			BeginLines();

			float size = 0.1f;

			for (int b = 0; b < 3; ++b)
			{
				Vec3 color;
				color[b] = 1.0f;
			
				Matrix33 frame(g_buffers->rigidRotations[i]);

				DrawLine(Vec3(g_buffers->rigidTranslations[i]),
						 Vec3(g_buffers->rigidTranslations[i] + frame.cols[b] * size),
						 Vec4(color, 0.0f));
			}

			EndLines();
		}
	}

	if (g_drawNormals)
	{
		NvFlexGetNormals(g_flex, g_buffers->normals.buffer, g_buffers->normals.size());

		BeginLines();

		for (int i = 0; i < g_buffers->normals.size(); ++i)
		{
			DrawLine(Vec3(g_buffers->positions[i]),
					 Vec3(g_buffers->positions[i] - g_buffers->normals[i] * g_buffers->normals[i].w),
					 Vec4(0.0f, 1.0f, 0.0f, 0.0f));
		}

		EndLines();
	}
}

void DrawShapes()
{
	for (int i = 0; i < g_buffers->shapeFlags.size(); ++i)
	{
		const int flags = g_buffers->shapeFlags[i];

		// unpack flags
		int type = int(flags&eNvFlexShapeFlagTypeMask);
		//bool dynamic = int(flags&eNvFlexShapeFlagDynamic) > 0;

		Vec3 color = Vec3(0.9f);

		if (flags & eNvFlexShapeFlagTrigger)
		{
			color = Vec3(0.6f, 1.0, 0.6f);

			SetFillMode(true);		
		}

		// render with prev positions to match particle update order
		// can also think of this as current/next
		const Quat rotation = g_buffers->shapePrevRotations[i];
		const Vec3 position = Vec3(g_buffers->shapePrevPositions[i]);

		NvFlexCollisionGeometry geo = g_buffers->shapeGeometry[i];

		if (type == eNvFlexShapeSphere)
		{
			Mesh* sphere = CreateSphere(20, 20, geo.sphere.radius);

			Matrix44 xform = TranslationMatrix(Point3(position))*RotationMatrix(Quat(rotation));
			sphere->Transform(xform);

			DrawMesh(sphere, Vec3(color));

			delete sphere;
		}
		else if (type == eNvFlexShapeCapsule)
		{
			Mesh* capsule = CreateCapsule(10, 20, geo.capsule.radius, geo.capsule.halfHeight);

			// transform to world space
			Matrix44 xform = TranslationMatrix(Point3(position))*RotationMatrix(Quat(rotation))*RotationMatrix(DegToRad(-90.0f), Vec3(0.0f, 0.0f, 1.0f));
			capsule->Transform(xform);

			DrawMesh(capsule, Vec3(color));

			delete capsule;
		}
		else if (type == eNvFlexShapeBox)
		{
			Mesh* box = CreateCubeMesh();

			Matrix44 xform = TranslationMatrix(Point3(position))*RotationMatrix(Quat(rotation))*ScaleMatrix(Vec3(geo.box.halfExtents)*2.0f);
			box->Transform(xform);

			DrawMesh(box, Vec3(color));
			delete box;			
		}
		else if (type == eNvFlexShapeConvexMesh)
		{
			if (g_convexes.find(geo.convexMesh.mesh) != g_convexes.end())
			{
				GpuMesh* m = g_convexes[geo.convexMesh.mesh];

				if (m)
				{
					Matrix44 xform = TranslationMatrix(Point3(g_buffers->shapePositions[i]))*RotationMatrix(Quat(g_buffers->shapeRotations[i]))*ScaleMatrix(geo.convexMesh.scale);
					DrawGpuMesh(m, xform, Vec3(color));
				}
			}
		}
		else if (type == eNvFlexShapeTriangleMesh)
		{
			if (g_meshes.find(geo.triMesh.mesh) != g_meshes.end())
			{
				GpuMesh* m = g_meshes[geo.triMesh.mesh];

				if (m)
				{
					Matrix44 xform = TranslationMatrix(Point3(position))*RotationMatrix(Quat(rotation))*ScaleMatrix(geo.triMesh.scale);
					DrawGpuMesh(m, xform, Vec3(color));
				}
			}
		}
		else if (type == eNvFlexShapeSDF)
		{
			if (g_fields.find(geo.sdf.field) != g_fields.end())
			{
				GpuMesh* m = g_fields[geo.sdf.field];

				if (m)
				{
					Matrix44 xform = TranslationMatrix(Point3(position))*RotationMatrix(Quat(rotation))*ScaleMatrix(geo.sdf.scale);
					DrawGpuMesh(m, xform, Vec3(color));
				}
			}
		}
	}

	SetFillMode(g_wireframe);
}


// returns the new scene if one is selected
int DoUI()
{
	// gui may set a new scene
	int newScene = -1;

	if (g_showHelp)
	{
		const int numParticles = NvFlexGetActiveCount(g_flex);
		const int numDiffuse = NvFlexGetDiffuseParticles(g_flex, NULL, NULL, NULL);

		int x = renderConfig.GetWidth() - 200;
		int y = renderConfig.GetHeight() - 23;

		// imgui
		unsigned char button = 0;
		if (g_lastb == SDL_BUTTON_LEFT)
			button = IMGUI_MBUT_LEFT;
		else if (g_lastb == SDL_BUTTON_RIGHT)
			button = IMGUI_MBUT_RIGHT;

		imguiBeginFrame(g_lastx, renderConfig.GetHeight() - g_lasty, button, 0);

		x += 180;

		int fontHeight = 13;

		if (1)
		{
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Frame: %d", g_frame); y -= fontHeight * 2;

			if (!g_ffmpeg)
			{
				DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Frame Time: %.2fms", g_realdt*1000.0f); y -= fontHeight * 2;

				// If detailed profiling is enabled, then these timers will contain the overhead of the detail timers, so we won't display them.
				if (!g_profile)
				{
					DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Sim Time (CPU): %.2fms", g_updateTime*1000.0f); y -= fontHeight;
					DrawImguiString(x, y, Vec3(0.97f, 0.59f, 0.27f), IMGUI_ALIGN_RIGHT, "Sim Latency (GPU): %.2fms", g_simLatency); y -= fontHeight * 2;
				}
				else
				{
					y -= fontHeight * 3;
				}
			}

			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Particle Count: %d", numParticles); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Diffuse Count: %d", numDiffuse); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Rigid Count: %d", g_buffers->rigidOffsets.size() > 0 ? g_buffers->rigidOffsets.size() - 1 : 0); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Spring Count: %d", g_buffers->springLengths.size()); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Num Substeps: %d", g_numSubsteps); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Num Iterations: %d", g_params.numIterations); y -= fontHeight * 2;

			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Device: %s", g_deviceName); y -= fontHeight * 2;
		}

		if (g_profile)
		{
			DrawImguiString(x, y, Vec3(0.97f, 0.59f, 0.27f), IMGUI_ALIGN_RIGHT, "Total GPU Sim Latency: %.2fms", g_timers.total); y -= fontHeight * 2;

			DrawImguiString(x, y, Vec3(0.0f, 1.0f, 0.0f), IMGUI_ALIGN_RIGHT, "GPU Latencies"); y -= fontHeight;

			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Predict: %.2fms", g_timers.predict); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Create Cell Indices: %.2fms", g_timers.createCellIndices); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Sort Cell Indices: %.2fms", g_timers.sortCellIndices); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Reorder: %.2fms", g_timers.reorder); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "CreateGrid: %.2fms", g_timers.createGrid); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Collide Particles: %.2fms", g_timers.collideParticles); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Collide Shapes: %.2fms", g_timers.collideShapes); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Collide Triangles: %.2fms", g_timers.collideTriangles); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Calculate Density: %.2fms", g_timers.calculateDensity); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Solve Densities: %.2fms", g_timers.solveDensities); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Solve Velocities: %.2fms", g_timers.solveVelocities); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Solve Rigids: %.2fms", g_timers.solveShapes); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Solve Springs: %.2fms", g_timers.solveSprings); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Solve Inflatables: %.2fms", g_timers.solveInflatables); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Solve Contacts: %.2fms", g_timers.solveContacts); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Apply Deltas: %.2fms", g_timers.applyDeltas); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Finalize: %.2fms", g_timers.finalize); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Update Triangles: %.2fms", g_timers.updateTriangles); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Update Normals: %.2fms", g_timers.updateNormals); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Update Bounds: %.2fms", g_timers.updateBounds); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Calculate Anisotropy: %.2fms", g_timers.calculateAnisotropy); y -= fontHeight;
			DrawImguiString(x, y, Vec3(1.0f), IMGUI_ALIGN_RIGHT, "Update Diffuse: %.2fms", g_timers.updateDiffuse); y -= fontHeight * 2;
		}

		x -= 180;

		int uiOffset = 250;
		int uiBorder = 20;
		int uiWidth = 200;
		int uiHeight = renderConfig.GetHeight() - uiOffset - uiBorder * 3;
		int uiLeft = uiBorder;

		if (g_tweakPanel)
		{
			static int scroll = 0;

			imguiBeginScrollArea("Options", uiLeft, renderConfig.GetHeight() - uiBorder - uiHeight  - uiBorder, uiWidth, uiHeight, &scroll);
			imguiSeparatorLine();

			// global options
			imguiLabel("Global");

			if (imguiCheck("Pause", g_pause))
				g_pause = !g_pause;

			imguiSeparatorLine();

			if (imguiCheck("Wireframe", g_wireframe))
				g_wireframe = !g_wireframe;

			if (imguiCheck("Draw Points", g_drawPoints))
				g_drawPoints = !g_drawPoints;

			if (imguiCheck("Draw Fluid", g_drawEllipsoids))
				g_drawEllipsoids = !g_drawEllipsoids;

			if (imguiCheck("Draw Mesh", g_drawMesh))
			{
				g_drawMesh = !g_drawMesh;
				g_drawRopes = !g_drawRopes;
			}

			if (imguiCheck("Draw Basis", g_drawBases))
				g_drawBases = !g_drawBases;

			if (imguiCheck("Draw Springs", bool(g_drawSprings != 0)))
				g_drawSprings = (g_drawSprings) ? 0 : 1;

			if (imguiCheck("Draw Contacts", g_drawContacts))
				g_drawContacts = !g_drawContacts;

			imguiSeparatorLine();

			// scene options
			g_scenes[g_scene]->DoGui();

		if (imguiButton("Reset Scene"))
			g_resetScene = true;

			imguiSeparatorLine();

			float n = float(g_numSubsteps);
			if (imguiSlider("Num Substeps", &n, 1, 10, 1))
				g_numSubsteps = int(n);

			n = float(g_params.numIterations);
			if (imguiSlider("Num Iterations", &n, 1, 20, 1))
				g_params.numIterations = int(n);

			imguiSeparatorLine();
			imguiSlider("Gravity X", &g_params.gravity[0], -50.0f, 50.0f, 1.0f);
			imguiSlider("Gravity Y", &g_params.gravity[1], -50.0f, 50.0f, 1.0f);
			imguiSlider("Gravity Z", &g_params.gravity[2], -50.0f, 50.0f, 1.0f);

			imguiSeparatorLine();
			imguiSlider("Radius", &g_params.radius, 0.01f, 0.5f, 0.01f);
			imguiSlider("Solid Radius", &g_params.solidRestDistance, 0.0f, 0.5f, 0.001f);
			imguiSlider("Fluid Radius", &g_params.fluidRestDistance, 0.0f, 0.5f, 0.001f);

			// common params
			imguiSeparatorLine();
			imguiSlider("Dynamic Friction", &g_params.dynamicFriction, 0.0f, 1.0f, 0.01f);
			imguiSlider("Static Friction", &g_params.staticFriction, 0.0f, 1.0f, 0.01f);
			imguiSlider("Particle Friction", &g_params.particleFriction, 0.0f, 1.0f, 0.01f);
			imguiSlider("Restitution", &g_params.restitution, 0.0f, 1.0f, 0.01f);
			imguiSlider("SleepThreshold", &g_params.sleepThreshold, 0.0f, 1.0f, 0.01f);
			imguiSlider("Shock Propagation", &g_params.shockPropagation, 0.0f, 10.0f, 0.01f);
			imguiSlider("Damping", &g_params.damping, 0.0f, 10.0f, 0.01f);
			imguiSlider("Dissipation", &g_params.dissipation, 0.0f, 0.01f, 0.0001f);
			imguiSlider("SOR", &g_params.relaxationFactor, 0.0f, 5.0f, 0.01f);

			imguiSlider("Collision Distance", &g_params.collisionDistance, 0.0f, 0.5f, 0.001f);
			imguiSlider("Collision Margin", &g_params.shapeCollisionMargin, 0.0f, 5.0f, 0.01f);

			// rigid params
			imguiSeparatorLine();
			imguiSlider("Plastic Creep", &g_params.plasticCreep, 0.0f, 1.0f, 0.001f);
			imguiSlider("Plastic Threshold", &g_params.plasticThreshold, 0.0f, 0.5f, 0.001f);

			// cloth params
			imguiSeparatorLine();
			imguiSlider("Drag", &g_params.drag, 0.0f, 1.0f, 0.01f);
			imguiSlider("Lift", &g_params.lift, 0.0f, 1.0f, 0.01f);
			imguiSeparatorLine();

			// fluid params
			if (imguiCheck("Fluid", g_params.fluid))
				g_params.fluid = !g_params.fluid;

			imguiSlider("Adhesion", &g_params.adhesion, 0.0f, 10.0f, 0.01f);
			imguiSlider("Cohesion", &g_params.cohesion, 0.0f, 0.2f, 0.0001f);
			imguiSlider("Surface Tension", &g_params.surfaceTension, 0.0f, 50.0f, 0.01f);
			imguiSlider("Viscosity", &g_params.viscosity, 0.0f, 120.0f, 0.01f);
			imguiSlider("Vorticicty Confinement", &g_params.vorticityConfinement, 0.0f, 120.0f, 0.1f);
			imguiSlider("Solid Pressure", &g_params.solidPressure, 0.0f, 1.0f, 0.01f);
			imguiSlider("Surface Drag", &g_params.freeSurfaceDrag, 0.0f, 1.0f, 0.01f);
			imguiSlider("Buoyancy", &g_params.buoyancy, -1.0f, 1.0f, 0.01f);

			imguiSeparatorLine();
			imguiSlider("Anisotropy Scale", &g_params.anisotropyScale, 0.0f, 30.0f, 0.01f);
			imguiSlider("Smoothing", &g_params.smoothing, 0.0f, 1.0f, 0.01f);

			// diffuse params
			imguiSeparatorLine();
			imguiSlider("Diffuse Threshold", &g_params.diffuseThreshold, 0.0f, 1000.0f, 1.0f);
			imguiSlider("Diffuse Buoyancy", &g_params.diffuseBuoyancy, 0.0f, 2.0f, 0.01f);
			imguiSlider("Diffuse Drag", &g_params.diffuseDrag, 0.0f, 2.0f, 0.01f);
			imguiSlider("Diffuse Scale", &g_diffuseScale, 0.0f, 1.5f, 0.01f);
			imguiSlider("Diffuse Alpha", &g_diffuseColor.w, 0.0f, 3.0f, 0.01f);
			imguiSlider("Diffuse Inscatter", &g_diffuseInscatter, 0.0f, 2.0f, 0.01f);
			imguiSlider("Diffuse Outscatter", &g_diffuseOutscatter, 0.0f, 2.0f, 0.01f);
			imguiSlider("Diffuse Motion Blur", &g_diffuseMotionScale, 0.0f, 5.0f, 0.1f);

			n = float(g_params.diffuseBallistic);
			if (imguiSlider("Diffuse Ballistic", &n, 1, 40, 1))
				g_params.diffuseBallistic = int(n);

			imguiEndScrollArea();
		}
		imguiEndFrame();

		// kick render commands
		imguiGraphDraw();
	}

	// update benchmark and change scene if one is requested
	if (g_benchmark)
		newScene = BenchmarkUpdate();

	return newScene;
}

void UpdateFrame()
{
	static double lastTime;

	// real elapsed frame time
	double frameBeginTime = GetSeconds();

	g_realdt = float(frameBeginTime - lastTime);
	lastTime = frameBeginTime;

	// do gamepad input polling
	double currentTime = frameBeginTime;
	static double lastJoyTime = currentTime;

	if (g_gamecontroller && currentTime - lastJoyTime > g_dt)
	{
		lastJoyTime = currentTime;

		int leftStickX = SDL_GameControllerGetAxis(g_gamecontroller, SDL_CONTROLLER_AXIS_LEFTX);
		int leftStickY = SDL_GameControllerGetAxis(g_gamecontroller, SDL_CONTROLLER_AXIS_LEFTY);
		int rightStickX = SDL_GameControllerGetAxis(g_gamecontroller, SDL_CONTROLLER_AXIS_RIGHTX);
		int rightStickY = SDL_GameControllerGetAxis(g_gamecontroller, SDL_CONTROLLER_AXIS_RIGHTY);
		int leftTrigger = SDL_GameControllerGetAxis(g_gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
		int rightTrigger = SDL_GameControllerGetAxis(g_gamecontroller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

		Vec2 leftStick(joyAxisFilter(leftStickX, 0), joyAxisFilter(leftStickY, 0));
		Vec2 rightStick(joyAxisFilter(rightStickX, 1), joyAxisFilter(rightStickY, 1));
		Vec2 trigger(leftTrigger / 32768.0f, rightTrigger / 32768.0f);

		if (leftStick.x != 0.0f || leftStick.y != 0.0f ||
			rightStick.x != 0.0f || rightStick.y != 0.0f)
		{
			// note constant factor to speed up analog control compared to digital because it is more controllable.
			g_camVel.z = -4 * g_camSpeed * leftStick.y;
			g_camVel.x = 4 * g_camSpeed * leftStick.x;

			// cam orientation
			g_camAngle.x -= rightStick.x * 0.05f;
			g_camAngle.y -= rightStick.y * 0.05f;
		}

		// Handle left stick motion
		static bool bLeftStick = false;

		if ((leftStick.x != 0.0f || leftStick.y != 0.0f) && !bLeftStick)
		{
			bLeftStick = true;
		}
		else if ((leftStick.x == 0.0f && leftStick.y == 0.0f) && bLeftStick)
		{
			bLeftStick = false;
			g_camVel.z = -4 * g_camSpeed * leftStick.y;
			g_camVel.x = 4 * g_camSpeed * leftStick.x;
		}

		// Handle triggers as controller button events
		void ControllerButtonEvent(SDL_ControllerButtonEvent event);

		static bool bLeftTrigger = false;
		static bool bRightTrigger = false;
		SDL_ControllerButtonEvent e;

		if (!bLeftTrigger && trigger.x > 0.0f)
		{
			e.type = SDL_CONTROLLERBUTTONDOWN;
			e.button = SDL_CONTROLLER_BUTTON_LEFT_TRIGGER;
			ControllerButtonEvent(e);
			bLeftTrigger = true;
		}
		else if (bLeftTrigger && trigger.x == 0.0f)
		{
			e.type = SDL_CONTROLLERBUTTONUP;
			e.button = SDL_CONTROLLER_BUTTON_LEFT_TRIGGER;
			ControllerButtonEvent(e);
			bLeftTrigger = false;
		}

		if (!bRightTrigger && trigger.y > 0.0f)
		{
			e.type = SDL_CONTROLLERBUTTONDOWN;
			e.button = SDL_CONTROLLER_BUTTON_RIGHT_TRIGGER;
			ControllerButtonEvent(e);
			bRightTrigger = true;
		}
		else if (bRightTrigger && trigger.y == 0.0f)
		{
			e.type = SDL_CONTROLLERBUTTONDOWN;
			e.button = SDL_CONTROLLER_BUTTON_RIGHT_TRIGGER;
			ControllerButtonEvent(e);
			bRightTrigger = false;
		}
	}

	//-------------------------------------------------------------------
	// Scene Update

	double waitBeginTime = GetSeconds();

	g_buffers->MapBuffers();

	double waitEndTime = GetSeconds();

	UpdateCamera();

	if (!g_pause || g_step)	
	{	
		UpdateScene();
	}

	//-------------------------------------------------------------------
	// Render

	double renderBeginTime = GetSeconds();

	if (g_profile && (!g_pause || g_step)) {
		if (g_benchmark) {
			g_numDetailTimers = NvFlexGetDetailTimers(g_flex, &g_detailTimers);
		}
		else {
			memset(&g_timers, 0, sizeof(g_timers));
			NvFlexGetTimers(g_flex, &g_timers);
		}
	}

	float newSimLatency = NvFlexGetDeviceLatency(g_flex);

	StartFrame(Vec4(g_clearColor, 1.0f));

	// main scene render
	RenderScene();
	RenderDebug();
	
	EndFrame();

	const int newScene = DoUI();

	g_buffers->UnmapBuffers();

	if (g_capture)
	{
		TgaImage img;
		img.m_width = renderConfig.GetWidth();
		img.m_height = renderConfig.GetHeight();
		img.m_data = new uint32_t[renderConfig.GetWidth()*renderConfig.GetHeight()];

		ReadFrame((int*)img.m_data, renderConfig.GetWidth(), renderConfig.GetHeight());

		fwrite(img.m_data, sizeof(uint32_t)*renderConfig.GetWidth()*renderConfig.GetHeight(), 1, g_ffmpeg);

		delete[] img.m_data;
	}

	double renderEndTime = GetSeconds();

	// if user requested a scene reset process it now
	if (g_resetScene) 
	{
		Reset();
		g_resetScene = false;
	}

	// if gui requested a scene change process it now
	if (newScene != -1)
	{
		g_scene = newScene;
		Init(g_scene);
		return;
	}

	//-------------------------------------------------------------------
	// Flex Update

	double updateBeginTime = GetSeconds();

	// send any particle updates to the solver
	NvFlexSetParticles(g_flex, g_buffers->positions.buffer, g_buffers->positions.size());
	NvFlexSetVelocities(g_flex, g_buffers->velocities.buffer, g_buffers->velocities.size());
	NvFlexSetPhases(g_flex, g_buffers->phases.buffer, g_buffers->phases.size());
	NvFlexSetActive(g_flex, g_buffers->activeIndices.buffer, g_buffers->activeIndices.size());

	// allow scene to update constraints etc
	SyncScene();

	if (g_shapesChanged)
	{
		NvFlexSetShapes(
			g_flex,
			g_buffers->shapeGeometry.buffer,
			g_buffers->shapePositions.buffer,
			g_buffers->shapeRotations.buffer,
			g_buffers->shapePrevPositions.buffer,
			g_buffers->shapePrevRotations.buffer,
			g_buffers->shapeFlags.buffer,
			int(g_buffers->shapeFlags.size()));

		g_shapesChanged = false;
	}

	if (!g_pause || g_step)
	{
		// tick solver
		NvFlexSetParams(g_flex, &g_params);
		NvFlexUpdateSolver(g_flex, g_dt, g_numSubsteps, g_profile);

		g_frame++;
		g_step = false;
	}

	// read back base particle data
	// Note that flexGet calls don't wait for the GPU, they just queue a GPU copy 
	// to be executed later.
	// When we're ready to read the fetched buffers we'll Map them, and that's when
	// the CPU will wait for the GPU flex update and GPU copy to finish.
	NvFlexGetParticles(g_flex, g_buffers->positions.buffer, g_buffers->positions.size());
	NvFlexGetVelocities(g_flex, g_buffers->velocities.buffer, g_buffers->velocities.size());
	NvFlexGetNormals(g_flex, g_buffers->normals.buffer, g_buffers->normals.size());

	// readback triangle normals
	if (g_buffers->triangles.size())
		NvFlexGetDynamicTriangles(g_flex, g_buffers->triangles.buffer, g_buffers->triangleNormals.buffer, g_buffers->triangles.size() / 3);

	// readback rigid transforms
	if (g_buffers->rigidOffsets.size())
		NvFlexGetRigidTransforms(g_flex, g_buffers->rigidRotations.buffer, g_buffers->rigidTranslations.buffer);

	if (!g_interop)
	{
		// if not using interop then we read back fluid data to host
		if (g_drawEllipsoids)
		{
			NvFlexGetSmoothParticles(g_flex, g_buffers->smoothPositions.buffer, g_buffers->smoothPositions.size());
			NvFlexGetAnisotropy(g_flex, g_buffers->anisotropy1.buffer, g_buffers->anisotropy2.buffer, g_buffers->anisotropy3.buffer);
		}

		// read back diffuse data to host
		if (g_drawDensity)
			NvFlexGetDensities(g_flex, g_buffers->densities.buffer, g_buffers->positions.size());

		if (g_diffuseRenderBuffers.mNumDiffuseParticles)
		{
			NvFlexGetDiffuseParticles(g_flex, g_buffers->diffusePositions.buffer, g_buffers->diffuseVelocities.buffer, g_buffers->diffuseIndices.buffer);
		}
	}

	double updateEndTime = GetSeconds();

	//-------------------------------------------------------
	// Update the on-screen timers

	float newUpdateTime = float(updateEndTime - updateBeginTime);
	float newRenderTime = float(renderEndTime - renderBeginTime);
	float newWaitTime = float(waitBeginTime - waitEndTime);

	// Exponential filter to make the display easier to read
	const float timerSmoothing = 0.05f;

	g_updateTime = (g_updateTime == 0.0f) ? newUpdateTime : Lerp(g_updateTime, newUpdateTime, timerSmoothing);
	g_renderTime = (g_renderTime == 0.0f) ? newRenderTime : Lerp(g_renderTime, newRenderTime, timerSmoothing);
	g_waitTime = (g_waitTime == 0.0f) ? newWaitTime : Lerp(g_waitTime, newWaitTime, timerSmoothing);
	g_simLatency = (g_simLatency == 0.0f) ? newSimLatency : Lerp(g_simLatency, newSimLatency, timerSmoothing);
	
	PresentFrame(g_vsync);
}

bool InputKeyboardDown(unsigned char key, int x, int y)
{
	float kSpeed = g_camSpeed;

	switch (key)
	{
	case 'w':
	{
		g_camVel.z = kSpeed;
		break;
	}
	case 's':
	{
		g_camVel.z = -kSpeed;
		break;
	}
	case 'a':
	{
		g_camVel.x = -kSpeed;
		break;
	}
	case 'd':
	{
		g_camVel.x = kSpeed;
		break;
	}
	case 'q':
	{
		g_camVel.y = kSpeed;
		break;
	}
	case 'z':
	{
		g_camVel.y = -kSpeed;
		break;
	}

	case 'u':
	{
		if (g_fullscreen)
		{
			SDL_SetWindowFullscreen(renderConfig.GetWindow(), 0);
			renderConfig.ReshapeWindow(1280, 720);
			g_fullscreen = false;
		}
		else
		{
			SDL_SetWindowFullscreen(renderConfig.GetWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);
			g_fullscreen = true;
		}
		break;
	}
	case 'r':
	{
		g_resetScene = true;
		break;
	}
	case 'p':
	{
		g_pause = !g_pause;
		break;
	}
	case 'o':
	{
		g_step = true;
		break;
	}
	case 'h':
	{
		g_showHelp = !g_showHelp;
		break;
	}
	case 'e':
	{
		g_drawEllipsoids = !g_drawEllipsoids;
		break;
	}
	case 't':
	{
		g_drawOpaque = !g_drawOpaque;
		break;
	}
	case 'v':
	{
		g_drawPoints = !g_drawPoints;
		break;
	}
	case 'f':
	{
		g_drawSprings = (g_drawSprings + 1) % 3;
		break;
	}
	case 'i':
	{
		g_drawDiffuse = !g_drawDiffuse;
		break;
	}
	case 'm':
	{
		g_drawMesh = !g_drawMesh;
		break;
	}
	case 'n':
	{
		g_drawRopes = !g_drawRopes;
		break;
	}
	case '.':
	{
		g_profile = !g_profile;
		break;
	}
	case 'g':
	{
		if (g_params.gravity[1] != 0.0f)
			g_params.gravity[1] = 0.0f;
		else
			g_params.gravity[1] = -9.8f;

		break;
	}
	case '-':
	{
		if (g_params.numPlanes)
			g_params.numPlanes--;

		break;
	}
	case ';':
	{
		g_debug = !g_debug;
		break;
	}
	};

	g_scenes[g_scene]->KeyDown(key);
	return false;
}

void InputKeyboardUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 's':
	{
		g_camVel.z = 0.0f;
		break;
	}
	case 'a':
	case 'd':
	{
		g_camVel.x = 0.0f;
		break;
	}
	case 'q':
	case 'z':
	{
		g_camVel.y = 0.0f;
		break;
	}
	};
}

void MouseFunc(int b, int state, int x, int y)
{
	switch (state)
	{
	case SDL_RELEASED:
	{
		g_lastx = x;
		g_lasty = y;
		g_lastb = -1;

		break;
	}
	case SDL_PRESSED:
	{
		g_lastx = x;
		g_lasty = y;
		g_lastb = b;

		if ((SDL_GetModState() & KMOD_LSHIFT) && g_lastb == SDL_BUTTON_LEFT)
		{

		}
		break;
	}
	};
}

void MousePassiveMotionFunc(int x, int y)
{
	g_lastx = x;
	g_lasty = y;
}

void MouseMotionFunc(unsigned state, int x, int y)
{
	float dx = float(x - g_lastx);
	float dy = float(y - g_lasty);

	g_lastx = x;
	g_lasty = y;

	if (state & SDL_BUTTON_RMASK)
	{
		const float kSensitivity = DegToRad(0.1f);
		const float kMaxDelta = FLT_MAX;

		g_camAngle.x -= Clamp(dx*kSensitivity, -kMaxDelta, kMaxDelta);
		g_camAngle.y -= Clamp(dy*kSensitivity, -kMaxDelta, kMaxDelta);
	}
}

bool g_Error = false;

void ControllerButtonEvent(SDL_ControllerButtonEvent event)
{
	// map controller buttons to keyboard keys
	if (event.type == SDL_CONTROLLERBUTTONDOWN)
	{
		InputKeyboardDown(GetKeyFromGameControllerButton(SDL_GameControllerButton(event.button)), 0, 0);

		if (event.button == SDL_CONTROLLER_BUTTON_LEFT_TRIGGER)
		{
			// Handle picking events using the game controller
			g_lastx = renderConfig.GetWidth() / 2;
			g_lasty = renderConfig.GetHeight() / 2;
			g_lastb = 1;
		}
	}
	else
	{
		InputKeyboardUp(GetKeyFromGameControllerButton(SDL_GameControllerButton(event.button)), 0, 0);

		if (event.button == SDL_CONTROLLER_BUTTON_LEFT_TRIGGER)
		{
			// Handle picking events using the game controller
			g_lastx = renderConfig.GetWidth() / 2;
			g_lasty = renderConfig.GetHeight() / 2;
			g_lastb = -1;
		}
	}
}

void ControllerDeviceUpdate()
{
	if (SDL_NumJoysticks() > 0)
	{
		SDL_JoystickEventState(SDL_ENABLE);
		if (SDL_IsGameController(0))
		{
			g_gamecontroller = SDL_GameControllerOpen(0);
		}
	}
}

void SDLMainLoop()
{
	bool quit = false;
	SDL_Event e;
	while (!quit)
	{
		UpdateFrame();

		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
				if (e.key.keysym.sym < 256 && (e.key.keysym.mod == KMOD_NONE || (e.key.keysym.mod & KMOD_NUM)))
					quit = InputKeyboardDown(e.key.keysym.sym, 0, 0);
				break;

			case SDL_KEYUP:
				if (e.key.keysym.sym < 256 && (e.key.keysym.mod == 0 || (e.key.keysym.mod & KMOD_NUM)))
					InputKeyboardUp(e.key.keysym.sym, 0, 0);
				break;

			case SDL_MOUSEMOTION:
				if (e.motion.state)
					MouseMotionFunc(e.motion.state, e.motion.x, e.motion.y);
				else
					MousePassiveMotionFunc(e.motion.x, e.motion.y);
				break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				MouseFunc(e.button.button, e.button.state, e.motion.x, e.motion.y);
				break;

			case SDL_WINDOWEVENT:
				if (e.window.windowID == sdlController.GetWindowId())
				{
					if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
						renderConfig.ReshapeWindow(e.window.data1, e.window.data2);
				}
				break;

			case SDL_WINDOWEVENT_LEAVE:
				g_camVel = Vec3(0.0f, 0.0f, 0.0f);
				break;

			case SDL_CONTROLLERBUTTONUP:
			case SDL_CONTROLLERBUTTONDOWN:
				ControllerButtonEvent(e.cbutton);
				break;

			case SDL_JOYDEVICEADDED:
			case SDL_JOYDEVICEREMOVED:
				ControllerDeviceUpdate();
				break;
			}
		}
	}
}

int main(int argc, char* argv[])
{
	// Read argument from console
	ConsoleController(argc, argv);

	// init controller
	sdlController.SDLInit(&renderConfig, "Flex Demo (CUDA)");
	if (g_fullscreen)
		SDL_SetWindowFullscreen(renderConfig.GetWindow(), SDL_WINDOW_FULLSCREEN_DESKTOP);

	// init gl
	renderConfig.InitRender(g_fullscreen);
	renderConfig.ReshapeWindow();

	// init flex
	flexController.InitFlex();
	
	// init benchmark (возможно стоит выпилить)
	if (g_benchmark)
		BenchmarkInit();

	// init default scene
	g_scenes.push_back(new Cell("Water Balloons"));
	Init(g_scene);

	// create shadow maps
	g_shadowMap = ShadowCreate();

	SDLMainLoop();
		
	if (g_fluidRenderer)
		DestroyFluidRenderer(g_fluidRenderer);

	DestroyFluidRenderBuffers(g_fluidRenderBuffers);
	DestroyDiffuseRenderBuffers(g_diffuseRenderBuffers);

	ShadowDestroy(g_shadowMap);
	DestroyRender();

	Shutdown();

	SDL_DestroyWindow(renderConfig.GetWindow());
	SDL_Quit();

	return 0;
}
