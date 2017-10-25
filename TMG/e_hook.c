
#include "g_local.h"
#include "bot.h"
#include "e_hook.h"
#include "g_cmds.h"
#include "timer.h"

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, 
	int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, 
	int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));


void hackLift(edict_t *player) 
{

	// A bug was introduced into Q2's physics with version 3.17 where 
	// a player given an upward velocity will stay stuck to the ground
	// if the velocity isn't large enough.  This workaround will lift
	// the player a small amount off the ground so that the sticking
	// doesn't occur.

	vec3_t traceTo;
	trace_t trace;

	// if there is an upward component to the player's velocity
	// (don't do this if the game is paused, it will move players out of the world!)
	if (player->velocity[2] > 0 ) 
	{

		// find the point immediately above the player's origin
		VectorCopy(player->s.origin, traceTo);
		traceTo[2] += 1;

		// trace to it
		trace = gi.trace(traceTo, player->mins, player->maxs, traceTo, player, MASK_PLAYERSOLID);

		// if there isn't a solid immediately above the player
		if (!trace.startsolid) 
		{
			player->s.origin[2] += 1;	// make sure player off ground
		}
	}
}

// Ended_Grappling: Returns true if the client just stopped grappling.
qboolean Ended_Grappling (gclient_t *client)
{
	return (!(client->buttons & BUTTON_USE) && client->oldbuttons & BUTTON_USE);
}

// Is_Grappling: Returns true if the client is grappling at the moment.
qboolean Is_Grappling (gclient_t *client)
{
	return (client->hook == NULL) ? false : true;
}

// Function name	: hook_laser_think
// Description	    : move the two ends of the laser beam to the proper positions
// Return type		: void 
// Argument         : edict_t *self
void hook_laser_think(edict_t *self)
{
	vec3_t	forward, right, offset, start;

	assert(self->owner);
	assert(self->owner->owner);
	assert(self->owner->owner->client);

	//QW// fix hook bugs, decide when to disconnect hook
	if (!self->owner || //QW// not sure when these can happen
		!self->owner->owner || //QW// but leaving them in for now.
		!self->owner->owner->client ||
		self->owner->deadflag ||	// if player died
		!self->owner->inuse ||	// or disconnected unexpectedly
		(self->owner->s.event == EV_PLAYER_TELEPORT) )	// if player goes through teleport
	{
		G_FreeEdict(self);
		return;	
	}

	// put start position into start
	AngleVectors (self->owner->owner->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, self->owner->owner->viewheight-8);
	//	if(hook_offhand->value)
	P_ProjectSource_Reverse (self->owner->owner->client, 
		self->owner->owner->s.origin, offset, forward, right, start);
	//	else
	//	P_ProjectSource (self->owner->owner->client, 
	//		self->owner->owner->s.origin, offset, forward, right, start);

	// move the two ends
	//	gi.unlinkentity(self);
	VectorCopy (start, self->s.origin);
	VectorCopy (self->owner->s.origin, self->s.old_origin);
	gi.linkentity(self);

	// set up to go again
	self->nextthink = level.time + FRAMETIME;
	return;
}



