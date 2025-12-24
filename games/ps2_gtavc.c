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

#define PI 3.14159265f
#define TAU 6.2831853f // 0x40C90FDB

#define GTAVC_FPCAMX 0x005BB15C
#define GTAVC_FPCAMY 0x005BB14C
#define GTAVC_ZOOM 0x005BB154

#define GTAVC_CAMY 0x005BB0F4

#define DEAD_ZONE 0.0f


static uint8_t PS2_GTAVC_Status(void);
static void PS2_GTAVC_Inject(void);


static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Grand Theft Auto Vice City",
	PS2_GTAVC_Status,
	PS2_GTAVC_Inject,
	1,// 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_GTAVC = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_GTAVC_Status(void)
{
	// SLUS_205.52 
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323035U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E35323BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_GTAVC_Inject(void)
{
	// if(xmouse == 0 && ymouse == 0 && rx == 0 && ry == 0) // if mouse or RightStick is idle
	//  	return;
	

	//disabling camX spring
	if (PS2_MEM_ReadUInt(0x00258254) == 0xE66000BC)
		PS2_MEM_WriteUInt(0x00258254, 0x00000000);

	if (PS2_MEM_ReadUInt(0x00258A84) == 0xE66000BC)
		PS2_MEM_WriteUInt(0x00258A84, 0x00000000);

	//CPad::ForceCameraBehindPlayer(void) STUB
	if (PS2_MEM_ReadUInt(0x0027A370) == 0x9482006E){
	 	PS2_MEM_WriteUInt(0x0027A370, 0x03E00008);
	 	PS2_MEM_WriteUInt(0x0027A374, 0x00000000);
	}

	//disabling camY spring
	if (PS2_MEM_ReadUInt(0x00260714) == 0xE4800000)
	  	PS2_MEM_WriteUInt(0x00260714, 0x00000000);

	float looksensitivity = (float)sensitivity;

	float fpcamY = PS2_MEM_ReadFloat(GTAVC_FPCAMY);
	float fpcamX = PS2_MEM_ReadFloat(GTAVC_FPCAMX);
	float zoom = PS2_MEM_ReadFloat(GTAVC_ZOOM);

	float camY = PS2_MEM_ReadFloat(GTAVC_CAMY);



	fpcamY -= (float)((invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 1000000.f + ry/327680000.f * zoom);
	fpcamY = ClampFloat(fpcamY, -1.562069654, 1.04719758);

	fpcamX -= (float)(xmouse * looksensitivity * zoom / 1000000.f + rx/327680000.f * zoom);



	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity /  6000.f + ry/1638400.f;

	float moveSpeed = fabs(PS2_MEM_ReadFloat(0x5BB1A8));

	if (camY > 1.8f && moveSpeed > 0.001f && ymouse == 0)
		camY -= 0.005f;
	if (camY < 1.5f && moveSpeed > 0.001f && ymouse == 0)
		camY += 0.005f;

	if (fpcamY > 0.19f && moveSpeed > 0.001f && ymouse == 0 && ry == 0)
		fpcamY -= fabs(fpcamY) / 300.f;
	if (fpcamY < 0.17f && moveSpeed > 0.001f && ymouse == 0 && ry == 0)
		fpcamY += fabs(fpcamY) / 400.f + 0.001f;

	

	camY = ClampFloat(camY, -1.36, 4.73);





	PS2_MEM_WriteFloat(GTAVC_FPCAMY, fpcamY);
	PS2_MEM_WriteFloat(GTAVC_FPCAMX, fpcamX);

	
	PS2_MEM_WriteFloat(GTAVC_CAMY, camY);

}

