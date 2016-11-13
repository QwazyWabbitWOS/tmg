//______________-Anti camping  mod -__________________//

#include "g_local.h"
#include "timer.h"

void CheckForCamping (edict_t *ent)
{
	vec3_t change;
	float dist;
	int timer;
	int move;
	
	if (match_state != STATE_PLAYING)
		return;
	
	// don't check observers/specs or bots.
	if ((ent->client->resp.spectator)
		|| (ent->deadflag == DEAD_DEAD)
		|| (ent->client->pers.pl_state != PL_PLAYING)
		|| (ent->bot_client))
		return;

	VectorSubtract (ent->s.origin, ent->client->camp_pos, change);
	dist = VectorLength (change);
	timer = level.time - ent->client->camp_time;
	move = ent->client->camp_move_time - level.time;

	//send the message out !
	if ((timer > CAMP_WARN_TIME) && ent->client->camp_warned)
	{ 
		safe_centerprintf (ent, "You will be forced to Spectator in %d seconds!\n", move);
	}
	
	if ((ent->client->camp_time == 0) || (dist > CAMP_DISTANCE))
	{
		// reset camping data
		VectorCopy (ent->s.origin, ent->client->camp_pos);
		ent->client->camp_time = level.time;
		ent->client->camp_warned = false;
		ent->client->camp_move_time = 0;
		//ent->client->check_camping = false;
	}
	else if ((timer > CAMP_WARN_TIME) && !ent->client->camp_warned)
	{
		safe_centerprintf (ent, "You will be forced to Spectator in %d seconds!\n", timer);
		ent->client->camp_warned = true;
		ent->client->camp_move_time = level.time +11;
	}
	else if (timer > CAMP_TIME)
	{
		safe_centerprintf (ent, CAMP_MSG);
		ent->client->camp_time = 0; // reset when player returns
		StuffCmd (ent, va("spec\n"));
	}
}

