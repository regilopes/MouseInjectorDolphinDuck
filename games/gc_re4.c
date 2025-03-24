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
//fix being able to rotate leon while on a ladder or cutscene
//fix harpoon during night


#define PI 3.14159265f // 0x40490FDB

//aimBase pointer
#define RE4_AIMBASEX 0x8027DF20
#define RE4_AIMBASEY 0x8027F180
#define RE4_HARPOONBASE 0x80218650
#define RE4_STATEBASE 0x8027DF20

//X axis
#define RE4_AIMX 0xA4 //offset from aimBaseX
#define RE4_HARPOONX 0x514
#define RE4_BHARPOONX 0x33344

//Y axis
#define RE4_AIMY 0x468
#define RE4_AIMY2 0x8021873C //aimY 2.0
#define RE4_SCOPEY 0x80212A4C
#define RE4_HARPOONY 0x500

//IDs
#define RE4_ROOMID 0x8027F7F9 //current room
#define RE4_WEPID 0x8028478C //current weapon
#define RE4_PLAYERID 0x80284794 //current character

//aim functions
#define RE4_LOCK_RAND 0x801447b8
#define RE4_LOCK_PITCH 0x80144d6c
#define RE4_LOCK_CTRL 0x801444d4

//States
#define RE4_GAMESTATE 0x8027F7F0
#define RE4_MENUSTATE 0x80219313
#define RE4_FOOTWORK 0xFD //offset from aimBaseX

