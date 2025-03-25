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

#define PI 3.14159265f // 0x40490FDB

//aimBase pointer
#define RE4_AIMBASEX 0x437780
#define RE4_AIMBASEY 0x437818
#define RE4_HARPOONBASE 0x

//X axis
#define RE4_AIMX 0xD4 //offset from aimBaseX
#define RE4_HARPOONX 0x
#define RE4_BHARPOONX 0x

//Y axis
#define RE4_AIMY 0x28
#define RE4_AIMY2 0x3D7564  //aimY 2.0
#define RE4_SCOPEY 0x003CA940
#define RE4_HARPOONY 0x

//IDs
#define RE4_WEPID 0x0042DA82 //current weapon
#define RE4_PLAYERID 0x0042DA8A //current character

//aim functions
#define RE4_LOCK_RAND 0x24EA28
#define RE4_LOCK_PITCH 0x24F0E0
#define RE4_LOCK_CTRL 0x24E2A0

//States
#define RE4_GAMESTATE 0x00425BB8
#define RE4_MENUSTATE 0x004175AC
#define RE4_FOOTWORK 0x119 //offset from aimBaseX

//cam
#define RE4_CAMX 0x003CA7B8
#define RE4_CAMY 0x003CA7B4

// #define RE4_fFOV1 0x102DCC
// #define RE4_fFOV2 0x102DDC
#define RE4_FOV 0x3CB014

static uint8_t PS2_RE4_Status(void);
static void PS2_RE4_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Resident Evil 4",
	PS2_RE4_Status,
	PS2_RE4_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_RE4 = &GAMEDRIVER_INTERFACE;

