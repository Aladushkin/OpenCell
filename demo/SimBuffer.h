#pragma once

#include "../core/cloth.h"
#include "GlobalVar.h"
#include "../include/NvFlexExt.h"

struct SimBuffers
{
	NvFlexVector<Vec4> positions;
	NvFlexVector<Vec4> restPositions;
	NvFlexVector<Vec3> velocities;
	NvFlexVector<int> phases;
	NvFlexVector<float> densities;
	NvFlexVector<Vec4> anisotropy1;
	NvFlexVector<Vec4> anisotropy2;
	NvFlexVector<Vec4> anisotropy3;
	NvFlexVector<Vec4> normals;
	NvFlexVector<Vec4> smoothPositions;
	NvFlexVector<Vec4> diffusePositions;
	NvFlexVector<Vec4> diffuseVelocities;
	NvFlexVector<int> diffuseIndices;
	NvFlexVector<int> activeIndices;

	// convexes
	NvFlexVector<NvFlexCollisionGeometry> shapeGeometry;
	NvFlexVector<Vec4> shapePositions;
	NvFlexVector<Quat> shapeRotations;
	NvFlexVector<Vec4> shapePrevPositions;
	NvFlexVector<Quat> shapePrevRotations;
	NvFlexVector<int> shapeFlags;

	// rigids
	NvFlexVector<int> rigidOffsets;
	NvFlexVector<int> rigidIndices;
	NvFlexVector<int> rigidMeshSize;
	NvFlexVector<float> rigidCoefficients;
	NvFlexVector<Quat> rigidRotations;
	NvFlexVector<Vec3> rigidTranslations;
	NvFlexVector<Vec3> rigidLocalPositions;
	NvFlexVector<Vec4> rigidLocalNormals;

	// inflatables
	NvFlexVector<int> inflatableTriOffsets;
	NvFlexVector<int> inflatableTriCounts;
	NvFlexVector<float> inflatableVolumes;
	NvFlexVector<float> inflatableCoefficients;
	NvFlexVector<float> inflatablePressures;

	// springs
	NvFlexVector<int> springIndices;
	NvFlexVector<float> springLengths;
	NvFlexVector<float> springStiffness;

	NvFlexVector<int> triangles;
	NvFlexVector<Vec3> triangleNormals;
	NvFlexVector<Vec3> uvs;

	SimBuffers(NvFlexLibrary* l) :
		positions(l), restPositions(l), velocities(l), phases(l), densities(l),
		anisotropy1(l), anisotropy2(l), anisotropy3(l), normals(l), smoothPositions(l),
		diffusePositions(l), diffuseVelocities(l), diffuseIndices(l), activeIndices(l),
		shapeGeometry(l), shapePositions(l), shapeRotations(l), shapePrevPositions(l),
		shapePrevRotations(l), shapeFlags(l), rigidOffsets(l), rigidIndices(l), rigidMeshSize(l),
		rigidCoefficients(l), rigidRotations(l), rigidTranslations(l),
		rigidLocalPositions(l), rigidLocalNormals(l), inflatableTriOffsets(l),
		inflatableTriCounts(l), inflatableVolumes(l), inflatableCoefficients(l),
		inflatablePressures(l), springIndices(l), springLengths(l),
		springStiffness(l), triangles(l), triangleNormals(l), uvs(l)
	{}
	
	~SimBuffers()
	{
		// particles
		this->positions.destroy();
		this->restPositions.destroy();
		this->velocities.destroy();
		this->phases.destroy();
		this->densities.destroy();
		this->anisotropy1.destroy();
		this->anisotropy2.destroy();
		this->anisotropy3.destroy();
		this->normals.destroy();
		this->diffusePositions.destroy();
		this->diffuseVelocities.destroy();
		this->diffuseIndices.destroy();
		this->smoothPositions.destroy();
		this->activeIndices.destroy();

		// convexes
		this->shapeGeometry.destroy();
		this->shapePositions.destroy();
		this->shapeRotations.destroy();
		this->shapePrevPositions.destroy();
		this->shapePrevRotations.destroy();
		this->shapeFlags.destroy();

		// rigids
		this->rigidOffsets.destroy();
		this->rigidIndices.destroy();
		this->rigidMeshSize.destroy();
		this->rigidCoefficients.destroy();
		this->rigidRotations.destroy();
		this->rigidTranslations.destroy();
		this->rigidLocalPositions.destroy();
		this->rigidLocalNormals.destroy();

		// springs
		this->springIndices.destroy();
		this->springLengths.destroy();
		this->springStiffness.destroy();

		// inflatables
		this->inflatableTriOffsets.destroy();
		this->inflatableTriCounts.destroy();
		this->inflatableVolumes.destroy();
		this->inflatableCoefficients.destroy();
		this->inflatablePressures.destroy();

		// triangles
		this->triangles.destroy();
		this->triangleNormals.destroy();
		this->uvs.destroy();
	}

	void MapBuffers()
	{
		this->positions.map();
		this->restPositions.map();
		this->velocities.map();
		this->phases.map();
		this->densities.map();
		this->anisotropy1.map();
		this->anisotropy2.map();
		this->anisotropy3.map();
		this->normals.map();
		this->diffusePositions.map();
		this->diffuseVelocities.map();
		this->diffuseIndices.map();
		this->smoothPositions.map();
		this->activeIndices.map();

		// convexes
		this->shapeGeometry.map();
		this->shapePositions.map();
		this->shapeRotations.map();
		this->shapePrevPositions.map();
		this->shapePrevRotations.map();
		this->shapeFlags.map();

		this->rigidOffsets.map();
		this->rigidIndices.map();
		this->rigidMeshSize.map();
		this->rigidCoefficients.map();
		this->rigidRotations.map();
		this->rigidTranslations.map();
		this->rigidLocalPositions.map();
		this->rigidLocalNormals.map();

		this->springIndices.map();
		this->springLengths.map();
		this->springStiffness.map();

		// inflatables
		this->inflatableTriOffsets.map();
		this->inflatableTriCounts.map();
		this->inflatableVolumes.map();
		this->inflatableCoefficients.map();
		this->inflatablePressures.map();

		this->triangles.map();
		this->triangleNormals.map();
		this->uvs.map();
	}