// Function name	: *hook_laser_start
// Description	    : create a laser and return a pointer to it
// Return type		: edict_t 
// Argument         : edict_t *ent
edict_t *abandon_hook_laser_start(edict_t *ent)
{
	edict_t *self;

	//char		*info;

	//	int			randyhook;

	self = G_Spawn();
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	self->s.renderfx |= RF_BEAM|RF_TRANSLUCENT;
	self->s.modelindex = 1;			// must be non-zero
	self->owner = ent;
	self->classname = "lbeam";
	// set the beam diameter
	self->s.frame = 3;

	//info = Info_ValueForKey (ent->owner->client->pers.userinfo, "hook_color");
	//set the color
	//gi.dprintf("Hook color selected it \"%s\".\n",info);

	//	randyhook = random() * 4294967295;

	// ADJ CTF team colours on the hook
	if (ctf->value)
	{
		switch (ent->owner->client->resp.ctf_team)
		{
		case CTF_TEAM1:
			self->s.skinnum = 0xf2f2f0f0; // red
			break;
		case CTF_TEAM2:
			self->s.skinnum = 0xf3f3f1f1; // blue
			break;
		}
	}
	else
	{
		if (hook_color->value == 1)
			self->s.skinnum = 0xf2f2f0f0;	// red
		else if (hook_color->value == 2)
			self->s.skinnum = 0xf3f3f1f1;	// sorta blue			
		else if (hook_color->value == 3)
			self->s.skinnum = 0xd0d1d2d3;	// green 
		else if (hook_color->value == 4)
			self->s.skinnum = 0xdcdddedf;	// brown
		else if (hook_color->value == 5)
			self->s.skinnum = 0xe0e1e2e3;	//yellow strobe
		else if (hook_color->value == 6)
			self->s.skinnum = 0xb0b1b2b3;	//purple
		else if (hook_color->value == 7)
			self->s.skinnum = 0xb7b7b7b7;	//purple
		else	
			self->s.skinnum = 0xe0e1e2e3;	//8 JR another purple
	}

	self->think = hook_laser_think;

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	gi.linkentity (self);

	self->spawnflags |= 0x80000001;
	self->svflags &= ~SVF_NOCLIENT;
	hook_laser_think (self);
	return(self);
}

// Function name	: hook_reset
// Description	    : reset the hook.  pull all entities out of the world and reset
//					  the clients weapon state
// Return type		: void 
// Argument         : edict_t *rhook
void abandon_hook_reset(edict_t *rhook)
{	
	int i;
	edict_t	*e;
	vec3_t v,vv;
	// start with NULL pointer checks
	if (!rhook) return;

	if (rhook->owner) 
	{ 
		if (rhook->owner->client) 
		{
			// client's hook is no longer out (duh)
			rhook->owner->client->hook_out = false;
			rhook->owner->client->hook_on = false;
			rhook->owner->client->hook = NULL;
			//reset hooktime to 0
			rhook->owner->hooktime = 0;
			// this should always be true and free the laser beam
			if (rhook->laser) 
				G_FreeEdict(rhook->laser);

			rhook->owner->client->buttons |= BUTTON_USE;
			//PON-CTF
			if(chedit->value && rhook->owner == &g_edicts[1] && rhook->owner->client->hook)
			{
				e = (edict_t*)rhook->owner->client->hook;
				VectorCopy(e->s.origin,vv);

				for(i = 1;(CurrentIndex - i) > 0;i++)
				{
					if(Route[CurrentIndex - i].state == GRS_GRAPHOOK) break;
					else if(Route[CurrentIndex - i].state == GRS_GRAPSHOT) break;
				}
				if(Route[CurrentIndex - i].state == GRS_GRAPHOOK)
				{
					Route[CurrentIndex].state = GRS_GRAPRELEASE;
					VectorCopy(rhook->owner->s.origin,Route[CurrentIndex].Pt);
					VectorSubtract(rhook->owner->s.origin,vv,v);
					Route[CurrentIndex].Tcourner[0] = VectorLength(v);
					//gi.bprintf(PRINT_HIGH,"length %f\n",VectorLength(v));
				}
				else if(Route[CurrentIndex - i].state == GRS_GRAPSHOT)
				{
					CurrentIndex = CurrentIndex - i -1;
				}
				//gi.bprintf(PRINT_HIGH,"length %f\n",VectorLength(v));

				if((CurrentIndex - i) > 0)
				{
					if(++CurrentIndex < MAXNODES)
					{
						gi.bprintf(PRINT_HIGH,"Grapple has been released.Last %i pod(s).\n",MAXNODES - CurrentIndex);
						memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
						Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
					}
				}
			} //PON-CTF
		}
	}

	// delete ourself
	G_FreeEdict(rhook);
}



