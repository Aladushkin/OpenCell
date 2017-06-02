#pragma once

#include "../../opengl/shader.h"

struct RenderParam {

	// data from Flex
	bool interop = true;

	// Light
	Vec3 lightDir;
	Vec3 lightPos;
	Vec3 lightTarget;
	
	// MSAA
	GLuint msaaFbo;
	GLuint msaaColorBuf;
	GLuint msaaDepthBuf;

	// param of type render
	bool drawCloth;
	bool drawBases = false;
	bool drawContacts = false;
	bool drawNormals = false;
	bool drawShapeGrid = false;
	bool drawDensity = false;
	
	// move planes along their normal for rendering
	float drawPlaneBias = 0.0f;

	// params of scale
	float pointScale = 1.0f;
	float g_ropeScale = 1.0f;

	float blur = 1.0f;
	float ior = 1.0f;

	float diffuseScale = 0.5f;
	float diffuseMotionScale = 1.0f;
	bool diffuseShadow = false;
	float diffuseInscatter = 0.8;
	float diffuseOutscatter = 0.53f;

	// colors
	Vec4 fluidColor = Vec4(0.1f, 0.4f, 0.8f, 1.0f);
	Vec4 diffuseColor;
	Vec3 meshColor = Vec3(0.9f, 0.9f, 0.9f);
	Vec3 clearColor;

	float lightDistance = 4.0f;
	float fogDistance = 0.005f;

	bool wireframe = false;

	//for cloth?
	float expandCloth = 0.0;

	// synchronize compute and render with realtime
	bool vsync = true;

	// scene size
	//Vec3 *sceneLower;
};