	void UnmapBuffers()
	{
		// particles
		this->positions.unmap();
		this->restPositions.unmap();
		this->velocities.unmap();
		this->phases.unmap();
		this->densities.unmap();
		this->anisotropy1.unmap();
		this->anisotropy2.unmap();
		this->anisotropy3.unmap();
		this->normals.unmap();
		this->diffusePositions.unmap();
		this->diffuseVelocities.unmap();
		this->diffuseIndices.unmap();
		this->smoothPositions.unmap();
		this->activeIndices.unmap();

		// convexes
		this->shapeGeometry.unmap();
		this->shapePositions.unmap();
		this->shapeRotations.unmap();
		this->shapePrevPositions.unmap();
		this->shapePrevRotations.unmap();
		this->shapeFlags.unmap();

		// rigids
		this->rigidOffsets.unmap();
		this->rigidIndices.unmap();
		this->rigidMeshSize.unmap();
		this->rigidCoefficients.unmap();
		this->rigidRotations.unmap();
		this->rigidTranslations.unmap();
		this->rigidLocalPositions.unmap();
		this->rigidLocalNormals.unmap();

		// springs
		this->springIndices.unmap();
		this->springLengths.unmap();
		this->springStiffness.unmap();

		// inflatables
		this->inflatableTriOffsets.unmap();
		this->inflatableTriCounts.unmap();
		this->inflatableVolumes.unmap();
		this->inflatableCoefficients.unmap();
		this->inflatablePressures.unmap();

		// triangles
		this->triangles.unmap();
		this->triangleNormals.unmap();
		this->uvs.unmap();
	}


	// first initialize methods
	void ResizePositionVelocityPhases() {
		this->positions.resize(0);
		this->velocities.resize(0);
		this->phases.resize(0);
	}

	void ResizeRigid() {
		this->rigidOffsets.resize(0);
		this->rigidIndices.resize(0);
		this->rigidMeshSize.resize(0);
		this->rigidRotations.resize(0);
		this->rigidTranslations.resize(0);
		this->rigidCoefficients.resize(0);
		this->rigidLocalPositions.resize(0);
		this->rigidLocalNormals.resize(0);
	}

	void ResizeSpring() {
		this->springIndices.resize(0);
		this->springLengths.resize(0);
		this->springStiffness.resize(0);
		this->triangles.resize(0);
		this->triangleNormals.resize(0);
		this->uvs.resize(0);
	}

	void ResizeShape() {
		this->shapeGeometry.resize(0);
		this->shapePositions.resize(0);
		this->shapeRotations.resize(0);
		this->shapePrevPositions.resize(0);
		this->shapePrevRotations.resize(0);
		this->shapeFlags.resize(0);
	}

	void ResizeDiffuseSmooth() {
		this->diffusePositions.resize(g_maxDiffuseParticles);
		this->diffuseVelocities.resize(g_maxDiffuseParticles);
		this->diffuseIndices.resize(g_maxDiffuseParticles);

		// for fluid rendering these are the Laplacian smoothed positions
		this->smoothPositions.resize(maxParticles);
	}

	void ResizeNormalsTriangles() {
		this->normals.resize(0);
		this->normals.resize(maxParticles);

		// initialize normals (just for rendering before simulation starts)
		int numTris = this->triangles.size() / 3;
		for (int i = 0; i < numTris; ++i)
		{
			Vec3 v0 = Vec3(this->positions[this->triangles[i * 3 + 0]]);
			Vec3 v1 = Vec3(this->positions[this->triangles[i * 3 + 1]]);
			Vec3 v2 = Vec3(this->positions[this->triangles[i * 3 + 2]]);

			Vec3 n = Cross(v1 - v0, v2 - v0);

			this->normals[this->triangles[i * 3 + 0]] += Vec4(n, 0.0f);
			this->normals[this->triangles[i * 3 + 1]] += Vec4(n, 0.0f);
			this->normals[this->triangles[i * 3 + 2]] += Vec4(n, 0.0f);
		}

		for (int i = 0; i < int(maxParticles); ++i)
			this->normals[i] = Vec4(SafeNormalize(Vec3(this->normals[i]), Vec3(0.0f, 1.0f, 0.0f)), 0.0f);


	}

	void ResizeActiveIndices() {
		// create active indices (just a contiguous block for the demo)
		this->activeIndices.resize(this->positions.size());
		for (int i = 0; i < this->activeIndices.size(); ++i)
			this->activeIndices[i] = i;
	}

	// post initialize methods, fit object of cell
	void ResizeFitCell() {
		// resize particle buffers to fit
		this->positions.resize(maxParticles);
		this->velocities.resize(maxParticles);
		this->phases.resize(maxParticles);

		this->densities.resize(maxParticles);
		this->anisotropy1.resize(maxParticles);
		this->anisotropy2.resize(maxParticles);
		this->anisotropy3.resize(maxParticles);

		// save rest positions
		this->restPositions.resize(this->positions.size());
		for (int i = 0; i < this->positions.size(); ++i)
			this->restPositions[i] = this->positions[i];


	}
};