// Function name	: hook_cond_reset
// Description	    : resets the hook if it needs to be
// Return type		: qboolean 
// Argument         : edict_t *self
qboolean hook_cond_reset(edict_t *self) {
	// this should never be true
	if ( !self || !self->inuse || !self->enemy || !self->owner) {
		abandon_hook_reset (self);
		return (true);
	}

	// drop the hook if either party dies/leaves the game/etc.
	if ((!self->enemy->inuse) || (!self->owner->inuse) ||
		(self->enemy->client && self->enemy->health <= 0) || 
		(self->owner->health <= 0))
	{
		abandon_hook_reset (self);
		return (true);
	}

	/*    // drop the hook if player lets go of button
	// and has the hook as current weapon
	if (!((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK)
	&& (strcmp(self->owner->client->pers.weapon->pickup_name, "Hook") == 0))
	{
	abandon_hook_reset (self);
	return (true);
	}
	*/
	return(false);
}


void SV_AddGravity (edict_t *ent);	//JSW

// Function name	: hook_service
// Description	    : Do all the service hook crap (move client, release etc)
// Return type		: void 
// Argument         : edict_t *self
void abandon_hook_service(edict_t *self) {
	vec3_t	hook_dir;


	if(!self)
		return;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	//timer in action ?
	if(hook_maxtime->value && self->owner->hooktime !=0
		&& self->owner->hooktime <= level.time)
	{
			abandon_hook_reset (self);
			return;
	}

	hackLift(self);

	// give the client some velocity ...
	if (self->enemy->client)
		_VectorSubtract(self->enemy->s.origin, self->owner->s.origin, hook_dir);
	else
		_VectorSubtract(self->s.origin, self->owner->s.origin, hook_dir);
	VectorNormalize(hook_dir);

	//JSW slow down flag carriers
	if (self->owner->hasflag)
		VectorScale(hook_dir, hook_carrierspeed->value, self->owner->velocity);
	else //end
		VectorScale(hook_dir, hook_pullspeed->value, self->owner->velocity);

	//JSW - Add gravity
	//self->owner->velocity[2] -= self->owner->gravity * sv_gravity->value * FRAMETIME * 2;

	// avoid "falling" damage JMC
	//VectorCopy(self->owner->velocity, self->owner->client->oldvelocity);
}

// Function name	: hook_track
// Description	    : keeps the invisible hook entity on hook->enemy (can be world or an entity)
// Return type		: void 
// Argument         : edict_t *self
void abandon_hook_track(edict_t *self) 
{
	vec3_t	normal;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) return;

	// bring the pAiN!
	if (self->enemy->client)
	{
		// move the hook along with the player.  It's invisible, but
		// we need this to make the sound come from the right spot
		//			gi.unlinkentity(self);
		VectorCopy(self->enemy->s.origin, self->s.origin);
		//			gi.linkentity(self);

		_VectorSubtract(self->owner->s.origin, self->enemy->s.origin, normal);

		T_Damage (self->enemy, self, self->owner, vec3_origin, 
			self->enemy->s.origin, normal, hook_damage->value, 
			0, DAMAGE_NO_KNOCKBACK, MOD_GRAPPLE);
	} 
	else 
	{
		// If the hook is not attached to the player, constantly copy
		// copy the target's velocity. Velocity copying DOES NOT work properly
		// for a hooked client. 
		VectorCopy(self->enemy->velocity, self->velocity);
	}

	gi.linkentity(self);
	self->nextthink = level.time + 0.1;
}