static uint32_t aimBaseX = 0;
static uint32_t aimBaseY = 0;
static uint8_t footwork = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_RE4_Status(void)
{
	//SLUS_211.34 (0x93390) (0x90708)
	// 53 4C 55 53 | 5F 32 31 31 | 2E 33 34 3B   
	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553 &&
			 PS2_MEM_ReadWord(0x93394) == 0x5F323131 &&
			 PS2_MEM_ReadWord(0x93398) == 0x2E33343B)||

			(PS2_MEM_ReadWord(0x90708) == 0x534C5553 &&
			 PS2_MEM_ReadWord(0x9070C) == 0x5F323131 &&
			 PS2_MEM_ReadWord(0x90710) == 0x2E33343B));
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_RE4_Inject(void)
{
	// if(xmouse == 0 && ymouse == 0) // if mouse is idle
	// 	return;

	//replacing idle mouse check above
	if(MEM_ReadUInt16(RE4_GAMESTATE) != 0x0003 && //not on playing state (pause, loading)
	   MEM_ReadUInt8(RE4_MENUSTATE) != 0x00) 	  //not in menu (inventory, map, merchant)
	   return;

	//setting cam distance to far (Leon&Ashley cam flag)
	// if (PS2_MEM_ReadUInt() == 0x)
	

	//disabling camY auto level
	if (PS2_MEM_ReadUInt(0x001AA710) == 0xE6200284)
		PS2_MEM_WriteUInt(0x001AA710, 0x00000000);
	
	//disabling camX auto center
	if (MEM_ReadUInt(0x001AA71C) == 0xE6200288)
	 	MEM_WriteUInt(0x001AA71C, 0x00000000);

	
	
	//disable knife ready animation
	if (PS2_MEM_ReadWord(0x00158828) == 0x04000224)
		PS2_MEM_WriteWord(0x00158828, 0x00000224);
		

	switch (PS2_MEM_ReadUInt8(RE4_WEPID)) //checking equiped weapon ID and disabling ready animation
	{
	case (0x01): 
		PS2_MEM_WriteWord(0x008AE82C, 0x00000424); //punisher
		break;
	case (0x02): 
		PS2_MEM_WriteWord(0x008AFFAC, 0x00000424); //handgun
		break;
	case (0x03): 
		PS2_MEM_WriteWord(0x008BCFAC, 0x00000424); //red9
		break;
	case (0x04): 
		PS2_MEM_WriteWord(0x008ADD9C, 0x00000424); //blacktail
		break;
	case (0x05): 
		PS2_MEM_WriteWord(0x008BC5F4, 0x00000424); //broken b.
		break;
	case (0x06): 
		PS2_MEM_WriteWord(0x008B759C, 0x00000424); //killer7
		break;
	case (0x07): 
		PS2_MEM_WriteWord(0x008ACC78, 0x00000000); //shotgun
		break;
	case (0x08): 
		PS2_MEM_WriteWord(0x008ADC78, 0x00000000); //striker
		break;
	case (0x21): 
		PS2_MEM_WriteWord(0x008BD478, 0x00000000); //riot
		break;
	case (0x0B): 
		PS2_MEM_WriteWord(0x008B816C, 0x00000424); //TMP
		break;
	case (0x0C): 
		PS2_MEM_WriteWord(0x008B2154, 0x00000424); //chicago t.
		break;
	}


	//checking and disabling aim shake function so aim on Y axis until find
	//a reliable pointer to work on all rooms
	// if (MEM_ReadUInt(RE4_LOCK_RAND) == 0x9421ffc8U) 
	//  	MEM_WriteUInt(RE4_LOCK_RAND, 0x4e800020U);


	aimBaseX = PS2_MEM_ReadPointer(RE4_AIMBASEX);
	aimBaseY = PS2_MEM_ReadPointer(RE4_AIMBASEY);
	footwork = PS2_MEM_ReadUInt8(aimBaseX + RE4_FOOTWORK);



	float fov = PS2_MEM_ReadFloat(RE4_FOV);
	float looksensitivity = (float)sensitivity;
	float scale = 800.f;


	//while aiming
	float aimX = PS2_MEM_ReadFloat(aimBaseX + RE4_AIMX);

	float aimY = PS2_MEM_ReadFloat(aimBaseY + RE4_AIMY);
	float aimY2 = PS2_MEM_ReadFloat(RE4_AIMY2); //aimY 2.0

	//while not aiming
	float camX = PS2_MEM_ReadFloat(RE4_CAMX);
	float camY = PS2_MEM_ReadFloat(RE4_CAMY);


	camY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 12.f);
	camY = ClampFloat(camY, -1.2f, 1.2f);


	//aimX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
	
	aimY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f);
	aimY = ClampFloat(aimY, -1.570796371f, 1.570796371f);
	aimY2 += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / (scale * 65.f); //aimY 2.0
	aimY2 = ClampFloat(aimY2, -1.f, 1.f);

	
	//check if weapon w/ scope is equiped
	if( PS2_MEM_ReadUInt8(RE4_WEPID) == 0x09 || // rifle
	 	PS2_MEM_ReadUInt8(RE4_WEPID) == 0x0A || // a. rifle
	 	PS2_MEM_ReadUInt8(RE4_WEPID) == 0x0D){  // r. launcher
			float scopeY = PS2_MEM_ReadFloat(RE4_SCOPEY);
			scopeY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * (fov / 45.f) / (scale * 45.f);
			scopeY = ClampFloat(scopeY, -1.22f, 1.22f);		
			PS2_MEM_WriteFloat(RE4_SCOPEY, scopeY);
		}
		
	// }else if(MEM_ReadUInt8(0x81052D30) == 0x81 || //checking if leon is aiming with harpoon day
	// 		 MEM_ReadUInt8(0x8105FD3C) == 0x3E){ //checking if leon is aiming with harpoon night
	// 	float harpoonY = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONY);
	// 	float harpoonX = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_HARPOONX); //moves only crosshair
	// 	float bharpoonX = MEM_ReadFloat(MEM_ReadUInt(RE4_HARPOONBASE) + RE4_BHARPOONX); //spin the entire boat
	// 	harpoonY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * (fov / 50.f) / 65;
	// 	harpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
	// 	bharpoonX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 45.f);
	// 	harpoonY = ClampFloat(harpoonY, -255.f, 255.f);
	// 	harpoonX = ClampFloat(harpoonX, -0.4f, 0.4f);
	// 	MEM_WriteFloat(MEM_ReadUInt(0x80218650) + RE4_HARPOONY, harpoonY);

		
	// 	if(-0.38f < harpoonX && harpoonX < 0.38f) //spin the boat only if crosshair is on the edge of screen
	// 		MEM_WriteFloat(MEM_ReadUInt(0x80218650) + RE4_HARPOONX, harpoonX);
	// 	else
	// 		MEM_WriteFloat(MEM_ReadUInt(0x80218650) + RE4_BHARPOONX, bharpoonX);
	// }

	camX += (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 30.f);
	camX = ClampFloat(camX, -PI, PI);
	if(camX <= -PI || camX >= PI) //camX wrap around
		camX = -camX;	
	
	//mouseturning type A from RE4_TWEAKS 
		if(footwork == 0x06){//while aiming
			aimX += camX; //aiming to direction camera is pointing
			camX = 0.f;	//always follow the direction of the lasersight

			//readjusting sensitivity while aiming
			camX -= (float)-xmouse * looksensitivity * (fov / 50.f) / (scale * 90.f);
		}

		if(footwork != 0x00 && footwork != 0x06){//checking if is not standing & not aiming
			//smoothing player model rotation to where the camera is pointing
			aimX += camX / 30.f; //was using 30.0f but when alt+tabing the emulator while playing makes turning sluggish
			camX -= camX / 30.f;
		}


	//neither with scope weapon or on boat
	PS2_MEM_WriteFloat(RE4_AIMY2, aimY2);  //aimY 2.0
	PS2_MEM_WriteFloat((aimBaseY + RE4_AIMY), (aimY));

	PS2_MEM_WriteFloat(RE4_CAMX, camX);

	PS2_MEM_WriteFloat(RE4_CAMY, camY);

	if(footwork != 0x04) //fix jerk movement while turning on keyboard/gamepad when not aiming
		PS2_MEM_WriteFloat(aimBaseX + RE4_AIMX, aimX);	
}