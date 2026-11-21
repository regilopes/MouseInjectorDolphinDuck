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
// #define BF2_CAM_BASE_PTR 0x782B98
#define BF2_SLOT_BASE 0xB6450C
#define BF2_VEHICLE_BASE 0x0
#define BF2_CAM_BASE_PTR 0xB295F0
#define BF2_INPUT_BASE 0xB2A020
//#define BF2_CAM_BASE_PTR 0xF0100 // Needs "Mouse Injector Required" patch active.
// offsets from soldierCamBase
#define BF2_CAMX 0x4
#define BF2_CAMY 0x8

//not offsets
#define BF2_FOV 0x77B3E8
#define BF2_SLOT 0xB6450C
#define BF2_ONVEHICLE 0x75D4EB

//addresses used on the code cave patch for the right analog input injection
#define BF2_CC_INPUTY 0x008D06C4 //most vehicles weapons use this one
#define BF2_CC_INPUTX 0x008D06C8 //most vehicles weapons use this one
#define BF2_CC_HINPUTX 0x008D06CC //humvee machinegun needs this one

static uint8_t PS2_BF2_Status(void);
static uint8_t PS2_BF2_DetectCamBase(void);
static void PS2_BF2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Battlefield 2: Modern Combat",
	PS2_BF2_Status,
	PS2_BF2_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_BATTLEFIELD2 = &GAMEDRIVER_INTERFACE;

static uint32_t soldierCamBase = 0;
static uint32_t inputBase = 0;

static float inputY = 0.f;
static float inputX = 0.f;
static float hInputX = 0.f;




//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_BF2_Status(void)
{
	// SLUS_210.26
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323130U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E32363BU);
}

static uint8_t PS2_BF2_DetectSoldierCamBase(void)
{
	//read the current slot in use (main, left, lower, right, upper)
	uint32_t slot = PS2_MEM_ReadPointer(BF2_SLOT);
	//printf("slot: %08X\n", slot);
	slot = PS2_MEM_ReadUInt(slot + 0x4F8);

	uint32_t tempSoldierCamBase = PS2_MEM_ReadUInt(BF2_CAM_BASE_PTR) + 0x1C8; // 1AD86B0 + 1C8 = 1AD8878
	tempSoldierCamBase = PS2_MEM_ReadUInt(tempSoldierCamBase)+(slot * 4); // 1DDA760
	tempSoldierCamBase = PS2_MEM_ReadUInt(tempSoldierCamBase); // 7F8EF0
	tempSoldierCamBase = PS2_MEM_ReadUInt(tempSoldierCamBase) + 0x1C; // 1B11420 + 1C = 1B1143C
	tempSoldierCamBase = PS2_MEM_ReadUInt(tempSoldierCamBase);

	//printf("tempsoldiercambase: %8x\n", tempSoldierCamBase);

	// TODO: sanity check
	//if (tempSoldierCamBase){
	 	soldierCamBase = tempSoldierCamBase;
	 	return 1;
	//}

	//return 0;
}

static uint8_t PS2_BF2_DetectInputBase(void)
{
	uint32_t tempInputBase = PS2_MEM_ReadUInt(BF2_INPUT_BASE);
	tempInputBase = PS2_MEM_ReadUInt(tempInputBase + 0x48);

	// TODO: sanity check
	if (tempInputBase){
	 	inputBase = tempInputBase;
	 	return 1;
	}

	return 0;
}

static uint8_t PS2_BF2_DetectVehicleBase(void)
{
	uint32_t tempVehicleBase = PS2_MEM_ReadUInt(BF2_VEHICLE_BASE);
	tempVehicleBase = PS2_MEM_ReadUInt(tempVehicleBase + 0x48);

	// TODO: sanity check
	if (tempVehicleBase){
	 	inputBase = tempVehicleBase;
	 	return 1;
	}

	return 0;
}

static void PS2_BF2_Inject(void)
{
	// TODO: tank barrel
	// TODO: turrets
	//			static
	//			car mounted

	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	// 	return;
	
	// if (!PS2_BF2_DetectSoldierCamBase()) // if no camera base detected
	//  	return;

	float looksensitivity = (float)sensitivity / 40.f;
	float scale = 500.f;
	float fov = PS2_MEM_ReadFloat(BF2_FOV) / 54.82500076f;

	

	//--------------------------------------------------------------------------------	
	//changing in-game sensitivity
	PS2_MEM_WriteUInt(0xB626CC, 768); 

	
	
	if(PS2_MEM_ReadUInt8(BF2_ONVEHICLE)){
		if (PS2_BF2_DetectInputBase()){

		//faking Right Analog input to move camera on vehicles.
		inputY = PS2_MEM_ReadFloat(BF2_CC_INPUTY);
		inputY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov * 8.f;
		inputY *= 0.9f;
		inputY = ClampFloat(inputY, -1.0f, 1.0f);

		inputX = PS2_MEM_ReadFloat(BF2_CC_INPUTX);
		inputX += (float)xmouse * looksensitivity / scale * fov * 8.f;
		inputX *= 0.9f;
		inputX = ClampFloat(inputX, -1.0f, 1.0f);

		hInputX = PS2_MEM_ReadFloat(BF2_CC_HINPUTX);
		hInputX += (float)xmouse * looksensitivity / scale * fov * 8.f;
		hInputX *= 0.9f;
		//hInputX += (float)rx * looksensitivity / 32767.f * fov;
		hInputX = ClampFloat(hInputX, -1.0f, 1.0f);

		PS2_MEM_WriteFloat(BF2_CC_INPUTY, (float)inputY);
		PS2_MEM_WriteFloat(BF2_CC_INPUTX, (float)inputX);
		PS2_MEM_WriteFloat(BF2_CC_HINPUTX, (float)hInputX);

		}	
	}else{
		if (PS2_BF2_DetectSoldierCamBase()){

			//on foot soldier camera update
			float camX = PS2_MEM_ReadFloat(soldierCamBase + BF2_CAMX);
			camX += (float)xmouse * looksensitivity / scale * fov;
			float camY = PS2_MEM_ReadFloat(soldierCamBase + BF2_CAMY);
			camY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * fov;


			PS2_MEM_WriteFloat(soldierCamBase + BF2_CAMX, (float)camX);
			PS2_MEM_WriteFloat(soldierCamBase + BF2_CAMY, (float)camY);
			
		}
	}
}