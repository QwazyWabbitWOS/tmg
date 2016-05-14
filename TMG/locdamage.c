#include "g_local.h"


/*
 * Most of the main functions are called in g_combat.c, in the T_Damage routine
 */

/*
 * Defines
 */
#define LOC_NONE	0
#define LOC_CHEST	1
#define LOC_HEAD	2
#define LOC_LEGS	3
//#define LOC_STOMAC	4
#define MSG_TIME 4
/*
 *
 * A leg shot is HALF damage
 * A stomac shot is 2/3 damage
 * A chest shot is NORMAL damage
 * An head shot is INSTANT DEATH
 *
 */

/*
 *
 * GetHitLocation
 *
 * Return (int) the type of location
 * related on where the shot
 * was placed
 *
 * Point : impact of where the shot hit
 * Ent : the Target of the shot
 * Mod : what type of gun was used
 *
 */
int GetHitLocation (vec3_t point, edict_t *ent, int mod)
{
	vec3_t hLoc;

	if (!ent->client)
		return LOC_NONE;	//apply on players only

	//check for the weapon type the shot was fired with
	if ((mod == MOD_BLASTER)		||
		(mod == MOD_SHOTGUN)		||
		(mod == MOD_SSHOTGUN)		||
		(mod == MOD_MACHINEGUN)		||
		(mod == MOD_CHAINGUN)		||
		(mod == MOD_HYPERBLASTER)	||
		(mod == MOD_RAILGUN)        ||
	//	(mod == MOD_HOOK)  )         
		(mod == MOD_GRAPPLE))
	{

		VectorSubtract(point, ent->s.origin, hLoc);

		if (ent->maxs[2] <= 4)
		{
			// target is crouching
			if (hLoc[2] > 0)
				return LOC_HEAD;
			else if (hLoc[2] < -6)
				return LOC_LEGS;
		}
		else
		{
			// target is standing
			if (hLoc[2] > 16)
				return LOC_HEAD;
			else if (hLoc[2] < 4)
				return LOC_LEGS;
		}
		return LOC_CHEST;
	}
	else
		return LOC_NONE;
}

/*
 *
 * ApplyLocationalSystem
 *
 * return the amount of damage
 * related on the impact origin of
 * the shot
 *
 */
int ApplyLocationalSystem (edict_t *attacker, edict_t *targ, vec3_t point, int mod, int d_damage)
{
	int loc;
	int damage = d_damage;

	if (!targ->client)
		return d_damage;

	loc = GetHitLocation(point, targ, mod);

	if (loc == LOC_NONE)
		return d_damage;

	switch (loc)
	{
	case LOC_HEAD:
		if (attacker->client)
		{
			if (hstime < level.time && extrasounds->value && mod == MOD_RAILGUN)
			{
				if (rand() & 1)
					gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/headshot.wav"), 1, ATTN_NORM, 0);
				else
					gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/headhunter.wav"), 1, ATTN_NORM, 0);
				hstime = level.time + 10;
			}

			if ((attacker->misc_time[MSG_TIME] < level.time) && (damage_display->value > 0))
			{
				safe_centerprintf (attacker, "You shot %s in the HEAD\n", targ->client->pers.netname);
				attacker->misc_time[MSG_TIME] = level.time + 1;
			}
		}
		damage *= 2;
		break;
	case LOC_LEGS:
		if (attacker->client)
		{
			if ((attacker->misc_time[MSG_TIME] < level.time) && (damage_display->value > 0))


			{	
					safe_centerprintf (attacker, "You shot %s in the Legs\n", targ->client->pers.netname);

				attacker->misc_time[MSG_TIME] = level.time + 1;
			}
		}
		damage *= 0.5;
		break;
	case LOC_CHEST:
		if (attacker->client)
		{
		if ((attacker->misc_time[MSG_TIME] < level.time) && (damage_display->value > 0))

			{
					
					safe_centerprintf (attacker, "You shot %s in the chest !\n", targ->client->pers.netname);

				attacker->misc_time[MSG_TIME] = level.time + 1;
			}
		}
	default:
		return d_damage;
		break;
	}

	return damage;

}