// Function name	: hook_touch
// Description	    : the hook has hit something.  what could it be? :)
// Return type		: void 
// Argument         : edict_t *self
// Argument         : edict_t *other
// Argument         : cplane_t *plane
// Argument         : csurface_t *surf
void hook_touch(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	dir, normal;
	short	i;

	self->owner->client->resp.hooks_deployed_count++;

	if ((surf && (surf->flags & SURF_SKY)) && (!hook_sky->value))
	{
		abandon_hook_reset(self);
		return;
	}

	// ignore hitting the person who launched us
	if (other == self->owner)
		return;

	// ignore hitting items/projectiles/etc.
	if (other->solid == SOLID_NOT || other->solid == SOLID_TRIGGER || other->movetype == MOVETYPE_FLYMISSILE)
		return;

	if (other->client) 
	{	// we hit a player
		// ignore hitting a teammate
		if (OnSameTeam(other, self->owner) || hook_reset->value)
		{
			abandon_hook_reset(self);
			return;
		}
		// we hit an enemy, so do a bit of damage
		_VectorSubtract(other->s.origin, self->owner->s.origin, dir);
		_VectorSubtract(self->owner->s.origin, other->s.origin, normal);
		T_Damage(other, self, self->owner, dir, self->s.origin, normal, hook_damage->value, hook_damage->value, 0, MOD_GRAPPLE);
	} 
	else 
	{	// we hit something thats not a player
		// if we can hurt it, then do a bit of damage
		if (other->takedamage) 
		{
			_VectorSubtract(other->s.origin, self->owner->s.origin, dir);
			_VectorSubtract(self->owner->s.origin, other->s.origin, normal);
			T_Damage(other, self, self->owner, dir, self->s.origin, normal, 1, 1, 0, MOD_HIT);
		}

		self->owner->client->resp.hooks_landed_count++;

		// stop moving
		VectorClear(self->velocity);

		// gi.sound() doesnt work because the origin of an entity with no model is not 
		// transmitted to clients or something.  hoped this would be fixed in Q2 ...
		gi.positioned_sound(self->s.origin, self, CHAN_WEAPON, gi.soundindex("flyer/Flyatck2.wav"), 1, ATTN_NORM, 0);
	}

	// handled in hook_cond_reset()
	// check to see if we already let up on the fire button
	//	if ( !((self->owner->client->latched_buttons|self->owner->client->buttons) & BUTTON_ATTACK) ) {
	//		abandon_hook_reset(self);
	//		return;
	//	}

	// remember who/what we hit
	// this must be set before hook_cond_reset() is called
	self->enemy = other;

	// if hook should be dropped, just return
	if (hook_cond_reset(self)) 
		return;

	// pull us off the ground (figuratively)
	//	self->owner->groundentity = NULL;

	// we are now anchored
	self->owner->client->hook_on = true;
	if(self->owner->bot_client)
		self->owner->hooktime = level.time + 2;

	// keep up with that thing
	self->think = abandon_hook_track;
	self->nextthink = level.time + 0.1;

	self->solid = SOLID_NOT;

	//lets make some sparks !!
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	gi.WriteDir (vec3_origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);


	//PON-CTF
	if(chedit->value && self->owner == &g_edicts[1])
	{
		i = CurrentIndex;
		while(--i > 0)
		{
			if(Route[i].state == GRS_GRAPSHOT)
			{
				VectorCopy(self->s.origin,Route[i].Tcourner);
				break;
			}
		}
		Route[CurrentIndex].state = GRS_GRAPHOOK;
		VectorCopy(self->owner->s.origin,Route[CurrentIndex].Pt);

		if(++CurrentIndex < MAXNODES)
		{
			gi.bprintf(PRINT_HIGH,"Grapple has been hook.Last %i pod(s).\n",MAXNODES - CurrentIndex);
			memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
		}
	}
	//PON-CTF
}

