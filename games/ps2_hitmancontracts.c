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

#define PI 3.14159265f // 0x40490FDB

// pointers
#define HC_GLOBALBASE 0x497238	 //pointer to many things
#define HC_CAMBASEX 0x497230	 //camX
#define HC_SCOPEBASE 0x01ff73f0	 //camX


// States
#define HC_PAUSESTATE 0x005108C8 // paused or not
#define HC_CUTSCENESTATE 0x005080F0 // in cutscene or not 
#define HC_SCOPESTATE 0xA58 //offset from playerBase
#define HC_LEANSTATE 0x1545 //side leaning (globalBase offset)
#define HC_FPSFLAG 0x00552559 //in FPS view or not



// cam offsets from playerBase
#define HC_CAMY 0x178     //third person
#define HC_FPY 0x178      //first person
#define HC_SCOPEY 0x9E8   //scope/binoculars
#define HC_NVSCOPEY 0x2FC // nightvision on + scope/binoculars
#define HC_LADDERY 0x184  //while climbing ladder
#define HC_LADDERX 0x180  //while climbing ladder


// cam offsets from camBaseX
#define HC_CAMXSIN 0x0
#define HC_CAMXCOS 0x8
#define HC_CAMXNSIN 0x20 
#define HC_CAMXCOS2 0x18


#define HC_ZOOM 0x538 //offset from globalBase

static uint8_t PS2_HC_Status(void);
static void PS2_HC_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"Hitman Contracts",
		PS2_HC_Status,
		PS2_HC_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_HITMANCONTRACTS = &GAMEDRIVER_INTERFACE;

static uint32_t globalBase = 0;
static uint32_t camBaseX = 0;
static uint32_t playerBase = 0;
static uint8_t leanState = 0;
static uint8_t scopeState = 0;


//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_HC_Status(void)
{
	// SLUS_208.82; (0x93390)

	return ((PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323038&&
			PS2_MEM_ReadWord(0x93398) == 0x2E38323B));		  
			  
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_HC_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   		return;

	// if(PS2_MEM_ReadUInt8(HC_PAUSESTATE) == 0x01) //paused or inventory
	// 	return;

	// if(PS2_MEM_ReadUInt8(HC_CUTSCENESTATE) == 0x01) //in cutscene
	// 	return;

	//DISABLE AIM ASSIST
	if(PS2_MEM_ReadUInt(0x003C41E4) == 0xAE120010)
		PS2_MEM_WriteUInt(0x003C41E4, 0x00000000);
		

	
	globalBase = PS2_MEM_ReadPointer(HC_GLOBALBASE);
	playerBase = PS2_MEM_ReadPointer(globalBase + 0xC6C);		
	playerBase = PS2_MEM_ReadPointer(globalBase + 0xC24);
	playerBase = PS2_MEM_ReadPointer(globalBase + 0x190);
	

	camBaseX = PS2_MEM_ReadPointer(HC_CAMBASEX);
	
	leanState = PS2_MEM_ReadUInt8(camBaseX + 0x491);

	camBaseX = PS2_MEM_ReadPointer(camBaseX + 0x8);
	camBaseX = PS2_MEM_ReadPointer(camBaseX + 0x50);

	

	float zoom = PS2_MEM_ReadFloat(globalBase + HC_ZOOM);
	float looksensitivity = (float)sensitivity;


	float camY = PS2_MEM_ReadFloat(playerBase + HC_CAMY);

	float scopeY = PS2_MEM_ReadFloat(playerBase + HC_SCOPEY);

	float nvscopeY = PS2_MEM_ReadFloat(playerBase + HC_NVSCOPEY);

	float fpY = PS2_MEM_ReadFloat(playerBase + HC_FPY);

	float ladderY = PS2_MEM_ReadFloat(playerBase + HC_LADDERY);
	float ladderX = PS2_MEM_ReadFloat(playerBase + HC_LADDERX);


	float camXsin = PS2_MEM_ReadFloat(camBaseX + HC_CAMXSIN);
	float camXcos = PS2_MEM_ReadFloat(camBaseX + HC_CAMXCOS);
	float angle = atan2f(camXsin, camXcos);


	//update cam
	scopeY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 300.f;
	scopeY = ClampFloat(scopeY, -88.f, 88.f);

	nvscopeY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 300.f;
	nvscopeY = ClampFloat(nvscopeY, -88.f, 88.f);

	camY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * zoom / 15000.f;
	camY = ClampFloat(camY, -1.04719758f, 1.431169987f);

	fpY += (float)(!invertpitch ? -ymouse : ymouse) * looksensitivity * zoom / 15000.f;
	fpY = ClampFloat(fpY, -1.22173059f, 1.22173059f);

	

	angle += (float)xmouse * looksensitivity * zoom / 15000.f;

	ladderX -= (float)xmouse * looksensitivity * zoom / 15000.f;
	ladderY += (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity * zoom / 15000.f;
	
	camXsin = sin(angle);
	camXcos = cos(angle);

	if(leanState == 0x20 || leanState == 0x40){ //leaning to left or right
		if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   		return;
		//ladderY = ClampFloat(ladderY, -0.785f, 0.785f);
		//camY = ClampFloat(camY, -0.785f, 0.785f);
		PS2_MEM_WriteFloat(playerBase + HC_LADDERY, ladderY);
		PS2_MEM_WriteFloat(playerBase + HC_LADDERX, ladderX);
	}else{
		PS2_MEM_WriteFloat(camBaseX + HC_CAMXSIN, camXsin);
		PS2_MEM_WriteFloat(camBaseX + HC_CAMXCOS, camXcos);
		PS2_MEM_WriteFloat(camBaseX + HC_CAMXNSIN, -camXsin);
		PS2_MEM_WriteFloat(camBaseX + HC_CAMXCOS2, camXcos);
	}
	
	if(PS2_MEM_ReadUInt8(HC_FPSFLAG) == 0x0F)//in FPS view
		PS2_MEM_WriteFloat(playerBase + HC_FPY, fpY);
	else
	 	PS2_MEM_WriteFloat(playerBase + HC_CAMY, camY);
		
	
	scopeState = PS2_MEM_ReadUInt8(playerBase + HC_SCOPESTATE);

	if(PS2_MEM_ReadUInt8(playerBase + HC_SCOPESTATE) == 0x0)
	  	PS2_MEM_WriteFloat(globalBase + HC_ZOOM, 1.176351905f);
	
	PS2_MEM_WriteFloat(playerBase + HC_NVSCOPEY, nvscopeY);
	
	PS2_MEM_WriteFloat(playerBase + HC_SCOPEY, scopeY);
	
}