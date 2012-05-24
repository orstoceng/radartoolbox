//
#include "cuda.h"
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <cufft.h>
#include <stdlib.h>
#include "GpuGrid.h"

//#include <cuPrintf.cu>
#include <cuComplex.h>
#include <cuda.h>
#ifndef _SIMPLETEXTURE_KERNEL_H_
#define _SIMPLETEXTURE_KERNEL_H_
#define PI 3.14159265
#define SPEED_OF_LIGHT 299702547

texture<float, cudaTextureType2D, cudaReadModeElementType > texRefPolar; //cudaReadModeNormalizedFloat
texture<float, cudaTextureType1D,  cudaReadModeElementType> texRefAzimuth;



__global__ void interpPolerKernel(float* output, int width, int height,short *azimuthMask) 
{ // Calculate normalized texture coordinates 
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y; 
	if ((y<width) && (x<height)) {
			float angle = tex1D(texRefAzimuth, ((float)x));
			 //tex2D(texRefPolar, ((float)y),angle);tex2D(texRefPolar, ((float)y),angle) *
			 output[x * width + y] = tex2D(texRefPolar, ((float)y),angle)*((float) azimuthMask[x]); //(float)x;//tex2D(texRef, tu, tv); 
	}
//	output[y*width + x] = tex2D(texRefPolar, (float)x, (float)y) ;
}

__global__ void interpCartKernel(float* output, int width, int height, float antennaHieght,int xOffset,int yOffset, float heading, float gridSize, float rangeBinSize, float angleStepInRadians,float maxrange,bool tvg, int maxAzimuthBin) 
{ // Calculate normalized texture coordinates 
	unsigned int idx = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int idy = blockIdx.y * blockDim.y + threadIdx.y; 
	if ((idy>0) && (idy<width) && (idx<height) && (idx>0)) {
		float xdistance = (xOffset-((float)height-(float)idx))*gridSize;
        float ydistance =(yOffset-(float)idy)*gridSize;		
        float azimuth =atan2(ydistance,xdistance)-heading;
		azimuth =2*PI-(azimuth + ((azimuth<0)*PI*2)); // convert to continous positive
		azimuth = (azimuth>2*PI)*(azimuth-2*PI) + (azimuth<2*PI)*(azimuth);
		azimuth=(azimuth)/angleStepInRadians;
		float azimuth1D= tex1D(texRefAzimuth, (azimuth)); // interp the bin
        float range = sqrt((xdistance*xdistance)+(ydistance*ydistance));
		range = sqrt((antennaHieght*antennaHieght)+(range*range))/rangeBinSize; // allow for the height of the antenna
		output[((height-1)*(width))-(idx * width) + idy] = tex2D(texRefPolar, range,azimuth1D)*10 * (float)(range<maxrange) * (float) (azimuth<maxAzimuthBin); //* ((float)(tvg)*range*rangeBinSize/1000); //(float)x;//tex2D(texRef, tu, tv); 

	}
}

__global__ void gpuinterpolateAngles(float *output, float *frameAngle, int numberOfAngleBins, float angleStep) 
 {
	  unsigned int idx = blockIdx.x * blockDim.x + threadIdx.x;
	  if (idx<360/angleStep) // initate the output
		  output[idx]=-9999;
		  int startbin = 1;
	  if (idx==0)
		  output[0] =0;
	  else if (idx<numberOfAngleBins) {
		  float x0 =frameAngle[idx-1];
		  float x1 = frameAngle[idx];
		  int startbin =(x0/angleStep)+1;
		  int endbin =	(x1/angleStep);
		  for (int i=startbin;i<=endbin;i++){
			 float x = (float)i*angleStep;
			 output[i] = idx-1 +(x-x0)*((1)/(x1-x0));
		  }
	  }

 }



 GpuGrid::GpuGrid(int collections, int rangeBins, float step,int xSize,int ySize,float sampleRate, bool applyTvg) 
{
	numberOfCollectionsPerRotation =collections;
	numberOfRangeBins =rangeBins;
	rangeBinSize = SPEED_OF_LIGHT/(sampleRate*1000000)/2;
	angleStep = step;
	numberOfAngleBins= 360/angleStep;
	xOutputSize = xSize;
	yOutputSize = ySize;
	tvg = applyTvg;
	// allocate memory on the GPU
	cudaMallocHost((void**)&rawFrameBuffer, numberOfCollectionsPerRotation*numberOfRangeBins*sizeof(float));
	cudaMallocHost((void**)&rawAngleBuffer, numberOfCollectionsPerRotation*sizeof(float));
	cudaMallocHost((void**)&interplatedPolarFrame, sizeof(float)*xOutputSize*yOutputSize);
    cudaMallocHost((void**)&azimuthArray,  numberOfAngleBins * sizeof(float));

	// allocate paged locked memory on the host
	cudaMalloc((void**)&deviceRawAngleBuffer, numberOfCollectionsPerRotation*sizeof(float));
	cudaMalloc((void**)&deviceAzimuth, numberOfAngleBins*sizeof(float));
	cudaMalloc((void**)&deviceFrameBuffer, xOutputSize*yOutputSize*sizeof(float));

	channelDescPolar = cudaCreateChannelDesc<float>();
	channelDescAzimuth = cudaCreateChannelDesc(32, 0, 0, 0, cudaChannelFormatKindFloat);
	cudaMallocArray(&deviceRawFrameArray, &channelDescPolar, numberOfRangeBins, numberOfCollectionsPerRotation);
	cudaMallocArray(&deviceAzimuthArray, &channelDescAzimuth, numberOfAngleBins, 0);

	selectedCudaDeviceId =initGPU()>0;
	gpuReady =selectedCudaDeviceId>-1;

}

  GpuGrid::~GpuGrid()
{
	// free the Host stuff
	cudaFreeHost(rawFrameBuffer);
	cudaFreeHost(rawAngleBuffer);
	cudaFreeHost(interplatedPolarFrame);
    cudaFreeHost(azimuthArray);

	// free device memory
	cudaFree(deviceRawAngleBuffer);
	cudaFree(deviceFrameBuffer);
	cudaFree(deviceAzimuth);
	

	// Free Cuda arrays
	cudaFreeArray(deviceRawFrameArray);
	cudaFreeArray(deviceAzimuthArray);

}

 void GpuGrid::cpyFrame(short *source)
 {
	// memcpy(rawFrameBuffer,source,numberOfCollectionsPerRotation*numberOfRangeBins*sizeof(short));
}

