#pragma once

// vars of particles

uint32_t numParticles;
uint32_t maxParticles;

unsigned char g_maxNeighborsPerParticle;
int g_numExtraParticles;


const int g_maxDiffuseParticles = 0;

// vars of time

float g_dt = 1.0f / 60.0f;	// the time delta used for simulation
float g_realdt;				// the real world time delta between updates

float g_waitTime;		// the CPU time spent waiting for the GPU
float g_updateTime;     // the CPU time spent on Flex
float g_renderTime;		// the CPU time spent calling OpenGL to render the scene
						// the above times don't include waiting for vsync
float g_simLatency;     // the time the GPU spent between the first and last NvFlexUpdateSolver() operation. Because some GPUs context switch, this can include graphics time.