//cam addresses
#define RE4_CAMX 0x80212928
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
static uint8_t footwork = 0;


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
	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	// 	return;

	//replacing idle mouse check above
	if(MEM_ReadUInt16(RE4_GAMESTATE) != 0x0003 && //not on playing state (pause, loading)
	   MEM_ReadUInt8(RE4_MENUSTATE) != 0x00) 	  //not in menu (inventory, map, merchant)
	   return;

	//setting cam distance to far (Leon&Ashley cam flag)
	if (MEM_ReadInt(0x800BC32C) == 0x4182000C)
		MEM_WriteInt(0x800BC32C, 0x60000000);


	//disabling camY auto level
	if (MEM_ReadUInt(0x800BBDBC) == 0xD1BF0200U)
		MEM_WriteUInt(0x800BBDBC, 0x60000000U);
	//disabling camX auto center
	if (MEM_ReadUInt(0x800BBDC0) == 0xD01F0204U)
		MEM_WriteUInt(0x800BBDC0, 0x60000000U);
	

	//disabling knife ready animation
	if (MEM_ReadInt(0x80044580) == 0x39200004)
		MEM_WriteInt(0x80044580, 0x60000000);
	

	switch (MEM_ReadUInt16(RE4_WEPID)) //checking equiped weapon ID and disabling ready animation
	{
	case (0x0001): //punisher
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x00) //leon
			MEM_WriteInt(0x80920C38, 0x60000000);
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x02) //ada
			MEM_WriteInt(0x8091A3AC, 0x60000000);
		break;

	case (0x0002): //handgun
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x00) //leon
			MEM_WriteInt(0x80923D84, 0x60000000); 
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x05) //wesker
			MEM_WriteInt(0x80923128, 0x60000000);
		break;

	case (0x0003): //red9
		MEM_WriteInt(0x80928AE4, 0x60000000); 
		break;

	case (0x0203): //red9 w/ stock
		MEM_WriteInt(0x80928AE4, 0x60000000); 
		break;

	case (0x0004): //blacktail
		MEM_WriteInt(0x8092012C, 0x60000000); 
		break;

	case (0x0005): //broken b.
		MEM_WriteInt(0x8092C464, 0x60000000); 
		break;

	case (0x0006): //killer7
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x00) //leon
			MEM_WriteInt(0x80927D6C, 0x60000000);
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x05) //wesker
			MEM_WriteInt(0x8092098C, 0x60000000);
		break;

	case (0x0007): //shotgun
		MEM_WriteInt(0x8092346C, 0x60000000); 
		break;

	case (0x0008): //striker
		MEM_WriteInt(0x8091F02C, 0x60000000); 
		break;

	case (0x0021): //riot
		MEM_WriteInt(0x8092E58C, 0x60000000); 
		break;

	case (0x000B): //TMP 
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x00) //leon
			MEM_WriteInt(0x80927F54, 0x60000000); 
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x02) //ada
			MEM_WriteInt(0x80915dC0, 0x60000000);
		if(MEM_ReadUInt8(RE4_PLAYERID) == 0x03) //hunk
			MEM_WriteInt(0x8091A214, 0x60000000);
		break;

	case (0x020B): //TMP w/ stock
		MEM_WriteInt(0x80927F54, 0x60000000); 
		break;

	case (0x000C): //chicago t.
		MEM_WriteInt(0x8091D5F8, 0x60000000); 
		break;

	case (0x001C): //Krauser Bow
		MEM_WriteInt(0x80a9a984, 0x60000000); 
		break;
	}

	//checking and disabling aim shake function so "aimY 2.0" works until I find
	//a reliable pointer for "aimY" to work on every room
	// if (MEM_ReadUInt(RE4_LOCK_RAND) == 0x9421ffc8U) 
	//  	MEM_WriteUInt(RE4_LOCK_RAND, 0x4e800020U);

	// if (MEM_ReadUInt(RE4_LOCK_PITCH) == 0xd02b0004U) //checking and disabling aimY reset to 0 when start aiming
	//  	MEM_WriteUInt(RE4_LOCK_PITCH, 0x60000000U);	


	aimBaseX = MEM_ReadUInt(RE4_AIMBASEX);
	aimBaseY = MEM_ReadUInt(RE4_AIMBASEY);
	footwork = MEM_ReadUInt8(aimBaseX + RE4_FOOTWORK);

	float fov = MEM_ReadFloat(RE4_FOV);
	float looksensitivity = (float)sensitivity;
	float scale = 800.f;
	

	//while aiming
	float aimX = MEM_ReadFloat(aimBaseX + RE4_AIMX); 

	float aimY = MEM_ReadFloat(aimBaseY + RE4_AIMY); 
	float aimY2 = MEM_ReadFloat(RE4_AIMY2); //aimY 2.0

	//while not aiming
	float camX = MEM_ReadFloat(RE4_CAMX); 
	float camY = MEM_ReadFloat(RE4_CAMY);


	camY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 12.f);
	camY = ClampFloat(camY, -1.2f, 1.2f);
	
	
	//aimX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
	

	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f);
	aimY = ClampFloat(aimY, -1.570796371f, 1.570796371f);
	aimY2 += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f); //aimY 2.0
	aimY2 = ClampFloat(aimY2, -1.f, 1.f);
	
	
	//check if weapon w/ scope is equiped
	if(MEM_ReadUInt8(RE4_WEPID) == 0x09 || // rifle
	   MEM_ReadUInt8(RE4_WEPID) == 0x0A || // a. rifle
	   MEM_ReadUInt8(RE4_WEPID) == 0x0D){  // r. launcher
		float scopeY = MEM_ReadFloat(RE4_SCOPEY);
		scopeY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * (fov / 45.f) / (scale * 45.f);
		scopeY = ClampFloat(scopeY, -1.22f, 1.22f);		
		MEM_WriteFloat(RE4_SCOPEY, scopeY);
		
	}else if(MEM_ReadUInt8(0x81052D30) == 0x81 || //checking if leon is aiming with harpoon day
			 MEM_ReadUInt8(0x8105FD3C) == 0x3E){ //checking if leon is aiming with harpoon night
		float harpoonY = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONY);
		float harpoonX = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONX); //only moves crosshair
		float bharpoonX = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_BHARPOONX); //spin the entire boat
		harpoonY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / 65;
		harpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
		bharpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
		harpoonY = ClampFloat(harpoonY, -255.f, 255.f);
		harpoonX = ClampFloat(harpoonX, -0.4f, 0.4f);
		MEM_WriteFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONY, harpoonY);

		
		if(-0.38f < harpoonX && harpoonX < 0.38f) //spin the boat only if crosshair is on the edge of screen
			MEM_WriteFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONX, harpoonX);
		else
			MEM_WriteFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_BHARPOONX, bharpoonX);
	}

	//neither with scope weapon or on boat
	MEM_WriteFloat(RE4_AIMY2, aimY2);  //aimY 2.0
	MEM_WriteFloat((aimBaseY + RE4_AIMY), aimY);
	
	
	camX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 30.f);
	camX = ClampFloat(camX, -PI, PI);
	if(camX <= -PI || camX >= PI) //camX wrap around
		camX = -camX;	
	
	//mouseturning type A from RE4_TWEAKS 
		if(footwork == 0x06){//while aiming
			aimX += camX; //aiming to direction camera is pointing
			camX = 0.f;	//always follow the direction of the lasersight

			//readjusting sensitivity while aiming
			aimX -= (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 145.f);
			camX -= (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 145.f);
		}

		if(footwork != 0x00 && footwork != 0x06){//checking if is not standing & not aiming
			//smoothing rotation to where the camera is directed
			aimX += camX / 30.f;
			camX -= camX / 30.f;
		}

		
	MEM_WriteFloat(RE4_CAMX, camX);
		
	MEM_WriteFloat(RE4_CAMY, camY);
	
	MEM_WriteFloat(aimBaseX + RE4_AIMX, aimX);
}