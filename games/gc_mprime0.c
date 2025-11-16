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
#include "math.h" 

#define PI 3.14159265f // 0x40490FDB

//camBasePtr
//#define MPRIME0_AIMBASEPTR_X 0x000000
//#define MPRIME0_AIMBASEPTR_Y 0x000000

//aim offsets from aimbase
#define MPRIME0_AIMX_COS 0x8046B9B0
#define MPRIME0_AIMX_SIN 0x8046B9B4
#define MPRIME0_AIMX_NSIN 0x8046B9C0
#define MPRIME0_AIMX_COS2 0x8046B9C4

#define MPRIME0_AIMY 0x8046BD68
#define MPRIME0_AIMY_SIN 0x80457E1C
//#define MPRIME0_AIMY 0x808BB154 //aimY alternative
//#define MPRIME0_AIMY 0x808AE774 //aimY alternative deprec



static uint8_t GC_MPRIME0_Status(void);
static void GC_MPRIME0_Inject(void);

static const GAMEDRIVER GAMEDRIVER_INTERFACE =
{
	"Metroid Prime rev0",
	GC_MPRIME0_Status,
	GC_MPRIME0_Inject,
	1, // 1000 Hz tickrate
	0 // crosshair sway not supported for driver
};

const GAMEDRIVER *GAME_GC_MPRIME0 = &GAMEDRIVER_INTERFACE;

//static uint32_t aimBaseX = 0;
//static uint32_t aimBaseY = 0;

//==========================================================================
// Purpose: return 1 if game is detected
//==========================================================================
static uint8_t GC_MPRIME0_Status(void)
{
	// GM8E01
	return (MEM_ReadUInt(0x80000000) == 0x474D3845U && 
			MEM_ReadUInt(0x80000004) == 0x30310000U);

}