void GpuGrid::cpyAngles(float *source){
	// memcpy(rawAngleBuffer,source,numberOfCollectionsPerRotation*sizeof(float));
}

 void GpuGrid::interpolateAngles(float *frameAngle)
 {
/*	 float *devptr;
	  float y0,y1,x,x0,x1;
	  int sumCount =1;
	  azimuthArray[0] =0;
	  for (int i=1;(i<numberOfAngleBins);i++) {
		  x = (float)i*angleStep;
		  while ((x>frameAngle[sumCount]) && (sumCount<numberOfCollectionsPerRotation))
		    sumCount++;
		  if(sumCount<numberOfCollectionsPerRotation) {
			x0 = frameAngle[sumCount-1];
			x1 = frameAngle[sumCount];
			y0 = sumCount-1;
			y1 = sumCount;
			azimuthArray[i] = y0 +(x-x0)*((y1-y0)/(x1-x0));
			maxAngleBin = i;
		  }
	  }

	//   cudaPrintfInit(); */
 	  dim3 dimBlock(32, 1);  
	  dim3 dimGrid((numberOfCollectionsPerRotation + dimBlock.x - 1) / dimBlock.x,1);
	  maxAngleBin = rawAngleBuffer[numberOfCollectionsPerRotation-1]/angleStep;
	  cudaMemcpy(deviceRawAngleBuffer,rawAngleBuffer, numberOfCollectionsPerRotation *sizeof(float), cudaMemcpyHostToDevice);
	//  cudaMemcpy(deviceAzimuth,azimuthArray, numberOfAngleBins *sizeof(float), cudaMemcpyHostToDevice);
	  gpuinterpolateAngles<<<dimGrid, dimBlock>>>(deviceAzimuth,deviceRawAngleBuffer,numberOfCollectionsPerRotation,angleStep);
//	  cudaPrintfDisplay(stdout, true);
	//	cudaPrintfEnd();

 }



 
 float *GpuGrid::interpolatePolarFrame(float *sourceFrame,float *collectionAngles)
 {
	  if (gpuReady) {
		  interpolateAngles(collectionAngles);
		// store polar frame on GPU
		cudaMemcpyToArray(deviceRawFrameArray, 0, 0, sourceFrame, numberOfCollectionsPerRotation*numberOfRangeBins*sizeof(float), cudaMemcpyHostToDevice);
		//cudaMemcpyToArray(deviceAzimuthArray, 0, 0, azimuthArray,numberOfAngleBins *sizeof(float), cudaMemcpyHostToDevice);
	 //   cudaMemcpy(deviceAzimuthMask, azimuthMask, numberOfAngleBins*sizeof(short), cudaMemcpyHostToDevice);
		// Set texture parameters 
		texRefPolar.addressMode[0] = cudaAddressModeClamp; 
		texRefPolar.addressMode[1] = cudaAddressModeClamp; 
		texRefPolar.filterMode = cudaFilterModeLinear; 
		texRefPolar.normalized = false;
		cudaBindTextureToArray(&texRefPolar, deviceRawFrameArray,&channelDescPolar);

		//Set texture parameters Azimuth
		texRefAzimuth.addressMode[0] = cudaAddressModeClamp; 
		texRefAzimuth.addressMode[1] = cudaAddressModeClamp; 
		texRefAzimuth.filterMode = cudaFilterModeLinear; 
		texRefAzimuth.normalized = false;
		cudaBindTextureToArray(&texRefAzimuth, deviceAzimuthArray,&channelDescAzimuth);


	dim3 dimBlock(4, 4); 
	dim3 dimGrid((numberOfAngleBins + dimBlock.x - 1) / dimBlock.x, (numberOfRangeBins + dimBlock.y - 1) / dimBlock.y); 
//	interpPolerKernel<<<dimGrid, dimBlock>>>(deviceFrameBuffer, numberOfRangeBins,numberOfAngleBins,deviceAzimuthMask,tvg,maxAngleBin);
//	cudaMemcpy(interplatedPolarFrame, deviceFrameBuffer, numberOfAngleBins*numberOfRangeBins*sizeof(float), cudaMemcpyDeviceToHost);
	cudaUnbindTexture( texRefPolar );
	cudaUnbindTexture( texRefAzimuth );
	return(interplatedPolarFrame);
	  }
	  return(NULL);
 }


