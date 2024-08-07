// g_weapon.c

#include "g_local.h"
#include "g_items.h"
#include "m_player.h"
#include "runes.h"

static qboolean is_strength;//RAV

qboolean	is_quad;
static byte		is_silenced;

void weapon_grenade_fire(edict_t* ent, qboolean held);

void P_ProjectSource(gclient_t* client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy(distance, _distance);
	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)
		_distance[1] = 0;
	G_ProjectSource(point, _distance, forward, right, result);
}


/**
Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
*/
void PlayerNoise(edict_t* who, vec3_t where, int type)
{
	edict_t* noise = NULL;

	// Make sure ent exists!
	if (!G_EntExists(who))
		return;

	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}

	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet(noise->mins, -8, -8, -8);
		VectorSet(noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy(where, noise->s.origin);
	VectorSubtract(where, noise->maxs, noise->absmin);
	VectorAdd(where, noise->maxs, noise->absmax);
	noise->teleport_time = level.time;
	gi.linkentity(noise);
}


qboolean Pickup_Weapon(edict_t* ent, edict_t* other)
{
	int			index;
	gitem_t* ammo;
	gclient_t* client;

	// Make sure ent exists!
	if (!G_EntExists(other)) return false;


	if (other->client)
		client = other->client;
	else
		return false;


	index = ITEM_INDEX(ent->item);

	if ((((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
			return false;	// leave the weapon for others to pickup
	}

	if ((ent->spawnflags & DROPPED_PLAYER_ITEM) && (ent->item->flags & IT_WEAPON))
	{
		if (!(dmflag & DF_WEAPONS_STAY))
			client->pers.inventory[index]++;
	}
	else
		client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		// give them some ammo with it
		ammo = FindItem(ent->item->ammo);
		if (!(dmflag & DF_INFINITE_AMMO))
		{
			Add_Ammo(other, ammo, ammo->quantity);
		}
		else
		{
			Add_Ammo(other, ammo, 1000);
		}

		if (!(ent->spawnflags & DROPPED_PLAYER_ITEM))
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn(ent, spawn_time->value);//RAV
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

	if (client->pers.weapon != ent->item &&
		(other->client->pers.inventory[index] == 1) &&
		(!deathmatch->value || client->pers.weapon == FindItem("blaster")))
		client->newweapon = ent->item;

	return true;
}

void ShowGun(edict_t* ent)
{
	int nIndex;
	char* pszIcon;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	if (!view_weapons->value)
	{
		ent->s.modelindex2 = 255;
		return;
	}

	if (!ent->client->pers.weapon)
	{
		ent->s.modelindex2 = 0;
		return;
	}

	// New engine support..
	// Determine the weapon's precache index.
	nIndex = 0;
	pszIcon = ent->client->pers.weapon->icon;
	if (strcmp(pszIcon, "w_blaster") == 0)
		nIndex = 1;
	else if (strcmp(pszIcon, "w_shotgun") == 0)
		nIndex = 2;
	else if (strcmp(pszIcon, "w_sshotgun") == 0)
		nIndex = 3;
	else if (strcmp(pszIcon, "w_machinegun") == 0)
		nIndex = 4;
	else if (strcmp(pszIcon, "w_chaingun") == 0)
		nIndex = 5;
	else if (strcmp(pszIcon, "a_grenades") == 0)
		nIndex = 6;
	else if (strcmp(pszIcon, "w_glauncher") == 0)
		nIndex = 7;
	else if (strcmp(pszIcon, "w_rlauncher") == 0)
		nIndex = 8;
	else if (strcmp(pszIcon, "w_hyperblaster") == 0)
		nIndex = 9;
	else if (strcmp(pszIcon, "w_railgun") == 0)
		nIndex = 10;
	else if (strcmp(pszIcon, "w_bfg") == 0)
		nIndex = 11;
	else if (strcmp(pszIcon, "w_grapple") == 0)
		nIndex = 12;

	// Clear previous weapon model.
	ent->s.skinnum &= 255;	// Set new weapon model.
	ent->s.skinnum |= (nIndex << 8);
	ent->s.modelindex2 = 255;

}
// ### Hentai ### END

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon(edict_t* ent)
{
	// Make sure ent exists!
	if (!G_EntExists(ent)) return;


	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire(ent, false);
		ent->client->grenade_time = 0;
	}

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index =
		ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));

	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex =
		gi.modelindex(ent->client->pers.weapon->view_model);

	// ### Hentai ### BEGIN	
	ShowGun(ent);
	// ### Hentai ### END
}

