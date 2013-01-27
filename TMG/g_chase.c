#include "g_local.h"

static char chase_modenames[][30] = {
	"FreeCam",
	"ChaseCam",
	"FloatCam",
	"EyeCam"
};
void ToggleChaseCam(edict_t *ent, pmenu_t *p);
void SwitchModeChaseCam(edict_t *ent)
{
	// if chase cam is off, turn it on !
	if (!ent->client->chase_target)
	{
		ToggleChaseCam(ent, NULL);
		return;
	}

	// if we are in the last chasecam mode, turn it off
	if (ent->client->chase_mode == CHASE_LASTMODE)
	{
		ToggleChaseCam(ent, NULL);
		return;
	}

	// switch modes
	safe_cprintf(ent, PRINT_HIGH, "Now using %s.\n", chase_modenames[++ent->client->chase_mode] );
}


// Turn the chasecam on/off
void ToggleChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	// if it's on, turn if off...
	if (ent->client->chase_target) {
		safe_cprintf(ent, PRINT_HIGH, "ChaseCam deactivated.\n");
		ent->client->chase_target = NULL;
	//	ent->client->resp.spectator = 1;
	//	ent->movetype = MOVETYPE_NOCLIP;

		PMenu_Close(ent);
		return;
	}

	// if it's off, find a new chase target and track it
	for (i = 1; i <= maxclients->value; i++) {
		e = g_edicts + i;

		// if we find a player that is in the game,
		// not spectating and not dead, track them !
		if (e->inuse && e->solid != SOLID_NOT){
			ent->client->chase_mode = CHASE_FIRSTMODE;
			ent->client->chase_target = e;
			PMenu_Close(ent);
			ent->client->update_chase = true;
			safe_cprintf(ent, PRINT_HIGH, "ChaseCam activated (using %s mode).\n",
				chase_modenames[ent->client->chase_mode]);
			return;
		}
	}

	// no target to chase...
	safe_cprintf(ent, PRINT_HIGH, "ChaseCam - no target to chase!\n");
}


// this places the client's viewpoint depending on the chase mode !

void UpdateChaseCam(edict_t *ent)
{
	vec3_t o, ownerv, goal;
	edict_t *targ;
	vec3_t forward, right;
	trace_t trace;
	int i;
	vec3_t oldgoal;
	vec3_t angles;
	int viewdist;

	// is our chase target gone?
	if (!ent->client->chase_target->inuse
		|| ent->client->chase_target->client->resp.spectator)
	{
		edict_t *old = ent->client->chase_target;
		ChaseNext(ent);
		if (ent->client->chase_target == old) {
			ent->client->chase_target = NULL;
			ent->client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
			return;
		}
	}
	targ = ent->client->chase_target;

	VectorCopy(targ->s.origin, ownerv);
	VectorCopy(ent->s.origin, oldgoal);

	ownerv[2] += targ->viewheight;

	// SUMFUKA : if in freecam mode, use that angle
	if (ent->client->chase_mode == CHASE_FREECAM)
//		VectorCopy(ent->client->v_angle, angles);
		for (i=0; i<3; i++)
			angles[i] = ent->client->resp.cmd_angles[i] + SHORT2ANGLE(ent->client->ps.pmove.delta_angles[i]);
	else
		VectorCopy(targ->client->ps.viewangles, angles);

	if (angles[PITCH] > 56)
		angles[PITCH] = 56;

	AngleVectors (angles, forward, right, NULL);
	VectorNormalize(forward);

	// SUMFUKA : view at different distances (default -30)
	viewdist = -60;	

	// different distances...
	if (ent->client->chase_mode == CHASE_FLOATCAM)
		viewdist = -200;
	else if (ent->client->chase_mode == CHASE_EYECAM)
		viewdist = 40;

	VectorMA(ownerv, viewdist, forward, o);

	if (o[2] < targ->s.origin[2] + -viewdist*2/3)
		o[2] = targ->s.origin[2] + -viewdist*2/3;

	// jump animation lifts
	if (!targ->groundentity)
		o[2] += 16;

	trace = gi.trace(ownerv, vec3_origin, vec3_origin, o, targ, MASK_SOLID);

	VectorCopy(trace.endpos, goal);

	VectorMA(goal, 2, forward, goal);

	// pad for floors and ceilings
	VectorCopy(goal, o);
	o[2] += 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] -= 6;
	}

	VectorCopy(goal, o);
	o[2] -= 6;
	trace = gi.trace(goal, vec3_origin, vec3_origin, o, targ, MASK_SOLID);
	if (trace.fraction < 1) {
		VectorCopy(trace.endpos, goal);
		goal[2] += 6;
	}

	// SUMFUKA : using a free-move chasecam mode ?
	if (ent->client->chase_mode == CHASE_FREECAM)
		ent->client->ps.pmove.pm_type = PM_SPECTATOR;
	else
		ent->client->ps.pmove.pm_type = PM_FREEZE;

	VectorCopy(goal, ent->s.origin);

	// SUMFUKA : only set angles if in a fixed viewangle mode
	if (ent->client->chase_mode != CHASE_FREECAM)
	{
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(targ->client->v_angle[i] - ent->client->resp.cmd_angles[i]);

		VectorCopy(targ->client->v_angle, ent->client->ps.viewangles);
		VectorCopy(targ->client->v_angle, ent->client->v_angle);
	}

	ent->viewheight = 0;
	ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
	gi.linkentity(ent);

	if ((!ent->client->showscores && !ent->client->menu &&
		!ent->client->showinventory && !ent->client->showhelp &&
		!(level.framenum & 31)) || ent->client->update_chase) {
	
		ent->client->update_chase = false;
		
	}

}

void ChaseNext(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i++;
		if (i > maxclients->value)
			i = 1;
		e = g_edicts + i;
		if (!e->inuse || e == ent)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}

void ChasePrev(edict_t *ent)
{
	int i;
	edict_t *e;

	if (!ent->client->chase_target)
		return;

	i = ent->client->chase_target - g_edicts;
	do {
		i--;
		if (i < 1)
			i = maxclients->value;
		e = g_edicts + i;
		if (!e->inuse || e == ent)
			continue;
		if (e->solid != SOLID_NOT)
			break;
	} while (e != ent->client->chase_target);

	ent->client->chase_target = e;
	ent->client->update_chase = true;
}


// anyone chasing this target must no longer do so...
void ChaseRemoveTarget(edict_t *target)
{
	edict_t *ent;
	int i;

	for_each_player(ent, i)
	{
		// chasing this target ?
		if (ent->client->chase_target == target)
		{
			// turn it off...
			safe_cprintf(ent, PRINT_HIGH, "ChaseCam deactivated - target lost!\n");
			ent->client->chase_target = NULL;
		}
	}

}
void GetChaseTarget(edict_t *ent)
{
	int i;
	edict_t *other;

	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;

		if (other->inuse && other->client->pers.pl_state !=0)
		{
			ent->client->chase_target = other;
			ent->client->update_chase = true;
			UpdateChaseCam(ent);
			return;
		}
	}
	safe_centerprintf(ent, "No other players to chase.");
}



// give a little help !
void ChaseHelp(edict_t *ent)
{
safe_centerprintf (ent, "(use fire to change ChaseCam mode)\n(and [ or ] to change ChaseCam target)\n");
	//safe_cprintf(ent, PRINT_HIGH, "(use fire to change ChaseCam mode, and [ or ] to change ChaseCam target).\n");
}

