
#include "g_local.h"
#include "g_items.h"
#include "m_player.h"
#include "bot.h"
#include "botstr.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4127)	// because hacks! (conditional expression is constant)
#endif

static void Get_AimAngle(edict_t* ent, float aim, float dist, int weapon)
{
	edict_t* target;
	vec3_t	targaim = { 0 };
	vec3_t	v = { 0 };
	trace_t	rs_trace;

	target = ent->client->zc.first_target;

	switch (weapon)
	{
	case WEAP_SHOTGUN:
	case WEAP_SUPERSHOTGUN:
	case WEAP_RAILGUN:
		if (target != ent->client->zc.last_target)
		{
			if (target->bot_client)
			{
				VectorSubtract(target->s.old_origin,
					target->s.origin, targaim);
			}
			else
			{
				VectorCopy(target->velocity, targaim);
				VectorInverse(targaim);
			}
			VectorNormalize(targaim);
			VectorMA(target->s.origin,
				random() * aim * AIMING_POSGAP * random(),
				targaim, targaim);
		}
		else
		{
			VectorSubtract(ent->client->zc.last_pos,
				target->s.origin, targaim);
			//VectorScale (targaim, vec_t scale, vec3_t out)
			VectorMA(target->s.origin, aim *
				/* VectorLength(targaim) * */
				random(), targaim, targaim);
		}

		VectorSubtract(targaim, ent->s.origin, targaim);

		ent->s.angles[YAW] = Get_yaw(targaim);
		ent->s.angles[PITCH] = Get_pitch(targaim);

		ent->s.angles[YAW] +=
			aim * AIMING_ANGLEGAP_S * (random() - 0.5) * 2;
		if (ent->s.angles[YAW] > 180)
			ent->s.angles[YAW] -= 360;
		else if (ent->s.angles[YAW] < -180)
			ent->s.angles[YAW] += 360;

		ent->s.angles[PITCH] +=
			(aim * AIMING_ANGLEGAP_S * (random() - 0.5) * 2);

		if (ent->s.angles[PITCH] > 90)
			ent->s.angles[PITCH] = 90;
		else if (ent->s.angles[PITCH] < -90)
			ent->s.angles[PITCH] = -90;

		break;

	case WEAP_MACHINEGUN:
	case WEAP_CHAINGUN:
		if (target != ent->client->zc.last_target)
		{
			if (target->bot_client)
			{
				VectorSubtract(target->s.old_origin,
					target->s.origin, targaim);
			}
			else
			{
				VectorCopy(target->velocity, targaim);
				VectorInverse(targaim);
			}
			VectorNormalize(targaim);
			VectorMA(target->s.origin,
				random() * aim * AIMING_POSGAP,
				targaim, targaim);
		}
		else
		{
			VectorSubtract(ent->client->zc.last_pos,
				target->s.origin, targaim);
			VectorMA(target->s.origin,
				random() * aim
				/** VectorLength(targaim)*/,
				targaim, targaim);
		}
		VectorSubtract(targaim, ent->s.origin, targaim);

		ent->s.angles[YAW] = Get_yaw(targaim);
		ent->s.angles[PITCH] = Get_pitch(targaim);

		ent->s.angles[YAW] +=
			aim * AIMING_ANGLEGAP_M * (random() - 0.5) * 2;

		if (ent->s.angles[YAW] > 180)
			ent->s.angles[YAW] -= 360;
		else if (ent->s.angles[YAW] < -180)
			ent->s.angles[YAW] += 360;

		ent->s.angles[PITCH] +=
			(aim * AIMING_ANGLEGAP_M * (random() - 0.5) * 2);

		if (ent->s.angles[PITCH] > 90)
			ent->s.angles[PITCH] = 90;
		else if (ent->s.angles[PITCH] < -90)
			ent->s.angles[PITCH] = -90;

		break;

	case WEAP_BLASTER:
	case WEAP_GRENADES:
	case WEAP_GRENADELAUNCHER:
	case WEAP_ROCKETLAUNCHER:
		if (target != ent->client->zc.last_target)
		{
			if (target->bot_client)
			{
				VectorSubtract(target->s.origin,
					target->s.old_origin, targaim);
			}
			else
			{
				VectorCopy(target->velocity, targaim);
				targaim[0] *= 32;
				targaim[1] *= 32;
				targaim[2] *= 32;
			}
			VectorNormalize(targaim);
			VectorMA(target->s.origin,
				(11 - aim) * dist / 25,
				targaim, targaim);
		}
		else
		{
			VectorSubtract(target->s.origin,
				ent->client->zc.last_pos, targaim);
			targaim[2] /= 2;
			VectorMA(target->s.origin,
				-aim * random() + dist / 75,
				targaim, targaim);
		}
		rs_trace = gi.trace(target->s.origin, NULL, NULL,
			targaim, target, MASK_SHOT);
		VectorCopy(rs_trace.endpos, targaim);

		if (weapon == WEAP_GRENADELAUNCHER
			|| weapon == WEAP_ROCKETLAUNCHER)
		{
			if (targaim[2] < (ent->s.origin[2] + JumpMax))
			{
				targaim[2] -= 24;

				VectorCopy(ent->s.origin, v);
				v[2] += ent->viewheight - 8;
				rs_trace = gi.trace(v, NULL, NULL,
					targaim, ent, MASK_SHOT);
				if (rs_trace.fraction != 1.0)
					targaim[2] += 24;
			}
			else if (targaim[2] > (ent->s.origin[2] + JumpMax))
				targaim[2] += 5;
		}

		VectorSubtract(targaim, ent->s.origin, targaim);

		ent->s.angles[YAW] = Get_yaw(targaim);
		ent->s.angles[PITCH] = Get_pitch(targaim);

		ent->s.angles[YAW] +=
			aim * AIMING_ANGLEGAP_M * (random() - 0.5) * 2;
		if (ent->s.angles[YAW] > 180)
			ent->s.angles[YAW] -= 360;
		else if (ent->s.angles[YAW] < -180)
			ent->s.angles[YAW] += 360;

		ent->s.angles[PITCH] +=
			(aim * AIMING_ANGLEGAP_M * (random() - 0.5) * 2);
		if (ent->s.angles[PITCH] > 90)
			ent->s.angles[PITCH] = 90;
		else if (ent->s.angles[PITCH] < -90)
			ent->s.angles[PITCH] = -90;
		break;

	case WEAP_HYPERBLASTER:
		if (target != ent->client->zc.last_target)
		{
			if (target->bot_client)
			{
				VectorSubtract(target->s.origin,
					target->s.old_origin, targaim);
			}
			else
			{
				VectorCopy(target->velocity, targaim);
				targaim[0] *= 32;
				targaim[1] *= 32;
				targaim[2] *= 32;
			}
			VectorNormalize(targaim);
			VectorMA(target->s.origin,
				(11 - aim) * dist / 100, targaim, targaim);
		}
		else
		{
			VectorSubtract(target->s.origin,
				ent->client->zc.last_pos, targaim);
			targaim[2] /= 2;
			VectorMA(target->s.origin,
				-aim + dist / 115, targaim, targaim);
		}
		rs_trace = gi.trace(target->s.origin, NULL, NULL,
			targaim, target, MASK_SHOT);
		VectorCopy(rs_trace.endpos, targaim);

		VectorSubtract(targaim, ent->s.origin, targaim);

		ent->s.angles[YAW] = Get_yaw(targaim);
		ent->s.angles[PITCH] = Get_pitch(targaim);

		ent->s.angles[YAW] +=
			aim * AIMING_ANGLEGAP_M * (random() - 0.5) * 2;
		if (ent->s.angles[YAW] > 180)
			ent->s.angles[YAW] -= 360;
		else if (ent->s.angles[YAW] < -180)
			ent->s.angles[YAW] += 360;

		ent->s.angles[PITCH] +=
			(aim * AIMING_ANGLEGAP_M * (random() - 0.5) * 2);
		if (ent->s.angles[PITCH] > 90)
			ent->s.angles[PITCH] = 90;
		else if (ent->s.angles[PITCH] < -90)
			ent->s.angles[PITCH] = -90;
		break;

	case WEAP_BFG:
		VectorCopy(ent->client->zc.vtemp, targaim);
		VectorSubtract(targaim, ent->s.origin, targaim);

		ent->s.angles[YAW] = Get_yaw(targaim);
		ent->s.angles[PITCH] = Get_pitch(targaim);
		break;

	default:
		break;
	}
}

