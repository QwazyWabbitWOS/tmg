
#include "g_local.h"
#include "g_items.h"
#include "m_player.h"
#include "timer.h"
#include "p_hud.h"
#include "bot.h"
#include "runes.h"
#include "performance.h"

edict_t		*current_player;
gclient_t	*current_client;

static	vec3_t	forward, right, up;
float	xyspeed;

float	bobmove;
int		bobcycle;		// odd cycles are right foot going forward
float	bobfracsin;		// sin(bobfrac*M_PI)

/*
===============
SV_CalcRoll

===============
*/
static float
SV_CalcRoll (vec3_t angles, vec3_t velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = DotProduct (velocity, right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = sv_rollangle->value;

	if (side < sv_rollspeed->value)
		side = side * value / sv_rollspeed->value;
	else
		side = value;
	
	return side*sign;
	
}


/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
static void
P_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	float	side;
	float	realcount, count, kick;
	vec3_t	v = { 0 };
	int		r, l;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};
//RAV runes
  static	vec3_t	rcolor = {1.0, 0.0, 1.0};
//// Make sure player exists!
  if (!G_EntExists(player)) return;

  
  client = player->client;

	// flash the backgrounds behind the status numbers
	client->ps.stats[STAT_FLASHES] = 0;
	if (client->damage_blood)
		client->ps.stats[STAT_FLASHES] |= 1;
	if (client->damage_armor && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
		client->ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (client->damage_blood + client->damage_armor + client->damage_parmor+ client->damage_rune);//RAV
	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
	{
		static int		i;

		client->anim_priority = ANIM_PAIN;
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else
		{
			i = (i+1)%3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}

	realcount = count;
	if (count < 10)
		count = 10;	// always make a visible effect

	// play an apropriate pain sound
	if ((level.time > player->pain_debounce_time) && !(player->flags & FL_GODMODE) && (client->invincible_framenum <= level.framenum))
	{
		r = 1 + (rand()&1);
		player->pain_debounce_time = level.time + 0.7;
		if (player->health < 25)
			l = 25;
		else if (player->health < 50)
			l = 50;
		else if (player->health < 75)
			l = 75;
		else
			l = 100;
		gi.sound (player, CHAN_VOICE, gi.soundindex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
	}

	// the total alpha of the blend is always proportional to count
	if (client->damage_alpha < 0)
		client->damage_alpha = 0;
	client->damage_alpha += count*0.01;
	if (client->damage_alpha < 0.2)
		client->damage_alpha = 0.2f;
	if (client->damage_alpha > 0.6)
		client->damage_alpha = 0.6f;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	VectorClear (v);
	if (client->damage_parmor)
		VectorMA (v, (float)client->damage_parmor/realcount, power_color, v);
	if (client->damage_armor)
		VectorMA (v, (float)client->damage_armor/realcount,  acolor, v);
	if (client->damage_blood)
		VectorMA (v, (float)client->damage_blood/realcount,  bcolor, v);
	VectorCopy (v, client->damage_blend);
//RAV
	if (client->damage_rune)
		VectorMA (v, (float)client->damage_rune/realcount,  rcolor, v);
//
	//
	// calculate view angle kicks
	//
	kick = abs(client->damage_knockback);
	if (kick && player->health > 0)	// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / player->health;

		if (kick < count*0.5)
			kick = count*0.5;
		if (kick > 50)
			kick = 50;

		VectorSubtract (client->damage_from, player->s.origin, v);
		VectorNormalize (v);
		
		side = DotProduct (v, right);
		client->v_dmg_roll = kick*side*0.3;
		
		side = -DotProduct (v, forward);
		client->v_dmg_pitch = kick*side*0.3;

		client->v_dmg_time = level.time + DAMAGE_TIME;
	}

	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
	//RAV
    client->damage_rune = 0;
	//
}




/*
===============
SV_CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 = 

  damage = deltavelocity*deltavelocity  * 0.0001

===============
*/
static void
SV_CalcViewOffset (edict_t *ent)
{
	float		*angles;
	float		bob;
	float		ratio;
	float		delta;
	vec3_t		v = { 0 };


  // Make sure ent exists!
  if (!G_EntExists(ent)) return;

  
//===================================

	// base angles
	angles = ent->client->ps.kick_angles;

	// if dead, fix the angle and don't add any kick
	if (ent->deadflag)
	{
		VectorClear (angles);

		ent->client->ps.viewangles[ROLL] = 40;
		ent->client->ps.viewangles[PITCH] = -15;
		ent->client->ps.viewangles[YAW] = ent->client->killer_yaw;
	}
	else
	{
		// add angles based on weapon kick

		VectorCopy (ent->client->kick_angles, angles);

		// add angles based on damage kick

		ratio = (ent->client->v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			ent->client->v_dmg_pitch = 0;
			ent->client->v_dmg_roll = 0;
		}
		angles[PITCH] += ratio * ent->client->v_dmg_pitch;
		angles[ROLL] += ratio * ent->client->v_dmg_roll;

		// add pitch based on fall kick

		ratio = (ent->client->fall_time - level.time) / FALL_TIME;
		if (ratio < 0)
			ratio = 0;
		angles[PITCH] += ratio * ent->client->fall_value;

		// add angles based on velocity

		delta = DotProduct (ent->velocity, forward);
		angles[PITCH] += delta*run_pitch->value;
		
		delta = DotProduct (ent->velocity, right);
		angles[ROLL] += delta*run_roll->value;

		// add angles based on bob

		delta = bobfracsin * bob_pitch->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		angles[PITCH] += delta;
		delta = bobfracsin * bob_roll->value * xyspeed;
		if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		if (bobcycle & 1)
			delta = -delta;
		angles[ROLL] += delta;
	}

//===================================

	// base origin

	VectorClear (v);

	// add view height

	v[2] += ent->viewheight;

	// add fall height

	ratio = (ent->client->fall_time - level.time) / FALL_TIME;
	if (ratio < 0)
		ratio = 0;
	v[2] -= ratio * ent->client->fall_value * 0.4f;

	// add bob height

	bob = bobfracsin * xyspeed * bob_up->value;
	if (bob > 6)
		bob = 6;
	//gi.DebugGraph (bob *2, 255);
	v[2] += bob;

	// add kick offset

	VectorAdd (v, ent->client->kick_origin, v);

	// absolutely bound offsets
	// so the view can never be outside the player box

	if (v[0] < -14)
		v[0] = -14;
	else if (v[0] > 14)
		v[0] = 14;
	if (v[1] < -14)
		v[1] = -14;
	else if (v[1] > 14)
		v[1] = 14;
	if (v[2] < -22)
		v[2] = -22;
	else if (v[2] > 30)
		v[2] = 30;

	VectorCopy (v, ent->client->ps.viewoffset);
}

/*
==============
SV_CalcGunOffset
==============
*/
static void
SV_CalcGunOffset (edict_t *ent)
{
	int		i;
	float	delta;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;


	// gun angles from bobbing
	ent->client->ps.gunangles[ROLL] = xyspeed * bobfracsin * 0.005;
	ent->client->ps.gunangles[YAW] = xyspeed * bobfracsin * 0.01;
	if (bobcycle & 1)
	{
		ent->client->ps.gunangles[ROLL] = -ent->client->ps.gunangles[ROLL];
		ent->client->ps.gunangles[YAW] = -ent->client->ps.gunangles[YAW];
	}

	ent->client->ps.gunangles[PITCH] = xyspeed * bobfracsin * 0.005;

	// gun angles from delta movement
	for (i=0 ; i<3 ; i++)
	{
		delta = ent->client->oldviewangles[i] - ent->client->ps.viewangles[i];
		if (delta > 180)
			delta -= 360;
		if (delta < -180)
			delta += 360;
		if (delta > 45)
			delta = 45;
		if (delta < -45)
			delta = -45;
		if (i == YAW)
			ent->client->ps.gunangles[ROLL] += 0.1*delta;
		ent->client->ps.gunangles[i] += 0.2 * delta;
	}

	// gun height
	VectorClear (ent->client->ps.gunoffset);
	//	ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for (i=0 ; i<3 ; i++)
	{
		ent->client->ps.gunoffset[i] += forward[i]*(gun_y->value);
		ent->client->ps.gunoffset[i] += right[i]*gun_x->value;
		ent->client->ps.gunoffset[i] += up[i]* (-gun_z->value);
	}
}


/*
=============
SV_AddBlend
=============
*/
static void
SV_AddBlend (float r, float g, float b, float a, float *v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}


/*
=============
SV_CalcBlend
=============
*/
static void
	SV_CalcBlend (edict_t *ent)
{
	int		contents;
	vec3_t	vieworg = { 0 };
	int		remaining;
	//RAV
	float	alpha;
	static qboolean is_liquid;
	edict_t	*e;
	int	i;


	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;


	is_liquid = rune_has_rune(ent, RUNE_LIQUID);
	//
	ent->client->ps.blend[0] = ent->client->ps.blend[1] =
		ent->client->ps.blend[2] = ent->client->ps.blend[3] = 0;

	// add for contents
	VectorAdd (ent->s.origin, ent->client->ps.viewoffset, vieworg);
	contents = gi.pointcontents (vieworg);
	if (contents & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER) )
		ent->client->ps.rdflags |= RDF_UNDERWATER;
	else
		ent->client->ps.rdflags &= ~RDF_UNDERWATER;

	//RAV
	if (contents & (CONTENTS_LAVA))
	{
		if (is_liquid)
			alpha = 0.1f;
		else
			alpha = 0.6f;

		SV_AddBlend (1.0f, 0.3f, 0.0f, alpha, ent->client->ps.blend);
	}
	else if (contents & CONTENTS_SLIME)
	{
		if (is_liquid)
			alpha = 0.1f;
		else
			alpha = 0.6f;

		SV_AddBlend (0.0f, 0.1f, 0.05f, alpha, ent->client->ps.blend);
	}
	else if (contents & CONTENTS_WATER)
	{
		if (is_liquid)
			alpha = 0.1f;
		else
			alpha = 0.5f;

		SV_AddBlend (0.5f, 0.3f, 0.2f, alpha, ent->client->ps.blend);
	}

	else if (contents & (CONTENTS_SOLID))
	{
		SV_AddBlend (0.3f, 0.3f, 0.3f, 0.3f, ent->client->ps.blend);
	}
	//
	/*
	if (contents & (CONTENTS_SOLID|CONTENTS_LAVA))
	SV_AddBlend (1.0, 0.3, 0.0, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_SLIME)
	SV_AddBlend (0.0, 0.1, 0.05, 0.6, ent->client->ps.blend);
	else if (contents & CONTENTS_WATER)
	SV_AddBlend (0.5, 0.3, 0.2, 0.4, ent->client->ps.blend);
	*/
	// add for powerups
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM,
			gi.soundindex("items/damage2.wav"), 1, ATTN_NORM, 0);
		if (remaining == 10 && ((int)quad_notify->value & QUAD_NOTIFY_EXPIRE))
		{
			//gi.dprintf("A quad damage has expired!\n");
			for (i = 1; i <= maxclients->value; i++)
			{
				e = &g_edicts[i];
				if (e && e->inuse && !e->bot_client)
					//safe_centerprintf(e, "A quad damage has expired!\n");
					gi.sound (e, CHAN_AUTO,
					gi.soundindex("items/quadexp.wav"), 1, ATTN_NONE, 0);
			}
		}
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 0, 1, 0.08f, ent->client->ps.blend);
	}
	else if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM,
			gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08f, ent->client->ps.blend);
	}
	//RAV
	else if (ent->client->respawn_framenum > level.framenum)
	{
		remaining = ent->client->respawn_framenum - level.framenum;
		//		if (remaining <= 30)	// beginning to fade
		//			gi.sound(ent, CHAN_ITEM,
		//					 gi.soundindex("items/protect2.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (1, 1, 0, 0.08f, ent->client->ps.blend);
	}
	//
	else if (ent->client->enviro_framenum > level.framenum)
	{
		remaining = ent->client->enviro_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM,
			gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0, 1, 0, 0.08f, ent->client->ps.blend);
	}
	else if (ent->client->breather_framenum > level.framenum)
	{
		remaining = ent->client->breather_framenum - level.framenum;
		if (remaining == 30)	// beginning to fade
			gi.sound(ent, CHAN_ITEM,
			gi.soundindex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			SV_AddBlend (0.4f, 1, 0.4f, 0.04f, ent->client->ps.blend);
	}

	// add for damage
	if (ent->client->damage_alpha > 0)
		SV_AddBlend (ent->client->damage_blend[0],
		ent->client->damage_blend[1],
		ent->client->damage_blend[2],
		ent->client->damage_alpha,
		ent->client->ps.blend);

	if (ent->client->bonus_alpha > 0)
		SV_AddBlend (0.85f, 0.7f, 0.3f,
		ent->client->bonus_alpha,
		ent->client->ps.blend);

	// drop the damage value
	ent->client->damage_alpha -= 0.06f;
	if (ent->client->damage_alpha < 0)
		ent->client->damage_alpha = 0;

	// drop the bonus value
	ent->client->bonus_alpha -= 0.1f;
	if (ent->client->bonus_alpha < 0)
		ent->client->bonus_alpha = 0;
}


