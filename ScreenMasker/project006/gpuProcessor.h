#ifndef _GPU_PROCESSOR_
#define _GPU_PROCESSOR_

int initGpuResources(	UINT8 *patternPtr, 
						UINT8 *stencilPtr, 
						int patternWidth, 
						int patternHeight, 
						int stencilWidth, 
						int stencilHeight, 
						int screenWidth, 
						int screenHeight	);

int drawBackground(	int patternWidth, 
					int patternHeight,
					int stencilWidth, 
					int stencilHeight, 
					int screenWidth, 
					int screenHeight, 
					int cursorX, 
					int cursorY, 
					UINT32 desiredColor, 
					UINT32 *backgroundImage	);

int freeGpuResources();

#endif