#pragma once 

#include "../core/cloth.h"
#include "../include/NvFlex.h"
#include <stdarg.h>

struct SimBuffers;

void CalculateRigidLocalPositions(const Vec4* restPositions, int numRestPositions, const int* offsets, const int* indices, int numRigids, Vec3* localPositions);

void AddRandomConvex(NvFlexLibrary *flexLib, SimBuffers *buffers, int numPlanes, Vec3 position, float minDist, float maxDist, Vec3 axis, float angle);