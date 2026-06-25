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

#define LCS_MAIMXSIN 0x017E6550
#define LCS_MAIMXCOS 0x017E6554
#define LCS_MAIMXY 0x017E6558

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
	PS2_MEM_ReadUInt(0x3D9898) != 0){  //or inside vehicle
		return;
	}else{
	//Wall/floor collision stutter fix
	//if (PS2_MEM_ReadUInt(0x00299634) == 0xDFB000D0)
	
	//disabling ped camY spring
	if (PS2_MEM_ReadUInt(0x00283184) == 0xE64000E0)
	PS2_MEM_WriteUInt(0x00283184, 0x00000000);
	}

	//disabling manual aim Y (maimY) spring middle snap
	if (PS2_MEM_ReadUInt(0x0034AD9C) == 0x0C04115A)
	PS2_MEM_WriteUInt(0x0034AD9C, 0x00000000);

	//disabling manual aim Y (maimY) spring
	if (PS2_MEM_ReadUInt(0x0034AC20) == 0x0C04115A)
	PS2_MEM_WriteUInt(0x0034AC20, 0x00000000);
	



// //disabling car camY spring
// 	if (PS2_MEM_ReadUInt(0x002855AC) == 0xE64000E0)
// 	PS2_MEM_WriteUInt(0x002855AC, 0x00000000);


	float looksensitivity = (float)sensitivity / 20.f;
	float scale = 400.f;
	
	float aimY = (PS2_MEM_ReadFloat(LCS_AIMY));
	float aimX = (PS2_MEM_ReadFloat(LCS_AIMX));

	float zoom = (PS2_MEM_ReadFloat(LCS_ZOOM));

	float camYMid = PS2_MEM_ReadFloat(LCS_CAMY_MID);
	float camXSinMid = PS2_MEM_ReadFloat(LCS_CAMXSIN_MID);
	float camXCosMid = PS2_MEM_ReadFloat(LCS_CAMXCOS_MID);

	float camXSin = (PS2_MEM_ReadFloat(LCS_CAMXSIN) - camXSinMid) / 10.f;
	float camXCos = (PS2_MEM_ReadFloat(LCS_CAMXCOS) - camXCosMid) / 10.f;
	
	float camY = (PS2_MEM_ReadFloat(LCS_CAMY) - camYMid) / 10.f;

	camY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale + ry/3276800.f;
	// camY = ClampFloat(camY, -0.707106, 0.422618);
	camY = (camY * 10.f) + camYMid;
	
	float angle = atan(camXSin / camXCos);
	if (camXCos < 0)
	angle += TAU / 2;
	
	angle += (float)xmouse * looksensitivity / scale;
	
	camXSin = (sin(angle) * 10.f) + camXSinMid;
	camXCos = (cos(angle) * 10.f) + camXCosMid;


	
	aimY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale * zoom / 140.f + ry/3276800.f * zoom / 100.f; //+ ry/3276800.f * zoom / 70.f
	
	//if(ry == 0)
	//	aimY -= aimY / 30.f;

	aimX -= (float)xmouse * looksensitivity / scale * zoom / 140.f + rx/3276800.f * zoom / 100.f;
	aimY = ClampFloat(aimY, -1.56, 1.04);

	
	if(PS2_MEM_ReadUInt8(0x3D9034) != 1){ //only when not locked on target
	PS2_MEM_WriteFloat(LCS_CAMXSIN, camXSin);
	PS2_MEM_WriteFloat(LCS_CAMXCOS, camXCos);
	PS2_MEM_WriteFloat(LCS_CAMY, camY);
	PS2_MEM_WriteFloat(LCS_CAMY2, camY);
	
		PS2_MEM_WriteFloat(LCS_AIMY, aimY);
		PS2_MEM_WriteFloat(LCS_AIMX, aimX);
	}

	//While manual aiming / aiming as a vehicle passenger
	float maimXSin = (PS2_MEM_ReadFloat(LCS_MAIMXSIN) - camXSinMid) / 30.f;
	float maimXCos = (PS2_MEM_ReadFloat(LCS_MAIMXCOS) - camXCosMid) / 30.f;
	float maimY = (PS2_MEM_ReadFloat(LCS_MAIMXY) - camYMid) / 30.f;

	maimY += (float)(invertpitch ? ymouse : -ymouse) * looksensitivity / scale;
	maimY = ClampFloat(maimY, -0.7, 0.7);
	//printf("maimY: %f\n", maimY);
	maimY = (maimY * 30.f) + camYMid;


	float maimAngle = atan(maimXSin / maimXCos); 
	if (maimXCos < 0)
		maimAngle += TAU / 2;

	maimAngle += (float)xmouse * looksensitivity / scale;

	maimXSin = (sin(maimAngle) * 30.f) + camXSinMid;
	maimXCos = (cos(maimAngle) * 30.f) + camXCosMid;



	PS2_MEM_WriteFloat(LCS_MAIMXSIN, maimXSin);
	PS2_MEM_WriteFloat(LCS_MAIMXCOS, maimXCos);
	PS2_MEM_WriteFloat(LCS_MAIMXY, maimY);

}