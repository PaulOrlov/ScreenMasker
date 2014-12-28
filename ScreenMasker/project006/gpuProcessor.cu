#include "stdafx.h"
#include <math.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

unsigned char *patternPtrGpu;
unsigned char *stencilPtrGpu;
unsigned int *backgroundImageGpu;

int blocksQuantity = 0;
int threadsQuantity = 0;

__device__ void computePixelValue(	unsigned int bA1, 
									unsigned int bA2, 
									unsigned int desiredColor, 
									unsigned int *outMass, 
									int massIndex	)
{
	unsigned int bA = (bA1 * bA2) >> 8;

	unsigned int bR = bA * (desiredColor >> 16 & 0xff) >> 8;
	unsigned int bG = bA * (desiredColor >> 8 & 0xff) >> 8;
	unsigned int bB = bA * (desiredColor & 0xff) >> 8;

	outMass[massIndex] = bA<<24 | bR<<16 | bG<<8 | bB;
}

__global__ void drawBackgroundKernel(	unsigned char *patternPtrGpu, 
										unsigned char *stencilPtrGpu, 
										int patternWidth, 
										int patternHeight, 
										int stencilWidth, 
										int stencilHeight, 
										int screenWidth, 
										int screenHeight, 
										int cursorX, 
										int cursorY, 
										unsigned int desiredColor, 
										unsigned int *backgroundImageGpu	)
{
	int threadIndex = blockIdx.x*blockDim.x + threadIdx.x;
	int xCoord = threadIndex%screenWidth;
	int yCoord = threadIndex/screenWidth;

	if(xCoord < screenWidth && yCoord < screenHeight)
	{
		int bgXCoord = xCoord%patternWidth;
		int bgYCoord = yCoord%patternWidth;
		int bgMassIndex = bgYCoord*patternWidth + bgXCoord;

		int x0 = cursorX - stencilWidth*0.5;
		int x1 = cursorX + stencilWidth*0.5;
		int y0 = cursorY - stencilHeight*0.5;
		int y1 = cursorY + stencilHeight*0.5;

		if( xCoord >= x0 && 
			xCoord < x1 && 
			yCoord >= y0 && 
			yCoord < y1 )
		{
			int stencilXCoord = xCoord - x0;
			int stencilYCoord = yCoord - y0;
			int stencilMassIndex = stencilYCoord*stencilWidth + stencilXCoord;

			computePixelValue(((~patternPtrGpu[bgMassIndex]) & 0xff), ((~stencilPtrGpu[stencilMassIndex]) & 0xff), desiredColor, backgroundImageGpu, threadIndex);
		}
		else
		{
			computePixelValue((~patternPtrGpu[bgMassIndex] & 0xff), 0xff, desiredColor, backgroundImageGpu, threadIndex);
		}
	}
}

int initGpuResources(	UINT8 *patternPtr, 
						UINT8 *stencilPtr, 
						int patternWidth, 
						int patternHeight, 
						int stencilWidth, 
						int stencilHeight, 
						int screenWidth, 
						int screenHeight	)
{
	int deviceCount = -1;
	cudaGetDeviceCount(&deviceCount);

	if(deviceCount < 1)
	{
		return -1;
	}

	int cudaDevice = 0;
	cudaSetDevice(cudaDevice);

	cudaDeviceProp cudaDeviceProps;

	cudaGetDeviceProperties(&cudaDeviceProps, cudaDevice);

	cudaMalloc(&backgroundImageGpu, sizeof(unsigned int)*screenWidth*screenHeight);
	cudaMalloc(&patternPtrGpu, sizeof(unsigned char)*patternWidth*patternHeight);
	cudaMalloc(&stencilPtrGpu, sizeof(unsigned char)*stencilWidth*stencilHeight);

	cudaMemcpy(patternPtrGpu, patternPtr, sizeof(unsigned char)*patternWidth*patternHeight, cudaMemcpyHostToDevice);
	cudaMemcpy(stencilPtrGpu, stencilPtr, sizeof(unsigned char)*stencilWidth*stencilHeight, cudaMemcpyHostToDevice);

	threadsQuantity = cudaDeviceProps.maxThreadsPerBlock;
	blocksQuantity = (int)(ceil((float)(screenWidth*screenHeight)/(float)(threadsQuantity)));

	return 0;
}

int drawBackground(	int patternWidth, 
					int patternHeight, 
					int stencilWidth, 
					int stencilHeight, 
					int screenWidth, 
					int screenHeight, 
					int cursorX, 
					int cursorY, 
					UINT32 desiredColor, 
					UINT32 *backgroundImage	)
{
	//cudaMemset(backgroundImageGpu, 0, sizeof(unsigned int)*screenWidth*screenHeight);

	drawBackgroundKernel<<<blocksQuantity, threadsQuantity>>>(	patternPtrGpu, 
																stencilPtrGpu, 
																patternWidth, 
																patternHeight, 
																stencilWidth, 
																stencilHeight, 
																screenWidth, 
																screenHeight, 
																cursorX, 
																cursorY, 
																desiredColor, 
																backgroundImageGpu	);

	cudaMemcpy(backgroundImage, backgroundImageGpu, sizeof(unsigned int)*screenWidth*screenHeight, cudaMemcpyDeviceToHost);

	return 0;
}

int freeGpuResources()
{
	cudaFree(patternPtrGpu);
	cudaFree(stencilPtrGpu);
	cudaFree(backgroundImageGpu);

	return 0;
}