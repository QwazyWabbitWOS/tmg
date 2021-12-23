
#include "g_local.h"
#include "g_items.h"
#include "bot.h"

// instantiate cvars here
cvar_t* use_bots;
cvar_t* bot_num;
cvar_t* bot_free_clients;
cvar_t* bot_insult;
cvar_t* bot_chat;
cvar_t* bot_camptime;
cvar_t* bot_walkspeed; //20
cvar_t* bot_runspeed;  //32
cvar_t* bot_duckpeed;  //10
cvar_t* bot_waterspeed;//16


edict_t* bot_team_flag1;
edict_t* bot_team_flag2;


void Bot_InitCvars(void)
{
	// bot commands
	use_bots = gi.cvar("use_bots", "1", CVAR_LATCH);
	bot_num = gi.cvar("bot_num", "2", 0);
	bot_free_clients = gi.cvar("bot_free_clients", "2", CVAR_ARCHIVE);
	bot_insult = gi.cvar("bot_insult", "1", 0);
	bot_chat = gi.cvar("bot_chat", "1", 0);
	bot_camptime = gi.cvar("bot_camptime", "30", 0);
	bot_walkspeed = gi.cvar("bot_walkspeed", "25", 0);     //20
	bot_runspeed = gi.cvar("bot_runspeed", "40", 0);       //32
	bot_duckpeed = gi.cvar("bot_duckpeed", "20", 0);       //10
	bot_waterspeed = gi.cvar("bot_waterspeed", "20", 0);   //16

}


void SetBotFlag1(edict_t* ent) { bot_team_flag1 = ent; }
void SetBotFlag2(edict_t* ent) { bot_team_flag2 = ent; }

edict_t* GetBotFlag1(void) { return bot_team_flag1; }
edict_t* GetBotFlag2(void) { return bot_team_flag2; }

qboolean ChkTFlg(void)
{
	if (bot_team_flag1 != NULL &&
		bot_team_flag2 != NULL)
		return true;
	else
		return false;
}

void SpawnItem4(edict_t* ent, gitem_t* item)
{
	ent->item = item;
	ent->nextthink = level.time;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = 0;
	ent->s.renderfx = RF_GLOW;
	droptofloor(ent);
	ent->s.modelindex = 0;
	ent->nextthink = level.time + 100 * FRAMETIME;    // items start after other solids
	ent->think = G_FreeEdict;
}

//=====================================

//
// BOT Using a visual determination
//

qboolean Bot_trace(edict_t* ent, edict_t* other)
{
	trace_t		rs_trace;
	vec3_t	ttx;
	vec3_t	tty;

	VectorCopy(ent->s.origin, ttx);
	VectorCopy(other->s.origin, tty);
	if (ent->maxs[2] >= 32)
	{
		if (tty[2] > ttx[2])
		{
			tty[2] += 16;
		}
		//else if(ttx[2] > tty[2] > 100 )
		//	tty[2] += 32;
		ttx[2] += 30;
	}
	else
	{
		ttx[2] -= 12;
	}

	rs_trace = gi.trace(ttx, NULL, NULL, tty, ent,
		CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA |
		CONTENTS_SLIME);
	if (rs_trace.fraction == 1.0 && !rs_trace.allsolid &&
		!rs_trace.startsolid)
	{
		return true;
	}
	if (ent->maxs[2] < 32)
	{
		return false;
	}

	if (other->classname[6] == 'F' || other->classname[0] == 'w')
	{
		// nothing
	}
	else if (other->classname[0] == 'i')
	{
		if (other->classname[5] == 'q'
			|| other->classname[5] == 'f'
			|| other->classname[5] == 't'
			|| other->classname[5] == 'i'
			|| other->classname[5] == 'h'
			|| other->classname[5] == 'a') 
		{
			// nothing
		}
		else
			return false;
	}
	else
		return false;

	if (rs_trace.ent != NULL)
	{
		if (rs_trace.ent->classname[0] == 'f'
			&& rs_trace.ent->classname[5] == 'd'
			&& rs_trace.ent->targetname == NULL)
			return true;
	}

	if (ent->s.origin[2] < other->s.origin[2] ||
		ent->s.origin[2] - 24 >  other->s.origin[2])
		return false;

	ttx[2] -= 36;
	rs_trace = gi.trace(ttx, NULL, NULL, other->s.origin, ent,
		CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA |
		CONTENTS_SLIME /*|CONTENTS_TRANSLUCENT*/);
	if (rs_trace.fraction == 1.0 &&
		!rs_trace.allsolid && !rs_trace.startsolid)
	{
		return true;
	}
	return false;
}


qboolean Bot_traceX(edict_t* ent, edict_t* other)
{
	trace_t		rs_trace;
	vec3_t	ttx, tty;
	VectorCopy(ent->s.origin, ttx);
	VectorCopy(other->s.origin, tty);
	ttx[2] += 16;
	tty[2] += 16;

	rs_trace = gi.trace(ttx, NULL, NULL, tty, ent,
		CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA | CONTENTS_SLIME);
	if (rs_trace.fraction == 1.0)
		return true;
	return false;
}

