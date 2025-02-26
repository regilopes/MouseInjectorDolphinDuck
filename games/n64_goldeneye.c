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
#define GOLD_ammo 0xD37FC

// OFFSET addresses, requires a base address to use
#define GOLD_camx 0x000D30A8 - 0x000D2F60 // 0x148
#define GOLD_camy 0x000D30B8 - 0x000D2F60 // 0x158
#define GOLD_controlscheme 0x800D59B8 - 0x800D2F60 // 0x2A58
#define GOLD_fov 0x800D4124 - 0x800D2F60 //0x11C4

// STATIC addresses
#define GOLD_playerbase 0x80079EE0
#define GOLD_tankbase 0x803644C // may not be needed but saving it here for now
#define GOLD_isingame1 0x800368B4
#define GOLD_isingame2 0x800484C0
#define GOLD_lookahead 0x80040AB4
#define GOLD_menucursorx 0x8002A908
#define GOLD_menucursory 0x8002A90C
#define GOLD_tankcamx 0x80036484
#define GOLD_isintank 0x80036448 // is the player in a tank? 0=no, 1=yes

static uint8_t N64_GOLD_Status(void);
static uint8_t N64_GOLD_DetectPlayer(void);
static void N64_GOLD_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"007: GoldenEye",
	N64_GOLD_Status,
	N64_GOLD_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_N64_GOLDENEYE = &GAMEDRIVER_INTERFACE;

static uint32_t playerbase = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t N64_GOLD_Status(void)
{
	return (N64_MEM_ReadUInt(0x80000000) == 0x40802800 && N64_MEM_ReadUInt(0x80000004) == 0x00000000); // unique header in RDRAM
	// return 1; // can't do because it will shortcut before even getting the emuoffset
}
//==========================================================================
// Purpose: determines if there is a player
//==========================================================================
static uint8_t N64_GOLD_DetectPlayer(void)
{
	const uint32_t tempplayerbase = N64_MEM_ReadUInt(GOLD_playerbase);

	if (N64WITHINMEMRANGE(tempplayerbase) && N64_MEM_ReadUInt(tempplayerbase + 0x20) == 0x3F800000) // arbitrary offset value that is stable
	{
		playerbase = tempplayerbase;
		return 1;
	}

	return 0;
}
//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void N64_GOLD_Inject(void)
{
	// infinite ammo lvl 1
	// PS1_MEM_WriteByte(GOLD_ammo, 0x7);

	MEM_WriteUInt(GOLD_lookahead, 0x0); // disable look ahead

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	// TODO:
	// Test all singleplayer
	// find value for gun sway left/right when turning, as of now he just rigidly turns
	// find fov to make zoom less sensitive
	// disable camera movement during level intros

	const float looksensitivity = (float)sensitivity / 40.f;

	// uint32_t ingame1 = N64_MEM_ReadUInt(GOLD_isingame1);
	// uint32_t ingame2 = N64_MEM_ReadUInt(GOLD_isingame2);
	
	float cursorx = N64_MEM_ReadFloat(GOLD_menucursorx); // bounds: 20-420
	float cursory = N64_MEM_ReadFloat(GOLD_menucursory); // 20-310

	cursorx += (float)xmouse * looksensitivity;
	cursory += (float)ymouse * looksensitivity;

	cursorx = ClampFloat(cursorx, 20.f, 420.f);
	cursory = ClampFloat(cursory, 20.f, 310.f);

	N64_MEM_WriteFloat(GOLD_menucursorx, cursorx);
	N64_MEM_WriteFloat(GOLD_menucursory, cursory);

	// lock control scheme to 1.4 Goodnight
	//uint32_t controls = N64_MEM_ReadUInt(playerbase + GOLD_controlscheme);
	// if (controls != 0x3)
	// {
	// 	N64_MEM_WriteUInt(playerbase + GOLD_controlscheme, 0x00000003);
	// 	N64_MEM_WriteUInt(playerbase + GOLD_controlscheme + 0x4, 0xFFFFFFE2);
	// 	N64_MEM_WriteUInt(playerbase + GOLD_controlscheme + 0x8, 0x405FFFEE);
	// 	N64_MEM_WriteUInt(playerbase + GOLD_controlscheme + 0xC, 0xFFFFFFE2);
	// }

	if (!(N64_GOLD_DetectPlayer())) // return if no playerbase found
		return;

	
	float fov = N64_MEM_ReadFloat(playerbase + GOLD_fov);
	// float fov = 60;


	if (N64_MEM_ReadUInt(GOLD_isintank))
	{
		float camx = N64_MEM_ReadFloat(GOLD_tankcamx);
		camx += (float)xmouse / 400.f * looksensitivity; // normal calculation method for X
		N64_MEM_WriteFloat(GOLD_tankcamx, camx);
	}
	else {
		float camx = N64_MEM_ReadFloat(playerbase + GOLD_camx);
		camx /= 360.f;
		camx += (float)xmouse / 10.f * looksensitivity / (360.f / TAU) / (90.f / fov); // normal calculation method for X
		camx *= 360.f;
		N64_MEM_WriteFloat(playerbase + GOLD_camx, camx);
	}

	float camy = N64_MEM_ReadFloat(playerbase + GOLD_camy);
	camy /= 360.f;
	camy -= (float)ymouse / 10.f * looksensitivity / (360.f / TAU) / (90.f / fov); // normal calculation method for X
	camy *= 360.f;
	N64_MEM_WriteFloat(playerbase + GOLD_camy, camy);
}