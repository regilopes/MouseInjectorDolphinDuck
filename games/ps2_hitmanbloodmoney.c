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
#include "../joystick.h"
#include "game.h"

#define PI 3.14159265f // 0x40490FDB

// pointers
#define HBM_CAMBASE 0x4A6FB4	//camY/fpY/scopeY and camX
#define HBM_FPBASE 0x4FF46C	//FPSINX/FPCOSX
#define HBM_FOVBASE 0x47B57C    //FOV

// Flags
#define HBM_FPSFLAG 0x4CF4C8 //in scope/FPS view or not
#define HBM_SCOPINGFLAG 0x51556C //in scope/FPS view or not


// cam offsets from playerBase
#define HBM_CAMY 0x8     ///first/third person and scope
#define HBM_CAMX 0x4     ///third person
//#define HBM_LADDERY 0x184  //while climbing ladder
//#define HBM_LADDERX 0x180  //while climbing ladder


// cam offsets from camBaseX
#define HBM_FPXSIN 0x0   //first person and scope
#define HBM_FPXCOS 0x8	 //first person and scope
#define HBM_FPXNSIN 0x20 //first person and scope
#define HBM_FPXCOS2 0x18 //first person and scope


#define HBM_FOV 0x40 //offset from fovBase
#define HBM_FOVW2000 -0x810 //W2000 Sniper, offset from fovBase

static uint8_t PS2_HBM_Status(void);
static void PS2_HBM_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Hitman Blood Money",
		PS2_HBM_Status,
		PS2_HBM_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_HITMANBLOODMONEY = &GAMEDRIVER_INTERFACE;

static uint32_t camBase = 0;
static uint32_t fpBase = 0;
static uint32_t fovBase = 0;
//static uint8_t leanState = 0;
static uint8_t scopeState = 0;
static uint32_t equipedID = 0;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_HBM_Status(void)
{
	// SLUS_208.82; (0x93390)
	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323131&&
			PS2_MEM_ReadWord(0x93398) == 0x2E30383B));
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_HBM_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   		return;

	// if(PS2_MEM_ReadUInt8(HBM_PAUSESTATE) == 0x01) //paused or inventory
	// 	return;

	// if(PS2_MEM_ReadUInt8(HBM_CUTSCENESTATE) == 0x01) //in cutscene
	// 	return;

	
	camBase = PS2_MEM_ReadPointer(HBM_CAMBASE);
	camBase = PS2_MEM_ReadPointer(camBase + 0x8C);		
	camBase = PS2_MEM_ReadPointer(camBase + 0x68);
		fovBase = PS2_MEM_ReadPointer(camBase + 0x20);
	camBase = PS2_MEM_ReadPointer(camBase + 0x94);
	

	fpBase = PS2_MEM_ReadPointer(HBM_FPBASE);
	fpBase = PS2_MEM_ReadPointer(fpBase + 0x5C);		
	fpBase = PS2_MEM_ReadPointer(fpBase + 0x98);
	fpBase = PS2_MEM_ReadPointer(fpBase + 0x4);		
	fpBase = PS2_MEM_ReadPointer(fpBase + 0x50);
	//printf("fpBase: %08X\n", fpBase);

	equipedID = PS2_MEM_ReadPointer(0x639160);
	//printf("equipedID: %08X\n", equipedID);
	equipedID = PS2_MEM_ReadUInt16(equipedID + 0x360);	

	
	uint8_t fpsFlag = PS2_MEM_ReadUInt8(HBM_FPSFLAG);
	uint8_t scopingFlag = PS2_MEM_ReadUInt8(HBM_SCOPINGFLAG);


	float looksensitivity = (float)sensitivity;
	
	float fov = PS2_MEM_ReadFloat(fovBase + HBM_FOV);

	//printf("equipedID: %04X		scopingFlag: %01X\n", equipedID, scopingFlag);
	if (equipedID == 0x0037 && scopingFlag == 0x1) //W2000 equiped and scoping
		fov = PS2_MEM_ReadFloat(fovBase + HBM_FOVW2000);


	//printf("fov: %f\n", fov);

	float camY = PS2_MEM_ReadFloat(camBase + HBM_CAMY);
	float camX = PS2_MEM_ReadFloat(camBase + HBM_CAMX);


	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * fov / 15000.f;
	camY = ClampFloat(camY, -1.04719758f, 1.431169987f);

	camX -= (float)xmouse * looksensitivity * fov / 15000.f;

	float fpXsin = PS2_MEM_ReadFloat(fpBase + HBM_FPXSIN);
	float fpXcos = PS2_MEM_ReadFloat(fpBase + HBM_FPXCOS);
	float angle = atan2f(fpXsin, fpXcos);

	angle += (float)xmouse * looksensitivity * fov / 15000.f;
	fpXsin = sin(angle);
	fpXcos = cos(angle);


	if(fpsFlag == 0x0){ //in scope/FPS
		PS2_MEM_WriteFloat(fpBase + HBM_FPXSIN, fpXsin);
		PS2_MEM_WriteFloat(fpBase + HBM_FPXCOS, fpXcos);
		PS2_MEM_WriteFloat(fpBase + HBM_FPXNSIN, -fpXsin);
		PS2_MEM_WriteFloat(fpBase + HBM_FPXCOS2, fpXcos);
	}

	PS2_MEM_WriteFloat(camBase + HBM_CAMY, camY);
	PS2_MEM_WriteFloat(camBase + HBM_CAMX, camX);			
}