qboolean Bot_traceY(edict_t* ent, edict_t* other)
{
	trace_t		rs_trace;
	vec3_t	ttx, tty;
	VectorCopy(ent->s.origin, ttx);
	VectorCopy(other->s.origin, tty);
	if (ent->maxs[2] >= 32)
	{
		ttx[2] += 24;
	}
	else
	{
		ttx[2] -= 12;
	}

	tty[2] += 16;

	rs_trace = gi.trace(ttx, NULL, NULL, tty, ent,
		CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA | CONTENTS_SLIME);
	if (rs_trace.fraction == 1.0)
		return true;
	return false;
}

//
// Bot_trace2
//

qboolean Bot_trace2(edict_t* ent, vec3_t ttz)
{
	trace_t		rs_trace;
	vec3_t	ttx;
	VectorCopy(ent->s.origin, ttx);
	if (ent->maxs[2] >= 32) ttx[2] += 24;
	else ttx[2] -= 12;

	rs_trace = gi.trace(ttx, NULL, NULL, ttz, ent,
		CONTENTS_SOLID | CONTENTS_LAVA | CONTENTS_SLIME);
	if (rs_trace.fraction != 1.0)
		return false;
	return true;
}

//
// Bot_traceS
//

qboolean Bot_traceS(edict_t* ent, edict_t* other)
{
	trace_t		rs_trace;
	vec3_t	start, end;
	int		mycont;


	VectorCopy(ent->s.origin, start);
	VectorCopy(other->s.origin, end);

	start[2] += ent->viewheight - 8;
	end[2] += other->viewheight - 8;

	if (Bot[ent->client->zc.botindex].param[BOP_NOSTHRWATER])
		goto WATERMODE;

	rs_trace = gi.trace(start, NULL, NULL, end, ent,
		CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA | CONTENTS_SLIME);

	if (rs_trace.fraction != 1.0)
		return false;
	return true;

WATERMODE:
	mycont = gi.pointcontents(start);

	if ((mycont & CONTENTS_WATER) && !other->waterlevel)
	{
		rs_trace = gi.trace(end, NULL, NULL, start, ent,
			CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA |
			CONTENTS_SLIME | CONTENTS_WATER);
		if (rs_trace.surface)
		{
			if (rs_trace.surface->flags & SURF_WARP)
				return false;
		}
		rs_trace = gi.trace(start, NULL, NULL, end, ent,
			CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA | CONTENTS_SLIME);
		if (rs_trace.fraction != 1.0)
			return false;
		return true;
	}
	else if ((mycont & CONTENTS_WATER) && other->waterlevel)
	{
		VectorCopy(other->s.origin, end);
		end[2] -= 16;
		rs_trace = gi.trace(start, NULL, NULL, end, ent,
			CONTENTS_SOLID | CONTENTS_WINDOW);
		if (rs_trace.fraction != 1.0)
			return false;
		return true;
	}

	if (other->waterlevel)
	{
		VectorCopy(other->s.origin, end);
		end[2] += 32;
		rs_trace = gi.trace(start, NULL, NULL, end, ent,
			CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_WATER);
		if (rs_trace.surface)
		{
			if (rs_trace.surface->flags & SURF_WARP)
			{
				return false;
			}
		}
		//		if(rs_trace.fraction != 1.0 ) return false;
		//		return true;
	}

	rs_trace = gi.trace(start, NULL, NULL, end, ent,
		CONTENTS_SOLID | CONTENTS_WINDOW | CONTENTS_LAVA | CONTENTS_SLIME);
	if (rs_trace.fraction != 1.0)
	{
		return false;
	}
	return true;
}

float Get_yaw(vec3_t vec)
{
	vec3_t		out;
	double		yaw;

	VectorCopy(vec, out);
	out[2] = 0;
	VectorNormalize2(out, out);

	yaw = acos((double)out[0]);
	yaw = yaw / M_PI * 180;

	if (asin((double)out[1]) < 0)
		yaw *= -1;

	return (float)yaw;
}

float Get_pitch(vec3_t vec)
{
	vec3_t		out;
	float		pitch;

	VectorNormalize2(vec, out);

	pitch = acos((double)out[2]);
	pitch = ((float)pitch) / M_PI * 180;

	pitch -= 90;
	if (pitch < -180)
		pitch += 360;

	return pitch;
}

float Get_vec_yaw(vec3_t vec, float yaw)
{
	float		vecsyaw;

	vecsyaw = Get_yaw(vec);

	if (vecsyaw > yaw)
		vecsyaw -= yaw;
	else
		vecsyaw = yaw - vecsyaw;

	if (vecsyaw > 180)
		vecsyaw = 360 - vecsyaw;

	return vecsyaw;
}

float Get_vec_yaw2(vec3_t vec, float yaw)
{
	float		vecsyaw;

	vecsyaw = Get_yaw(vec);

	vecsyaw -= yaw;
	if (vecsyaw > 180)
		vecsyaw -= 360;
	else if (vecsyaw < -180)
		vecsyaw += 360;

	return vecsyaw;
}
