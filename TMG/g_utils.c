// g_utils.c -- misc utility functions for game module

#include "g_local.h"


void G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}

//JSW
edict_t *Find_Player_Edict_t (char *s)
{
	int i;
	edict_t *player;
	for(i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];
		if (player->inuse && (Q_stricmp(s, player->client->pers.netname) == 0))
			return player;
	}
	return NULL;
}
//end
/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
edict_t *G_Find (edict_t *from, int fieldofs, char *match)
{
	char	*s;

	if (!from)
		from = g_edicts;
	else
		from++;

	for ( ; from <  & g_edicts[globals.num_edicts] ; from++)
	{
		if (!from->inuse)
			continue;
		s = *(char **) ((byte *)from + fieldofs);
		if (!s)
			continue;
		if (!Q_stricmp (s, match))
			return from;
	}

	return NULL;
}


/*
=================
findradius

Returns entities that have origins within a spherical area

findradius (origin, radius)
=================
*/
edict_t *findradius (edict_t *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	int		j;

	if (!from)
		from = g_edicts;
	else
		from++;
	for ( ; from < &g_edicts[globals.num_edicts]; from++)
	{
		if (!from->inuse)
			continue;
		if (from->solid == SOLID_NOT)
			continue;
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);
		if (VectorLength(eorg) > rad)
			continue;
		return from;
	}

	return NULL;
}


/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
#define MAXCHOICES	8

