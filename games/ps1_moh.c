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

#define MOH_CAMBASE 0x9D680 

//offset from cambase
#define MOH_CAMY 0x546
#define MOH_CAMX 0x552
#define MOH_AIMY 0x34A
#define MOH_FOV 0x470
#define MOH_AIMSTATE 0x10
#define MOH_TURRETSTATE 0xE //offset from cambase

#define MOH_TURRETBASEX 0x99EC4
#define MOH_TURRETX 0x12A //offset from turretbasex
#define MOH_TURRETBASEX_SANITY 0x18C //offset from turretbasex
#define MOH_TURRETBASEY 0x18
#define MOH_TURRETY -0xF6 //offset from turretbaseY


#define MOH_rightstick_y 0xD0C89

static uint8_t PS1_MOH_Status(void);
static void PS1_MOH_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Medal of Honor",
	PS1_MOH_Status,
	PS1_MOH_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway supported for driver
};

const GAMEDRIVER *GAME_PS1_MEDALOFHONOR = &GAMEDRIVER_INTERFACE;

static float xAccumulator = 0.f;
static float yAccumulator = 0.f;
static float txAccumulator = 0.f; //turret
static float tyAccumulator = 0.f; //turret

static uint32_t camBase = 0;
static uint32_t turretBaseX = 0;
static uint32_t turretBaseY = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS1_MOH_Status(void)
{
	return (PS1_MEM_ReadWord(0x92D4) == 0x534C5553U && PS1_MEM_ReadWord(0x92D8) == 0x5F303039U && PS1_MEM_ReadWord(0x92DC) == 0x2E37343BU);
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS1_MOH_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	camBase = (PS1_MEM_ReadPointer(MOH_CAMBASE));

	uint8_t aimState = PS1_MEM_ReadByte(camBase + MOH_AIMSTATE);
	uint8_t turretState = PS1_MEM_ReadByte(camBase + MOH_TURRETSTATE);

	
	uint16_t camX = PS1_MEM_ReadHalfword(camBase + MOH_CAMX);
	uint16_t camY = PS1_MEM_ReadHalfword(camBase  + MOH_CAMY);

	uint16_t fov = PS1_MEM_ReadHalfword(camBase  + MOH_FOV);

	float camXF = (float)camX;
	float camYF = (float)camY;
	
	
	const float looksensitivity = (float)sensitivity / 20.f;
	float scale = 1.f;
	
	float dx = (float)xmouse * looksensitivity * scale / (fov / 400.f);
	AccumulateAddRemainder(&camXF, &xAccumulator, xmouse, dx);
	
	float ym = (float)(invertpitch ? -ymouse : ymouse);
	float dy = -ym * looksensitivity * scale / (fov / 400.f);
	AccumulateAddRemainder(&camYF, &yAccumulator, -ym, dy);
	
	
	// clamp y-axis
	if (camYF > 60000 && camYF < 64854)
	camYF = 64854;
	if (camYF > 682 && camYF < 4000)
	camYF = 682;
	
	
	turretBaseY = PS1_MEM_ReadPointer(MOH_TURRETBASEY);
	
	
	
	if (turretState == 0x02) //in turret
	{
		if (turretBaseX == 0)
		{
			//return if not a pointer
			if (PS1_MEM_ReadByte(MOH_TURRETBASEX + 0x3) != 0x80) 
				return;

				turretBaseX = PS1_MEM_ReadPointer(MOH_TURRETBASEX);

			uint32_t sanity_address = turretBaseX + MOH_TURRETBASEX_SANITY;

			// check that pointer points to turret
			if (PS1_MEM_ReadByte(sanity_address) != 0x96) //sanity value
			{
				turretBaseX = 0;
				return;
			}
		}


		uint16_t turretX = PS1_MEM_ReadHalfword(turretBaseX + MOH_TURRETX);
		uint16_t turretY = PS1_MEM_ReadHalfword(turretBaseY + MOH_TURRETY);
		
		//for clamping calculation
		uint16_t turretXcenter = PS1_MEM_ReadHalfword(turretBaseX + MOH_TURRETX); 
		uint16_t turretYcenter = PS1_MEM_ReadHalfword(turretBaseY + MOH_TURRETY);
		
		float turretXF = (float)turretX;
		float turretYF = (float)turretY;


		float dtx = (float)xmouse * looksensitivity * scale;
		AccumulateAddRemainder(&turretXF, &txAccumulator, xmouse, dtx);
		
		float tym = (float)(invertpitch ? ymouse : -ymouse);
		float dty = -tym * looksensitivity * scale / 6.f;
		AccumulateAddRemainder(&turretYF, &tyAccumulator, -tym, dty);
		
		
		// // clamp y-axis
		// if (turretYF > 60000 && turretYF < 65273)
		// turretYF = 65273;
		// if (turretYF > 65443 && turretYF < 65535 || turretYF < 1000)
		// turretYF = 65443;
		
		PS1_MEM_WriteHalfword(turretBaseY + MOH_TURRETY, (uint16_t)turretYF);
		PS1_MEM_WriteHalfword(turretBaseX + MOH_TURRETX, (uint16_t)turretXF);
	}else{ //on foot
		turretBaseX = 0; //for turret sanity check work
		if(aimState == 0x10){ //only when aiming

			uint8_t stick_y = PS1_MEM_ReadByte(MOH_rightstick_y);
		
			// simulate right stick movement due to not being able to find a writeable camy value
			if (ymouse < 0)
			stick_y = 8;
			else
			stick_y = 247;
			
			PS1_MEM_WriteByte(MOH_rightstick_y, stick_y);
		}else{
			PS1_MEM_WriteHalfword(camBase + MOH_CAMY, (uint16_t)camYF);
		}
		PS1_MEM_WriteHalfword(camBase + MOH_CAMX, (uint16_t)camXF);
	}
	
	
	//printf("turretBaseX is: 0x%08X\n", turretBaseX);
	
	//printf("camXF is: 0x%i\n", (int8_t)stick_y);
	
}