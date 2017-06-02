#include "../core/mesh.h"
#include "shaders.h"
#include "Utilits.h"
#include "SimBuffer.h"

// calculates local space positions given a set of particles and rigid indices
void CalculateRigidLocalPositions(const Vec4* restPositions, int numRestPositions, const int* offsets, const int* indices, int numRigids, Vec3* localPositions)
{
	// To improve the accuracy of the result, first transform the restPositions to relative coordinates (by finding the mean and subtracting that from all points)
	// Note: If this is not done, one might see ghost forces if the mean of the restPositions is far from the origin.

	// Calculate mean
	Vec3 shapeOffset(0.0f);

	for (int i = 0; i < numRestPositions; i++)
	{
		shapeOffset += Vec3(restPositions[i]);
	}

	shapeOffset /= float(numRestPositions);

	int count = 0;

	for (int r = 0; r < numRigids; ++r)
	{
		const int startIndex = offsets[r];
		const int endIndex = offsets[r + 1];

		const int n = endIndex - startIndex;

		assert(n);

		Vec3 com;

		for (int i = startIndex; i < endIndex; ++i)
		{
			const int r = indices[i];

			// By substracting meshOffset the calculation is done in relative coordinates
			com += Vec3(restPositions[r]) - shapeOffset;
		}

		com /= float(n);

		for (int i = startIndex; i < endIndex; ++i)
		{
			const int r = indices[i];

			// By substracting meshOffset the calculation is done in relative coordinates
			localPositions[count++] = (Vec3(restPositions[r]) - shapeOffset) - com;
		}
	}
}