/**
Decide if player can use weapon, set weaponstate appropriately.
Returns: true if ready, 2 if not. Why 2?
*/
static int CanUsewep(edict_t* ent, int weapon)
{
	gitem_t* item;
	gclient_t* client;
	int mywep;
	int ammoindex;

	client = ent->client;

	mywep = Get_KindWeapon(client->pers.weapon);

	switch (weapon)
	{
	case WEAP_BLASTER:
		if (voosh->value)
		{
			item = Fdi_RAILGUN;//FindItem("Blaster");
			if (client->pers.inventory[ITEM_INDEX(item)])
			{
				if (mywep == WEAP_RAILGUN
					|| client->weaponstate == WEAPON_READY)
				{
					item->use(ent, item);
					if (client->weaponstate == WEAPON_READY)
						return true;
					else
						return 2;
				}
			}
		}
		else
		{
			item = Fdi_BLASTER;
			if (client->pers.inventory[ITEM_INDEX(item)])
			{
				if (mywep == WEAP_BLASTER
					|| client->weaponstate == WEAPON_READY)
				{
					item->use(ent, item);
					if (client->weaponstate == WEAPON_READY)
						return true;
					else
						return 2;
				}
			}
		}
		break;

	case WEAP_SHOTGUN:
		item = Fdi_SHOTGUN;
		ammoindex = ITEM_INDEX(Fdi_SHELLS);
		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_SHOTGUN
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_SUPERSHOTGUN:
		item = Fdi_SUPERSHOTGUN;
		ammoindex = ITEM_INDEX(Fdi_SHELLS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 1)
		{
			if (mywep == WEAP_SUPERSHOTGUN
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_MACHINEGUN:
		item = Fdi_MACHINEGUN;
		ammoindex = ITEM_INDEX(Fdi_BULLETS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (client->pers.weapon != item) item->use(ent, item);

			if (mywep == WEAP_MACHINEGUN
				|| client->weaponstate == WEAPON_READY
				|| client->weaponstate == WEAPON_FIRING)
			{
				if (client->weaponstate == WEAPON_READY
					|| client->weaponstate == WEAPON_FIRING)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_CHAINGUN:
		item = FindItem("Chaingun");
		ammoindex = ITEM_INDEX(Fdi_BULLETS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_CHAINGUN
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY
					|| client->weaponstate == WEAPON_FIRING)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_GRENADES:
		item = Fdi_GRENADES;
		ammoindex = ITEM_INDEX(Fdi_GRENADES);

		if (client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_GRENADES
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_GRENADELAUNCHER:
		item = Fdi_GRENADELAUNCHER;
		ammoindex = ITEM_INDEX(Fdi_GRENADES);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_GRENADELAUNCHER
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_ROCKETLAUNCHER:
		item = Fdi_ROCKETLAUNCHER;
		ammoindex = ITEM_INDEX(Fdi_ROCKETS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_ROCKETLAUNCHER
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_HYPERBLASTER:
		item = Fdi_HYPERBLASTER;
		ammoindex = ITEM_INDEX(Fdi_CELLS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_HYPERBLASTER
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY
					|| client->weaponstate == WEAPON_FIRING)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_RAILGUN:
		item = Fdi_RAILGUN;
		ammoindex = ITEM_INDEX(Fdi_SLUGS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] > 0)
		{
			if (mywep == WEAP_RAILGUN
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else
					return 2;
			}
		}
		break;

	case WEAP_BFG:
		item = Fdi_BFG;
		ammoindex = ITEM_INDEX(Fdi_CELLS);

		if (client->pers.inventory[ITEM_INDEX(item)]
			&& client->pers.inventory[ammoindex] >= 50)
		{
			if (mywep == WEAP_BFG
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else return 2;
			}
		}
		break;

	default:	//case WEAP_BLASTER:
		item = Fdi_BLASTER;//FindItem("Blaster");
		if (client->pers.inventory[ITEM_INDEX(item)])
		{
			if (mywep == WEAP_BLASTER
				|| client->weaponstate == WEAPON_READY)
			{
				item->use(ent, item);
				if (client->weaponstate == WEAPON_READY)
					return true;
				else return 2;
			}
		}
		break;
	}
	return false;
}

//------------------------------------------------------------

//	Use BFG

//------------------------------------------------------------
static qboolean B_UseBfg(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int k;
	int mywep;
	zgcl_t* zc;
	gclient_t* client;

	client = ent->client;
	zc = &client->zc;

	k = CanUsewep(ent, WEAP_BFG);
	if (k)
	{
		mywep = Get_KindWeapon(client->pers.weapon);

		Get_AimAngle(ent, aim, distance, mywep);

		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;

		k = Bot_traceS(ent, target);
		if (k)
			VectorCopy(target->s.origin, zc->vtemp);

		if (FFlg[bot_skill] & FIRE_STAYFIRE)
		{
			if (k /*&& random() < 0.8*/)
			{
				client->buttons |= BUTTON_ATTACK;
				zc->battlemode |= FIRE_STAYFIRE;
				zc->battlecount = 8 + (int)(10 * random());
				trace_priority = TRP_ALLKEEP;
				return true;
			}
		}

		else if ((FFlg[bot_skill] & FIRE_EXPAVOID)
			&& distance < 300
			/*&& random() < 0.5 */
			&& Bot_traceS(ent, target))
		{
			if (ent->groundentity || zc->waterstate)
			{
				zc->battlemode |= FIRE_EXPAVOID;
				zc->battlecount = 6 + (int)(6 * random());
				trace_priority = TRP_ALLKEEP;
				return true;
			}
		}

		else if (!(FFlg[bot_skill] & (FIRE_STAYFIRE | FIRE_EXPAVOID)))
		{
			if (k /*&& random() < 0.8*/)
			{
				zc->battlemode |= FIRE_BFG;
				zc->battlecount = 6 + (int)(6 * random());
				trace_priority = TRP_ANGLEKEEP;
				return true;
			}
		}

		else if ((FFlg[bot_skill] & FIRE_EXPAVOID)
			&& Bot_traceS(ent, target))
		{
			if (k /*&& random() < 0.8*/)
			{
				zc->battlemode |= FIRE_BFG;
				zc->battlecount = 6 + (int)(6 * random());
				trace_priority = TRP_ANGLEKEEP;
				return true;
			}
		}
	}
	return false;
}

//------------------------------------------------------------

//	Use Hyper Blaster

//------------------------------------------------------------
static qboolean B_UseHyperBlaster(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_HYPERBLASTER))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}


//------------------------------------------------------------

//	Use Rocket

//------------------------------------------------------------
static qboolean B_UseRocket(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	zgcl_t* zc;
	gclient_t* client;

	client = ent->client;
	zc = &client->zc;

	if (CanUsewep(ent, WEAP_ROCKETLAUNCHER))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;

		if ((FFlg[bot_skill] & FIRE_PRESTAYFIRE)
			&& ((distance > 500 && random() < 0.1)
				|| fabs(ent->s.angles[PITCH]) > 45)
			&& Bot_traceS(ent, target)
			&& (enewep <= WEAP_MACHINEGUN
				|| enewep == WEAP_GRENADES))
		{
			if (ent->groundentity || zc->waterstate)
			{
				zc->battlemode |= FIRE_PRESTAYFIRE;
				zc->battlecount = 2 + (int)(6 * random());
				trace_priority = TRP_ALLKEEP;
				return true;
			}
		}

		if ((FFlg[bot_skill] & FIRE_JUMPROC) && random() < 0.3
			&& (target->s.origin[2] - ent->s.origin[2]) < JumpMax
			&& !(client->ps.pmove.pm_flags && PMF_DUCKED))
		{
			if (ent->groundentity && !ent->waterlevel)
			{
				if (zc->route_trace)
				{
					if (Bot_Fall(ent, ent->s.origin, 0))
					{
						trace_priority = TRP_ALLKEEP;
						if (Bot_traceS(ent, target))
							client->buttons |= BUTTON_ATTACK;
						return true;
					}
				}
				else
				{
					ent->moveinfo.speed = 0;

					ent->velocity[2] += VEL_BOT_JUMP;
					gi.sound(ent, CHAN_VOICE,
						gi.soundindex("*jump1.wav"),
						1, ATTN_NORM, 0);
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);	//pon
					Set_BotAnim(ent, ANIM_JUMP, FRAME_jump1 - 1, FRAME_jump6);
					trace_priority = TRP_ALLKEEP;
					if (Bot_traceS(ent, target))
						client->buttons |= BUTTON_ATTACK;
					return true;
				}
			}
		}
		else if ((FFlg[bot_skill] & FIRE_EXPAVOID)
			&& distance < 300 && random() < 0.5
			&& Bot_traceS(ent, target))
		{
			if (ent->groundentity || zc->waterstate)
			{
				zc->battlemode |= FIRE_EXPAVOID;
				zc->battlecount = 4 + (int)(6 * random());
				trace_priority = TRP_ALLKEEP;
				return true;
			}
		}
		if (Bot_traceS(ent, target))
			client->buttons |= BUTTON_ATTACK;
		return true;
	}
	return false;
}


//------------------------------------------------------------

//	Use Railgun

//------------------------------------------------------------
static qboolean B_UseRailgun(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_RAILGUN))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}

