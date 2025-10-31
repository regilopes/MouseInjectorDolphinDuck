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

#define GTAVC_FPCAMY2 0x004E3248
#define GTAVC_FPCAMX2 0x004E3234
#define GTAVC_ZOOM2 0x004E3274

#define GTAVC_CAMY 0x005BB0F4
#define GTAVC_CAMY2 0x4E31A8
#define GTAVC_BODYANGLE 0xB658E0


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
	if(xmouse == 0 && ymouse == 0 && rx == 0 && ry == 0) // if mouse or RightStick is idle
	 	return;
	

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

	float fpcamY2 = PS2_MEM_ReadFloat(GTAVC_FPCAMY2);
	float fpcamX2 = PS2_MEM_ReadFloat(GTAVC_FPCAMX2);
	float zoom2 = PS2_MEM_ReadFloat(GTAVC_ZOOM2);

	float camY = PS2_MEM_ReadFloat(GTAVC_CAMY);
	float bodyAngle = PS2_MEM_ReadFloat(GTAVC_BODYANGLE);

	

	bodyAngle -= (float)xmouse * looksensitivity * zoom / 1000000.f;
	

	fpcamY -= (float)((invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 1000000.f + ry/327680000.f * zoom);
	fpcamY = ClampFloat(fpcamY, -1.562069654, 1.04719758);

	fpcamX -= (float)(xmouse * looksensitivity * zoom / 1000000.f + rx/327680000.f * zoom);



	fpcamY2 -= (float)((invertpitch ? -ymouse : ymouse) * looksensitivity * zoom2 / 1000000.f + ry/327680000.f * zoom2);
	fpcamY2 = ClampFloat(fpcamY, -1.562069654, 1.04719758);

	fpcamX2 -= (float)(xmouse * looksensitivity * zoom2 / 1000000.f + rx/327680000.f * zoom2);


	

	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity /  6000.f + ry/1638400.f;
	


	camY = ClampFloat(camY, -1.06, 3.73);

		
	

	PS2_MEM_WriteFloat(GTAVC_FPCAMY, fpcamY);
	PS2_MEM_WriteFloat(GTAVC_FPCAMX, fpcamX);

	
	//PS2_MEM_WriteFloat(GTAVC_FPCAMY2, fpcamY2);
	//PS2_MEM_WriteFloat(GTAVC_FPCAMX2, fpcamX2);

	PS2_MEM_WriteFloat(GTAVC_CAMY, camY);
	//PS2_MEM_WriteFloat(GTAVC_CAMY2, camY);

}