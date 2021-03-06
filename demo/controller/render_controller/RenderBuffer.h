#pragma once

#include "../../../core/mesh.h"
#include <vector>
#include <map>

#include "Fluid/FluidBuffer.h"

struct RenderBuffers {

	// mesh used for deformable object rendering
	Mesh* mesh;
	
	std::vector<int> meshSkinIndices;
	std::vector<float> meshSkinWeights;
	std::vector<Point3> meshRestPositions;
	
	// mapping of collision mesh to render mesh
	std::map<NvFlexConvexMeshId, GpuMesh*> convexes;
	std::map<NvFlexTriangleMeshId, GpuMesh*> meshes;
	std::map<NvFlexDistanceFieldId, GpuMesh*> fields;

	FluidRenderBuffers fluidRenderBuffers;
};