edict_t *G_PickTarget (char *targetname)
{
	edict_t	*ent = NULL;
	int		num_choices = 0;
	edict_t	*choice[MAXCHOICES];

	if (!targetname)
	{
		gi.dprintf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_Find (ent, FOFS(targetname), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
		gi.dprintf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}



void Think_Delay (edict_t *ent)
{
	G_UseTargets (ent, ent->activator);
	G_FreeEdict (ent);
}

/*
==============================
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void G_UseTargets (edict_t *ent, edict_t *activator)
{
	edict_t		*t;

//
// check for a delay
//
	if (ent->delay)
	{
	// create a temp object to fire at a later time
		t = G_Spawn();
		t->classname = "DelayedUse";
		t->nextthink = level.time + ent->delay;
		t->think = Think_Delay;
		t->activator = activator;
		if (!activator)
			gi.dprintf ("MAP DESIGN ERROR: Think_Delay with no activator\n");
		t->message = ent->message;
		t->target = ent->target;
		t->killtarget = ent->killtarget;
		return;
	}
	
	
//
// print the message
//
	if ((ent->message) && !(activator->svflags & SVF_MONSTER))
	{
		if (!activator->bot_client)
			gi.centerprintf (activator, "%s", ent->message);
		if (ent->noise_index)
			gi.sound (activator, CHAN_AUTO, ent->noise_index, 1, ATTN_NORM, 0);
		else
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}

//
// kill killtargets
//
	if (ent->killtarget)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->killtarget)))
		{
			G_FreeEdict (t);
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

//
// fire targets
//
	if (ent->target)
	{
		t = NULL;
		while ((t = G_Find (t, FOFS(targetname), ent->target)))
		{
			// doors fire area portals in a specific way
			if (!Q_stricmp(t->classname, "func_areaportal") &&
				(!Q_stricmp(ent->classname, "func_door") || !Q_stricmp(ent->classname, "func_door_rotating")))
				continue;

			if (t == ent)
			{
				gi.dprintf ("WARNING: Entity used itself.\n");
			}
			else
			{
				if (t->use)
					t->use (t, ent, activator);
			}
			if (!ent->inuse)
			{
				gi.dprintf("entity was removed while using targets\n");
				return;
			}
		}
	}
}


/**
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float	*tv (float x, float y, float z)
{
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}


/**
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos (vec3_t v)
{
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}


vec3_t VEC_UP		= {0, -1, 0};
vec3_t MOVEDIR_UP	= {0, 0, 1};
vec3_t VEC_DOWN		= {0, -2, 0};
vec3_t MOVEDIR_DOWN	= {0, 0, -1};

void G_SetMovedir (vec3_t angles, vec3_t movedir)
{
	if (VectorCompare (angles, VEC_UP))
	{
		VectorCopy (MOVEDIR_UP, movedir);
	}
	else if (VectorCompare (angles, VEC_DOWN))
	{
		VectorCopy (MOVEDIR_DOWN, movedir);
	}
	else
	{
		AngleVectors (angles, movedir, NULL, NULL);
	}

	VectorClear (angles);
}


float vectoyaw (vec3_t vec)
{
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0)
		yaw = 0;
	else
	{
		yaw = (int) (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}


void vectoangles (vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;

		forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}

char *G_CopyString (char *in)
{
	char	*out;
	
	out = gi.TagMalloc ((int) strlen(in) + 1, TAG_LEVEL);
	strcpy (out, in);
	return out;
}


void G_InitEdict (edict_t *e)
{
	e->inuse = true;
	e->classname = "noclass";
	e->gravity = 1.0;
	e->s.number = (int) (e - g_edicts);

	// Clear what the free-edict list may have set.
	e->chain = NULL;

	// This is another headache.
	e->think = NULL;
	e->nextthink = 0;
}

//=================
/**
G_Spawn

 Either finds a free edict, or allocates a new one.

 Try to avoid reusing an entity that was recently freed, because it
 can cause the client to think the entity morphed into something else
 instead of being removed and recreated, which can cause interpolated
 angles and bad trails.
*/
//=================
edict_t *G_Spawn (void)
{
	int			i;
	edict_t		*e;

	e = &g_edicts[(int)maxclients->value+1];
	for ( i=maxclients->value+1 ; i<globals.num_edicts ; i++, e++)
	{
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!e->inuse && ( e->freetime < 2 || level.time - e->freetime > 0.5 ) )
		{
			G_InitEdict (e);
			return e;
		}
	}
	
	if (i == game.maxentities)
		gi.error ("ED_Alloc: no free edicts");
		
	globals.num_edicts++;
	
	G_InitEdict (e);
	return e;
}

/*
=================
G_FreeEdict

Marks the edict as free
=================
*/
void G_FreeEdict (edict_t *ed)
{
	gi.unlinkentity (ed);		// unlink from world

	if ((ed - g_edicts) <= (maxclients->value + BODY_QUEUE_SIZE))
	{
		//gi.drintf("tried to free special edict\n");
		return;
	}
	
	if(debug_spawn->value && ed != NULL)
		DbgPrintf ("%s movetype %d inuse %d linkcount %d classname %s time: %.1f\n",
		__func__, ed->movetype, ed->inuse, ed->linkcount, ed->classname, level.time);

	memset (ed, 0, sizeof(*ed));
	ed->classname = "freed";
	ed->freetime = level.time;
	ed->inuse = false;
	ed->chain = NULL;
}


/*
============
G_TouchTriggers

============
*/
void	G_TouchTriggers (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	// dead things don't activate triggers!
	if ((ent->client || (ent->svflags & SVF_MONSTER)) && (ent->health <= 0))
		return;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch
		, MAX_EDICTS, AREA_TRIGGERS);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (!hit->touch)
			continue;
		hit->touch (hit, ent, NULL, NULL);
	}
}

/*
============
G_TouchSolids

Call after linking a new trigger in during gameplay
to force all entities it covers to immediately touch it
============
*/
void	G_TouchSolids (edict_t *ent)
{
	int			i, num;
	edict_t		*touch[MAX_EDICTS], *hit;

	num = gi.BoxEdicts (ent->absmin, ent->absmax, touch,
		MAX_EDICTS, AREA_SOLID);

	// be careful, it is possible to have an entity in this
	// list removed before we get to it (killtriggered)
	for (i=0 ; i<num ; i++)
	{
		hit = touch[i];
		if (!hit->inuse)
			continue;
		if (ent->touch)
			ent->touch (hit, ent, NULL, NULL);
		if (!ent->inuse)
			break;
	}
}

