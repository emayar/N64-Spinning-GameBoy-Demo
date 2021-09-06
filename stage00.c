#include <assert.h>
#include <nusys.h> 
#include "graphic.h"
#include "main.h"
#include "stage00.h"
#include "gameboy.h"
#include "gameboy_back.h"
#include "gameboy_side.h"
#include "gameboy_side_alt.h"

Vec3d cameraPos = {0.0f, 0.0f, -200.0f};
Vec3d cameraTarget = {0.0f, 0.0f, 0.0f};
Vec3d cameraUp = {0.0f, 1.0f, 0.0f};

struct Vec3d gameboyVector = {0.0f, 0.0f, 0.0f};
struct Vec3d backgroundVector = {0.0f, 0.0f, 300.0f};

float theAng = 100.0f;

void initStage00() { /* DO NOTHING */  }
void updateGame00() { /* DO NOTHING */ }

void makeDL00() { // Draw
	  
	unsigned short perspNorm;
	GraphicsTask * gfxTask;
	gfxTask = gfxSwitchTask(); 

	gfxRCPInit();
	gfxClearCfb();

	////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//	Setup the camera matrix and projection matrix before drawing polygons
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////


	// Create a Projection Matrix and store it in &gfxTask->projection and also stores a calculated "scaling constant" in perspnorm needed for gspPerspNormalize()
	guPerspective(&gfxTask->projection, &perspNorm, FOVY, ASPECT, NEAR_PLANE, FAR_PLANE, 1.0);

	gSPPerspNormalize(displayListPtr++, perspNorm); // Add to the displaylist. Normalizes the perspective projection to a coordinate system between -1,1 (it's just required when using a perspective projection)

	// Create a ModelView/Viewing Matrix (aka Camera) and store it in &gfxTask->modelview
	guLookAt(&gfxTask->modelview, cameraPos.x, cameraPos.y, cameraPos.z, cameraTarget.x, cameraTarget.y, cameraTarget.z, cameraUp.x, cameraUp.y, cameraUp.z);

	// Insert Projection Matrix operation into the display list (the gSPMatrix macro inserts matrix into displaylist)
	// (aka load the projection matrix into the matrix stack. given the combination of G_MTX_flags we provide, effectively this means "replace the projection matrix with this new matrix")
	gSPMatrix(
		displayListPtr++,
		OS_K0_TO_PHYSICAL(&(gfxTask->projection)), // The matrix (needs a physical address, that's why OS_K0_TO_PHYSICAL is used)
		G_MTX_PROJECTION | // We want to work on the projection matrix (not the modelview matrix)
		G_MTX_LOAD | // Load that matrix at top (aka don't multiply matrix by previously-top matrix in stack)
		G_MTX_NOPUSH // Don't push on matrix stack before matrix operation (don't push another matrix onto the stack before operation)
	);

	// Insert ModelView (aka Camera) Matrix operation into the display list
	// similarly this combination means "replace the modelview matrix with this new matrix"
	gSPMatrix(displayListPtr++, OS_K0_TO_PHYSICAL(&(gfxTask->modelview)), G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_LOAD);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	{

		
		
		
		guPosition(
			&gfxTask->objectTransform[1],
			0.0f, // angle of rotation of roll (in degrees, x-axis)
			0.0f, // angle of ration of pitch (in degrees, y-axis)
			0.0f, // angle of rotation of heading/yaw (in degrees, z-axis)
			1.0f, // scale
			backgroundVector.x, backgroundVector.y, backgroundVector.z // amount of translation along x, y, and z-axis
		);

		// Push relative transformation matrix
		gSPMatrix(displayListPtr++,
			OS_K0_TO_PHYSICAL(&(gfxTask->objectTransform[1])),
			G_MTX_MODELVIEW | // working on the modelview matrix stack
			G_MTX_PUSH | // ... push another matrix onto the stack ...
			G_MTX_MUL
		);

		drawBackground();

		// pop the matrix that we added back off the stack, to move the drawing position
		// back to where it was before we rendered this object
		gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
		
		
		if (theAng >= 360) {
			theAng = 0.0f;
		}
		
		guPosition(
			&gfxTask->objectTransform[0],
			-15.0f, // angle of rotation of roll (in degrees, x-axis)
			theAng+=0.8, // angle of ration of pitch (in degrees, y-axis)
			0.0f, // angle of rotation of heading/yaw (in degrees, z-axis)
			1.0f, // scale
			gameboyVector.x, gameboyVector.y, gameboyVector.z // amount of translation along x, y, and z-axis
		);

		// Push relative transformation matrix
		gSPMatrix(displayListPtr++,
			OS_K0_TO_PHYSICAL(&(gfxTask->objectTransform[0])),
			G_MTX_MODELVIEW | // working on the modelview matrix stack
			G_MTX_PUSH | // ... push another matrix onto the stack ...
			G_MTX_MUL
		);
		
		drawGameboy();
		gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);		
		
	}
	
	// mark the end of the displaylist
	gDPFullSync(displayListPtr++);
	gSPEndDisplayList(displayListPtr++);

	assert(displayListPtr - gfxTask->displayList < MAX_DISPLAY_LIST_COMMANDS);

	nuGfxTaskStart(
		gfxTask->displayList,
		(int)(displayListPtr - gfxTask->displayList) * sizeof (Gfx),
		NU_GFX_UCODE_F3DEX,
		NU_SC_SWAPBUFFER
	);
}

