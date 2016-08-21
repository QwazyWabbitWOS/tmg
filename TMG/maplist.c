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
	sprintf(buffer, "%s/baseq2/maps/%s.bsp", basedir->string, mapname);
	mf = fopen (buffer, "r");
	if (mf != NULL) {
		fclose(mf);
		return true;
	}

	// check gamedir
	sprintf(buffer, "%s/maps/%s.bsp", game_dir->string, mapname);
	mf = fopen (buffer, "r");
	if (mf != NULL) {	
		fclose(mf);
		return true;
	}
	return false; // no file found
}

/*
* Called by EndDMLevel()
* MaplistNext returns QTRUE if it has stored the name of
* the next map to run in level.nextmap.
* The EndDMLevel function can vary from mod to mod.
* A sample EndDMLevel function that calls this code
* is at the end of this file.
*/
//qboolean Maplist_Next (void)
//{
//	long i;
//	int	ilp;
//	long offset;
//	FILE *in;
//	char *res_cp;
//	char buffer[MAX_QPATH];
//	qboolean ok;
//	
//	// Make sure we can find the game directory.
//	if (!gamedir || !gamedir->string[0]) {
//		gi.dprintf ("No maplist -- can't find gamedir\n");
//		return false;
//	}
//	
//	// Make sure we can find the maplist file name.
//	if (!maplistfile || !maplistfile->string[0]) {
//		gi.dprintf ("Error: Maplist file name is null.\n");
//		return false;
//	}
//	
//	// Get the offset in the maplist.txt file.  
//	// Zero means maplist is turned off.
//	offset = (int)(maplist->value);
//	if (offset <= 0) 
//		return false;
//	
//	Maplist_VariableLoad();	// use player load sense if enabled.
//	Maplist_VariesMonthly();	// test and use monthly map lists
//	offset = (int)(maplist->value); //get the updated value if list was switched
//	
//	ilp = 0;
//	do	// now we get the map name from the maplist file
//	{
//		sprintf (buffer, "./%s/%s", gamedir->string, mymaplistfile->string);
//		in = fopen (buffer, "r");
//		if (in == NULL)
//		{
//			gi.dprintf ("No maplist -- can't open ./%s/%s\n",
//				gamedir->string, mymaplistfile->string);
//			return false;
//		}
//		
//		i = 0;
//		do	// index line by line to match offset else hit EOF and wrap around
//		{
//			res_cp = fgets (buffer, sizeof (buffer), in);
//			if (res_cp == NULL)
//			{
//				// End-of-file errors are OK.
//				if (feof (in)) 
//				{
//					i = 0;  // begin at the beginning
//					offset = 1;
//					rewind(in);
//					if (ilp++ < 1)	// infinite loop prevention
//						continue;
//					else
//					{
//						gi.dprintf ("ERROR: Maplist file is empty! /%s/%s\n",
//							gamedir->string, mymaplistfile->string);
//						return false;
//					}
//				}
//				// Other errors are not OK.
//				gi.dprintf ("No maplist -- error reading ./%s/%s\n",
//					gamedir->string, mymaplistfile->string);
//				return false;	// abort map change
//			}
//			i++;
//		} while (i < offset); // we found the line we want
//		
//		if (in != NULL)
//			fclose (in);
//		ok = true; // we think we have a map name
//		
//		// Trim any newline(s) or spaces from the end of the string.
//		res_cp = buffer + strlen (buffer) - 1;
//		while (res_cp >= buffer && (*res_cp == '\n' || *res_cp == ' ' || *res_cp == '\r'))
//		{
//			*res_cp = '\0';
//			res_cp--;
//		}
//		
//		if (!buffer[0]) // oops, buffer line is blank, warn and recover at next line
//		{
//			gi.bprintf(PRINT_HIGH,
//				"WARNING: Maplist line %i is blank, using next map in list.\n", offset);
//			offset++;
//			ok = false;
//			continue;
//		}
//		
//		if (!Q_strcasecmp(buffer, level.mapname)) //if nextmap is same map, skip to next one
//		{
//			gi.bprintf(PRINT_HIGH,
//				"WARNING: Skipping double map, using next map in list.\n", offset);
//			offset++;
//			ok = false;
//			continue;
//		}
//		
//		// If we get this far we have a string that is supposed to be a valid map name.
//		// Check the list of 48 stock Quake 2 maps and see if it's one of them.
//		// If it isn't a stock map, check for the existence of a map file (bsp)
//		if (!Maplist_CheckStockmaps(buffer) && !Maplist_CheckFileExists(buffer)) 
//		{
//			gi.bprintf(PRINT_HIGH, 
//				"WARNING: Maplist line %i, map %s was not found. Using next map in list.\n", offset, buffer);
//			offset++;
//			ok = false;
//			continue; // try next one in list
//		}
//	} while (!ok); //exit do-while if ok.
//	
//	// passed all the tests and we have a file
//	strcpy (level.nextmap, buffer); // stuff it into the nextmap member
//	offset++;
//	sprintf (buffer, "%ld", offset);	//save the current counter in the maplist pointer cvar
//	maplist = gi.cvar_set (maplist->name, buffer);//no matter what maplist counter we are using
//	return true;	// good to go
//}

//static int Maplist_CountPlayers(void)
//{
//	int i, count;
//	edict_t *e;
//	
//	count = 0;
//	for (i = 1; i <= maxclients->value; i++) {
//		e = g_edicts + i;
//		if (e->inuse)	//only active players
//			count++;
//	}
//	return (count);
//}

