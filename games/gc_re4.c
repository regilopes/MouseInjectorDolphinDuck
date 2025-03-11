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

// TODO:
//-find reliable aimY pointer
//-del lago harpoon aiming not working on first try

#define TAU 6.2831853f // 0x40C90FDB
//aimBase pointer
#define RE4_AIMBASEX 0x8027DF20
#define RE4_AIMBASEY_1 0x80278C08

//X axis
#define RE4_AIMX 0xA4 //offset from aimBaseX
#define RE4_HARPOONX 0x81052D34
#define RE4_BHARPOONX 0x81085B64

//Y axis
#define RE4_AIMY 0x8021873C //2.0
#define RE4_AIMY_OFFSET 0x7428
#define RE4_SCOPEY 0x80212A4C
#define RE4_HARPOONY 0x81052D20


//weapon equiped
#define RE4_WEPID 0x8028478C
#define RE4_HARPOON 0x80FDB768



//aim functions
#define RE4_LOCK_RAND 0x801447b8
#define RE4_LOCK_PITCH 0x80144d6c
#define RE4_LOCK_CTRL 0x801444d4


//cam coords
// #define RE4_CAMX 0x80212928
#define RE4_CAMY 0x80212924


// #define RE4_fFOV1 0x102DCC
// #define RE4_fFOV2 0x102DDC
#define RE4_FOV 0x80213084

static uint8_t GC_RE4_Status(void);
static void GC_RE4_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Resident Evil 4",
	GC_RE4_Status,
	GC_RE4_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_GC_RE4 = &GAMEDRIVER_INTERFACE;

static uint32_t aimBaseX = 0;
static uint32_t aimBaseY = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t GC_RE4_Status(void)
{
	// G4BE08
	return (MEM_ReadUInt(0x80000000) == 0x47344245U && 
			MEM_ReadUInt(0x80000004) == 0x30380000U);

}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void GC_RE4_Inject(void)
{
	//cam dist to far
	if (MEM_ReadInt(0x800BC32C) == 0x4182000C) //
		MEM_WriteInt(0x800BC32C, 0x60000000);

	//disable ready animation
	if (MEM_ReadInt(0x80044580) == 0x39200004) //knife
		MEM_WriteInt(0x80044580, 0x60000000);
	if (MEM_ReadInt(0x80923D84) == 0x39400004) //handgun
		MEM_WriteInt(0x80923D84, 0x60000000);
	if (MEM_ReadInt(0x80928AE4) == 0x39400004) //red9
		MEM_WriteInt(0x80928AE4, 0x60000000);

	if (MEM_ReadInt(0x8092346C) == 0x39000000) //shotgun
		MEM_WriteInt(0x8092346C, 0x60000000);
	if (MEM_ReadInt(0x8091F02C) == 0x39000000) //striker
		MEM_WriteInt(0x8091F02C, 0x60000000);
	if (MEM_ReadInt(0x8092E58C) == 0x39000000) //riot
		MEM_WriteInt(0x8092E58C, 0x60000000);

	if (MEM_ReadInt(0x80927F54) == 0x39600004) //SMG
		MEM_WriteInt(0x80927F54, 0x60000000);
	if (MEM_ReadInt(0x8091D5F8) == 0x39600004) //thompson
		MEM_WriteInt(0x8091D5F8, 0x60000000);

	//disabling camY re-centering
	if (MEM_ReadUInt(0x800BBDBC) == 0xD1BF0200U)
		MEM_WriteUInt(0x800BBDBC, 0x60000000U);
	

	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;
		

	aimBaseX = MEM_ReadUInt(RE4_AIMBASEX);
	aimBaseY = MEM_ReadUInt(RE4_AIMBASEY_1);

	//checking and disabling aim shake function so aim on Y axis doesn't need
	//an address with a pointer to work on all rooms
	if (MEM_ReadUInt(RE4_LOCK_RAND) == 0x9421ffc8U) 
	 	MEM_WriteUInt(RE4_LOCK_RAND, 0x4e800020U);

	// if (MEM_ReadUInt(RE4_LOCK_PITCH) == 0xd02b0004U) //checking and disabling aimY reset to 0 when aiming
	//  	MEM_WriteUInt(RE4_LOCK_PITCH, 0x60000000U);	

	

	

	float fov = MEM_ReadFloat(RE4_FOV);
	float looksensitivity = (float)sensitivity;
	
	float scale = 800.f;
	

	//aimBaseX = MEM_ReadPointer(RE4_AIMBASEPTR_X);
	//aimBaseY = MEM_ReadPointer(RE4_AIMBASEPTR_Y);

	float aimX = MEM_ReadFloat(aimBaseX + RE4_AIMX); //while aiming
	//float aimY = MEM_ReadFloat(0x811924C8); 
	float aimY = MEM_ReadFloat(RE4_AIMY); //aimY 2.0

	// float camX = MEM_ReadFloat(RE4_CAMX); //while not aiming
	//float camY = MEM_ReadFloat(RE4_CAMY);


	// camX += (float)-xmouse * looksensitivity / (scale * 2);
	// camY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 2);
	// camX = ClampFloat(camX, -2.0f, 2.f);

	
	
	aimX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
	

	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f); //aimY 2.0
	//aimY = ClampFloat(aimY, -1.570796371f, 1.570796371f);
	aimY = ClampFloat(aimY, -1.f, 1.f);  //aimY2.0
	
	
	MEM_WriteFloat(aimBaseX + RE4_AIMX, aimX);


	if(MEM_ReadUInt8(RE4_WEPID) == 0x9 || MEM_ReadUInt8(RE4_WEPID) == 0x0A){ //check if rifle is equiped
		float scopeY = MEM_ReadFloat(RE4_SCOPEY);
		scopeY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * (fov / 45.f) / (scale * 45.f);
		scopeY = ClampFloat(scopeY, -1.22f, 1.22f);		
		MEM_WriteFloat(RE4_SCOPEY, scopeY);
		MEM_WriteFloat(RE4_AIMY, aimY);  //aimY 2.0
	}else if(MEM_ReadUInt(RE4_HARPOON) == 0x00000001){ //check if it's aiming with harpoon
		float harpoonY = MEM_ReadFloat(RE4_HARPOONY);
		float harpoonX = MEM_ReadFloat(RE4_HARPOONX);
		float bharpoonX = MEM_ReadFloat(RE4_BHARPOONX);
		harpoonY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / 65;
		harpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
		bharpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
		harpoonY = ClampFloat(harpoonY, -255.f, 255.f);
		harpoonX = ClampFloat(harpoonX, -0.4f, 0.4f);
		MEM_WriteFloat(RE4_HARPOONY, harpoonY);

		if(-0.38f < harpoonX && harpoonX < 0.38f)
			MEM_WriteFloat(RE4_HARPOONX, harpoonX);
		else
			MEM_WriteFloat(RE4_BHARPOONX, bharpoonX);
	}else{
		MEM_WriteFloat(RE4_AIMY, aimY);  //aimY 2.0
	}
	
		


	MEM_WriteFloat((aimBaseY + RE4_AIMY_OFFSET), (aimY*1.570796371f));	
	

	// MEM_WriteFloat(RE4_CAMX, camX);
	MEM_WriteFloat(RE4_CAMY, aimY*1.4f);
	
}