// Static array of model vertext data
Vtx gameboyVerts[] = {
	//  x,   y,  z, flag, S, T,    r,    g,    b,    a
	{ -20,  32,  6,   0, 0  << 6, 0  << 6, 0xff, 0x00, 0x00, 0xff  },
	{  20,  32,  6,   0, 15 << 6, 0  << 6, 0xff, 0x00, 0x00, 0xff  },
	{  20, -20,  6,   0, 15 << 6, 15 << 6, 0xff, 0x00, 0x00, 0xff  },
	{ -20, -20,  6,   0, 0  << 6, 15 << 6, 0xff, 0x00, 0x00, 0xff  },


	{ -20,  32, -6,   0, 0  << 6, 0  << 6, 0x00, 0xff, 0x00, 0xff  },
	{  20,  32, -6,   0, 15 << 6, 0  << 6, 0x00, 0xff, 0x00, 0xff  },
	{  20, -20, -6,   0, 15 << 6, 15 << 6, 0x00, 0xff, 0x00, 0xff  },
	{ -20, -20, -6,   0, 0  << 6, 15 << 6, 0x00, 0xff, 0x00, 0xff  },


	{ -20,  32,  6,   0, 0, 0, 0x00, 0x00, 0xff, 0xff  },
	{  20,  32,  6,   0, 0, 0, 0x00, 0x00, 0xff, 0xff  },
	{  20,  32, -6,   0, 0, 0, 0x00, 0x00, 0xff, 0xff  },
	{ -20,  32, -6,   0, 0, 0, 0x00, 0x00, 0xff, 0xff  },


	{ -20, -20,  6,   0, 0, 0, 0x00, 0xff, 0xff, 0xff  },
	{ -20, -20, -6,   0, 0, 0, 0x00, 0xff, 0xff, 0xff  },
	{  20, -20, -6,   0, 0, 0, 0x00, 0xff, 0xff, 0xff  },
	{  20, -20,  6,   0, 0, 0, 0x00, 0xff, 0xff, 0xff  },




	{  20,  32,  6,   0, 0  << 6, 0  << 6, 0xff, 0xff, 0xff, 0xff  },
	{  20,  32, -6,   0, 15 << 6, 0  << 6, 0xff, 0xff, 0xff, 0xff  },
	{  20, -20, -6,   0, 15 << 6, 15 << 6, 0xff, 0xff, 0xff, 0xff  },
	{  20, -20,  6,   0, 0  << 6, 15 << 6, 0xff, 0xff, 0xff, 0xff  },


	{ -20,  32,  6,   0, 0  << 6, 0  << 6, 0xff, 0x00, 0xff, 0xff  },
	{ -20,  32, -6,   0, 15 << 6, 0  << 6, 0xff, 0x00, 0xff, 0xff  },
	{ -20, -20, -6,   0, 15 << 6, 15 << 6, 0xff, 0x00, 0xff, 0xff  },  
	{ -20, -20,  6,   0, 0  << 6, 15 << 6, 0xff, 0x00, 0xff, 0xff  },
};