/*
Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
qboolean KillBox (edict_t *ent)
{
  trace_t    tr;

  gi.unlinkentity (ent);

  while (1)
  {
    tr = gi.trace (ent->s.origin, ent->mins, ent->maxs,
				   ent->s.origin, NULL, MASK_PLAYERSOLID);
    if (!tr.ent)
      break;

    // nail it
    T_Damage (tr.ent, ent, ent, vec3_origin, ent->s.origin,
			  vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);

    // if we didn't kill it, fail
    if (tr.ent->solid)
      return false;
  }

  return true;    // all clear
}

// Ridah
/*
==================
  QC equivalent, sends a command to the client's console
==================
*/
void StuffCmd(edict_t *ent, char *text)
{

	if(!G_EntExists(ent) || !ent)
		return;

	if(ent->bot_client)
	{
		gi.dprintf ("%s\n", text);
		return;
	}
	gi.WriteByte(11);				// 11 = svc_stufftext
	gi.WriteString(text);
	gi.unicast(ent, true);
}

float	entdist(edict_t *ent1, edict_t *ent2)
{
	vec3_t	vec;

	VectorSubtract(ent1->s.origin, ent2->s.origin, vec);
	return VectorLength(vec);
}

/*
=================
AddModelSkin

  Adds a skin reference to an .md2 file, saving as filename.md2new
=================
*/
void AddModelSkin (char *modelfile, char *skinname)
{
	FILE	*f, *out;
	int		buffer_int;
	size_t	i = 0;
	char	filename[MAX_QPATH];
	char	infilename[MAX_QPATH];
	char	buffer;
	size_t	count;

	Com_sprintf(infilename, sizeof infilename, "%s", modelfile);

	f = fopen (infilename, "rb");

	if (!f)
	{
		gi.dprintf("Cannot open file %s\n", infilename);
		return;
	}

	Com_sprintf(filename, sizeof filename, "%s/"MOD"/%snew", basedir->string, modelfile);

	out = fopen (filename, "wb");

	if (!out)
		return;

	// mirror header stuff before skinnum
	for (i=0; i<5; i++)
	{
		count = fread(&buffer_int, sizeof(buffer_int), 1, f);
		fwrite(&buffer_int, sizeof(buffer_int), 1, out);
	}

	// increment skinnum
	count = fread(&buffer_int, sizeof(buffer_int), 1, f);
	++buffer_int;
	fwrite(&buffer_int, sizeof(buffer_int), 1, out);

	// mirror header stuff before skin_ofs
	for (i=0; i<5; i++)
	{
		count = fread(&buffer_int, sizeof(buffer_int), 1, f);
		fwrite(&buffer_int, sizeof(buffer_int), 1, out);
	}

	// copy the skins offset value, since it doesn't change
	count = fread(&buffer_int, sizeof(buffer_int), 1, f);
	fwrite(&buffer_int, sizeof(buffer_int), 1, out);

	// increment all offsets by 64 to make way for new skin
	for (i=0; i<5; i++)
	{
		count = fread(&buffer_int, sizeof(buffer_int), 1, f);
		buffer_int += 64;
		fwrite(&buffer_int, sizeof(buffer_int), 1, out);
	}

	// write the new skin
	for (i=0; i<strlen(skinname); i++)
	{
		fwrite(&(skinname[i]), 1, 1, out);
	}
	
	buffer = '\0';
	fwrite(&buffer, 1, 1, out);
	buffer = 'x';
	fwrite(&buffer, 1, 1, out);

	buffer = '\0';
	for (i = (strlen(skinname) + 2); i < 64; i++)
	{
		fwrite(&buffer, 1, 1, out);
	}

	// copy the rest of the file
	count = fread(&buffer, sizeof(buffer), 1, f);
	while (!feof(f))
	{
		fwrite(&buffer, sizeof(buffer), 1, out);
		count = fread(&buffer, sizeof(buffer), 1, f);
	}

	fclose (f);
	fclose (out);

	// copy the new file over the old file
	remove(infilename);
	rename(filename, infilename);

	gi.dprintf("Model skin added.\n", filename);
}