//------------------------------------------------------------

//	Use Grenade Launcher

//------------------------------------------------------------
static qboolean B_UseGrenadeLauncher(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	zgcl_t* zc;
	gclient_t* client;

	client = ent->client;
	zc = &client->zc;

	if (CanUsewep(ent, WEAP_GRENADELAUNCHER))
	{
		mywep = Get_KindWeapon(client->pers.weapon);

		Get_AimAngle(ent, aim, distance, mywep);

		if ((FFlg[bot_skill] & FIRE_STAYFIRE)
			&& random() < 0.3
			&& target->s.origin[2] < ent->s.origin[2])
		{
			if (ent->groundentity || zc->waterstate)
			{
				if (Bot_traceS(ent, target))
				{
					zc->battlemode |= FIRE_STAYFIRE;
					zc->battlecount = 5 + (int)(10 * random());
					trace_priority = TRP_ALLKEEP;
					client->buttons |= BUTTON_ATTACK;
					return true;
				}
			}
		}
		else if ((FFlg[bot_skill] & FIRE_EXPAVOID)
			&& distance < 300 && random() < 0.5
			&& Bot_traceS(ent, target))
		{
			if (ent->groundentity || zc->waterstate)
			{
				zc->battlemode |= FIRE_EXPAVOID;
				zc->battlecount = 2 + (int)(6 * random());
				trace_priority = TRP_ALLKEEP;
				return true;
			}
		}
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}

//------------------------------------------------------------

//	Use Chain Gun

//------------------------------------------------------------
static qboolean B_UseChainGun(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_CHAINGUN))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}