void drawGameboy() {
	
	gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
	gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
	gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
	gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER);
	
	
	gSPTexture(displayListPtr++,0x8000, 0x8000, 0, 0, G_ON);
	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(displayListPtr++,G_TF_AVERAGE);
	gDPLoadTextureBlock(displayListPtr++,gameboy, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
		 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
		 4, 4, G_TX_NOLOD, G_TX_NOLOD);
	gSPVertex(displayListPtr++, &(gameboyVerts[0]), 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	gSPTexture(displayListPtr++,0, 0, 0, 0, G_OFF);
	
 
	gSPTexture(displayListPtr++,0x8000, 0x8000, 0, 0, G_ON);
	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(displayListPtr++,G_TF_AVERAGE);
	gDPLoadTextureBlock(displayListPtr++,gameboy_back, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
		 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
		 4, 4, G_TX_NOLOD, G_TX_NOLOD);
	gSPVertex(displayListPtr++, &(gameboyVerts[0])+4, 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	gSPTexture(displayListPtr++,0, 0, 0, 0, G_OFF);
	

	gSPTexture(displayListPtr++,0x8000, 0x8000, 0, 0, G_ON);
	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(displayListPtr++,G_TF_AVERAGE);
	gDPLoadTextureBlock(displayListPtr++,gameboy_side_alt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
		 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
		 4, 4, G_TX_NOLOD, G_TX_NOLOD);
	gSPVertex(displayListPtr++, &(gameboyVerts[0])+8, 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	gSPTexture(displayListPtr++,0, 0, 0, 0, G_OFF);


	gSPTexture(displayListPtr++,0x8000, 0x8000, 0, 0, G_ON);
	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(displayListPtr++,G_TF_AVERAGE);
	gDPLoadTextureBlock(displayListPtr++,gameboy_side_alt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
		 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
		 4, 4, G_TX_NOLOD, G_TX_NOLOD);
	gSPVertex(displayListPtr++, &(gameboyVerts[0])+12, 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	gSPTexture(displayListPtr++,0, 0, 0, 0, G_OFF);	

	
	gSPTexture(displayListPtr++,0x8000, 0x8000, 0, 0, G_ON);
	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(displayListPtr++,G_TF_AVERAGE);
	gDPLoadTextureBlock(displayListPtr++,gameboy_side, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
		 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
		 4, 4, G_TX_NOLOD, G_TX_NOLOD);	
	gSPVertex(displayListPtr++, &(gameboyVerts[0])+16, 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	gSPTexture(displayListPtr++,0, 0, 0, 0, G_OFF);
	

	gSPTexture(displayListPtr++,0x8000, 0x8000, 0, 0, G_ON);
	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
	gDPSetTextureFilter(displayListPtr++,G_TF_AVERAGE);
	gDPLoadTextureBlock(displayListPtr++,gameboy_side_alt, G_IM_FMT_RGBA, G_IM_SIZ_16b, 16, 16, 0,
		 G_TX_WRAP | G_TX_MIRROR, G_TX_WRAP | G_TX_MIRROR,
		 4, 4, G_TX_NOLOD, G_TX_NOLOD);		
	gSPVertex(displayListPtr++, &(gameboyVerts[0])+20, 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	gSPTexture(displayListPtr++,0, 0, 0, 0, G_OFF);	

	gDPPipeSync(displayListPtr++);
	
	
}

Vtx backgroundVerts[] = {
    //  gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0);	0,1,2,[0-flag],0,2,3,[0-flag]
	
	//  x,   y,  z, flag, S, T,    r,    g,    b,    a
/*	
	{ -400,  400,  200,    0, 0 <<  6, 0  << 6, 0xff, 0xd9, 0xfe, 0xff  },
	{  400,  400,  200,    0, 15 << 6, 0  << 6, 0xff, 0xd9, 0xfe, 0xff  },
	{  400, -400,  200,    0, 15 << 6, 15 << 6, 0xff, 0xd9, 0xfe, 0xff  },
	{ -400, -400,  200,    0, 0  << 6, 15 << 6, 0xff, 0xd9, 0xfe, 0xff  },
*/	
	
	{ -400,  350,  200,    0, 0, 0, 0xc7, 0xe4, 0xff, 0xff  },
	{  400,  350,  200,    0, 0, 0, 0xc7, 0xe4, 0xff, 0xff  },
	{  400, -350,  200,    0, 0, 0, 0xff, 0xff, 0xff, 0xff  },
	{ -400, -350,  200,    0, 0, 0, 0xff, 0xff, 0xff, 0xff  },
    
};

void drawBackground() {


	gSPVertex(displayListPtr++, &(backgroundVerts[0]), 4, 0); // loads vertices (args: displist pointer, segment address of vertext list, number of vertices, starting index in vertex buffer where vertices are to be loaded)
	gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
	gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
	gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
	gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER);
	// Create faces (normals)
	
	
	gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0); // Generates triangles using the vertices loaded into the vertex buffer by the gSPVertex macro.
	
	
	gDPPipeSync(displayListPtr++);
}


void stage00(int pendingGfx) {
	if(pendingGfx < 1)
		makeDL00();

	updateGame00();
}