// Safely print formatted text to players
// Then convert to white text for the log
// when running dedicated server
void my_bprintf (int printlevel, char *fmt, ...)
{
	char	buffer[0x10000];
	va_list		argptr;

	va_start (argptr, fmt);
	vsprintf (buffer, fmt, argptr);
	va_end (argptr);

	// Highlighted text to players
	gi.bprintf(printlevel, "%s", buffer);

	//QW// This is needed for Wallfly
	if (dedicated->value)
	{
		white_text(buffer, buffer);
		gi.dprintf ("%s", buffer); // White text to log/console
	}
}

//======================================================================
// New ACE-compatible message routines

// botsafe cprintf
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...)
{
	char	buffer[0x10000];
	va_list	argptr;

	if (!ent || !ent->inuse || 
		ent->bot_client)
		return;

	va_start (argptr, fmt);
	(void) vsprintf (buffer, fmt, argptr);
	va_end (argptr);

	// Safety check...
	if (G_EntExists(ent))
		gi.cprintf(ent, printlevel, "%s", buffer);
	
}

// botsafe centerprintf
void safe_centerprintf (edict_t *ent, char *fmt, ...)
{
	char	buffer[0x10000];
	va_list	argptr;

	if (!ent->inuse || 
		ent->bot_client)
		return;

	va_start (argptr, fmt);
	(void) vsprintf (buffer, fmt, argptr);
	va_end (argptr);

	// Safety check...
	if (G_EntExists(ent))
		gi.centerprintf(ent, "%s", buffer);
	
}

// botsafe bprintf
void safe_bprintf (int printlevel, char *fmt, ...)
{
	int		i;
	char	buffer[0x10000];
	va_list	argptr;
	edict_t	*cl_ent;

	va_start (argptr, fmt);
	(void) vsprintf (buffer, fmt, argptr);
	va_end (argptr);

	// This is to be compatible with Eraser (ACE)
	// Ridah, changed this so CAM works
	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;

	if (G_EntExists(cl_ent) && !cl_ent->bot_client)
			gi.cprintf(cl_ent, printlevel, "%s", buffer);
	}

	if (dedicated->value)
	{
		white_text(buffer, buffer);
		gi.cprintf(NULL, printlevel, "%s", buffer);
	}
}

//======================================================
// True if Ent is valid, has client, and edict_t inuse.
//======================================================
qboolean G_EntExists(edict_t *ent)
{
	return ((ent) && 
		(ent->client) && 
		(ent->inuse) && 
		(ent->client->ping < 800));
}

//======================================================
// True if ent is not DEAD or DEAD or DEAD (and BURIED!)
//======================================================
qboolean G_ClientNotDead(edict_t *ent)
{
	qboolean buried = true;
	qboolean b1 = ent->client->ps.pmove.pm_type != PM_DEAD;
	qboolean b2 = ent->deadflag != DEAD_DEAD;
	qboolean b3 = ent->health > 0;
	return ((b3 || b2 || b1) && buried);
}

//======================================================
// True if ent is not DEAD and not just did a Respawn.
//======================================================
qboolean G_ClientInGame(edict_t *ent)
{
	if (!G_EntExists(ent))
		return false;
	if (!G_ClientNotDead(ent))
		return false;
	return (ent->client->respawn_time + 5.0 < level.time);
}

