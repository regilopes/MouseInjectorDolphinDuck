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
//implement camX look only when standing

#define TAU 6.2831853f // 0x40C90FDB
//aimBase pointer
#define RE4_AIMBASEX 0x8027DF20
#define RE4_AIMBASEY 0x8027F180
#define RE4_HARPOONBASE 0x80218650

//X axis
#define RE4_AIMX 0xA4 //offset from aimBaseX
#define RE4_HARPOONX 0x514
#define RE4_BHARPOONX 0x33344

//Y axis
#define RE4_AIMY 0x468
#define RE4_AIMY2 0x8021873C //aimY 2.0
#define RE4_SCOPEY 0x80212A4C
#define RE4_HARPOONY 0x500

//current room ID
#define RE4_ROOMID 0x8027F7F9

//weapon equiped
#define RE4_WEPID 0x8028478C


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
			MEM_ReadUInt(0x80000004) == 0x30380000U || // Disc 1
			MEM_ReadUInt(0x80000004) == 0x30380100U);  // Disc 2

}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void GC_RE4_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
		return;

	//setting cam dist to far 
	if (MEM_ReadInt(0x800BC32C) == 0x4182000C) //
		MEM_WriteInt(0x800BC32C, 0x60000000);


	//disabling camY re-centering
	if (MEM_ReadUInt(0x800BBDBC) == 0xD1BF0200U)
		MEM_WriteUInt(0x800BBDBC, 0x60000000U);


	//disable knife ready animation
	if (MEM_ReadInt(0x80044580) == 0x39200004)
	MEM_WriteInt(0x80044580, 0x60000000);
	
	switch (MEM_ReadUInt8(RE4_WEPID)) //checking equiped weapon ID and disabling ready animation
	{
	case (0x01): 
		MEM_WriteInt(0x80920C38, 0x60000000); //punisher
		break;
	case (0x02): 
		MEM_WriteInt(0x80923D84, 0x60000000); //handgun
		break;
	case (0x03): 
		MEM_WriteInt(0x80928AE4, 0x60000000); //red9
		break;
	case (0x04): 
		MEM_WriteInt(0x8092012C, 0x60000000); //blacktail
		break;
	case (0x05): 
		MEM_WriteInt(0x8092C464, 0x60000000); //broken b.
		break;
	case (0x06): 
		MEM_WriteInt(0x80927D6C, 0x60000000); //killer7
		break;
	case (0x07): 
		MEM_WriteInt(0x8092346C, 0x60000000); //shotgun
		break;
	case (0x08): 
		MEM_WriteInt(0x8091F02C, 0x60000000); //striker
		break;
	case (0x21): 
		MEM_WriteInt(0x8092E58C, 0x60000000); //riot
		break;
	case (0x0B): 
		MEM_WriteInt(0x80927F54, 0x60000000); //TMP
		break;
	case (0x0C): 
		MEM_WriteInt(0x8091D5F8, 0x60000000); //chicago t.
		break;

	}
	

	//checking and disabling aim shake function so aim on Y axis until find
	//a reliable pointer to work on all rooms
	// if (MEM_ReadUInt(RE4_LOCK_RAND) == 0x9421ffc8U) 
	//  	MEM_WriteUInt(RE4_LOCK_RAND, 0x4e800020U);

	// if (MEM_ReadUInt(RE4_LOCK_PITCH) == 0xd02b0004U) //checking and disabling aimY reset to 0 when aiming
	//  	MEM_WriteUInt(RE4_LOCK_PITCH, 0x60000000U);	


	aimBaseX = MEM_ReadUInt(RE4_AIMBASEX);
	aimBaseY = MEM_ReadUInt(RE4_AIMBASEY);
	

	float fov = MEM_ReadFloat(RE4_FOV);
	float looksensitivity = (float)sensitivity;
	
	float scale = 800.f;
	

	float aimX = MEM_ReadFloat(aimBaseX + RE4_AIMX); //while aiming

	float aimY = MEM_ReadFloat(aimBaseY + RE4_AIMY); 
	float aimY2 = MEM_ReadFloat(RE4_AIMY2); //aimY 2.0

	// float camX = MEM_ReadFloat(RE4_CAMX); //while not aiming
	float camY = MEM_ReadFloat(RE4_CAMY);


	camY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 12.f);
	camY = ClampFloat(camY, -1.4f, 1.4f);
	

	// camX += (float)-xmouse * looksensitivity / (scale * 2);
	// camX = ClampFloat(camX, -2.0f, 2.f);

	
	
	aimX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
	

	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f);
	aimY = ClampFloat(aimY, -1.570796371f, 1.570796371f);
	aimY2 += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f); //aimY 2.0
	aimY2 = ClampFloat(aimY2, -1.f, 1.f);
	
	
// 	#define RE4_HARPOON_D 0x81052D30
// #define RE4_HARPOON_N 0x8105FD3C

//

	//check if weapon w/ scope is equiped
	if(MEM_ReadUInt8(RE4_WEPID) == 0x09 || // rifle
	   MEM_ReadUInt8(RE4_WEPID) == 0x0A || // a. rifle
	   MEM_ReadUInt8(RE4_WEPID) == 0x0D){  // r. launcher
		float scopeY = MEM_ReadFloat(RE4_SCOPEY);
		scopeY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * (fov / 45.f) / (scale * 45.f);
		scopeY = ClampFloat(scopeY, -1.22f, 1.22f);		
		MEM_WriteFloat(RE4_SCOPEY, scopeY);
		MEM_WriteFloat(RE4_AIMY2, aimY2);  //aimY 2.0
	}else if(MEM_ReadUInt8(0x81052D30) == 0x81 || //checking if leon is aiming with harpoon day
			 MEM_ReadUInt8(0x8105FD3C) == 0x3E){ //checking if leon is aiming with harpoon night
		float harpoonY = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONY);
		float harpoonX = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONX); //moves only crosshair
		float bharpoonX = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_BHARPOONX); //spin the entire boat
		harpoonY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / 65;
		harpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
		bharpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
		harpoonY = ClampFloat(harpoonY, -255.f, 255.f);
		harpoonX = ClampFloat(harpoonX, -0.4f, 0.4f);
		MEM_WriteFloat(MEM_ReadUInt(0x80218650) + RE4_HARPOONY, harpoonY);

		
		if(-0.38f < harpoonX && harpoonX < 0.38f) //spin the boat only if crosshair is on the edge of screen
			MEM_WriteFloat(MEM_ReadUInt(0x80218650) + RE4_HARPOONX, harpoonX);
		else
			MEM_WriteFloat(MEM_ReadUInt(0x80218650) + RE4_BHARPOONX, bharpoonX);
	}else{
		//neither with scope weapon or on boat
		MEM_WriteFloat(RE4_AIMY2, aimY2);  //aimY 2.0
		MEM_WriteFloat((aimBaseY + RE4_AIMY), (aimY));
	}
	
	
	// MEM_WriteFloat(RE4_CAMX, camX);
	MEM_WriteFloat(RE4_CAMY, camY);
	

	MEM_WriteFloat(aimBaseX + RE4_AIMX, aimX);
	
}