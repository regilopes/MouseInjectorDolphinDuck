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

#define SYFI2_CAMY 0x128EC4
#define SYFI2_CAMX 0x128EC8
#define SYFI2_RETICLEY 0x128EF4
#define SYFI2_RETICLEX 0x128EF8

#define SYFI2_BODY 0x1205E0 //body rotation base
#define SYFI2_BODY_ROT -0x11E0 //offset from body base

#define SYFI2_ZOOM 0x1285E8

static uint8_t PS1_SYFI2_Status(void);
static void PS1_SYFI2_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Syphon Filter 2",
	PS1_SYFI2_Status,
	PS1_SYFI2_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_PS1_SYPHONFILTER2 = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.f;
static float yAccumulator = 0.f;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS1_SYFI2_Status(void)
{
	return ((PS1_MEM_ReadWord(0x92BC) == 0x53435553U && // SCUS_946.40;
			 PS1_MEM_ReadWord(0x92C0) == 0x5F393434U &&
			 PS1_MEM_ReadWord(0x92C4) == 0x2E35313BU)||

		    (PS1_MEM_ReadWord(0x92BC) == 0x53435553U && // SCUS_944.92;
		     PS1_MEM_ReadWord(0x92C0) == 0x5F393434U &&
			 PS1_MEM_ReadWord(0x92C4) == 0x2E39323BU)
		); 
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS1_SYFI2_Inject(void)
{
	// disable cam auto-center
	PS1_MEM_WriteWord(0x2F878, 0x00000000);

	if(xmouse == 0 && ymouse == 0 && rx == 0 && ry == 0) // if mouse is idle
		return;

	uint32_t body = PS1_MEM_ReadPointer(SYFI2_BODY);
	//body &= 0x00FFFFFF; // clear upper byte

	int16_t camX = PS1_MEM_ReadHalfword(SYFI2_CAMX);
	int16_t camY = PS1_MEM_ReadHalfword(SYFI2_CAMY);
	int16_t zoom = PS1_MEM_ReadHalfword(SYFI2_ZOOM);
	float camXF = (float)camX;
	float camYF = (float)camY;
	float zoomF = (float)zoom;

	const float looksensitivity = (float)sensitivity / 30.f;

	float xm = (float)xmouse + rx/8192.f;
	float dx = xm * looksensitivity * (zoom / 827.f);
	AccumulateAddRemainder(&camXF, &xAccumulator, xm, dx);

	float ym = (float)(invertpitch ? -ymouse : ymouse) + ry/8192.f;
	float dy = ym * looksensitivity * (zoom / 827.f);
	AccumulateAddRemainder(&camYF, &yAccumulator, ym, dy);



	// clamp y-axis
	if (camYF > 853 )
	 	camYF = 853;
	if (camYF < -853)
	 	camYF = -853;

	PS1_MEM_WriteHalfword(SYFI2_CAMY, (int16_t)camYF);
	PS1_MEM_WriteHalfword(SYFI2_RETICLEY, (int16_t)camYF);
	PS1_MEM_WriteHalfword(SYFI2_CAMX, (int16_t)camXF);
	PS1_MEM_WriteHalfword(SYFI2_RETICLEX, (int16_t)camXF);
	PS1_MEM_WriteHalfword(body + SYFI2_BODY_ROT, (int16_t)camXF);
	//PS1_MEM_WriteHalfword(0x197AB0, (int16_t)camXF);


	
	//printf("camYF: %f\n", camYF);
	// printf("body: %i\n", ry);
}