// maplist varies with number of players
//static void Maplist_VariableLoad(void)
//{
//	int num;
//	
//	// running load sensing.
//	if (maplistvaries->value == 1)
//	{
//		num = Maplist_CountPlayers();
//		gi.bprintf (PRINT_HIGH, "QwazyWabbit's load sensing counted %i players.\n", num);
//		
//		// maplist points to the cvar based on the load sensed
//		if (num <= map_lowcount->value) 
//		{
//			maplist = maplist1;
//			mymaplistfile = gi.cvar_set ("mymaplistfile", maplistfile1->string); // the file name
//		}
//		if (num <= map_medcount->value && num > map_lowcount->value) 
//		{
//			maplist = maplist2;
//			mymaplistfile = gi.cvar_set ("mymaplistfile", maplistfile2->string);
//		}
//		if (num > map_medcount->value) 
//		{
//			maplist = maplist3;
//			mymaplistfile = gi.cvar_set ("mymaplistfile", maplistfile3->string);
//		}
//	} 
//	else if (maplistvaries->value != 2) 
//	{
//		// the active maplist is the flat load maplist
//		mymaplistfile = gi.cvar_set("mymaplistfile", maplistfile->string);
//		// recover maplist pointer in case we changed maplistvaries
//		maplist = gi.cvar ("maplist", "0", 0);
//	}
//}

// automatic monthly maplist rotation
// tests to be sure maplist files exist for each month
// and if not, disable the rotation and go back to single
// maplist file.
//static void Maplist_VariesMonthly(void)
//{
//	time_t	ct;
//	struct	tm *lt;
//	char	month[4];
//	char	file[MAX_QPATH + 1];
//	char	buff[MAX_QPATH + 1];
//	FILE	*mf;
//	
//	// running monthly auto rotation
//	if (maplistvaries->value == 2)
//	{
//		time(&ct);
//		lt = localtime(&ct);
//		if (lt != NULL)
//		{
//			sprintf(month,"%02i", lt->tm_mon + 1); //JAN thru DEC = 01 thru 12
//			sprintf(file, "maplist%s.txt", month); // maplistNN.txt
//			mymaplistfile = gi.cvar_set ("mymaplistfile", file);
//			sprintf(buff, "%s/%s", gamedir->string, mymaplistfile->string);
//			mf = fopen (buff, "r");		// check maplist file exists
//			if (mf == NULL) // oops, nope
//			{
//				gi.dprintf ("Error: Maplist file %s doesn't exist. Monthly autorotation disabled.\n",buff);
//				maplistvaries = gi.cvar_set("maplistvaries", "0");
//				mymaplistfile = gi.cvar_set("mymaplistfile", maplistfile->string);
//			}
//			else
//				fclose(mf);
//		}
//		else //lt is NULL
//		{
//			// clock time erred, can't trust it.
//			maplistvaries = gi.cvar_set("maplistvaries", "0");
//			mymaplistfile = gi.cvar_set("mymaplistfile", maplistfile->string);
//			gi.dprintf ("Error: Maplist detected bad clock time. Monthly autorotation disabled.\n");
//		}
//	} 
//	else if (maplistvaries->value != 1)
//	{
//		// the active maplist is the flat load maplist
//		mymaplistfile = gi.cvar_set("mymaplistfile", maplistfile->string);
//		// recover maplist pointer in case we changed maplistvaries
//		maplist = gi.cvar ("maplist", "0", 0);
//	}
//}

//QW// CODE SAMPLE CreateTargetChangeLevel() and EndDMLevel() //QW//
//
// These functions can vary depending on mod and vintage
// it's important to examine the equivalent functions in your own code
// before copying this into them.

#if 0	//this is sample code

//create a target_changelevel entitity for the next map
edict_t *CreateTargetChangeLevel(char *map)
{
	edict_t *ent;
	
	ent = G_Spawn ();
	ent->classname = "target_changelevel";
	Com_sprintf(level.nextmap, sizeof(level.nextmap), "%s", map);
	ent->map = level.nextmap;
	return ent;
}

// Compatible with CTF and coop mods
// =================
// EndDMLevel
//
// The timelimit or fraglimit has been exceeded
// =================
//
void EndDMLevel (void)
{
	edict_t	*ent;
	
	// go to round 2 if CTF
	if (ctf->value && TeamplayCheckRound1())
	{
		TeamplayStartRound2();
		return;
	}
	
	// stay on same level flag
	else if ((int)dmflags->value & DF_SAME_LEVEL)
		ent = CreateTargetChangeLevel(level.mapname);
	
	// get the next one out of the maplist
	else if (Maplist_Next()) //returns QTRUE if maplist in use and map exists
		ent = CreateTargetChangeLevel(level.nextmap);
	
	// go to a specific map
	else if (level.nextmap[0])
		ent = CreateTargetChangeLevel(level.nextmap);
	
	// search for a changelevel within the current map.
	// this is for the single-player or coop game.
	else
	{
		ent = G_Find (NULL, FOFS(classname), "target_changelevel");
		if (!ent)
			// the map designer didn't include a changelevel,
			// so create a fake ent that goes back to the same level
			ent = CreateTargetChangeLevel(level.mapname);
	}
	
	gi.dprintf ("Map is changing to %s\n", level.nextmap);
	
	BeginIntermission (ent);
}
#endif
