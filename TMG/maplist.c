/*
* Copyright (C) 2006 by QwazyWabbit and ClanWOS.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
*
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
* You may freely use and alter this code so long as this banner
* remains and credit is given for its source. 
*/

/**************************************************/
/*                Maplist Selection               */
/**************************************************/

#include "g_local.h"
#include "maplist.h"

// This module gracefully handles maplist file errors such as
// extra blank lines, spaces at the end of names and it
// protects against non-existent maps.

// Interface to this code by adding the maplist.c and maplist.h
// files to your project. Then call the Maplist_InitVars function
// in InitGame to set the cvars to their default values. Customize
// the cvars in your server.cfg to adjust them. Create your flat,
// low-load, medium-load and high load map rotations.
// Call Maplist_Next inside your EndDMLevel function to execute the
// rotations.

// these are the stock quake2 maps in pak0.pak, pak1.pak and pak3,pak
static char *stockmaps[48] = 
{
	"base1", "base2", "base3", "biggun", "boss1", 
	"boss2", "bunk1", "city1", "city2", "city3", 
	"command", "cool1", "fact1", "fact2", "fact3", 
	"hangar1", "hangar2", "jail1", "jail2", "jail3", 
	"jail4", "jail5", "lab", "mine1", "mine2", 
	"mine3", "mine4", "mintro", "power1", "power2", 
	"security", "space", "strike", "train", "ware1", 
	"ware2", "waste1", "waste2", "waste3", "q2dm1", 
	"q2dm2", "q2dm3", "q2dm4", "q2dm5", "q2dm6", 
	"q2dm7", "q2dm8", // pak1.pak
	"match1" //pak3.pak
}; 

// Called by InitGame() to
// instantiate cvars for maplists and set defaults
void Maplist_InitVars(void)
{
	//QW// custom maplist cvars
	// when maplistvaries = 0, maplist.txt is used for rotation
//	maplist = gi.cvar ("maplist", "0", 0); // current line
	//maplist1 = gi.cvar ("maplist1", "1", 0); // secondary counters (low)
	//maplist2 = gi.cvar ("maplist2", "1", 0); // med
	//maplist3 = gi.cvar ("maplist3", "1", 0); // high
	//maplistfile = gi.cvar ("maplistfile", "maplist.txt", 0); // standard maplist
	//maplistfile1 = gi.cvar ("maplistfile1", "maplist_low.txt", 0); // low player count maps
	//maplistfile2 = gi.cvar ("maplistfile2", "maplist_med.txt", 0);	// medium load maps
	//maplistfile3 = gi.cvar ("maplistfile3", "maplist_high.txt", 0);	// lots of players
	//maplistvaries = gi.cvar ("maplistvaries", "0", 0);	// maplist file changes with load or per month
	//map_lowcount = gi.cvar ("map_lowcount", "6", 0);	// default low load upper threshold
	//map_medcount = gi.cvar ("map_medcount", "12", 0);	// default medium load upper threshold
	gamedir = gi.cvar ("gamedir", "", CVAR_NOSET);	// created by engine, we need to expose it for mod
	basedir = gi.cvar ("basedir", "", CVAR_NOSET);	// strictly read-only
}

// input argument is pointer to selected map
// returns QTRUE if it is a stock map, else QFALSE
qboolean Maplist_CheckStockmaps(char *thismap)
{
	int i;
	
	for (i = 0; i < 48; i++)
	{
		if (strcmp (thismap, stockmaps[i]) == 0)
			return true; 	// it's a stock map
	}
	return false;
}

qboolean Maplist_CheckFileExists(char *mapname)
{
	FILE *mf;
	char buffer[MAX_QPATH];
	
	// check basedir
	Com_sprintf(buffer, sizeof buffer, "%s/baseq2/maps/%s.bsp", basedir->string, mapname);
	mf = fopen (buffer, "r");
	if (mf != NULL) {
		fclose(mf);
		return true;
	}

	// check gamedir
	Com_sprintf(buffer, sizeof buffer, "%s/maps/%s.bsp", game_dir->string, mapname);
	mf = fopen (buffer, "r");
	if (mf != NULL) {	
		fclose(mf);
		return true;
	}
	return false; // no file found
}