float *GpuGrid::interpolateCartFrame(float *sourceFrame,float *collectionAngles,int xOffset, int yOffset, float gridSize, float heading)
 {
	  if (gpuReady) {
	    interpolateAngles(collectionAngles);
		// store polar frame on GPU
	//    cudaMemcpy(deviceAzimuthMask, azimuthMask, numberOfAngleBins*sizeof(short), cudaMemcpyHostToDevice);
		cudaMemcpyToArray(deviceRawFrameArray, 0, 0, sourceFrame, numberOfCollectionsPerRotation*numberOfRangeBins*sizeof(float), cudaMemcpyHostToDevice);
		cudaMemcpyToArray(deviceAzimuthArray, 0, 0, deviceAzimuth,numberOfAngleBins *sizeof(float), cudaMemcpyDeviceToDevice);

		// Set texture parameters 
		texRefPolar.addressMode[0] = cudaAddressModeClamp; 
		texRefPolar.addressMode[1] = cudaAddressModeClamp; 
		texRefPolar.filterMode = cudaFilterModeLinear; 
		texRefPolar.normalized = false;
		cudaBindTextureToArray(&texRefPolar, deviceRawFrameArray,&channelDescPolar);

		//Set texture parameters Azimuth
		texRefAzimuth.addressMode[0] = cudaAddressModeClamp; 
		texRefAzimuth.addressMode[1] = cudaAddressModeClamp; 
		texRefAzimuth.filterMode = cudaFilterModeLinear; 
		texRefAzimuth.normalized = false;
		cudaBindTextureToArray(&texRefAzimuth, deviceAzimuthArray,&channelDescAzimuth);
	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, selectedCudaDeviceId);
	dim3 dimBlock(16, 16); 
	int xgrid = (xOutputSize + dimBlock.x - 1) / dimBlock.x;
	int ygrid = (yOutputSize + dimBlock.y - 1) / dimBlock.y;
	if (xgrid<ygrid){
		if (ygrid % prop.multiProcessorCount!=0)
			ygrid = ygrid +prop.multiProcessorCount- ygrid % prop.multiProcessorCount;
	}else{
		if (xgrid % prop.multiProcessorCount!=0)
			xgrid = xgrid +prop.multiProcessorCount- xgrid % prop.multiProcessorCount;
	}
	dim3 dimGrid(xgrid,ygrid); 
	//convert heading to radians
	heading = PI*heading/180; 
	interpCartKernel<<<dimGrid, dimBlock>>>(deviceFrameBuffer, yOutputSize,xOutputSize,14.0,xOffset,yOffset,heading-(PI/2),gridSize,rangeBinSize,(PI*angleStep)/180,numberOfRangeBins,tvg,maxAngleBin);
	cudaMemcpy(interplatedPolarFrame, deviceFrameBuffer, yOutputSize*xOutputSize*sizeof(float), cudaMemcpyDeviceToHost);
	cudaUnbindTexture( texRefPolar );
	cudaUnbindTexture( texRefAzimuth );
	return(interplatedPolarFrame);
	  }
	  return(NULL);
 }





int GpuGrid::initGPU()
{
	cudaDeviceProp prop;
	bool result = false;
	int dCnt = 0;
	int selectedCudaDeviceId = 0;
	cudaGetDeviceCount(&dCnt);
	printf("number of cuda gpu devices: %d\n", dCnt);
	if (dCnt > 0) {
		if (dCnt > 1) {
			int multiprocessor_cnt = 0;

			for (int deviceId=0; deviceId<dCnt; ++deviceId) {
				if (cudaSuccess == cudaGetDeviceProperties(&prop, deviceId)) {
					if (prop.multiProcessorCount > multiprocessor_cnt) {
						multiprocessor_cnt = prop.multiProcessorCount;
						selectedCudaDeviceId = deviceId;
					}
				}
			}
		} else {
			selectedCudaDeviceId = 0;
		}
	printf("selected device with most multiprocessors: %d\n", selectedCudaDeviceId);
		cudaSetDevice(selectedCudaDeviceId);
		cudaGetDeviceProperties(&prop, selectedCudaDeviceId);
		return(selectedCudaDeviceId);
	}
	return (-1);
}
#endif // #ifndef _SIMPLETEXTURE_KERNEL_