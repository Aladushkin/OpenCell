#pragma once

#include "../../../include/NvFlex.h"

#include "../../benchmark.h"

#include "../SDLController.h"
#include "../compute_controller/FlexController.h"
#include "../render_controller/RenderController.h"

#include "imgui.h"

#include "../../SimBuffer.h"


extern int g_frame;

extern FILE* g_ffmpeg;

extern int g_scene;

#include <vector>
class Scene;
std::vector<Scene*> g_scenes;
#include "../../scenes.h"

#include "../../Timer.h"

extern Timer timer;

class IMGUIController {
private:
	int x, y;
	int fontHeight;

	FlexController *flexController;
	RenderController *renderController;
	SDLController *sdlController;

	FlexParams *flexParams;
	RenderParam *renderParam;

	SimBuffers *buffers;

	bool tweakPanel = true;

	void DoStatistic(int numParticles, int numDiffuse) {

		x += 180;
		fontHeight = 13;

		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Frame: %d", g_frame); y -= fontHeight * 2;

		if (!g_ffmpeg)
		{
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Frame Time: %.2fms", timer.realdt*1000.0f); y -= fontHeight * 2;

			// If detailed profiling is enabled, then these timers will contain the overhead of the detail timers, so we won't display them.
			if (!renderController->GetProfile())
			{
				imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Sim Time (CPU): %.2fms", timer.updateTime*1000.0f); y -= fontHeight;
				imguiDrawString(x, y, 0.97f, 0.59f, 0.27f, IMGUI_ALIGN_RIGHT, "Sim Latency (GPU): %.2fms", timer.simLatency); y -= fontHeight * 2;
			}
			else
			{
				y -= fontHeight * 3;
			}
		}

		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Particle Count: %d", numParticles); y -= fontHeight;
		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Diffuse Count: %d", numDiffuse); y -= fontHeight;
		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Rigid Count: %d", buffers->rigidOffsets.size() > 0 ? buffers->rigidOffsets.size() - 1 : 0); y -= fontHeight;
		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Spring Count: %d", buffers->springLengths.size()); y -= fontHeight;
		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Num Substeps: %d", flexParams->numSubsteps); y -= fontHeight;
		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Num Iterations: %d", g_params.numIterations); y -= fontHeight * 2;

		imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Device: %s", flexController->GetDeviceName()); y -= fontHeight * 2;

		if (renderController->GetProfile())
		{
			imguiDrawString(x, y, 0.97f, 0.59f, 0.27f, IMGUI_ALIGN_RIGHT, "Total GPU Sim Latency: %.2fms", flexController->GetTimers().total); y -= fontHeight * 2;

			imguiDrawString(x, y, 0.0f, 1.0f, 0.0f, IMGUI_ALIGN_RIGHT, "GPU Latencies"); y -= fontHeight;

			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Predict: %.2fms", flexController->GetTimers().predict); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Create Cell Indices: %.2fms", flexController->GetTimers().createCellIndices); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Sort Cell Indices: %.2fms", flexController->GetTimers().sortCellIndices); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Reorder: %.2fms", flexController->GetTimers().reorder); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "CreateGrid: %.2fms", flexController->GetTimers().createGrid); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Collide Particles: %.2fms", flexController->GetTimers().collideParticles); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Collide Shapes: %.2fms", flexController->GetTimers().collideShapes); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Collide Triangles: %.2fms", flexController->GetTimers().collideTriangles); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Calculate Density: %.2fms", flexController->GetTimers().calculateDensity); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Solve Densities: %.2fms", flexController->GetTimers().solveDensities); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Solve Velocities: %.2fms", flexController->GetTimers().solveVelocities); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Solve Rigids: %.2fms", flexController->GetTimers().solveShapes); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Solve Springs: %.2fms", flexController->GetTimers().solveSprings); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Solve Inflatables: %.2fms", flexController->GetTimers().solveInflatables); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Solve Contacts: %.2fms", flexController->GetTimers().solveContacts); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Apply Deltas: %.2fms", flexController->GetTimers().applyDeltas); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Finalize: %.2fms", flexController->GetTimers().finalize); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Update Triangles: %.2fms", flexController->GetTimers().updateTriangles); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Update Normals: %.2fms", flexController->GetTimers().updateNormals); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Update Bounds: %.2fms", flexController->GetTimers().updateBounds); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Calculate Anisotropy: %.2fms", flexController->GetTimers().calculateAnisotropy); y -= fontHeight;
			imguiDrawString(x, y, 1.0f, 1.0f, 1.0f, IMGUI_ALIGN_RIGHT, "Update Diffuse: %.2fms", flexController->GetTimers().updateDiffuse); y -= fontHeight * 2;
		}

		x -= 180;
	}

	void DoMenu() {
		int uiOffset = 250;
		int uiBorder = 20;
		int uiWidth = 200;
		int uiHeight = renderController->GetHeight() - uiOffset - uiBorder * 3;
		int uiLeft = uiBorder;

		if (tweakPanel)
		{
			static int scroll = 0;

			imguiBeginScrollArea("Options", uiLeft, renderController->GetHeight() - uiBorder - uiHeight - uiBorder, uiWidth, uiHeight, &scroll);
			imguiSeparatorLine();

			// global options
			imguiLabel("Global");

			if (imguiCheck("Pause", g_pause))
				g_pause = !g_pause;

			imguiSeparatorLine();

			if (imguiCheck("Wireframe", renderParam->wireframe))
				renderParam->wireframe = !renderParam->wireframe;

			if (imguiCheck("Draw Points", renderController->GetDrawPoints()))
				renderController->SetDrawPoints(!renderController->GetDrawPoints());

			if (imguiCheck("Draw Fluid", renderController->GetDrawEllipsoids()))
				renderController->SetDrawEllipsoids(!renderController->GetDrawEllipsoids());

			if (imguiCheck("Draw Mesh", renderController->GetDrawMesh()))
			{
				renderController->SetDrawMesh(!renderController->GetDrawMesh());
				//g_drawRopes = !g_drawRopes;
			}

			if (imguiCheck("Draw Basis", renderParam->drawBases))
				renderParam->drawBases = !renderParam->drawBases;

			if (imguiCheck("Draw Springs", bool(renderController->GetDrawSprings() != 0)))
				renderController->SetDrawSprings((renderController->GetDrawSprings()) ? 0 : 1);

			if (imguiCheck("Draw Contacts", renderParam->drawContacts))
				renderParam->drawContacts = !renderParam->drawContacts;

			imguiSeparatorLine();

			// scene options
			g_scenes[g_scene]->DoGui();

			if (imguiButton("Reset Scene"))
				g_reset = true;

			imguiSeparatorLine();

			float n = float(flexParams->numSubsteps);
			if (imguiSlider("Num Substeps", &n, 1, 10, 1))
				flexParams->numSubsteps = int(n);

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
			imguiSlider("Diffuse Scale", &renderParam->diffuseScale, 0.0f, 1.5f, 0.01f);
			imguiSlider("Diffuse Alpha", &renderParam->diffuseColor.w, 0.0f, 3.0f, 0.01f);
			imguiSlider("Diffuse Inscatter", &renderParam->diffuseInscatter, 0.0f, 2.0f, 0.01f);
			imguiSlider("Diffuse Outscatter", &renderParam->diffuseOutscatter, 0.0f, 2.0f, 0.01f);
			imguiSlider("Diffuse Motion Blur", &renderParam->diffuseMotionScale, 0.0f, 5.0f, 0.1f);

			n = float(g_params.diffuseBallistic);
			if (imguiSlider("Diffuse Ballistic", &n, 1, 40, 1))
				g_params.diffuseBallistic = int(n);

			imguiEndScrollArea();
		}
	}

	void Draw()
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glActiveTexture(GL_TEXTURE0);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_RECTANGLE_ARB);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE3);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE4);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_CUBE_MAP);
		glActiveTexture(GL_TEXTURE5);
		glDisable(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);

		glDisable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glDisable(GL_BLEND);
		glDisable(GL_POINT_SPRITE);

		// save scene camera transform
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		const Matrix44 ortho = OrthographicMatrix(0.0f, float(renderController->GetWidth()), 0.0f, float(renderController->GetHeight()), -1.0f, 1.0f);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(ortho);

		glUseProgram(0);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

		imguiRenderGLDraw();

		// restore camera transform (for picking)
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