/*
=================
P_FallingDamage
=================
*/
static void
P_FallingDamage (edict_t *ent)
{
	float	delta;
	int		damage;
	vec3_t	dir = { 0 };
	static qboolean is_jump;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;


	is_jump = rune_has_rune(ent, RUNE_JUMP);

	if (is_jump)
		return;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	if (ent->movetype == MOVETYPE_NOCLIP)
		return;

	if ((ent->client->oldvelocity[2] < 0) &&
		(ent->velocity[2] > ent->client->oldvelocity[2]) &&
		(!ent->groundentity))
	{
		delta = ent->client->oldvelocity[2];
	}
	else
	{
		if (!ent->groundentity)
			return;
		delta = ent->velocity[2] - ent->client->oldvelocity[2];
	}
	delta = delta*delta * 0.0001;

	//ZOID
	// never take damage if just release grapple or on grapple
	if (level.time - ent->client->ctf_grapplereleasetime <= FRAMETIME * 2 ||
		(ent->client->ctf_grapple &&
		 ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY))
		return;
	//ZOID

	//RAV  hook
	if (ent->client->hook_on && ent->client->hook)
		return;
	//


	// never take falling damage if completely underwater
	if (ent->waterlevel == 3)
		return;
	if (ent->waterlevel == 2)
		delta *= 0.25;
	if (ent->waterlevel == 1)
		delta *= 0.5;

	if (delta < 1)
		return;

	if (delta < 15)
	{
		ent->s.event = EV_FOOTSTEP;
		return;
	}

	ent->client->fall_value = delta*0.5;
	if (ent->client->fall_value > 40)
		ent->client->fall_value = 40;
	ent->client->fall_time = level.time + FALL_TIME;

	if (delta > 30)
	{
		if (ent->health > 0)
		{
			if (delta >= 55)
				ent->s.event = EV_FALLFAR;
			else
				ent->s.event = EV_FALL;
		}
		ent->pain_debounce_time = level.time;	// no normal pain sound
		damage = (delta-30)/2;
		if (damage < 1)
			damage = 1;
		VectorSet (dir, 0, 0, 1);

		if (!deathmatch->value || !(dmflag & DF_NO_FALLING) )
			T_Damage (ent, world, world, dir, ent->s.origin,
					  vec3_origin, damage, 0, 0, MOD_FALLING);
	}
	else
	{
		ent->s.event = EV_FALLSHORT;
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);	//ponko
		return;
	}
}



