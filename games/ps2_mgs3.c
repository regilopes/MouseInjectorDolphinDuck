//===========================================================
// Mouse Injector for Dolphin
//==========================================================================
// Copyright (C) 2019-2020 Carnivorous
// All rights reserved.
//
// Mouse Injector is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, visit http://www.gnu.org/licenses/gpl-2.0.html
//==========================================================================
#include <stdint.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

// TODO:
// auto ADS weapon when aiming

// cam and aim pointer
#define MGS3_CAMBASE 0x0020CF18 //
#define MGS3_CAMBASE_SANITY_1 -0xD4 //offset from cambase
#define MGS3_CAMBASE_SANITY_1_VALUE 0xFDFF7F3F
#define MGS3_AIMBASE 0x0020CF18	   
#define MGS3_CUTSCNBASE 0x00213670 // cutscene cam base

// States
#define MGS3_SNAKESTATE 0x001D744C //standing, crouching, prone, aiming, swiming, etc.
#define MGS3_VIEWSTATE 0x0020CF9F  // FPV or TPV
#define MGS3_GAMESTATE 0x0024EC8B // loading new area or not
#define MGS3_CUTSCNSTATE 0x50	  // is FP on cutscene or not (offset from cutscnebase)
#define MGS3_WEPSTATE 0x1E8A	  // wepon ready, reloading, empty... (offset from aimbase)

// cam offsets from cambase
#define MGS3_CAMY -0x100
#define MGS3_CAMX -0x108

// aim offsets from viewbase
#define MGS3_AIMY 0x2E0 // aim base
#define MGS3_AIMX 0x2E2 // aim base

#define MGS3_CUTSCNY 0x5C
#define MGS3_CUTSCNX 0x60


#define MGS3_FOV 0x0020CF9C

static uint8_t PS2_MGS3_Status(void);
static uint8_t PS2_MGS3_DetectCambase(void);
static void PS2_MGS3_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Metal Gear Solid 3: Subsistence",
		PS2_MGS3_Status,
		PS2_MGS3_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MGS3 = &GAMEDRIVER_INTERFACE;

static uint32_t camBase = 0;
static uint32_t aimBase = 0;
static uint32_t cutscnBase = 0;
static uint8_t cutscnState = 0;
static uint8_t snakeState = 0;
static uint8_t gameState = 0;
static uint8_t areaLoading = 0;
static float xAccumulator = 0.f;
static float yAccumulator = 0.f;
int flag = 0;
int flag2 = 0;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MGS3_Status(void)
{
	// SLUS_213.59 (0x93390)
	//  53 4C 55 53 | 5F 32 31 33 | 2E 35 39 3B
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553 &&
			PS2_MEM_ReadWord(0x93394) == 0x5F323133 &&
			PS2_MEM_ReadWord(0x93398) == 0x2E35393B);
}