//------------------------------------------------------------

//	Use Machine Gun

//------------------------------------------------------------
static qboolean B_UseMachineGun(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_MACHINEGUN))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}

//------------------------------------------------------------

//	Use S-Shotgun

//------------------------------------------------------------
static qboolean B_UseSuperShotgun(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_SUPERSHOTGUN))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}

//------------------------------------------------------------

//	Use Shotgun

//------------------------------------------------------------
static qboolean B_UseShotgun(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_SHOTGUN))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}

//------------------------------------------------------------

//	Use Hand Grenade

//------------------------------------------------------------
static qboolean B_UseHandGrenade(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_GRENADES))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		if (ent->client->weaponstate == WEAPON_READY)
			client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;
	}
	return false;
}



//------------------------------------------------------------

//	Use Blaster

//------------------------------------------------------------
static qboolean B_UseBlaster(edict_t* ent, edict_t* target,
	int enewep, float aim, float distance, int bot_skill)
{
	int mywep;
	gclient_t* client;

	client = ent->client;

	if (CanUsewep(ent, WEAP_BLASTER))
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		client->buttons |= BUTTON_ATTACK;
		if (trace_priority < TRP_ANGLEKEEP)
			trace_priority = TRP_ANGLEKEEP;
		return true;;
	}
	return false;
}

//return weapon
void Combat_LevelX(edict_t* ent, int foundedenemy,
	int enewep, float aim, float distance, int bot_skill)
{
	gclient_t* client;
	zgcl_t* zc;
	edict_t* target;
	int			mywep;
	int			k;
	vec3_t		v = { 0 };

	client = ent->client;
	zc = &client->zc;
	target = zc->first_target;

	k = false;
	if (zc->battlemode & FIRE_ESTIMATE)
	{
		mywep = Get_KindWeapon(client->pers.weapon);

		//Rocket
		if (distance > 100 || mywep == WEAP_ROCKETLAUNCHER)
		{
			if (B_UseRocket(ent, target, enewep, aim, distance, bot_skill))
				k = true;
		}

		//Grenade Launcher
		if (distance > 100 && distance < 400
			&& (target->s.origin[2] - ent->s.origin[2]) < 200)
		{
			if (B_UseGrenadeLauncher(ent, target, enewep, aim, distance, bot_skill))
				k = true;
		}

		//Hand Grenade
		if (distance < 1200)
		{
			if (B_UseHandGrenade(ent, target, enewep, aim, distance, bot_skill))
				k = true;
		}
		VectorSubtract(zc->vtemp, ent->s.origin, v);
		ent->s.angles[YAW] = Get_yaw(v);
		ent->s.angles[PITCH] = Get_pitch(v);
		if (k)
			trace_priority = TRP_ALLKEEP;
		else
			trace_priority = TRP_ANGLEKEEP;
		return;
	}
	VectorSubtract(target->s.origin, ent->s.origin, v);
	ent->s.angles[YAW] = Get_yaw(v);
	ent->s.angles[PITCH] = Get_pitch(v);
	trace_priority = TRP_ANGLEKEEP;
}