/*
=============
P_WorldEffects
=============
*/
static void
P_WorldEffects (void)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, old_waterlevel;
	//RAV
	static gitem_t *flag1;
	static gitem_t *flag2;
	static qboolean has_flag;
	static qboolean is_liquid;
	is_liquid = rune_has_rune(current_player, RUNE_LIQUID);

	flag1 = FindItemByClassname("item_flag_team1");
	flag2 = FindItemByClassname("item_flag_team2");

	has_flag = ((current_player->client->pers.inventory[ITEM_INDEX(flag1)])
				||(current_player->client->pers.inventory[ITEM_INDEX(flag2)]));

	//
	if ((current_player->movetype == MOVETYPE_NOCLIP) ||
		(current_player->health <= 0))
	{
		current_player->air_finished = level.time + 12;	// don't need air
		return;
	}
	//RAV
	if((is_liquid)&&(!has_flag))

		return;
	//
	waterlevel = current_player->waterlevel;
	old_waterlevel = current_client->old_waterlevel;
	current_client->old_waterlevel = waterlevel;

	breather = current_client->breather_framenum > level.framenum;
	envirosuit = current_client->enviro_framenum > level.framenum;

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		if (current_player->watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY,
					  gi.soundindex("player/lava_in.wav"), 1, ATTN_NORM, 0);

		else if (current_player->watertype & CONTENTS_SLIME)
			gi.sound (current_player, CHAN_BODY,
					  gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);

		else if (current_player->watertype & CONTENTS_WATER)
			gi.sound (current_player, CHAN_BODY,
					  gi.soundindex("player/watr_in.wav"), 1, ATTN_NORM, 0);

		current_player->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		current_player->damage_debounce_time = level.time - 1;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && ! waterlevel)
	{
		PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		gi.sound (current_player, CHAN_BODY,
				  gi.soundindex("player/watr_out.wav"), 1, ATTN_NORM, 0);

		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		gi.sound (current_player, CHAN_BODY,
				  gi.soundindex("player/watr_un.wav"), 1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			gi.sound (current_player, CHAN_VOICE,
					  gi.soundindex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			PlayerNoise(current_player, current_player->s.origin, PNOISE_SELF);
		}
		else  if (current_player->air_finished < level.time + 11)
		{	// just break surface
			gi.sound (current_player, CHAN_VOICE,
					  gi.soundindex("player/gasp2.wav"), 1, ATTN_NORM, 0);
		}
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10;

			if (((int)(current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				if (!current_client->breather_sound)
					gi.sound (current_player, CHAN_AUTO,
							  gi.soundindex("player/u_breath1.wav"),
							  1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_AUTO,
							  gi.soundindex("player/u_breath2.wav"),
							  1, ATTN_NORM, 0);
				current_client->breather_sound ^= 1;
				PlayerNoise(current_player, current_player->s.origin,
							PNOISE_SELF);
				//FIXME: release a bubble?
			}
		}

		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if (current_player->client->next_drown_time < level.time
				&& current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("player/drown1.wav"),
							  1, ATTN_NORM, 0);
				else if (rand() & 1)
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("*gurp1.wav"),
							  1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("*gurp2.wav"),
							  1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, world, world, vec3_origin,
						  current_player->s.origin, vec3_origin,
						  current_player->dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12;
		current_player->dmg = 2;
	}

	//
	// check for sizzle damage
	//
	if (waterlevel &&
		(current_player->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum
				//RAV
				&& current_client->respawn_framenum < level.framenum
				//
				)
			{
				if (rand()&1)
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("player/burn1.wav"),
							  1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("player/burn2.wav"),
							  1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time + 1;
			}

			if (envirosuit)	// take 1/3 damage with envirosuit
				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, 1 * waterlevel, 0, 0, MOD_LAVA);
			else
				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, 3 * waterlevel, 0, 0, MOD_LAVA);
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, 1 * waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}


