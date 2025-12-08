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

#define TAU 6.2831853f // 0x40C90FDB

#define MGBH_CAMY_BASE 0x4CEA04
// offset from camYBase
#define MGBH_CAMY 0x6AC
#define MGBH_CAMX_BASE 0x70
// offset from camXBase
#define MGBH_CAMXSIN 0x20
#define MGBH_CAMXCOS 0x28
#define MGBH_CAMXCOS2 0x0
#define MGBH_CAMXSIN_NEG 0x8


#define MGBH_SHIPCTRLBASE 0x0060B61C

#define MGBH_SHIPCTRLY 0x918
#define MGBH_SHIPCTRLX 0x91C

#define MGBH_SHIPFLAG 0x52BE08



// #define MGBH_CAMY 0xD79414
// #define MGBH_CAMXSIN 0x602090
// #define MGBH_CAMXCOS 0x602098
// #define MGBH_CAMXCOS2 0x602070
// #define MGBH_CAMXSIN_NEG 0x602078

static uint8_t PS2_MGBH_Status(void);
static void PS2_MGBH_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Mace Griffin: Bounty Hunter",
	PS2_MGBH_Status,
	PS2_MGBH_Inject,
	1,
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_MACEGRIFFIN = &GAMEDRIVER_INTERFACE;

static uint32_t camYBase = 0;
static uint32_t camXBase = 0;
static uint32_t shipBase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_MGBH_Status(void)
{
	return (PS2_MEM_ReadWord(0x00093390) == 0x534C5553U && 
			PS2_MEM_ReadWord(0x00093394) == 0x5F323035U &&
			PS2_MEM_ReadWord(0x00093398) == 0x2E30353BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_MGBH_Inject(void)
{
	// TODO: camYBase sanity check
	// TODO: ship controls

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	float looksensitivity = (float)sensitivity / 20.f;
	float scale = 800.f;

	camYBase = PS2_MEM_ReadUInt(MGBH_CAMY_BASE);
	camXBase = PS2_MEM_ReadUInt(camYBase + MGBH_CAMX_BASE);

	float camY = PS2_MEM_ReadFloat(camYBase + MGBH_CAMY);
	camY -= (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
	camY = ClampFloat(camY, -1.308996916f, 1.570796371f);

	float camXSin = PS2_MEM_ReadFloat(camXBase + MGBH_CAMXSIN);
	float camXCos = PS2_MEM_ReadFloat(camXBase + MGBH_CAMXCOS);

	float angle = atan(camXSin / camXCos);
	if (camXCos < 0)
		angle += TAU / 2;

	angle += (float)xmouse * looksensitivity / scale;

	camXSin = sin(angle);
	camXCos = cos(angle);



	shipBase = PS2_MEM_ReadPointer(MGBH_SHIPCTRLBASE);

	float rstickY = PS2_MEM_ReadFloat(shipBase + MGBH_SHIPCTRLY);
	float rstickX = PS2_MEM_ReadFloat(shipBase + MGBH_SHIPCTRLX);

	rstickY += (float)(invertpitch ? -ymouse : ymouse) * looksensitivity / scale;
	rstickX += (float)xmouse * looksensitivity / scale;

	ClampFloat(rstickY, -1.f, 1.f);
	ClampFloat(rstickX, -1.f, 1.f);


	
	
	if (PS2_MEM_ReadUInt8(MGBH_SHIPFLAG)){
	
		PS2_MEM_WriteFloat(shipBase + MGBH_SHIPCTRLY, rstickY);
		PS2_MEM_WriteFloat(shipBase + MGBH_SHIPCTRLX, rstickX);

	}else{
		PS2_MEM_WriteFloat(camXBase + MGBH_CAMXSIN, camXSin);
		PS2_MEM_WriteFloat(camXBase + MGBH_CAMXCOS, camXCos);
		PS2_MEM_WriteFloat(camXBase + MGBH_CAMXCOS2, camXCos);
		PS2_MEM_WriteFloat(camXBase + MGBH_CAMXSIN_NEG, -camXSin);
		PS2_MEM_WriteFloat(camYBase + MGBH_CAMY, camY);
	}
}