public:
	void Initialize(FlexController *flexController, FlexParams *flexParams, SimBuffers *buffers, 
					RenderController *renderController, RenderParam *renderParam,
					SDLController *sdlController) {

		this->flexController   = flexController;
		this->flexParams = flexParams;
		this->buffers = buffers;
		
		this->renderController = renderController;
		this->renderParam = renderParam;

		this->sdlController = sdlController;
		
	}

	void SetTweakPanel(bool tweakPanel) {
		this->tweakPanel = tweakPanel;
	}

	// returns the new scene if one is selected
	// returns the new scene if one is selected
	int DoUI(int numParticles, int numDiffuse)
	{
		// gui may set a new scene
		int newScene = -1;

		if (renderController->GetShowGUI())
		{
			x = renderController->GetWidth() - 200;
			y = renderController->GetHeight() - 23;

			// imgui
			unsigned char button = 0;
			if (sdlController->GetLastB() == SDL_BUTTON_LEFT)
				button = IMGUI_MBUT_LEFT;
			else if (sdlController->GetLastB() == SDL_BUTTON_RIGHT)
				button = IMGUI_MBUT_RIGHT;

			imguiBeginFrame(sdlController->GetLastX(), renderController->GetHeight() - sdlController->GetLastY(), button, 0);
			
			DoStatistic(numParticles, numDiffuse);
			DoMenu();

			imguiEndFrame();

			// kick render commands
			Draw();
		}

		// update benchmark and change scene if one is requested
		if (g_benchmark)
//			newScene = BenchmarkUpdate();

		return newScene;
	}

};