/*
===============
G_SetClientEffects
===============
*/
static void
G_SetClientEffects (edict_t *ent)
{
	int		pa_type;
	int		remaining;

  // Make sure ent exists!
  if (!G_EntExists(ent)) return;


	ent->s.effects = 0;
	ent->s.renderfx = 0;
	
/*	//JSW
	if (ent->client->resp.bonus)
	{
		ent->s.renderfx |= RF_TRANSLUCENT;
		ent->s.modelindex2 = 0;
	}
	else
		ent->s.modelindex2 = 255;
	//end
*/
	if (ent->health <= 0 || level.intermissiontime)
		return;

	if (ent->powerarmor_time > level.time)
	{
		pa_type = PowerArmorType (ent);
		if (pa_type == POWER_ARMOR_SCREEN)
		{
			ent->s.effects |= EF_POWERSCREEN;
		}
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			ent->s.effects |= EF_COLOR_SHELL;
			ent->s.renderfx |= RF_SHELL_GREEN;
		}
	}

//ZOID
	if (ctf->value)
		CTFEffects(ent);
//ZOID
//RAV
	if (ent->client->rune_time > level.time)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= RF_SHELL_GREEN;
	}
//
	if (ent->client->quad_framenum > level.framenum)
	{
		remaining = ent->client->quad_framenum - level.framenum;
		if (remaining > 40 || (remaining & 1) )
		{
			if(ent->client->resp.ctf_team == CTF_TEAM1)
			{
				ent->s.effects |= EF_COLOR_SHELL;
				ent->s.renderfx |= (RF_SHELL_RED);
			}
			else
			{
				ent->s.effects |= EF_QUAD;
			}
		}
	}
	if (ent->client->invincible_framenum > level.framenum)
	{
		remaining = ent->client->invincible_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_PENT;
	}
	//RAV
	//respawning gives a yellow shell.
	if (ent->client->respawn_framenum > level.framenum)
	{
		remaining = ent->client->respawn_framenum - level.framenum;
		if (remaining > 30 || (remaining & 4) )
			ent->s.effects |= EF_YELLOWSHELL;
	}