//JSW
// From QDevels, posted by outlaw
//void convert_string(char *src, char start, char end, char add, char *dest)
//{
//    int n = -1;
//    while ((dest[++n] = src[n]))
//        if ((dest[n] >= start) && (dest[n] <= end) && (dest[n] != '\n'))
//            dest[n] += add;
//}

/* Examples of convert_string usage
 {
 char  text[] = "abcdefgABCDEFG1234567\n\0";
 convert_string(text, 'a', 'z', ('A'-'a'), text); // a -> A
	my_bprintf (PRINT_CHAT, "text = %s\n", text);
 convert_string(text, 'A', 'Z', ('a'-'A'), text); // A -> a
	my_bprintf (PRINT_CHAT, "text = %s\n", text);
 convert_string(text, 0, 127, 128, text); // white -> green
	my_bprintf (PRINT_CHAT, "text = %s\n", text);
 convert_string(text, 128, 255, -128, text); // green -> white
	my_bprintf (PRINT_CHAT, "text = %s\n", text);
 }
 */


//QW//
/**
 Replace characters in destination string.
 Parameter 'add' is added to each character 
 found in source and result is placed in dest.
 Parameters 'start' and 'end' specify character range to replace.
 Source text must be a valid C string.
 */
//QwazyWabbit// A pointer version to eliminate undefined behavior.
// Cover both line endings in case Windows file ends up on *nix server.
void convert_string(char *src, char start, char end, char add, char *dest)
{
	while ((*dest = *src))
	{
		if ( (*dest >= start) && (*dest <= end) && (*dest != '\n') && (*dest != '\r') )
			*dest += add;
		src++, dest++;
	}
}

/**
 Set msb in specified string characters, copying them to destination. 
 Text must be a valid C string.
 Source and destination can be the same.
 If dest == NULL the action occurs in-place.
 */
void highlight_text (char *src, char *dest)
{
	if (dest == NULL)
		dest = src;
	convert_string(src, 0, 0x7f, 0x80, dest); // white -> green
}

/**
 Clear msb in specified string characters, copying them to destination.
 Text must be a valid C string.
 Source and destination can be the same.
 If dest == NULL the action occurs in-place.
 */
void white_text (char *src, char *dest)
{
	if (dest == NULL)
		dest = src;
	convert_string(src, 0x80, 0xff, -128, dest); // green -> white
}

/**
 Make text uppercase.
 Text must be a valid C string.
 Source and destination can be the same.
 If dest == NULL the action occurs in-place.
 */
void toupper_text(char *src, char *dest)
{
	if (dest == NULL)
		dest = src;
	convert_string(src, 'a', 'z', ('A'-'a'), dest); // a -> A
}

/**
 Make text lowercase.
 Text must be a valid C string.
 Source and destination can be the same string.
 If dest == NULL the action occurs in-place.
 */
void tolower_text(char *src, char *dest)
{
	if (dest == NULL)
		dest = src;
	convert_string(src, 'A', 'Z', ('a'-'A'), dest); // A -> a
}


qboolean CheckFlood (edict_t *who)
{
	int	i;
	gclient_t *cl;

	cl = who->client;

	//DB
	if (level.time < cl->flood_locktill)
	{
		safe_cprintf(who, PRINT_HIGH, "You can't talk for %d more seconds\n",
					 (int)(cl->flood_locktill - level.time));
		return false;
	}

	i = cl->flood_whenhead - flood_msgs->value + 1;

	if (i < 0)
		i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;

	if (cl->flood_when[i] &&
		level.time - cl->flood_when[i] < flood_persecond->value)
	{
		cl->flood_locktill = level.time + flood_waitdelay->value;
		safe_cprintf(who, PRINT_CHAT,
					 "Flood protection: You can't talk for %d seconds.\n",
					 (int)flood_waitdelay->value);
		return false;
	}

	cl->flood_whenhead = (cl->flood_whenhead + 1)
		% (sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));

	cl->flood_when[cl->flood_whenhead] = level.time;
	return true;
}


