#include <windows.h>


#ifndef GPUGRID_H
#define GPUGRID_H

#include "cuda.h"
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <cufft.h>
#include <stdlib.h>
#include <cuComplex.h>
#include <cuda.h>

class GpuGrid {
	float *interpAngles;
	short*azimuthMask; //,*deviceAzimuthMask;
	float *interplatedPolarFrame, *deviceFrameBuffer,*rawAngleBuffer,*deviceRawAngleBuffer, *rawFrameBuffer,*deviceAzimuth;
	float *azimuthArray,rangeBinSize;
	cudaArray *deviceAzimuthArray,*deviceRawFrameArray;
	int xOutputSize,yOutputSize,numberOfCollectionsPerRotation, numberOfRangeBins, numberOfAngleBins;
	float angleStep;
	bool gpuReady;
	cudaChannelFormatDesc channelDescPolar,channelDescAzimuth;
	void interpolateAngles(float *frameAngle);
	bool tvg;
	int maxAngleBin,selectedCudaDeviceId;
	int initGPU();
public:
	GpuGrid(int collections, int rangeBins, float step,int xSize,int ySize,float sampleRate,bool applyTgv);
	~GpuGrid();
	void cpyFrame(short *source);
	void cpyAngles(float *source);
	float *interpolatePolarFrame(float *sourceFrame,float *collectionAngles);
	float *interpolateCartFrame(float *sourceFrame,float *collectionAngles,int xOffset, int yOffset, float gridSize, float heading);
	bool isGpuReady() {
		return(gpuReady);
	};
	float *getRawFrameBuffer() {
		return(rawFrameBuffer);
	};
	float *getRawAngleBuffer() {
		return(rawAngleBuffer);
	};
};

#endif