//
//flies.
	if (ent->health < 25 && !ent->waterlevel)
	{
		ent->s.effects |= EF_FLIES;
		ent->s.sound = gi.soundindex ("infantry/inflies1.wav");
	}
	else
	{
		ent->s.effects &= ~EF_FLIES;
		ent->s.sound = 0;
	}
		//flies end

	// show cheaters!!!
	if (ent->flags & FL_GODMODE || ent->client->pers.pl_state == PL_CHEATBOT)
	{
		ent->s.effects |= EF_COLOR_SHELL;
		ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}
}


/*
===============
G_SetClientEvent
===============
*/
static void
G_SetClientEvent (edict_t *ent)
{
	if (ent->s.event)
		return;

	if ( ent->groundentity && xyspeed > 225)
	{
		//RAV
		if(rune_has_rune(ent, RUNE_INVIS))
			return;
		//

		if ( (int)(current_client->bobtime+bobmove) != bobcycle )
			ent->s.event = EV_FOOTSTEP;
	}
}

/*
===============
G_SetClientSound
===============
*/
static void
G_SetClientSound (edict_t *ent)
{
	char	*weap;

  // Make sure ent exists!
  if (!G_EntExists(ent)) return;


	if (ent->client->resp.game_helpchanged != game.helpchanged)
	{
		ent->client->resp.game_helpchanged = game.helpchanged;
		ent->client->resp.helpchanged = 1;
	}

	// help beep (no more than three times)
	if (ent->client->resp.helpchanged &&
		ent->client->resp.helpchanged <= 3 &&
		!(level.framenum & 63) )
	{
		ent->client->resp.helpchanged++;
		gi.sound (ent, CHAN_VOICE,
				  gi.soundindex ("misc/pc_up.wav"),
				  1, ATTN_STATIC, 0);
	}


	if (ent->client->pers.weapon)
		weap = ent->client->pers.weapon->classname;
	else
		weap = "";

	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) )
		ent->s.sound = snd_fry;

	//JSW - Allow server to turn off all players railgun hum
	else if (strcmp(weap, "weapon_railgun") == 0 && !no_hum->value)
	{
		//JSW - Allow player to turn off their own railgun hum
		if (!ent->client->resp.no_hum)
			ent->s.sound = gi.soundindex("weapons/rg_hum.wav");
	}

	else if (strcmp(weap, "weapon_bfg") == 0)
		ent->s.sound = gi.soundindex("weapons/bfg_hum.wav");
	else if (ent->client->weapon_sound)
		ent->s.sound = ent->client->weapon_sound;
	else
		ent->s.sound = 0;
}

