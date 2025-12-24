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

#define TAU 6.2831853f // 0x40C90FDB

#define SA_PED_POINTER 0x7095d0
#define SA_PLAYER_POINTER 0x14

#define SA_AIMING_STATE 0x560 //offset from ped pointer
#define SA_BODYANGLE 0x598 //offset from ped pointer

#define SA_CAMXSIN 0x6FE87C
#define SA_CAMXCOS 0x6FE880
#define SA_CAMY 0x6FE884
#define SA_CAMY2 0x6FE890

#define SA_CAMXSIN_MID 0x6FE7EC
#define SA_CAMXCOS_MID 0x6FE7F0
#define SA_CAMY_MID 0x6FE7F4

#define SA_AIMY 0x6FE750
#define SA_AIMX 0x6FE760

#define SA_ZOOM 0x6FE758

static uint8_t PS2_SA_Status(void);
static void PS2_SA_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Grand Theft Auto: San Andreas",
	PS2_SA_Status,
	PS2_SA_Inject,
	1,
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_GTASA = &GAMEDRIVER_INTERFACE;
static uint32_t ped_p = 0;
static uint32_t aim_state = 0;
static uint32_t aimlock_state = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_SA_Status(void)
{
	// SLUS_209.46
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323039U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E34363BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_SA_Inject(void)
{
	ped_p = PS2_MEM_ReadPointer(SA_PED_POINTER);
	aim_state = PS2_MEM_ReadUInt8(ped_p + SA_AIMING_STATE);
	aimlock_state = PS2_MEM_ReadUInt8(0x688254);


	if(xmouse == 0 && ymouse == 0 && rx == 0 && ry == 0) // if mouse and gamepad idle
		return;


	//disabling ped camY spring
	if (PS2_MEM_ReadUInt(0x002095F0) == 0xE60100AC)
	  	PS2_MEM_WriteUInt(0x002095F0, 0x00000000);

	//disabling ped camX wobble
	if (PS2_MEM_ReadUInt(0x00209524) == 0xE60100BC)
		PS2_MEM_WriteUInt(0x00209524, 0x00000000);


	float looksensitivity = (float)sensitivity / 20.f;
	float scale = 400.f;
	
	float aimY = (PS2_MEM_ReadFloat(SA_AIMY));
	float aimX = (PS2_MEM_ReadFloat(SA_AIMX));

	float zoom = (PS2_MEM_ReadFloat(SA_ZOOM));

	float bodyAngle = PS2_MEM_ReadFloat(ped_p + SA_BODYANGLE);

	float camYMid = PS2_MEM_ReadFloat(SA_CAMY_MID);
	float camXSinMid = PS2_MEM_ReadFloat(SA_CAMXSIN_MID);
	float camXCosMid = PS2_MEM_ReadFloat(SA_CAMXCOS_MID);
	
	float camY = (PS2_MEM_ReadFloat(SA_CAMY) - camYMid) / 10.f;


	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
	// camY = ClampFloat(camY, -0.707106, 0.422618);
	camY = (camY * 10.f) + camYMid;
	
	
	float camXSin = (PS2_MEM_ReadFloat(SA_CAMXSIN) - camXSinMid) / 10.f;
	float camXCos = (PS2_MEM_ReadFloat(SA_CAMXCOS) - camXCosMid) / 10.f;
	
	float angle = atan(camXSin / camXCos);
	if (camXCos < 0)
	angle += TAU / 2;
	
	angle += (float)xmouse * looksensitivity / scale / 2.f;
	
	camXSin = (sin(angle) * 10.f) + camXSinMid;
	camXCos = (cos(angle) * 10.f) + camXCosMid;

	
	aimY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * zoom / 140.f + ry/3276805.f * zoom / 100.f; // + ry/3276800.f * zoom / 70.f
	aimY = ClampFloat(aimY, -1.483153, 0.785006); 

	aimX -= (float)xmouse * looksensitivity / scale * zoom / 140.f + rx/3276805.f * zoom / 100.f;



	//while aiming, needed to transition from aim lock to manual aiming when moving outside the current target
	if(aimlock_state == 255){ 
		if (xmouse > 0){
			PS2_MEM_WriteUInt8(0x700944, 255);
			PS2_MEM_WriteUInt8(0x7009C4, 255);
		}
	
		if (xmouse < 0){
			PS2_MEM_WriteUInt8(0x700944, 0);
			PS2_MEM_WriteUInt8(0x7009C4, 0);
		}
	}
	

	if(xmouse != 0 && ymouse != 0){ //only mouse movement
		PS2_MEM_WriteFloat(SA_CAMXSIN, camXSin);
		PS2_MEM_WriteFloat(SA_CAMXCOS, camXCos);
		PS2_MEM_WriteFloat(SA_CAMY, camY);
		PS2_MEM_WriteFloat(SA_CAMY2, camY);
	}


	if(PS2_MEM_ReadUInt(0x6FE8C0) == ped_p){ //on foot
		PS2_MEM_WriteFloat(SA_AIMY, aimY);
		PS2_MEM_WriteFloat(SA_AIMX, aimX);
	}
	//printf("xmouse: %i\n", xmouse);
}