//return weapon
void Combat_Level0(edict_t* ent, int foundedenemy,
	int enewep, float aim, float distance, int bot_skill)
{
	float		f;
	gclient_t* client;
	zgcl_t* zc;
	edict_t* target;
	int			mywep, i, j, k;
	vec3_t	v = { 0 };
	vec3_t	vv = { 0 };
	vec3_t	v1 = { 0 };
	vec3_t	v2 = { 0 };
	trace_t		rs_trace;

	client = ent->client;
	zc = &client->zc;
	target = zc->first_target;


	if (zc->battlemode == FIRE_CHIKEN) aim *= 0.7f;

	if (zc->battlemode & FIRE_SHIFT)
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);

		if (--zc->battlesubcnt > 0)
		{
			if (ent->groundentity)
			{
				if (zc->battlemode & FIRE_SHIFT_R)
				{
					zc->moveyaw = ent->s.angles[YAW] + 90;
					if (zc->moveyaw > 180)
						zc->moveyaw -= 360;
				}
				else
				{
					zc->moveyaw = ent->s.angles[YAW] - 90;
					if (zc->moveyaw < -180)
						zc->moveyaw += 360;
				}
				trace_priority = TRP_MOVEKEEP;
			}
		}
		else
		{
			zc->battlemode &= ~FIRE_SHIFT;
		}
	}

	//dodge=============================
	if (Bot[ent->client->zc.botindex].param[BOP_DODGE]
		&& ent->groundentity && !ent->waterlevel)
	{
		AngleVectors(target->client->v_angle, v, NULL, NULL);
		VectorScale(v, 300, v);

		VectorSet(vv, 0, 0, target->viewheight - 8);
		VectorAdd(target->s.origin, vv, vv);
		VectorAdd(vv, v, v);

		VectorSet(v1, -4, -4, -4);
		VectorSet(v2, 4, 4, 4);
		rs_trace = gi.trace(vv, v1, v2, v, target, MASK_SHOT);

		if (rs_trace.ent == ent)
		{
			if (rs_trace.endpos[2] > (ent->s.origin[2] + 4) && random() < 0.4)
			{
				client->ps.pmove.pm_flags |= PMF_DUCKED;
				zc->battleduckcnt = 2 + 8 * random();
			}
			else if (rs_trace.endpos[2] < (ent->s.origin[2] + JumpMax - 24))
			{
				if (zc->route_trace)
				{
					if (Bot_Fall(ent, ent->s.origin, 0))
						trace_priority = TRP_MOVEKEEP;;
				}
				else
				{
					ent->moveinfo.speed = 0.5;

					ent->velocity[2] += VEL_BOT_JUMP;
					gi.sound(ent, CHAN_VOICE,
						gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
					PlayerNoise(ent, ent->s.origin, PNOISE_SELF);	//pon
					Set_BotAnim(ent, ANIM_JUMP, FRAME_jump1 - 1, FRAME_jump6);
				}
			}
		}
	}

	if (zc->battlemode & FIRE_IGNORE)
	{
		if (--zc->battlecount > 0)
		{
			if (zc->first_target != zc->last_target)
			{
				zc->battlemode = 0;
			}
			else return;
		}
		zc->battlemode = 0;
	}

	if (zc->battlemode & FIRE_PRESTAYFIRE)
	{
		if (--zc->battlecount > 0)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);
			if (target->client->weaponstate == WEAPON_FIRING
				&& ent->groundentity)
				ent->client->ps.pmove.pm_flags |= PMF_DUCKED;

			trace_priority = TRP_ALLKEEP;
			return;
		}
		if (!(zc->battlemode & FIRE_SHIFT))
			zc->battlemode = FIRE_STAYFIRE;
		zc->battlecount = 5 + (int)(20 * random());
	}

	if (zc->battlemode & FIRE_STAYFIRE)
	{
		if (--zc->battlecount > 0)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			if (1/*mywep == WEAP_BFG*/)
				CanUsewep(ent, WEAP_BFG);
			aim *= 0.95f;
			Get_AimAngle(ent, aim, distance, mywep);
			if (target->client->weaponstate == WEAPON_FIRING
				&& ent->groundentity)
			{
				if (mywep == WEAP_BFG)
				{
					if (target->s.origin[2] > ent->s.origin[2])
						client->ps.pmove.pm_flags |= PMF_DUCKED;
				}
				else client->ps.pmove.pm_flags |= PMF_DUCKED;
			}
			if (!(zc->battlemode & FIRE_SHIFT))
				trace_priority = TRP_ALLKEEP;
			if (Bot_traceS(ent, target)
				|| mywep == WEAP_BFG
				|| mywep == WEAP_GRENADELAUNCHER)
				client->buttons |= BUTTON_ATTACK;
			return;
		}
		zc->battlemode = 0;
	}

	//FIRE_RUSH	========================
	if (zc->battlemode & FIRE_RUSH)
	{
		if (--zc->battlecount > 0)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			if (1/*mywep == WEAP_BFG*/) CanUsewep(ent, WEAP_BFG);
			aim *= 0.95f;
			Get_AimAngle(ent, aim, distance, mywep);
			if (target->client->weaponstate == WEAPON_FIRING
				&& ent->groundentity)
			{
				if (mywep == WEAP_BFG)
				{
					if (target->s.origin[2] > ent->s.origin[2])
						client->ps.pmove.pm_flags |= PMF_DUCKED;
				}
				else client->ps.pmove.pm_flags |= PMF_DUCKED;
			}
			trace_priority = TRP_MOVEKEEP;
			zc->moveyaw = ent->s.angles[YAW];

			if (Bot_traceS(ent, target))
				client->buttons |= BUTTON_ATTACK;
			return;
		}
		zc->battlemode = 0;
	}

	if (zc->battlemode & FIRE_EXPAVOID)
	{
		if (--zc->battlecount > 0)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			if (1/*mywep == WEAP_BFG*/)
				CanUsewep(ent, WEAP_BFG);
			aim *= 0.95f;
			Get_AimAngle(ent, aim, distance, mywep);
			if (target->client->weaponstate == WEAPON_FIRING && ent->groundentity)
			{
				if (mywep == WEAP_BFG)
				{
					if (target->s.origin[2] > ent->s.origin[2])
						client->ps.pmove.pm_flags |= PMF_DUCKED;
				}
				else client->ps.pmove.pm_flags |= PMF_DUCKED;
			}
			trace_priority = TRP_MOVEKEEP;
			zc->moveyaw = ent->s.angles[YAW] + 180;

			if (zc->moveyaw > 180)
				zc->moveyaw -= 360;

			if (Bot_traceS(ent, target)
				|| mywep == WEAP_BFG
				|| mywep == WEAP_GRENADELAUNCHER)
				client->buttons |= BUTTON_ATTACK;
			return;
		}
		zc->battlemode = 0;
	}

	if (zc->battlemode & FIRE_BFG)
	{
		if (--zc->battlecount > 0)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			if (1) /*mywep == WEAP_BFG*/
				CanUsewep(ent, WEAP_BFG);
			aim *= 0.95f;
			Get_AimAngle(ent, aim, distance, mywep);
			if (target->client->weaponstate == WEAPON_FIRING
				&& ent->groundentity)
			{
				if ((1)) /*mywep == WEAP_BFG*/
				{
					if (target->s.origin[2] > ent->s.origin[2])
						client->ps.pmove.pm_flags |= PMF_DUCKED;
				}
				else
					client->ps.pmove.pm_flags |= PMF_DUCKED;
			}
			trace_priority = TRP_ANGLEKEEP;

			if (Bot_traceS(ent, target)
				|| mywep == WEAP_BFG
				|| mywep == WEAP_GRENADELAUNCHER)
				client->buttons |= BUTTON_ATTACK;
			return;
		}
		zc->battlemode = 0;
	}

	if (zc->battlemode & FIRE_REFUGE)
	{
		if (--zc->battlecount > 0)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			//CanUsewep(ent,WEAP_BFG);
			aim *= 0.95f;
			Get_AimAngle(ent, aim, distance, mywep);
			if (target->client->weaponstate == WEAPON_FIRING &&
				ent->groundentity)
			{
				if (mywep == WEAP_BFG)
				{
					if (target->s.origin[2] > ent->s.origin[2])
						client->ps.pmove.pm_flags |= PMF_DUCKED;
				}
				else client->ps.pmove.pm_flags |= PMF_DUCKED;
			}
			trace_priority = TRP_ANGLEKEEP;
			//			trace_priority = TRP_ALLKEEP;
			if (Bot_traceS(ent, target)
				|| mywep == WEAP_BFG
				|| mywep == WEAP_GRENADELAUNCHER) client->buttons |= BUTTON_ATTACK;
			return;
		}
		zc->battlemode = 0;
		zc->routeindex -= 2;
	}

	if (!(client->zc.zccmbstt & CTS_ENEM_NSEE)
		&& (zc->zcstate & STS_WAITSMASK2)
		&& (target->s.origin[2] - ent->s.origin[2]) < -300)
	{
		k = CanUsewep(ent, WEAP_GRENADELAUNCHER);
		if (k)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);

			if ((target->client->weaponstate == WEAPON_FIRING
				&& ent->groundentity)
				|| (zc->zcstate & STS_WAITSMASK2))
				ent->client->ps.pmove.pm_flags |= PMF_DUCKED;

			client->buttons |= BUTTON_ATTACK;
			trace_priority = TRP_ANGLEKEEP;
			return;
		}
		k = CanUsewep(ent, WEAP_GRENADES);
		if (k)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);

			if (target->client->weaponstate == WEAPON_FIRING
				&& ent->groundentity)
				ent->client->ps.pmove.pm_flags |= PMF_DUCKED;

			if (ent->client->weaponstate == WEAPON_READY)
				client->buttons |= BUTTON_ATTACK;

			trace_priority = TRP_ANGLEKEEP;
			return;
		}
	}

	mywep = Get_KindWeapon(client->pers.weapon);

	if (!(zc->battlemode & FIRE_SHIFT)
		&& bot_skill > (random() * bot_skill)
		/*&& distance < 250*/
		&& (30 * random()) < Bot[zc->botindex].param[BOP_OFFENCE])
	{
		k = false;
		if (zc->route_trace && enewep != WEAP_RAILGUN)
		{
			for (i = zc->routeindex; i < (zc->routeindex + 10); i++)
			{
				if (i >= CurrentIndex)
					break;
				if (Route[i].state == GRS_ITEMS)
				{
					if (Route[i].ent->solid == SOLID_TRIGGER)
					{
						k = true;
						break;
					}
				}
			}
		}
		if (!k)
		{
			Get_AimAngle(ent, aim, distance, mywep);
			f = target->s.angles[YAW] - ent->s.angles[YAW];

			if (f > 180)
			{
				f = -(360 - f);
			}
			if (f < -180)
			{
				f = -(f + 360);
			}

			if (f <= -160)
			{

				zc->battlemode |= FIRE_SHIFT_L;
				zc->battlesubcnt = 5 + (int)(16 * random());
			}
			else if (f >= 160)
			{
				zc->battlemode |= FIRE_SHIFT_R;
				zc->battlesubcnt = 5 + (int)(16 * random());
			}
		}
	}

	if ((FFlg[bot_skill] & FIRE_AVOIDINV)
		&& target->client->invincible_framenum > level.framenum)
	{
		// mywep = Get_KindWeapon(client->pers.weapon);
		Get_AimAngle(ent, aim, distance, mywep);
		trace_priority = TRP_MOVEKEEP;
		zc->moveyaw = ent->s.angles[YAW] + 180;
		if (zc->moveyaw > 180)
			zc->moveyaw -= 360;
		return;
	}

	if ((FFlg[bot_skill] & FIRE_QUADUSE)
		&& (ent->client->quad_framenum > level.framenum)
		&& distance < 300)
	{
		j = false;
		if (enewep < WEAP_MACHINEGUN || enewep == WEAP_GRENADES)
			j = true;

		//Hyper Blaster
		k = CanUsewep(ent, WEAP_HYPERBLASTER);
		if (k)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);
			client->buttons |= BUTTON_ATTACK;
			trace_priority = TRP_ANGLEKEEP;
			if (j)
			{
				zc->battlemode |= FIRE_RUSH;
				zc->battlecount = 8 + (int)(10 * random());
			}
			return;
		}
		//Chain Gun
		k = CanUsewep(ent, WEAP_CHAINGUN);
		if (k)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);
			client->buttons |= BUTTON_ATTACK;
			trace_priority = TRP_ANGLEKEEP;
			if (j)
			{
				zc->battlemode |= FIRE_RUSH;
				zc->battlecount = 8 + (int)(10 * random());
			}
			return;
		}
		//Machine Gun
		k = CanUsewep(ent, WEAP_MACHINEGUN);
		if (k)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);
			client->buttons |= BUTTON_ATTACK;
			trace_priority = TRP_ANGLEKEEP;
			if (j)
			{
				zc->battlemode |= FIRE_RUSH;
				zc->battlecount = 8 + (int)(10 * random());
			}
			return;
		}
		//S-Shotgun
		k = CanUsewep(ent, WEAP_SUPERSHOTGUN);
		if (k)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			Get_AimAngle(ent, aim, distance, mywep);
			client->buttons |= BUTTON_ATTACK;
			trace_priority = TRP_ANGLEKEEP;
			if (j)
			{
				zc->battlemode |= FIRE_RUSH;
				zc->battlecount = 8 + (int)(10 * random());
			}
			return;
		}
	}

	if ((FFlg[bot_skill] & FIRE_REFUGE)
		&& zc->battlemode == 0
		&& zc->route_trace
		&& zc->routeindex > 1)
	{
		j = false;
		if (enewep >= WEAP_CHAINGUN && enewep != WEAP_GRENADES)
			j = true;


		Get_RouteOrigin(zc->routeindex - 2, v);

		if (fabsf(v[2] - ent->s.origin[2]) < JumpMax && j)
		{
			mywep = Get_KindWeapon(client->pers.weapon);
			if (mywep == WEAP_GRENADELAUNCHER
				|| mywep == WEAP_ROCKETLAUNCHER)
			{
				zc->battlemode |= FIRE_REFUGE;
				zc->battlecount = 8 + (int)(10 * random());
				trace_priority = TRP_ALLKEEP;
				return;
			}
		}
	}

	if (!zc->route_trace && distance < 100)
	{
		zc->battlemode |= FIRE_EXPAVOID;
		zc->battlecount = 4 + (int)(8 * random());
		trace_priority = TRP_ALLKEEP;
	}


	//BFG
	if (distance > 200)
	{
		if (B_UseBfg(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	for (i = 0; i < 3; i++)
	{
		mywep = Get_KindWeapon(client->pers.weapon);

		if (i == 0 && zc->secwep_selected)
			continue;

		//try to select secondary weapon
		if (i == 0 && zc->secwep_selected)
			i = 1;
		else if (i == 0 && foundedenemy < 3
			&& target->health < 50 && !zc->secwep_selected
			&& ent->health >= 50)
		{
			if ((9 * random()) < Bot[zc->botindex].param[BOP_COMBATSKILL])
			{
				zc->secwep_selected = 2;
				i = 1;
			}
		}

		if (i == 2)
		{
			if (zc->secwep_selected)
			{
				zc->secwep_selected = 0;
				j = 0;
			}
			else break;
		}
		else j = i;

		if (distance > 100 && (mywep == WEAP_BFG || random() < 0.5))
		{
			if (B_UseBfg(ent, target, enewep, aim, distance, bot_skill))
				goto FIRED;
		}

		switch (Bot[zc->botindex].param[BOP_PRIWEP + j])
		{
		case WEAP_RAILGUN:

			if (distance > 1 || voosh->value)
			{
				if (B_UseRailgun(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;


		case WEAP_BFG:
			if (distance > 100)
			{
				if (B_UseBfg(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;
		case WEAP_HYPERBLASTER:
			if (distance < 1200)
			{
				if (B_UseHyperBlaster(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;

		case WEAP_ROCKETLAUNCHER:
			if (distance > 100 && distance < 1200
				/*|| mywep == WEAP_ROCKETLAUNCHER*/)
			{
				if (B_UseRocket(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;

		case WEAP_GRENADELAUNCHER:
			if (distance > 100 && distance < 400
				&& (target->s.origin[2] - ent->s.origin[2]) < 200)
			{
				if (B_UseGrenadeLauncher(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;
		case WEAP_CHAINGUN:
		case WEAP_MACHINEGUN:
			if (distance < 1200)
			{
				if (B_UseChainGun(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			if (distance < 1200)
			{
				if (B_UseMachineGun(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;
		case WEAP_SUPERSHOTGUN:
		case WEAP_SHOTGUN:
			if (distance < 1200)
			{
				if (B_UseSuperShotgun(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			if (distance < 1200)
			{
				if (B_UseShotgun(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;
		case WEAP_GRENADES:
			if (distance < 1200)
			{
				if (B_UseHandGrenade(ent, target, enewep, aim, distance, bot_skill))
					goto FIRED;
			}
			break;
		default:
			break;
		}
	}


	zc->secwep_selected = 0;
	//BFG
	if (distance > 200)
	{
		if (B_UseBfg(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//Hyper Blaster
	if (distance < 1200)
	{
		if (B_UseHyperBlaster(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//Rocket
	if ((distance > 100 && distance < 1200)
		/*|| mywep == WEAP_ROCKETLAUNCHER*/)
	{
		if (B_UseRocket(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//Railgun
	if (distance > 1200 || voosh->value)
	{
		if (B_UseRailgun(ent, target, enewep, aim, distance, bot_skill) || voosh->value)
			goto FIRED;
	}

	//Grenade Launcher
	if (distance > 100 && distance < 400
		&& (target->s.origin[2] - ent->s.origin[2]) < 200)
	{
		if (B_UseGrenadeLauncher(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//Chain Gun
	if (distance < 1200)
	{
		if (B_UseChainGun(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//Machine Gun
	if (distance < 1200)
	{
		if (B_UseMachineGun(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//S-Shotgun
	if (distance < 1200)
	{
		if (B_UseSuperShotgun(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	if ((FFlg[bot_skill] & FIRE_IGNORE)
		&& distance > 400 && ent->groundentity
		&& !(zc->zcstate & STS_WAITSMASK))
	{
		zc->battlemode = FIRE_IGNORE;
		zc->battlecount = 5 + (int)(10 * random());

	}

	//Shotgun
	if (distance < 1200)
	{
		if (B_UseShotgun(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}
	//Hand Grenade
	if (distance < 400)
	{
		if (B_UseHandGrenade(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	//Blaster
	if (distance < 1200)
	{
		if (B_UseBlaster(ent, target, enewep, aim, distance, bot_skill))
			goto FIRED;
	}

	VectorSubtract(zc->vtemp, ent->s.origin, v);
	ent->s.angles[YAW] = Get_yaw(v);
	ent->s.angles[PITCH] = Get_pitch(v);
	trace_priority = TRP_ANGLEKEEP;
	return;

FIRED:
	if (zc->secwep_selected == 2)
		zc->secwep_selected = 1;

	if (zc->battlemode == FIRE_CHIKEN)
	{
		if (--zc->battlesubcnt > 0 && ent->groundentity && ent->waterlevel < 2)
		{
			f = target->s.angles[YAW] - ent->s.angles[YAW];

			if (f > 180)
			{
				f = -(360 - f);
			}
			if (f < -180)
			{
				f = -(f + 360);
			}
			if (fabs(f) >= 150)
			{
				zc->battlemode = 0;
			}
			else
			{
				if (client->weaponstate != WEAPON_READY
					&& target->s.origin[2] < ent->s.origin[2])
				{
					if (mywep == WEAP_ROCKETLAUNCHER
						|| mywep == WEAP_GRENADELAUNCHER
						|| mywep == WEAP_RAILGUN)
						client->ps.pmove.pm_flags |= PMF_DUCKED;
					else if (Bot[zc->botindex].param[BOP_COMBATSKILL] >= 7)
					{
						if (mywep == WEAP_SHOTGUN
							|| mywep == WEAP_SUPERSHOTGUN
							|| mywep == WEAP_BLASTER)
							client->ps.pmove.pm_flags |= PMF_DUCKED;
					}
				}
				trace_priority = TRP_ALLKEEP;
			}
			return;
		}
		else zc->battlemode = 0;
	}
	else if (zc->battlemode == 0 && distance > 200
		&& ent->groundentity && ent->waterlevel < 2
		&& (9 * random()) > Bot[zc->botindex].param[BOP_OFFENCE])
	{
		mywep = Get_KindWeapon(client->pers.weapon);
		if (mywep > WEAP_BLASTER && target->client->zc.first_target != ent)
		{
			f = target->s.angles[YAW] - ent->s.angles[YAW];

			if (f > 180)
			{
				f = -(360 - f);
			}
			if (f < -180)
			{
				f = -(f + 360);
			}
			if (fabs(f) < 150)
			{
				zc->battlemode = FIRE_CHIKEN;
				zc->battlesubcnt = 5 + (int)(random() * 8);
				trace_priority = TRP_ALLKEEP;
			}
		}
	}
}

void UsePrimaryWeapon(edict_t* ent)
{
	if (CanUsewep(ent, WEAP_BFG))
		return;

	CanUsewep(ent, Bot[ent->client->zc.botindex].param[BOP_PRIWEP]);
}

void UpdateExplIndex(edict_t* ent)
{
	int	i;
	qboolean bmod = false;

	for (i = 0; i < MAX_EXPLINDEX; i++)
	{
		if (ExplIndex[i] != NULL)
		{
			if (ExplIndex[i]->inuse == false)
				ExplIndex[i] = NULL;
		}
		if (!bmod && ExplIndex[i] == NULL)
		{
			ExplIndex[i] = ent;
			bmod = true;
		}
	}
}

#ifdef _WIN32
#pragma warning(pop)
#endif
