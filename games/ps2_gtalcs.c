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

#define LCS_CAMXSIN 0x43C030
#define LCS_CAMXCOS 0x43C034
#define LCS_CAMY 0x43C038
#define LCS_CAMY2 0x43C048

#define LCS_CAMXSIN_MID 0x43BF70
#define LCS_CAMXCOS_MID 0x43BF74
#define LCS_CAMY_MID 0x43BF78

#define LCS_AIMY 0x43BEF0
#define LCS_AIMX 0x43BF00

#define LCS_ZOOM 0x43BEF8

static uint8_t PS2_LCS_Status(void);
static void PS2_LCS_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Grand Theft Auto: Liberty City Stories",
	PS2_LCS_Status,
	PS2_LCS_Inject,
	1,
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_GTALCS = &GAMEDRIVER_INTERFACE;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_LCS_Status(void)
{
	// SLUS_214.23
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323134U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E32333BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_LCS_Inject(void)
{
	if(xmouse == 0 && ymouse == 0 && // if mouse idle
	PS2_MEM_ReadInt(0x3D9898) != 0)  //or inside vehicle
		return;


	//Wall/floor collision stutter fix
	if (PS2_MEM_ReadUInt(0x002995EC) == 0x45000011)
		  PS2_MEM_WriteUInt(0x002995EC, 0x00000000);


	//disabling ped camY spring
	if (PS2_MEM_ReadUInt(0x00283184) == 0xE64000E0)
	  	PS2_MEM_WriteUInt(0x00283184, 0x00000000);

	// //disabling car camY spring
	// if (PS2_MEM_ReadUInt(0x002855AC) == 0xE64000E0)
	// PS2_MEM_WriteUInt(0x002855AC, 0x00000000);


	float looksensitivity = (float)sensitivity / 20.f;
	float scale = 400.f;
	
	float aimY = (PS2_MEM_ReadFloat(LCS_AIMY));
	float aimX = (PS2_MEM_ReadFloat(LCS_AIMX));

	float zoom = (PS2_MEM_ReadFloat(LCS_ZOOM));

	float camYMid = PS2_MEM_ReadFloat(LCS_CAMY_MID);
	float camXSinMid = PS2_MEM_ReadFloat(LCS_CAMXSIN_MID);
	float camXCosMid = PS2_MEM_ReadFloat(LCS_CAMXCOS_MID);
	
	float camY = (PS2_MEM_ReadFloat(LCS_CAMY) - camYMid) / 10.f;
	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
	// camY = ClampFloat(camY, -0.707106, 0.422618);
	camY = (camY * 10.f) + camYMid;
	
	
	float camXSin = (PS2_MEM_ReadFloat(LCS_CAMXSIN) - camXSinMid) / 10.f;
	float camXCos = (PS2_MEM_ReadFloat(LCS_CAMXCOS) - camXCosMid) / 10.f;
	
	float angle = atan(camXSin / camXCos);
	if (camXCos < 0)
	angle += TAU / 2;
	
	angle += (float)xmouse * looksensitivity / scale;
	
	camXSin = (sin(angle) * 10.f) + camXSinMid;
	camXCos = (cos(angle) * 10.f) + camXCosMid;


	
	aimY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * zoom / 70.f + ry/3276800.f * zoom / 100.f; //+ ry/3276800.f * zoom / 70.f
	
	//if(ry == 0)
	//	aimY -= aimY / 30.f;

	aimX -= (float)xmouse * looksensitivity / scale * zoom / 70.f;


	PS2_MEM_WriteFloat(LCS_CAMXSIN, camXSin);
	PS2_MEM_WriteFloat(LCS_CAMXCOS, camXCos);
	PS2_MEM_WriteFloat(LCS_CAMY, camY);
	PS2_MEM_WriteFloat(LCS_CAMY2, camY);
	PS2_MEM_WriteFloat(LCS_AIMY, aimY);
	PS2_MEM_WriteFloat(LCS_AIMX, aimX);

}