static uint8_t PS2_MGS3_DetectCambase(void)
{
	uint32_t tempCamBase = PS2_MEM_ReadPointer(MGS3_CAMBASE);
	// printf("tempCamBase: %X\n", tempCamBase);
	// printf("sanity value: %X\n", PS2_MEM_ReadUInt(tempCamBase + MGS3_CAMBASE_SANITY_1));
	if (tempCamBase != 0)
	{
		if (PS2_MEM_ReadWord(tempCamBase + MGS3_CAMBASE_SANITY_1) == MGS3_CAMBASE_SANITY_1_VALUE)
		{
			camBase = tempCamBase;
			//printf("camBase: %X\n", camBase);
			return 1;
		}
		if (PS2_MEM_ReadWord(tempCamBase + MGS3_CAMBASE_SANITY_1) == 0x20000000)
		{
			aimBase = tempCamBase;
			//printf("aimBase: %X\n", aimBase);
			return 1;
		}
		if (PS2_MEM_ReadUInt8(cutscnBase + MGS3_CUTSCNSTATE) == 1) // if in cutscene first person
		{
			return 1;
		}
	}

	return 0;
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MGS3_Inject(void)
{

	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	//    	return;

	if (!PS2_MGS3_DetectCambase())
		return;

	gameState = PS2_MEM_ReadUInt8(MGS3_GAMESTATE); //in-game, codec screen, pause menu, cutscene, etc.
	areaLoading = PS2_MEM_ReadUInt8(0x24EC8D); // In transition to a new area (loading screen) or not
	// if (gameState != 0x01 && gameState != 0x11) // if isn't 'ingame' and 'cutscene'
	if ((gameState & (0 << 0))) //not in-game
	 	return;

	if ((gameState & (1 << 1))) // in codec screen
	 	return;

	if ((gameState & (1 << 2))) // in pause menu
	 	return;

	if ((gameState & (1 << 7))) // in game over screen
	 	return;

	if ((areaLoading & (1 << 1)) && (gameState & (1 << 3))){ // if loading new area
		// flag = 0;
		// flag2 = 0;
		// camBase = 0;
		// aimBase = 0;
		
		//printf("loading new area\n");
		return;
	}


	//if(areaLoading == 0){ // if loading new area (bit 1 == 0))
	
	// if((snakeState & (1 << 7))){ // in first person view (bit 7 == 1) 
	// 	if(flag == 0){ //necessary to get the pointers only once per map section, otherwise it will crash the game occasionally when changing view modes.
	// 			aimBase = PS2_MEM_ReadPointer(MGS3_AIMBASE);
	// 			flag = 1;
	// 			//}
			

	// 	}
	// }else{

		
	// 		if(flag2 == 0){ //necessary to get the pointers only once per map section, otherwise it will crash the game occasionally when changing view modes.
	
				//camBase = PS2_MEM_ReadPointer(MGS3_CAMBASE);
				//flag2 = 1;
	// 		}
	// 	}
	// }

	// printf("FlagAim: %i\n", flag);
	// printf("FlagCam: %i\n", flag2);
	// printf("camBase: %X\n", camBase);
	// printf("aimBase: %X\n", aimBase);
	// printf("areaload: %X\n", areaLoading);

	//cutscnBase = PS2_MEM_ReadPointer(MGS3_CUTSCNBASE);


	cutscnState = PS2_MEM_ReadUInt8(cutscnBase + MGS3_CUTSCNSTATE);
	cutscnBase = PS2_MEM_ReadPointer(MGS3_CUTSCNBASE);
	//printf("cutscnBase: %X\n", cutscnBase);

	snakeState = PS2_MEM_ReadUInt8(MGS3_SNAKESTATE);


	float fov = PS2_MEM_ReadFloat(MGS3_FOV);
	float looksensitivity = (float)sensitivity;


	// while not aiming
	float camY = PS2_MEM_ReadFloat(camBase + MGS3_CAMY);
	int16_t camX = PS2_MEM_ReadInt16(camBase + MGS3_CAMX);

	// while aiming
	int16_t aimY = PS2_MEM_ReadInt16(aimBase + MGS3_AIMY);
	int16_t aimX = PS2_MEM_ReadInt16(aimBase + MGS3_AIMX);
	float aimYf = (float)aimY;
	float aimXf = (float)aimX;

	//update cam
	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity / 50000.f;
	camY = ClampFloat(camY, 0.f, 1.f);
	camX += (int16_t)-xmouse * looksensitivity / 2;

	//update aim
	float ym = (float)(invertpitch ? -ymouse : ymouse);
	float dy = ym * looksensitivity / fov / 2.f;
	AccumulateAddRemainder(&aimYf, &yAccumulator, ym, dy);
	float dx = (float)-xmouse * looksensitivity / fov / 2.f;
	AccumulateAddRemainder(&aimXf, &xAccumulator, (float)xmouse, dx);


	//while fp on cutscene
	int32_t cutY = PS2_MEM_ReadInt(cutscnBase + MGS3_CUTSCNY);
	int32_t cutX = PS2_MEM_ReadInt(cutscnBase + MGS3_CUTSCNX);
	
		
	//update cutscene
	cutY += (int16_t)((!invertpitch ? ymouse : -ymouse) * looksensitivity / 2);
	cutX += (int16_t)(xmouse * looksensitivity / 2);


	//printf("cutscene state: %i", cutscnState);
	if ((gameState & (1 << 4))){ // in cutscene
		if (cutscnState){
		PS2_MEM_WriteInt(cutscnBase + MGS3_CUTSCNY, cutY);
		PS2_MEM_WriteInt(cutscnBase + MGS3_CUTSCNX, cutX);
		}
	}
	

	
	if((snakeState & (1 << 7))) // in first person view (bit 7 == 1)
	{ 
		PS2_MEM_WriteInt16(aimBase + MGS3_AIMY, (int16_t)aimYf);
		PS2_MEM_WriteInt16(aimBase + MGS3_AIMX, (int16_t)aimXf);
	}else if((gameState != 0x11 && gameState != 0x10 && gameState != 0x09))// Not in cutscene	
	{ 
		PS2_MEM_WriteFloat(camBase + MGS3_CAMY, camY);
		PS2_MEM_WriteInt16(camBase + MGS3_CAMX, camX);
	}
}