/*
=================
NoAmmoWeaponChange
=================
*/
static void
NoAmmoWeaponChange(edict_t* ent)
{
	gclient_t* client;

	// Make sure ent exists!
	if (!G_EntExists(ent)) return;


	if (ent->client)
		client = ent->client;
	else
		return;

	if (client->pers.inventory[ITEM_INDEX(FindItem("slugs"))]
		&& client->pers.inventory[ITEM_INDEX(FindItem("railgun"))])
	{
		client->newweapon = FindItem("railgun");
		return;
	}
	if (client->pers.inventory[ITEM_INDEX(FindItem("cells"))]
		&& client->pers.inventory[ITEM_INDEX(FindItem("hyperblaster"))])
	{
		client->newweapon = FindItem("hyperblaster");
		return;
	}
	if (client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&& client->pers.inventory[ITEM_INDEX(FindItem("chaingun"))])
	{
		client->newweapon = FindItem("chaingun");
		return;
	}
	if (client->pers.inventory[ITEM_INDEX(FindItem("bullets"))]
		&& client->pers.inventory[ITEM_INDEX(FindItem("machinegun"))])
	{
		client->newweapon = FindItem("machinegun");
		return;
	}
	if (client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&& client->pers.inventory[ITEM_INDEX(FindItem("super shotgun"))])
	{
		client->newweapon = FindItem("super shotgun");
		return;
	}
	if (client->pers.inventory[ITEM_INDEX(FindItem("shells"))]
		&& client->pers.inventory[ITEM_INDEX(FindItem("shotgun"))])
	{
		client->newweapon = FindItem("shotgun");
		return;
	}
	client->newweapon = FindItem("blaster");
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon(edict_t* ent)
{
	// Make sure ent exists!
	if (!G_EntExists(ent)) return;


	// if just died, put the weapon away
	if (ent->health < 1)
	{
		ent->client->newweapon = NULL;
		ChangeWeapon(ent);
	}

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
		is_quad = (ent->client->quad_framenum > level.framenum);
		//RAV
		is_strength = rune_has_rune(ent, RUNE_STRENGTH);
		//

		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;
		ent->client->pers.weapon->weaponthink(ent);
	}
}


