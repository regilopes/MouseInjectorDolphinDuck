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

#define GTA3_FPCAMY 0x004E2FAC
#define GTA3_FPCAMX 0x004E2F98
#define GTA3_ZOOM 0x004E2FD8

#define GTA3_FPCAMY2 0x004E3248
#define GTA3_FPCAMX2 0x004E3234
#define GTA3_ZOOM2 0x004E3274

#define GTA3_CAMY 0x4E2F0C
#define GTA3_CAMY2 0x4E31A8
#define GTA3_BODYANGLE 0xB658E0


static uint8_t PS2_GTA3_Status(void);
static void PS2_GTA3_Inject(void);


static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Grand Theft Auto III",
	PS2_GTA3_Status,
	PS2_GTA3_Inject,
	1,// 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_GTA3 = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_GTA3_Status(void)
{
	// SLUS_200.62
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323030U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E36323BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_GTA3_Inject(void)
{
	if(xmouse == 0 && ymouse == 0 && rx == 0 && ry == 0) // if mouse or RightStick is idle
	 	return;
	

	// //disabling camX spring
	if (PS2_MEM_ReadUInt(0x2632F8) == 0xE6A000D8)
		PS2_MEM_WriteUInt(0x002632F8, 0x00000000);

	if (PS2_MEM_ReadUInt(0x00262474) == 0xE6A000D8)
		PS2_MEM_WriteUInt(0x00262474, 0x00000000);

	//CPad::ForceCameraBehindPlayer(void) STUB
	if (PS2_MEM_ReadUInt(0x0027F900) == 0x9082005D){
	 	PS2_MEM_WriteUInt(0x0027F900, 0x03E00008);
	 	PS2_MEM_WriteUInt(0x0027F904, 0x00000000);
	}

	//disabling camY spring
	if (PS2_MEM_ReadUInt(0x00247588) == 0xE4800000)
	  	PS2_MEM_WriteUInt(0x00247588, 0x00000000);

	//fixing cam wall colision twitching
	if (PS2_MEM_ReadUInt(0x00262830) == 0xE6A000D8)
	PS2_MEM_WriteUInt(0x00262830, 0x00000000);


	float looksensitivity = (float)sensitivity;

	float fpcamY = PS2_MEM_ReadFloat(GTA3_FPCAMY);
	float fpcamX = PS2_MEM_ReadFloat(GTA3_FPCAMX);
	float zoom = PS2_MEM_ReadFloat(GTA3_ZOOM);

	float fpcamY2 = PS2_MEM_ReadFloat(GTA3_FPCAMY2);
	float fpcamX2 = PS2_MEM_ReadFloat(GTA3_FPCAMX2);
	float zoom2 = PS2_MEM_ReadFloat(GTA3_ZOOM2);

	float camY = PS2_MEM_ReadFloat(GTA3_CAMY);
	float bodyAngle = PS2_MEM_ReadFloat(GTA3_BODYANGLE);

	

	bodyAngle -= (float)xmouse * looksensitivity * zoom / 1000000.f;
	

	fpcamY -= (float)((invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 1000000.f + ry/327680000.f * zoom);
	fpcamY = ClampFloat(fpcamY, -1.562069654, 1.04719758);

	fpcamX -= (float)(xmouse * looksensitivity * zoom / 1000000.f + rx/327680000.f * zoom);



	fpcamY2 -= (float)((invertpitch ? -ymouse : ymouse) * looksensitivity * zoom2 / 1000000.f + ry/327680000.f * zoom2);
	fpcamY2 = ClampFloat(fpcamY, -1.562069654, 1.04719758);

	fpcamX2 -= (float)(xmouse * looksensitivity * zoom2 / 1000000.f + rx/327680000.f * zoom2);


	

	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity /  6000.f + ry/1638400.f;
	


	camY = ClampFloat(camY, -1.06, 3.73);

		
	

	PS2_MEM_WriteFloat(GTA3_FPCAMY, fpcamY);
	PS2_MEM_WriteFloat(GTA3_FPCAMX, fpcamX);

	
	PS2_MEM_WriteFloat(GTA3_FPCAMY2, fpcamY2);
	PS2_MEM_WriteFloat(GTA3_FPCAMX2, fpcamX2);

	PS2_MEM_WriteFloat(GTA3_CAMY, camY);
	PS2_MEM_WriteFloat(GTA3_CAMY2, camY);

}