/*
===============
G_SetClientFrame
===============
*/
static void
G_SetClientFrame (edict_t *ent)
{
	gclient_t	*client;
	qboolean	duck, run;

  // Make sure ent exists!
  if (!G_EntExists(ent)) return;

	if (ent->s.modelindex != 255)
		return;		// not in the player model

	client = ent->client;

	if (client->ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if (xyspeed)
		run = true;
	else
		run = false;

	if (!((duck != client->anim_duck && client->anim_priority < ANIM_DEATH)
		|| (run != client->anim_run && client->anim_priority == ANIM_BASIC)
		|| (!ent->groundentity && client->anim_priority <= ANIM_WAVE)))
	{
		if (ent->s.frame < client->anim_end)
		{	// continue an animation
			ent->s.frame++;
			return;
		}

		if (client->anim_priority == ANIM_DEATH)
			return;		// stay there
		if (client->anim_priority == ANIM_JUMP)
		{
			if (!ent->groundentity)
				return;		// stay there
			ent->client->anim_priority = ANIM_WAVE;
			ent->s.frame = FRAME_jump3;
			ent->client->anim_end = FRAME_jump6;
			return;
		}
	}

	// return to either a running or standing frame
	client->anim_priority = ANIM_BASIC;
	client->anim_duck = duck;
	client->anim_run = run;

	if (!ent->groundentity)
	{
//ZOID: if on grapple, don't go into jump frame, go into standing
//frame
		if (client->ctf_grapple) {
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		} else {
//ZOID
			client->anim_priority = ANIM_JUMP;
			if (ent->s.frame != FRAME_jump2)
				ent->s.frame = FRAME_jump1;
			client->anim_end = FRAME_jump2;
		}
	}
	else if (run)
	{	// running
		if (duck)
		{
			ent->s.frame = FRAME_crwalk1;
			client->anim_end = FRAME_crwalk6;
		}
		else
		{
			ent->s.frame = FRAME_run1;
			client->anim_end = FRAME_run6;
		}
	}
	else
	{	// standing
		if (duck)
		{
			ent->s.frame = FRAME_crstnd01;
			client->anim_end = FRAME_crstnd19;
		}
		else
		{
			ent->s.frame = FRAME_stand01;
			client->anim_end = FRAME_stand40;
		}
	}
}


/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void ClientEndServerFrame (edict_t *ent)
{
	float	bobtime;
	int		i;
	edict_t *e;

	// Safety check...
	if (!G_EntExists(ent))
		return;

	if (!ent || !ent->inuse)
		return;

	current_player = ent;
	current_client = ent->client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	//
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
	for (i=0 ; i<3 ; i++)
	{
		current_client->ps.pmove.origin[i] = ent->s.origin[i] * 8.0;
		current_client->ps.pmove.velocity[i] = ent->velocity[i] * 8.0;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{

		// FIXME: add view drifting here?
		current_client->ps.blend[3] = 0;
		current_client->ps.fov = 90;
		G_SetStats (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	P_WorldEffects ();

	//
	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	//
	if (ent->client->v_angle[PITCH] > 180)
		ent->s.angles[PITCH] = (-360 + ent->client->v_angle[PITCH])/3;
	else
		ent->s.angles[PITCH] = ent->client->v_angle[PITCH]/3;

	ent->s.angles[YAW] = ent->client->v_angle[YAW];
	ent->s.angles[ROLL] = 0;
	ent->s.angles[ROLL] = SV_CalcRoll (ent->s.angles, ent->velocity)*4;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
	xyspeed = sqrtf(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			bobmove = 0.25;
		else if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}

	bobtime = (current_client->bobtime += bobmove);

	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	//RAV
	if (rune_has_rune(ent, RUNE_SPEED))
		bobtime *= 2;
	//

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// determine the view offsets
	SV_CalcViewOffset (ent);

	// determine the gun offsets
	SV_CalcGunOffset (ent);

	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	SV_CalcBlend (ent);

	//ZOID
	//	if (!ent->client->chase_target)
	//ZOID

	G_SetStats (ent);

	//ZOID
	//update chasecam follower stats
	//	for (i = 1; i <= maxclients->value; i++)
	//	{
	//		edict_t *e = g_edicts + i;
	//
	//		// ERASER, use player list
	//		for (i = 0; i < num_players; i++)
	//		{
	//			e = players[i];
	//		}
	//		//ERASER
	//	}

	for (i = 1; i <= maxclients->value; i++)
	{
		e = g_edicts + i;
		if (e && e->inuse && !e->bot_client)
		{
			if (!ent->inuse || e->client->chase_target != ent)
				continue;

			memcpy(e->client->ps.stats,
				ent->client->ps.stats,
				sizeof(ent->client->ps.stats));

			e->client->ps.stats[STAT_LAYOUTS] = 1;
			break;
		}
	}

	G_SetClientEvent (ent);
	G_SetClientEffects (ent);
	G_SetClientSound (ent);
	G_SetClientFrame (ent);

	VectorCopy (ent->velocity, ent->client->oldvelocity);
	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

	// if the scoreboard is up, update it
	if(ent && ent->inuse && !ent->bot_client)
	{
		if ((ent->client->showscores && !(level.framenum & 31))
			|| ((ent->client->pers.db_hud)
			&& (level.framenum >= ent->client->hudtime)))
		{
			// delay before showing HUD (3 seconds)
			//if (ent->client->resp.enterframe + 10 > level.framenum)
			//{
			//	ent->client->pers.motd_seen = false;
			//	return;
			//}

			// delay 3 more seconds, then put client in server after motd shown
			// this fixes TastySpleen WallFly blockout.
			//if (ent->client->resp.enterframe + 60 > level.framenum && !ent->client->pers.motd_seen)
			//{
			//	if (DEBUG_HUD) 
			//		DbgPrintf("%s in HUD display delay, framenum: %d\n", __func__, level.framenum);
			//}
			//else 

			if (!ent->client->pers.motd_seen)
			{
				ent->client->pers.motd_seen = true;
				Spectate(ent, NULL);
			}

			//ZOID
			if (ent->client->menu)
			{
				if(ent->client->resp.menu_time > level.framenum)
				{
					return;
				}
				else
				{
					PMenu_Update(ent);
					ent->client->resp.menu_time = level.framenum + 2;
				}
			}
			else
				DeathmatchScoreboardMessage (ent, ent->enemy);

			if(ent->client->chase_target)
				ent->client->hudtime = level.framenum + 5;	// HUD update interval (chasing)
			else
				ent->client->hudtime = level.framenum + 2;	// HUD update interval players

			gi.unicast (ent, false);
		}
	}
}

//						BOT AREA

/*
=============
B_WorldEffects
=============
*/
static void
B_WorldEffects (edict_t *ent)
{
	qboolean	breather;
	qboolean	envirosuit;
	int			waterlevel, old_waterlevel;
	gclient_t	*client;

	client = ent->client;

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->air_finished = level.time + 12;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel;
	old_waterlevel = client->old_waterlevel;
	client->old_waterlevel = waterlevel;

	breather = client->breather_framenum > level.framenum;
	envirosuit = client->enviro_framenum > level.framenum;

	//
	// if just entered a water volume, play a sound
	//
	if (!old_waterlevel && waterlevel)
	{
		PlayerNoise(current_player,
					current_player->s.origin, PNOISE_SELF);
		if (ent->watertype & CONTENTS_LAVA)
			gi.sound (current_player, CHAN_BODY,
					  gi.soundindex("player/lava_in.wav"),
					  1, ATTN_NORM, 0);

		else if (ent->watertype & CONTENTS_SLIME)
			gi.sound (ent, CHAN_BODY,
					  gi.soundindex("player/watr_in.wav"),
					  1, ATTN_NORM, 0);

		else if (ent->watertype & CONTENTS_WATER)
			gi.sound (current_player, CHAN_BODY,
					  gi.soundindex("player/watr_in.wav"),
					  1, ATTN_NORM, 0);

		ent->flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		ent->damage_debounce_time = level.time - 1;
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (old_waterlevel && ! waterlevel)
	{
		PlayerNoise(current_player,
					current_player->s.origin, PNOISE_SELF);
		gi.sound (current_player, CHAN_BODY,
				  gi.soundindex("player/watr_out.wav"),
				  1, ATTN_NORM, 0);

		current_player->flags &= ~FL_INWATER;
	}

	//
	// check for head just going under water
	//
	if (old_waterlevel != 3 && waterlevel == 3)
	{
		gi.sound (current_player, CHAN_BODY,
				  gi.soundindex("player/watr_un.wav"),
				  1, ATTN_NORM, 0);
	}

	//
	// check for head just coming out of water
	//
	if (old_waterlevel == 3 && waterlevel != 3)
	{
		if (current_player->air_finished < level.time)
		{	// gasp for air
			gi.sound (current_player, CHAN_VOICE,
					  gi.soundindex("player/gasp1.wav"),
					  1, ATTN_NORM, 0);
			PlayerNoise(current_player,
						current_player->s.origin, PNOISE_SELF);
		}

		else  if (current_player->air_finished < level.time + 11)
		{	// just break surface
			gi.sound (current_player, CHAN_VOICE,
					  gi.soundindex("player/gasp2.wav"),
					  1, ATTN_NORM, 0);
		}
	}

	//
	// check for drowning
	//
	if (waterlevel == 3)
	{
		// breather or envirosuit give air
		if (breather || envirosuit)
		{
			current_player->air_finished = level.time + 10;

			if (((int)(current_client->breather_framenum - level.framenum) % 25) == 0)
			{
				if (!current_client->breather_sound)
					gi.sound (current_player, CHAN_AUTO,
							  gi.soundindex("player/u_breath1.wav"),
							  1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_AUTO,
							  gi.soundindex("player/u_breath2.wav"),
							  1, ATTN_NORM, 0);

				current_client->breather_sound ^= 1;
				PlayerNoise(current_player,
							current_player->s.origin, PNOISE_SELF);
				//FIXME: release a bubble?
			}
		}

		// if out of air, start drowning
		if (current_player->air_finished < level.time)
		{	// drown!
			if (current_player->client->next_drown_time < level.time 
				&& current_player->health > 0)
			{
				current_player->client->next_drown_time = level.time + 1;

				// take more damage the longer underwater
				current_player->dmg += 2;
				if (current_player->dmg > 15)
					current_player->dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if (current_player->health <= current_player->dmg)
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("player/drown1.wav"),
							  1, ATTN_NORM, 0);
				else if (rand()&1)
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("*gurp1.wav"),
							  1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("*gurp2.wav"),
							  1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time;

				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, current_player->dmg,
						  0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		current_player->air_finished = level.time + 12;
		current_player->dmg = 2;
	}

	//
	// check for sizzle damage
	//
	if (waterlevel &&
		(current_player->watertype & (CONTENTS_LAVA|CONTENTS_SLIME)) )
	{
		if (current_player->watertype & CONTENTS_LAVA)
		{
			if (current_player->health > 0
				&& current_player->pain_debounce_time <= level.time
				&& current_client->invincible_framenum < level.framenum)
			{
				if (rand()&1)
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("player/burn1.wav"),
							  1, ATTN_NORM, 0);
				else
					gi.sound (current_player, CHAN_VOICE,
							  gi.soundindex("player/burn2.wav"),
							  1, ATTN_NORM, 0);

				current_player->pain_debounce_time = level.time + 1;
			}

			if (envirosuit)	// take 1/3 damage with envirosuit
				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, 1 * waterlevel, 0, 0, MOD_LAVA);
			else
				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		}

		if (current_player->watertype & CONTENTS_SLIME)
		{
			if (!envirosuit)
			{	// no damage from slime with envirosuit
				T_Damage (current_player, world, world,
						  vec3_origin, current_player->s.origin,
						  vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}


/*
===============
B_DamageFeedback

Handles color blends and view kicks
===============
*/
static void
B_DamageFeedback (edict_t *player)
{
	gclient_t	*client;
	float	count;
	int		r, l;

	client = player->client;

	if(player->deadflag) return;

	// total points of damage shot at the player this frame
	count = (client->damage_blood +
			 client->damage_armor +
			 client->damage_parmor);

	if (count == 0)
		return;		// didn't take any damage

	// start a pain animation if still in the player model
	if (client->anim_priority < ANIM_PAIN && player->s.modelindex == 255)
	{
		static int		i;
		client->anim_priority = ANIM_PAIN;
		if (client->ps.pmove.pm_flags & PMF_DUCKED)
		{
			player->s.frame = FRAME_crpain1-1;
			client->anim_end = FRAME_crpain4;
		}
		else
		{
			i = (i+1)%3;
			switch (i)
			{
			case 0:
				player->s.frame = FRAME_pain101-1;
				client->anim_end = FRAME_pain104;
				break;
			case 1:
				player->s.frame = FRAME_pain201-1;
				client->anim_end = FRAME_pain204;
				break;
			case 2:
				player->s.frame = FRAME_pain301-1;
				client->anim_end = FRAME_pain304;
				break;
			}
		}
	}


	// play an appropriate pain sound
	if ((level.time > player->pain_debounce_time) &&
		!(player->flags & FL_GODMODE) &&
		(client->invincible_framenum <= level.framenum))
	{
		r = 1 + (rand()&1);
		player->pain_debounce_time = level.time + 0.7;
		if (player->health < 25)
			l = 25;
		else if (player->health < 50)
			l = 50;
		else if (player->health < 75)
			l = 75;
		else
			l = 100;
		gi.sound (player, CHAN_VOICE,
				  gi.soundindex(va("*pain%i_%i.wav", l, r)),
				  1, ATTN_NORM, 0);
	}
	//
	// clear totals
	//
	client->damage_blood = 0;
	client->damage_armor = 0;
	client->damage_parmor = 0;
	client->damage_knockback = 0;
}


/*
=================
BotEndServerFrame

Called for each bot at the end of the server frame
and right after spawning
=================
*/
void BotEndServerFrame (edict_t *ent)
{
	float	bobtime;
	vec3_t	v = { 0 };

	current_player = ent;
	current_client = ent->client;

	//
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	//
//	for (i=0 ; i<3 ; i++)
//	{
//		current_client->ps.pmove.origin[i] = ent->s.origin[i]*8.0;
//		current_client->ps.pmove.velocity[i] = ent->velocity[i]*8.0;
//	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if (level.intermissiontime)
	{
		current_client->ps.fov = 90;
		G_SetStats (ent);
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, up);

	// burn from lava, etc
	B_WorldEffects (ent);

	ent->s.angles[ROLL] = 0;
	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//
//	xyspeed = sqrt(ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);
	VectorSubtract(ent->s.origin,ent->s.old_origin,v);
	v[2] = 0;
	xyspeed = VectorLength(v) * 10;


	if (xyspeed < 5)
	{
		bobmove = 0;
		current_client->bobtime = 0;	// start at beginning of cycle again
	}
	else if (ent->groundentity)
	{	// so bobbing only cycles when on ground
		if (xyspeed > 210)
			bobmove = 0.25;
		else if (xyspeed > 100)
			bobmove = 0.125;
		else
			bobmove = 0.0625;
	}
	
	bobtime = (current_client->bobtime += bobmove);

	if (current_client->ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	bobcycle = (int)bobtime;
	bobfracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	P_FallingDamage (ent);

	// apply all the damage taken this frame
	B_DamageFeedback (ent);

	G_SetStats (ent);

	if(!ent->deadflag)
	{
	// determine the view offsets
	SV_CalcViewOffset (ent);

	// determine the gun offsets
	SV_CalcGunOffset (ent);
	G_SetClientEvent (ent);
	G_SetClientEffects (ent);
	G_SetClientSound (ent);

	}

	if(ent->deadflag)
	{
		G_SetClientEffects (ent);
		ent->client->anim_priority = ANIM_DEATH;
		if(ent->s.modelindex != skullindex && ent->s.modelindex != headindex)
		{
			if(ent->s.frame < ent->client->anim_end) G_SetClientFrame (ent);
		}
		else ent->s.frame = 0;
	}	
	else 
		G_SetClientFrame (ent);

	VectorCopy (ent->velocity, ent->client->oldvelocity);
//	VectorCopy (ent->client->ps.viewangles, ent->client->oldviewangles);

	// clear weapon kicks
	VectorClear (ent->client->kick_origin);
	VectorClear (ent->client->kick_angles);

}