/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon(edict_t* ent, gitem_t* item)
{
	int			ammo_index;
	gitem_t* ammo_item;

	// Make sure ent exists!
	if (!G_EntExists(ent)) return;


	// see if we're already using it
	if (item == ent->client->pers.weapon)
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
			if (!ent->bot_client)
				safe_cprintf(ent, PRINT_HIGH,
					"No %s for %s.\n",
					ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
			if (!ent->bot_client)
				safe_cprintf(ent, PRINT_HIGH,
					"Not enough %s for %s.\n",
					ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon(edict_t* ent, gitem_t* item)
{
	int		index;

	// Make sure ent exists!
	if (!G_EntExists(ent)) return;


	if (dmflag & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if (((item == ent->client->pers.weapon) ||
		(item == ent->client->newweapon)) &&
		(ent->client->pers.inventory[index] == 1))
	{
		if (!ent->bot_client)
			safe_cprintf(ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item(ent, item);
	ent->client->pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

static void
Weapon_Generic2(edict_t* ent,
	int FRAME_ACTIVATE_LAST,
	int FRAME_FIRE_LAST,
	int FRAME_IDLE_LAST,
	int FRAME_DEACTIVATE_LAST,
	int* pause_frames,
	int* fire_frames,
	void (*fire)(edict_t* ent))
{
	int		n;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;
	/*
		if (ent->client->weaponstate == WEAPON_DROPPING)
		{

			//if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
			//{
				ChangeWeapon (ent);
				return;
			//}

			//ent->client->ps.gunframe++;
			//return;
		}
	*/
	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		ent->client->ps.gunframe = FRAME_DEACTIVATE_LAST;
		ChangeWeapon(ent);
		return;
#if 0 
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon(ent);
			return;
		}
		ent->client->ps.gunframe++;
		return;
#endif 
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		//if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		//{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = FRAME_IDLE_FIRST;
		return;
		//}

		//ent->client->ps.gunframe++;
		//return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (((ent->client->latched_buttons | ent->client->buttons) & BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) ||
				(ent->client->pers.inventory[ent->client->ammo_index] >=
					ent->client->pers.weapon->quantity))
			{
				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->invincible_framenum = level.framenum;
				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1 - 1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1 - 1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE,
						gi.soundindex("weapons/noammo.wav"),
						1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange(ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand() & 15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				//ZOID
				if (!CTFApplyStrengthSound(ent))
					//ZOID
					if (ent->client->quad_framenum > level.framenum)
						gi.sound(ent, CHAN_ITEM,
							gi.soundindex("items/damage3.wav"),
							1, ATTN_NORM, 0);
				//ZOID
				CTFApplyHasteSound(ent);
				//ZOID
								//RAV
				if (rune_has_rune(ent, RUNE_STRENGTH))
					gi.sound(ent, CHAN_ITEM,
						gi.soundindex("boss3/bs3pain2.wav"),
						0.7f, ATTN_NORM, 0);
				if (rune_has_rune(ent, RUNE_HASTE))
					gi.sound(ent, CHAN_ITEM,
						gi.soundindex("flipper/flppain1.wav"),
						0.7f, ATTN_NORM, 0);

				fire(ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST + 1)
			ent->client->weaponstate = WEAPON_READY;
	}
}

//ZOID
void Weapon_Generic(edict_t* ent,
	int FRAME_ACTIVATE_LAST,
	int FRAME_FIRE_LAST,
	int FRAME_IDLE_LAST,
	int FRAME_DEACTIVATE_LAST,
	int* pause_frames,
	int* fire_frames,
	void (*fire)(edict_t* ent))
{
	weaponstate_t oldstate;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	if (ent->client->pers.pl_state < PL_PLAYING || ent->client->resp.spectator
		|| (ctf->value && ent->client->resp.ctf_team < 1))
		return;

	if (ent->client->newweapon)
	{
		ChangeWeapon(ent);
		return;
	}

	oldstate = ent->client->weaponstate;
	Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,
		FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames,
		fire_frames, fire);

	// run the weapon frame again if hasted
	if (Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
		ent->client->weaponstate == WEAPON_FIRING)
		return;

	if ((CTFApplyHaste(ent) ||
		(Q_stricmp(ent->client->pers.weapon->pickup_name, "Grapple") == 0 &&
			ent->client->weaponstate != WEAPON_FIRING)) && oldstate == ent->client->weaponstate)
	{
		Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames,
			fire_frames, fire);
	}
	if (rune_has_rune(ent, RUNE_HASTE))
	{
		Weapon_Generic2(ent, FRAME_ACTIVATE_LAST, FRAME_FIRE_LAST,
			FRAME_IDLE_LAST, FRAME_DEACTIVATE_LAST, pause_frames,
			fire_frames, fire);
	}
}
//ZOID

/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0f
#define GRENADE_MINSPEED	400.0f
#define GRENADE_MAXSPEED	800.0f

void weapon_grenade_fire(edict_t* ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = (int)g_damage->value;
	float	timer;
	int		speed;
	float	radius;

	radius = damage + 40;
	if (is_quad)
		damage *= 4;

	//DB
	if (is_strength)
		damage *= 2;
	//DB

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	timer = ent->client->grenade_time - level.time;
	speed = (int)(GRENADE_MINSPEED + (GRENADE_TIMER - timer) *
		((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER));

	fire_grenade2(ent, start, forward, damage, speed, timer, radius, held);

	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;

	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_Grenade(edict_t* ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon(ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if (((ent->client->latched_buttons | ent->client->buttons)
			& BUTTON_ATTACK))
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE,
						gi.soundindex("weapons/noammo.wav"),
						1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
				NoAmmoWeaponChange(ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) ||
			(ent->client->ps.gunframe == 34) ||
			(ent->client->ps.gunframe == 39) ||
			(ent->client->ps.gunframe == 48))
		{
			if (rand() & 15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON,
				gi.soundindex("weapons/hgrena1b.wav"),
				1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound =
					gi.soundindex("weapons/hgrenc1b.wav");
			}

			// they waited too long, detonate it in their hand
			if (!ent->client->grenade_blew_up &&
				level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire(ent, true);
				ent->client->grenade_blew_up = true;
			}

			if (ent->client->buttons & BUTTON_ATTACK)
				return;

			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire(ent, false);
		}

		if ((ent->client->ps.gunframe == 15) &&
			(level.time < ent->client->grenade_time))
			return;

		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

static void
weapon_grenadelauncher_fire(edict_t* ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = gl_damage->value;
	float	radius;

	radius = damage + 40;
	if (is_quad)
		damage *= 4;
	//DB
	if (is_strength)
		damage *= 2;
	//DB
	VectorSet(offset, 8, 8, ent->viewheight - 8);
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	P_ProjectSource(ent->client, ent->s.origin,
		offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_grenade(ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_GRENADE | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher(edict_t* ent)
{
	static int	pause_frames[] = { 34, 51, 59, 0 };
	static int	fire_frames[] = { 6, 0 };

	Weapon_Generic(ent, 5, 16, 59, 64,
		pause_frames, fire_frames,
		weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

static void
Weapon_RocketLauncher_Fire(edict_t* ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius;
	int		radius_damage;

	damage = rl_damage->value + (int)(random() * 20.0);
	radius_damage = rl_radius_damage->value;;
	damage_radius = rl_radius->value;;
	if (is_quad)
	{
		damage *= 4;
		radius_damage *= 4;
	}
	//DB
	if (is_strength)
		damage *= 2;
	//DB
	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin,
		offset, forward, right, start);
	fire_rocket(ent, start, forward, damage,
		rocket_speed->value, damage_radius, radius_damage);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_ROCKET | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_RocketLauncher(edict_t* ent)
{
	static int	pause_frames[] = { 25, 33, 42, 50, 0 };
	static int	fire_frames[] = { 5, 0 };

	Weapon_Generic(ent, 4, rocket_wait->value, 50, 54,
		pause_frames, fire_frames,
		Weapon_RocketLauncher_Fire);
}


/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

static void
Blaster_Fire(edict_t* ent,
	vec3_t g_offset,
	int damage,
	qboolean hyper,
	int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;

	if (is_quad)
		damage *= 4;

	//DB
	if (is_strength)
		damage *= 2;
	//DB
	AngleVectors(ent->client->v_angle, forward, right, NULL);
	VectorSet(offset, 24, 8, ent->viewheight - 8);
	VectorAdd(offset, g_offset, offset);
	P_ProjectSource(ent->client, ent->s.origin,
		offset, forward, right, start);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_blaster(ent, start, forward, damage, 1000, effect, hyper);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	if (hyper)
		gi.WriteByte(MZ_HYPERBLASTER | is_silenced);
	else
		gi.WriteByte(MZ_BLASTER | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	PlayerNoise(ent, start, PNOISE_WEAPON);
}


static void
Weapon_Blaster_Fire(edict_t* ent)
{
	int		damage;

	if (deathmatch->value)
		damage = b_damage->value;
	else
		damage = 10;

	Blaster_Fire(ent, vec3_origin, damage, false, EF_BLASTER);
	ent->client->ps.gunframe++;
}

void Weapon_Blaster(edict_t* ent)
{
	static int	pause_frames[] = { 19, 32, 0 };
	static int	fire_frames[] = { 5, 0 };

	Weapon_Generic(ent, 4, 8, 52, 55,
		pause_frames, fire_frames,
		Weapon_Blaster_Fire);
}


static void
Weapon_HyperBlaster_Fire(edict_t* ent)
{
	float	rotation;
	vec3_t	offset;
	int		effect;
	int		damage;

	ent->client->weapon_sound = gi.soundindex("weapons/hyprbl1a.wav");
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (!ent->client->pers.inventory[ent->client->ammo_index])
		{
			if (level.time >= ent->pain_debounce_time)
			{
				gi.sound(ent, CHAN_VOICE,
					gi.soundindex("weapons/noammo.wav"),
					1, ATTN_NORM, 0);

				ent->pain_debounce_time = level.time + 1;
			}
			NoAmmoWeaponChange(ent);
		}
		else
		{
			rotation = (ent->client->ps.gunframe - 5.0) * 2.0 * M_PI / 6.0;
			offset[0] = -4 * sin(rotation);
			offset[1] = 0;
			offset[2] = 4 * cos(rotation);

			if ((ent->client->ps.gunframe == 6) ||
				(ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			if (deathmatch->value)
				damage = hb_damage->value;
			else
				damage = 20;
			Blaster_Fire(ent, offset, damage, true, effect);
			if (!(dmflag & DF_INFINITE_AMMO))
				ent->client->pers.inventory[ent->client->ammo_index]--;
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 &&
			ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
		gi.sound(ent, CHAN_AUTO,
			gi.soundindex("weapons/hyprbd1a.wav"),
			1, ATTN_NORM, 0);

		ent->client->weapon_sound = 0;
	}

}

void Weapon_HyperBlaster(edict_t* ent)
{
	static int	pause_frames[] = { 0 };
	static int	fire_frames[] = { 6, 7, 8, 9, 10, 11, 0 };

	Weapon_Generic(ent, 5, 20, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

static void
Machinegun_Fire(edict_t* ent)
{
	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		angles;
	int			damage = mg_damage->value;
	int			kick = mg_kick->value;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE,
				gi.soundindex("weapons/noammo.wav"),
				1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange(ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	//DB
	if (is_strength)
	{
		damage *= 2;
		kick *= 2;
	}
	//DB
	for (i = 1; i < 3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}
	ent->client->kick_origin[0] = crandom() * 0.35;
	ent->client->kick_angles[0] = ent->client->machinegun_shots * -1.5;

	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
	VectorAdd(ent->client->v_angle, ent->client->kick_angles, angles);
	AngleVectors(angles, forward, right, NULL);
	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	fire_bullet(ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD,
		DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);

	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_MACHINEGUN | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Machinegun(edict_t* ent)
{
	static int	pause_frames[] = { 23, 45, 0 };
	static int	fire_frames[] = { 4, 5, 0 };

	Weapon_Generic(ent, 3, 5, 45, 49,
		pause_frames, fire_frames,
		Machinegun_Fire);
}

static void
Chaingun_Fire(edict_t* ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = cg_kick->value;

	if (deathmatch->value)
		damage = cg_damage->value;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO,
			gi.soundindex("weapons/chngnu1a.wav"),
			1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) &&
		!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) &&
		(ent->client->buttons & BUTTON_ATTACK) &&
		ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO,
			gi.soundindex("weapons/chngnd1a.wav"),
			1, ATTN_IDLE, 0);
	}
	else
	{
		//		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE,
				gi.soundindex("weapons/noammo.wav"),
				1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
		NoAmmoWeaponChange(ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	//DB
	if (is_strength)
	{
		damage *= 2;
		kick *= 2;
	}
	//DB
	for (i = 0; i < 3; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i = 0; i < shots; i++)
	{
		// get start / end positions
		AngleVectors(ent->client->v_angle, forward, right, up);
		r = 7 + crandom() * 4;
		u = crandom() * 4;
		VectorSet(offset, 0, r, u + ent->viewheight - 8);
		P_ProjectSource(ent->client,
			ent->s.origin, offset, forward, right, start);

		fire_bullet(ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD,
			DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun(edict_t* ent)
{
	static int	pause_frames[] = { 38, 43, 51, 61, 0 };
	static int	fire_frames[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0 };

	Weapon_Generic(ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

static void
weapon_shotgun_fire(edict_t* ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = sg_damage->value;
	int			kick = sg_kick->value;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	//DB
	if (is_strength)
		damage *= 2;

	if (deathmatch->value)
		fire_shotgun(ent, start, forward, damage, kick, 500, 500,
			DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun(ent, start, forward, damage, kick, 500, 500,
			DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_SHOTGUN | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun(edict_t* ent)
{
	static int	pause_frames[] = { 22, 28, 34, 0 };
	static int	fire_frames[] = { 8, 9, 0 };

	Weapon_Generic(ent, 7, 18, 36, 39,
		pause_frames, fire_frames,
		weapon_shotgun_fire);
}


static void
weapon_supershotgun_fire(edict_t* ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = ssg_damage->value;
	int			kick = ssg_kick->value;

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8, ent->viewheight - 8);
	P_ProjectSource(ent->client,
		ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}
	//DB
	if (is_strength)
		damage *= 2;
	//DB

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW] = ent->client->v_angle[YAW] - 5;
	v[ROLL] = ent->client->v_angle[ROLL];
	AngleVectors(v, forward, NULL, NULL);
	fire_shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD,
		DEFAULT_SHOTGUN_VSPREAD,
		DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);
	v[YAW] = ent->client->v_angle[YAW] + 5;
	AngleVectors(v, forward, NULL, NULL);
	fire_shotgun(ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD,
		DEFAULT_SHOTGUN_VSPREAD,
		DEFAULT_SSHOTGUN_COUNT / 2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_SSHOTGUN | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun(edict_t* ent)
{
	static int	pause_frames[] = { 29, 42, 57, 0 };
	static int	fire_frames[] = { 7, 0 };

	Weapon_Generic(ent, 6, sshotgun_wait->value, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

static void
weapon_railgun_fire(edict_t* ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;
	//RAV
	if (ent->client->pers.pl_state == PL_CHEATBOT)
		return;

	if (ent->client->pers.pl_state < PL_PLAYING || ent->client->resp.spectator
		|| (ctf->value && ent->client->resp.ctf_team < 1))
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		ent->client->pers.pl_state = PL_SPECTATOR;
		gi.linkentity(ent);
		ent->client->resp.spectator = true;
		return;
	}

	//
	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = rg_damage->value;
		kick = rg_kick->value;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}//DB
	if (is_strength)
	{
		damage *= 2;
		kick *= 2;
	}

	//QW// Count shots
	ent->client->resp.shots++;

	//DB
	//RAV
	if (voosh->value)
	{
		damage = (int)raildamage->value;
		kick = (int)railkick->value;
	}
	//
	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail(ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte(svc_muzzleflash);
	gi.WriteShort(ent - g_edicts);
	gi.WriteByte(MZ_RAILGUN | is_silenced);
	gi.multicast(ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		if (voosh->value == 0)//RAV
			ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun(edict_t* ent)
{
	static int	pause_frames[] = { 56, 0 };
	static int	fire_frames[] = { 4, 0 };
	//RAV
	if (voosh->value)
		Weapon_Generic(ent, 3, railwait->value, 56, 61,
			pause_frames, fire_frames,
			weapon_railgun_fire);
	else
		//
		Weapon_Generic(ent, 3, railgun_wait->value, 56, 61,
			pause_frames, fire_frames,
			weapon_railgun_fire);

}


/*
======================================================================

BFG10K

======================================================================
*/

static void
weapon_bfg_fire(edict_t* ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = bfg_damage->value;
	else
		damage = 500;

	VectorCopy(ent->s.origin, start);

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte(svc_muzzleflash);
		gi.WriteShort(ent - g_edicts);
		gi.WriteByte(MZ_BFG | is_silenced);
		gi.multicast(ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;
	//DB
	if (is_strength)
		damage *= 2;
	//DB

	AngleVectors(ent->client->v_angle, forward, right, NULL);

	VectorScale(forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
	ent->client->v_dmg_pitch = -40;
	ent->client->v_dmg_roll = crandom() * 8;
	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight - 8);
	P_ProjectSource(ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg(ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);
	//RAV
		//allow for camper checking
	ent->client->check_camping = true;
	//
	if (!(dmflag & DF_INFINITE_AMMO))
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG(edict_t* ent)
{
	static int	pause_frames[] = { 39, 45, 50, 55, 0 };
	static int	fire_frames[] = { 9, 17, 0 };

	Weapon_Generic(ent, 8, bfg_wait->value, 55, 58,
		pause_frames, fire_frames,
		weapon_bfg_fire);
}


//======================================================================
