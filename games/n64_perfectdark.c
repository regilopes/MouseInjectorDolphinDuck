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
#include <stdio.h>
#include "../main.h"
#include "../memory.h"
#include "../mouse.h"
#include "game.h"

#define TAU 6.2831853f // 0x40C90FDB
#define PI 3.1415927f // 0x40490FDB

#define PD_ammo 0x80206EF8

// OFFSET addresses, requires a base address to use
#define PD_camy 0x154
#define PD_camx 0x144
#define PD_fov 0x1848

#define PD_camspybase 0x480
#define PD_camspyflag 0x34
#define PD_camspyx 0x1c
#define PD_camspyy 0x28
#define PD_camspyycos 0x2c
#define PD_camspyysin 0x30

// // STATIC addresses
#define PD_playerbase 0x8009A024
#define PD_controlstyle 0x80372728 // instruction reads the current controller style
#define PD_reversepitch 0x803727A0 // instruction reads the current reverse pitch option
#define PD_radialmenutimer 0x802EA2BC // time instruction for radial menu to appear (15 ticks)
#define PD_radialmenualphainit 0x803D2CDC // initial alpha value for all menus
#define PD_camspylookspringup 0x802F13E8 // save instruction for adjusted camspy look spring pitch
#define PD_camspylookspringdown 0x802F143C // save instruction for adjusted camspy look spring pitch


static uint8_t N64_PD_Status(void);
static uint8_t N64_PD_DetectPlayer(void);
static void N64_PD_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Perfect Dark",
	N64_PD_Status,
	N64_PD_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_N64_PERFECTDARK = &GAMEDRIVER_INTERFACE;

uint32_t playerbase = 0;
uint32_t camspybase = 0;
uint32_t camspyflag = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t N64_PD_Status(void)
{
	return (N64_MEM_ReadUInt(0x80000000) == 0x3C1A7000 && N64_MEM_ReadUInt(0x80000004) == 0x275A3500); // unique header in RDRAM
	// return 1; // can't do because it will shortcut before even getting the emuoffset
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void N64_PD_Inject(void)
{
	
	//MEM_WriteUInt(PD_lookahead, 0x0); // disable look ahead
	
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	return;
	
	if(N64_MEM_ReadUInt(PD_camspylookspringup) == 0xE4640028) // add code to remove look spring logic for camspy
		N64_MEM_WriteUInt(PD_camspylookspringup, 0x00000000); // replace instruction with nop
	
	if(N64_MEM_ReadUInt(PD_camspylookspringdown) == 0xE4680028) // add code to remove look spring logic for camspy
		N64_MEM_WriteUInt(PD_camspylookspringdown, 0x00000000); // replace instruction with nop
	
	// if(N64_MEM_ReadUInt(PD_controlstyle) == 0x9042C7FC) // if safe to overwrite
	// 	N64_MEM_WriteUInt(PD_controlstyle, 0x34020001); // always force game to use 1.2 control style

	// if(N64_MEM_ReadUInt(PD_reversepitch) == 0x000F102B) // if safe to overwrite
	// 	N64_MEM_WriteUInt(PD_reversepitch, 0x34020001); // always force game to use upright pitch

	if(N64_MEM_ReadUInt(PD_radialmenutimer) == 0x28410010) // make radial menu trigger quicker (from 15 to 8 ticks)
		N64_MEM_WriteUInt(PD_radialmenutimer, 0x28410009);

	if(N64_MEM_ReadUInt(PD_radialmenualphainit) == 0x3E99999A) // make radial menus initialize with 75% alpha
		N64_MEM_WriteFloat(PD_radialmenualphainit, 0.75f);

	

	playerbase = N64_MEM_ReadUInt(PD_playerbase);

	camspybase = N64_MEM_ReadUInt(playerbase + PD_camspybase);

	camspyflag = N64_MEM_ReadUInt(camspybase + PD_camspyflag);

	

	
	const float looksensitivity = (float)sensitivity / 40.f;


	if(camspyflag){ // if camspy is active, process mouse input
		float camspyx = N64_MEM_ReadFloat(camspybase + PD_camspyx), camspyy = N64_MEM_ReadFloat(camspybase + PD_camspyy);
		camspyx += xmouse / 200.0f * sensitivity; // regular mouselook calculation
		while(camspyx < 0)
			camspyx += 360;
		while(camspyx >= 360)
			camspyx -= 360;
		N64_MEM_WriteFloat(camspybase + PD_camspyx, camspyx);


		if(camspyy > 90) // adjust minus range so clamping is easier to calculate
			camspyy -= 360;

		camspyy += (!invertpitch ? -ymouse : ymouse) / 200.0f * (sensitivity / 1.5f); // slightly weaken y axis so camspy's fisheye effect isn't so sickening

		camspyy = ClampFloat(camspyy, -75, 75); // limit range to usable field of view

		if(camspyy < 0)
			camspyy += 360;
		N64_MEM_WriteFloat(camspybase + PD_camspyy, camspyy);
		const float camspyycos = cosf(camspyy * (1.f * PI / 180.f)), camspyysin = sinf(camspyy * (1.f * PI / 180.f)); // update view matrix angles (engine only does this when necessary - so we manually update it every tick)
		N64_MEM_WriteFloat(camspybase + PD_camspyycos, camspyycos);
		N64_MEM_WriteFloat(camspybase + PD_camspyysin, camspyysin);
	} 


	

	float fov = N64_MEM_ReadFloat(playerbase + PD_fov);

	float camx = N64_MEM_ReadFloat(playerbase + PD_camx);
	camx /= 360.f;
	camx += (float)xmouse / 70.f * looksensitivity / (360.f / TAU) / (90.f / fov); // normal calculation method for X
	camx *= 360.f;
	N64_MEM_WriteFloat(playerbase + PD_camx, camx);


	float camy = N64_MEM_ReadFloat(playerbase + PD_camy);
	camy /= 360.f;
	camy -= (float)(!invertpitch ? ymouse : -ymouse) / 70.f * looksensitivity / (360.f / TAU) / (90.f / fov); // normal calculation method for X
	camy *= 360.f;
	N64_MEM_WriteFloat(playerbase + PD_camy, camy);

}