//==========================================================================
// Purpose: calculate mouse look and inject into current game
//==========================================================================
static void GC_MPRIME0_Inject(void)
{
	// if(xmouse == 0 && ymouse == 0 && rx == 0 && ry == 0) // if mouse is idle
	// 	return;

	//Skip if not First Person camera or outside menu
	if(MEM_ReadUInt(0x8046BC70) || MEM_ReadUInt(0x8045B138))
		return;


		

	float looksensitivity = (float)sensitivity;
	
	float scale = 800.f;


	float aimXsin = MEM_ReadFloat(MPRIME0_AIMX_SIN);//while aiming
	float aimXcos = MEM_ReadFloat(MPRIME0_AIMX_COS); 
	float angle = atan(aimXsin / aimXcos);


	float aimYsin = MEM_ReadFloat(MPRIME0_AIMY_SIN); 

	if(aimXcos < 0)
		angle -= PI;

	angle += (float)(xmouse * looksensitivity / (scale * 20.0f));
	angle += (float)rx/327680.f * 0.4f;

	aimXsin = sin(angle);
	aimXcos = cos(angle);

	
	
	float aimY = MEM_ReadFloat(MPRIME0_AIMY); 
	
	if (MEM_ReadUInt(0x8046BC83)){ //Locked Aim == true
		float aimY = asin(aimYsin); //Update aimY angle to where it is locked;
	}else{
		aimY += (float)((!invertpitch ? -ymouse : ymouse) * looksensitivity / (scale * 25.0f));
		aimY -= (float)ry/327680.f * 0.4f;
	}

		
	MEM_WriteFloat(MPRIME0_AIMX_COS, aimXcos);
	MEM_WriteFloat(MPRIME0_AIMX_SIN, aimXsin);
	MEM_WriteFloat(MPRIME0_AIMX_NSIN, -aimXsin);
	MEM_WriteFloat(MPRIME0_AIMX_COS2, aimXcos);
	
	
	aimY = ClampFloat(aimY, -1.42f, 1.42f);
	MEM_WriteFloat(MPRIME0_AIMY, aimY);
	

	//Code below belongs to PrimeHack project, it modernize several aspects of the gameplay movement controls.
	//Huge thanks for making this mods possible!

	//tweak_player + 0x134
	MEM_WriteFloat(0x8045C33C, 1.52f);

	//grapple swing speed
	MEM_WriteFloat(0x8045C4B8, 1000.f);

	// Freelook rotation speed tweak
	MEM_WriteUInt(0x8045C488, 0x4F800000);

	// Air translational friction changes to make diagonal strafe match normal speed
	MEM_WriteFloat(0x8045C388, 0.25f);

	//add strafe code
	MEM_WriteUInt(0x805afc00, 0x94210018);
	MEM_WriteUInt(0x805afc04, 0x7c0802a6);
	MEM_WriteUInt(0x805afc08, 0x9001001c);
	MEM_WriteUInt(0x805afc0c, 0x80ada118);
	MEM_WriteUInt(0x805afc10, 0x809d02b0);
	MEM_WriteUInt(0x805afc14, 0x2c040002);
	MEM_WriteUInt(0x805afc18, 0x38800004);
	MEM_WriteUInt(0x805afc1c, 0x40820008);
	MEM_WriteUInt(0x805afc20, 0x809d02ac);
	MEM_WriteUInt(0x805afc24, 0x5484103a);
	MEM_WriteUInt(0x805afc28, 0x7c642a14);
	MEM_WriteUInt(0x805afc2c, 0xc0230044);
	MEM_WriteUInt(0x805afc30, 0xc0430004);
	MEM_WriteUInt(0x805afc34, 0xec6206f2);
	MEM_WriteUInt(0x805afc38, 0xc01d00e8);
	MEM_WriteUInt(0x805afc3c, 0xec210032);
	MEM_WriteUInt(0x805afc40, 0xec211824);
	MEM_WriteUInt(0x805afc44, 0xc00300a4);
	MEM_WriteUInt(0x805afc48, 0xd0010010);
	MEM_WriteUInt(0x805afc4c, 0xec210032);
	MEM_WriteUInt(0x805afc50, 0xc002bda0);
	MEM_WriteUInt(0x805afc54, 0xfc1e0040);
	MEM_WriteUInt(0x805afc58, 0xc002bdc8);
	MEM_WriteUInt(0x805afc5c, 0x40810008);
	MEM_WriteUInt(0x805afc60, 0xc002bd80);
	MEM_WriteUInt(0x805afc64, 0xec000072);
	MEM_WriteUInt(0x805afc68, 0xc0610010);
	MEM_WriteUInt(0x805afc6c, 0xec630828);
	MEM_WriteUInt(0x805afc70, 0xec6307b2);
	MEM_WriteUInt(0x805afc74, 0xec00182a);
	MEM_WriteUInt(0x805afc78, 0xd0010018);
	MEM_WriteUInt(0x805afc7c, 0xd0410014);
	MEM_WriteUInt(0x805afc80, 0x38610004);
	MEM_WriteUInt(0x805afc84, 0x389d0034);
	MEM_WriteUInt(0x805afc88, 0x38bd0138);
	MEM_WriteUInt(0x805afc8c, 0x4bd62d99);
	MEM_WriteUInt(0x805afc90, 0xc0010018);
	MEM_WriteUInt(0x805afc94, 0xc0210004);
	MEM_WriteUInt(0x805afc98, 0xec000828);
	MEM_WriteUInt(0x805afc9c, 0xc0210010);
	MEM_WriteUInt(0x805afca0, 0xec000824);
	MEM_WriteUInt(0x805afca4, 0xc022bdc8);
	MEM_WriteUInt(0x805afca8, 0xfc000840);
	MEM_WriteUInt(0x805afcac, 0x4080000c);
	MEM_WriteUInt(0x805afcb0, 0xfc000890);
	MEM_WriteUInt(0x805afcb4, 0x48000014);
	MEM_WriteUInt(0x805afcb8, 0xc022bd80);
	MEM_WriteUInt(0x805afcbc, 0xfc000840);
	MEM_WriteUInt(0x805afcc0, 0x40810008);
	MEM_WriteUInt(0x805afcc4, 0xfc000890);
	MEM_WriteUInt(0x805afcc8, 0xc0210014);
	MEM_WriteUInt(0x805afccc, 0xec200072);
	MEM_WriteUInt(0x805afcd0, 0x8001001c);
	MEM_WriteUInt(0x805afcd4, 0x7c0803a6);
	MEM_WriteUInt(0x805afcd8, 0x3821ffe8);
	MEM_WriteUInt(0x805afcdc, 0x4e800020);

	// Apply strafe force instead of torque v1.00
	MEM_WriteUInt(0x802875c4 + 0x0, 0xc022bda0);
	MEM_WriteUInt(0x802875c4 + 0x4, 0xc002be44);
	MEM_WriteUInt(0x802875c4 + 0x8, 0xec3e0828);
	MEM_WriteUInt(0x802875c4 + 0xc, 0xfc200a10);
	MEM_WriteUInt(0x802875c4 + 0x10, 0xfc010040);
	MEM_WriteUInt(0x802875c4 + 0x14, 0x4081002c);
	MEM_WriteUInt(0x802875c4 + 0x18, 0x48328625);
	MEM_WriteUInt(0x802875c4 + 0x1c, 0x4bd93f55);
	MEM_WriteUInt(0x802875c4 + 0x20, 0x7c651b78);
	MEM_WriteUInt(0x802875c4 + 0x24, 0x7fa3eb78);
	MEM_WriteUInt(0x802875c4 + 0x28, 0xc002bda0);
	MEM_WriteUInt(0x802875c4 + 0x2c, 0xd0210010);
	MEM_WriteUInt(0x802875c4 + 0x30, 0xd0010014);
	MEM_WriteUInt(0x802875c4 + 0x34, 0xd0010018);
	MEM_WriteUInt(0x802875c4 + 0x38, 0x38810010);	

	// disable rotation on LR analog
	MEM_WriteUInt(0x80286fe0, 0x4bfffc71);
    MEM_WriteUInt(0x80286c88, 0x4800000c);
    MEM_WriteUInt(0x8028739c, 0x60000000);
    MEM_WriteUInt(0x802873e0, 0x60000000);
    MEM_WriteUInt(0x8028707c, 0x60000000);
    MEM_WriteUInt(0x802871bc, 0x60000000);
    MEM_WriteUInt(0x80287288, 0x60000000);

	// max speed values table @ 805afce0
	MEM_WriteUInt(0x805afce0, 0x41480000);
	MEM_WriteUInt(0x805afce4, 0x41480000);
	MEM_WriteUInt(0x805afce8, 0x41480000);
	MEM_WriteUInt(0x805afcec, 0x41480000);
	MEM_WriteUInt(0x805afcf0, 0x41480000);
	MEM_WriteUInt(0x805afcf4, 0x41480000);
	MEM_WriteUInt(0x805afcf8, 0x41480000);
	MEM_WriteUInt(0x805afcfc, 0x41480000);

	// Show crosshair but don't consider pressing R button
    MEM_WriteUInt(0x80016ee4, 0x3b000001);  // li r24, 1
    MEM_WriteUInt(0x80016ee8, 0x8afd09c4);  // lbz r23, 0x9c4(r29)
    MEM_WriteUInt(0x80016eec, 0x53173672);  // rlwimi r23, r24, 6, 25, 25 (00000001)
    MEM_WriteUInt(0x80016ef0, 0x9afd09c4);  // '''''''''''''''''''stb''''''''''''''''''' r23, 0x9c4(r29)
    MEM_WriteUInt(0x80016ef4, 0x4e800020);  // blr

	// MEM_WriteUInt(0x8000f63c, 0x48000048);
    MEM_WriteUInt(0x800ea15c, 0x38810044);  // output cannon bob only for viewbob
    MEM_WriteUInt(0x8000e538, 0x60000000);
    // MEM_WriteUInt(0x80016ee4, 0x4e800020);
    MEM_WriteUInt(0x80014820, 0x4e800020);
    MEM_WriteUInt(0x8000e73c, 0x60000000);
    MEM_WriteUInt(0x8000f810, 0x48000244);
    // When attached to a grapple point and spinning around it
    // the player's yaw is adjusted, this ensures only position is updated
    // Grapple point yaw fix
    MEM_WriteUInt(0x8017a18c, 0x7fa3eb78);
    MEM_WriteUInt(0x8017a190, 0x3881006c);
    MEM_WriteUInt(0x8017a194, 0x4bed8cf9);
	
}