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
#define TAU 6.2831853f // 0x40C90FDB

// pointers
#define TP_PLAYERBASE 0x66A1E8 //player base pointer
#define TP_CAMBASE 0x4B0EF8
#define TP_ZOOMBASE 0x0066B988   //sniper zoom FOV pointer 

// States

// aim offsets from viewbase
#define TP_CAMY 0x320 //offset from cambase

//CAMX OFFSETS FROM CAMBASE
#define TP_CAMXSIN  0x31C
#define TP_CAMXCOS  0x324
#define TP_CAMXSINB  0x30C
#define TP_CAMXCOSB  0x304


#define TP_CAMPOS_X  0x48A328  // Exemplo hipotético
#define TP_CAMPOS_Y  0x48A330  // Exemplo hipotético
#define TP_CAMPOS_Z  0x48A33C  // Exemplo hipotético


#define TP_PLAYERPOS_X 0x154b948 // Offset da pos X do jogador (no playerBase)
#define TP_PLAYERPOS_Y 0x154b950 // Offset da pos Y do jogador (no playerBase)
#define TP_PLAYERPOS_Z 0x154b94C // Offset da pos Z do jogador (no playerBase)


#define TP_BODYX  0x77C

#define TP_FOV 0x53A4F4



static uint8_t PS2_PUNISHER_Status(void);
static void PS2_PUNISHER_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
	{
		"The Punisher",
		PS2_PUNISHER_Status,
		PS2_PUNISHER_Inject,
		1, // 1000 Hz tickrate
		0  // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_PS2_PUNISHER = &GAMEDRIVER_INTERFACE;

static uint32_t playerBase = 0;
static uint32_t camBase = 0;
static uint32_t zoomBase = 0;



//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t PS2_PUNISHER_Status(void)
{
	// SLUS_208.64 (0x93390)
	return (PS2_MEM_ReadWord(0x93390) == 0x534C5553&&
			PS2_MEM_ReadWord(0x93394) == 0x5F323038&&
			PS2_MEM_ReadWord(0x93398) == 0x2E36343B);
}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void PS2_PUNISHER_Inject(void)
{
	if(xmouse == 0 && ymouse == 0) // if mouse is idle
	   	return;

	// playerBase = PS2_MEM_ReadPointer(TP_PLAYERBASE);
	camBase = PS2_MEM_ReadPointer(TP_CAMBASE);

	float looksensitivity = (float)sensitivity;
	float scale = 20000.f;


	float fov = pow(PS2_MEM_ReadFloat(TP_FOV)/60.f, 3.f);

	float camY = PS2_MEM_ReadFloat(camBase + TP_CAMY);
		   
	float camXsin = PS2_MEM_ReadFloat(camBase + TP_CAMXSIN);
	float camXcos = PS2_MEM_ReadFloat(camBase + TP_CAMXCOS);
	float angle = atan2f(camXsin, camXcos);

	float bodyX = PS2_MEM_ReadFloat(camBase + TP_BODYX);


	camY -= (float)(!invertpitch ? ymouse : -ymouse) * looksensitivity / scale * fov;
	camY = ClampFloat(camY, -0.82f, 0.82f);



	angle += (float)xmouse * looksensitivity / scale * fov;

	camXsin = (float)sinf(angle);
	camXcos = (float)cosf(angle);

	float bodyAngle = angle;
	while (bodyAngle > PI) bodyAngle -= TAU;
	while (bodyAngle < -PI) bodyAngle += TAU;

	// 2. Injeta a rotação no corpo do personagem e nas matrizes secundárias
	//PS2_MEM_WriteFloat(camBase + TP_BODYX, bodyAngle);



	float cameraDistance = 4.0f; // Descubra este valor no Cheat Engine!
    float hDist = cameraDistance * cosf(camY);

    float pX = PS2_MEM_ReadFloat(TP_PLAYERPOS_X);
    float pY = PS2_MEM_ReadFloat(TP_PLAYERPOS_Y);
    float pZ = PS2_MEM_ReadFloat(TP_PLAYERPOS_Z);

    float camPosX = pX - (hDist * sinf(angle));
    float camPosZ = pZ - (hDist * cosf(angle));
    float camPosY = pY + (cameraDistance * sinf(camY));

    // INJETAR A POSIÇÃO ESPACIAL FORÇA A CÂMERA A FICAR PARADA RELATIVA AO JOGADOR
    PS2_MEM_WriteFloat(TP_CAMPOS_X, camPosX);
    PS2_MEM_WriteFloat(TP_CAMPOS_Y, camPosY);
    PS2_MEM_WriteFloat(TP_CAMPOS_Z, camPosZ);


	PS2_MEM_WriteFloat(camBase + TP_CAMY, camY);


	PS2_MEM_WriteFloat(camBase + TP_CAMXSIN, camXsin);
	PS2_MEM_WriteFloat(camBase + TP_CAMXCOS, camXcos);
	PS2_MEM_WriteFloat(camBase + TP_CAMXSINB, -camXsin);
	PS2_MEM_WriteFloat(camBase + TP_CAMXCOSB, camXcos);



}