// Function name	: fire_hook
// Description	    : creates the invisible hook entity and sends it on its way attaches a laser to it
// Return type		: void 
// Argument         : edict_t *owner
// Argument         : vec3_t start
// Argument         : vec3_t forward
void abandon_fire_hook(edict_t *owner, vec3_t start, vec3_t forward) 
{
	edict_t	*hook;
	trace_t tr;

	if(match_state != STATE_PLAYING || !owner->client || !owner )
		return;

	hook = G_Spawn();

	if(!hook)
		return;
	hook->movetype = MOVETYPE_FLYMISSILE;
	hook->solid = SOLID_BBOX;
	hook->clipmask = MASK_SHOT;
	hook->owner = owner;			// this hook belongs to me
	owner->client->hook = hook;		// this is my hook
	hook->classname = "hook";		// this is a hook

	vectoangles (forward, hook->s.angles);
	VectorScale(forward, hook_speed->value, hook->velocity);

	hook->touch = hook_touch;

	hook->think = abandon_hook_reset;
	hook->nextthink = level.time + 5;
	hook->noblock = true;

	//QW// Deleted, not needed.
	// Causes GAME WARNING: SV_FindIndex: NULL or empty name, ignored
	// Note to modders: Empty model names are never welcome.
	// If you don't want a model, don't set a model.
	//gi.setmodel(hook, "");

	VectorCopy(start, hook->s.origin);
	VectorCopy(hook->s.origin, hook->s.old_origin);

	VectorClear(hook->mins);
	VectorClear(hook->maxs);

	// start up the laser
	hook->laser = abandon_hook_laser_start(hook);

	// put it in the world
	gi.linkentity(hook);

	//start the hook reset timer 
	owner->hooktime = level.time +(int)hook_maxtime->value;

	// from id's code.  I don't question these things...		
	tr = gi.trace (owner->s.origin, NULL, NULL, hook->s.origin, hook, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (hook->s.origin, -10, forward, hook->s.origin);
		hook->touch (hook, tr.ent, NULL, NULL);
	}
}



// Function name	: hook_fire
// Description	    : a call has been made to fire the hook
// Return type		: void 
// Argument         : edict_t *ent
void abandon_hook_fire(edict_t *ent) 
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	// due to the way Weapon_Generic was written up, if weaponstate
	// is not WEAPON_FIRING, then we can switch away
	// since we don't want to be in any other real state,
	// we just set it to some 'invalid' state and everything works
	// fine :)
	/*	if (ent->client->pers.weapon &&
	strcmp(ent->client->pers.weapon->pickup_name, "Hook") == 0)
	ent->client->weaponstate = -1;	// allow weapon change
	*/
	if (!use_hook->value)
	{
		if (!ent->client->pers.hookmsg)
		{
			safe_centerprintf(ent, "Offhand hook not available\n");
			ent->client->pers.hookmsg = 1;
		}
		return;
	}

	if (!ent || !ent->client || ent->deadflag == DEAD_DEAD || ent->client->pers.pl_state != PL_PLAYING)
		return;

	if(ent->client->resp.hook_wait > level.time)
		return;

	if (ent->client->ps.pmove.pm_type == PM_SPECTATOR)
		return;

	if (ent->client->hook_out)		// reject subsequent calls from Weapon_Generic
		return;

	ent->client->hook_out = true;

	// calculate start position and forward direction
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight-8);
	//	if(hook_offhand->value)
	P_ProjectSource_Reverse (ent->client, ent->s.origin, offset, forward, right, start);
	//	else
	//	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	// kick back??
	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	hackLift(ent);

	// actually launch the hook off
	abandon_fire_hook (ent, start, forward);

	gi.sound(ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck3.wav"), 1, ATTN_NORM, 0);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	//PON-CTF
	if(chedit->value && ent == &g_edicts[1])
	{
		Route[CurrentIndex].state = GRS_GRAPSHOT;
		VectorCopy(ent->s.origin,Route[CurrentIndex].Pt);

		if(++CurrentIndex < MAXNODES)
		{
			gi.bprintf(PRINT_HIGH,"Hook has been fired.Last %i pod(s).\n",MAXNODES - CurrentIndex);
			memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
		}
	}
	//PON-CTF
}

// Function name	: Weapon_Hook
// Description	    : boring service routine
// Return type		: void 
// Argument         : edict_t *ent
void abandon_Weapon_Hook(edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, abandon_hook_fire);
}
