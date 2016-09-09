#include "g_local.h"
#include "g_items.h"
#include "anticheat.h"
#include "bot.h"
#include "filehand.h"
#include "g_cmds.h"
#include "m_player.h"
#include "p_client.h"
#include "g_chase.h"
#include "s_map.h"
#include "e_hook.h"
#include "timer.h"
#include "p_hud.h"
#include "filtering.h"
#include "hud.h"
#include "runes.h"
#include "maplist.h"

int LIGHTS = 0;
void no (edict_t *ent);
void yes (edict_t *ent);//RAV

char *ClientTeam1 (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	if (!G_EntExists(ent))
		return value;


	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');

	if (!p)
		return value;

	//jsw	if ((int)(dmflags->value) & DF_MODELTEAMS)
	if (dmflag & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

//RAV
int ClientTeam (edict_t *ent)
{

	if (!G_EntExists(ent))
		return 0;

	if  (ent->client->resp.ctf_team < 3
	  && ent->client->resp.ctf_team > 0)
		return	ent->client->resp.ctf_team;

	return 0;
}

int ClientPermTeam (edict_t *ent)
{
	if(ctf->value)
		return ent->client->resp.ctf_team;
	else
		return 0;
}

//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	//RAV
	if (!ent1->client || !ent2->client)
		return false;

	if(!ctf->value)
	{
		if (!(dmflag & (DF_MODELTEAMS | DF_SKINTEAMS | DF_NO_FRIENDLY_FIRE)))
			return false;

		strcpy (ent1Team, ClientTeam1 (ent1));
		strcpy (ent2Team, ClientTeam1 (ent2));

		if (strcmp(ent1Team, ent2Team) == 0)
			return true;
	}
	else
	{
		//RAV
		if (ClientTeam(ent1) == ClientTeam(ent2))
			return true;
	}
	return false;
}

void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Next(ent);
		return;
	} else if (cl->chase_target) {
		ChaseNext(ent);
		return;
	}
//ZOID

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}

void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

//ZOID
	if (cl->menu) {
		PMenu_Prev(ent);
		return;
	} else if (cl->chase_target) {
		ChasePrev(ent);
		return;
	}
//ZOID

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}



void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}


//=============================================================================

static void MustSetCheats(edict_t *ent)
{
	safe_cprintf (ent, PRINT_HIGH,
	"You must run the server with '+set cheats 1' to enable this command.\n");
}

static void NoAccess(edict_t *ent)
{
	safe_cprintf(ent, PRINT_HIGH,
				 "You do not have access to this command.\n");
}

/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void Cmd_Give_f (edict_t *ent)
{
	char		*name;
	gitem_t		*it;
	int			index;
	int			i;
	qboolean	give_all;
	edict_t		*it_ent;

	if (deathmatch->value && !sv_cheats->value)
	{
		MustSetCheats(ent);
		return;
	}

	name = gi.args();

	if (Q_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if (give_all || Q_stricmp(gi.argv(1), "health") == 0)
	{
		if (gi.argc() == 3)
			ent->health = atoi(gi.argv(2));
		else
			ent->health = ent->max_health;
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "weapons") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_WEAPON))
				continue;
			ent->client->pers.inventory[i] += 1;
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "ammo") == 0)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (!(it->flags & IT_AMMO))
				continue;
			Add_Ammo (ent, it, 1000);
		}
		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = FindItem("Jacket Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Combat Armor");
		ent->client->pers.inventory[ITEM_INDEX(it)] = 0;

		it = FindItem("Body Armor");
		info = (gitem_armor_t *)it->info;
		ent->client->pers.inventory[ITEM_INDEX(it)] = info->max_count;

		if (!give_all)
			return;
	}

	if (give_all || Q_stricmp(name, "Power Shield") == 0)
	{
		it = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);

		if (!give_all)
			return;
	}

	if (give_all)
	{
		for (i=0 ; i<game.num_items ; i++)
		{
			it = itemlist + i;
			if (!it->pickup)
				continue;
			if (it->flags & (IT_ARMOR|IT_WEAPON|IT_AMMO))
				continue;
			ent->client->pers.inventory[i] = 1;
		}
		return;
	}

	it = FindItem (name);
	if (!it)
	{
		name = gi.argv(1);
		it = FindItem (name);
		if (!it)
		{
			gi.dprintf ("unknown item\n");
			return;
		}
	}

	if (!it->pickup)
	{
		gi.dprintf ("non-pickup item\n");
		return;
	}

	index = ITEM_INDEX(it);

	if (it->flags & IT_AMMO)
	{
		if (gi.argc() == 3)
			ent->client->pers.inventory[index] = atoi(gi.argv(2));
		else
			ent->client->pers.inventory[index] += it->quantity;
	}
	else
	{
		it_ent = G_Spawn();
		it_ent->classname = it->classname;
		SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
}


/*
==================
Cmd_God_f

Sets client to godmode

argv(0) god
==================
*/
void Cmd_God_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		MustSetCheats(ent);
		return;
	}

	ent->flags ^= FL_GODMODE;
	if (!(ent->flags & FL_GODMODE) )
		msg = "godmode OFF\n";
	else
		msg = "godmode ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void Cmd_Notarget_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		MustSetCheats(ent);
		return;
	}

	ent->flags ^= FL_NOTARGET;
	if (!(ent->flags & FL_NOTARGET) )
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f (edict_t *ent)
{
	char	*msg;

	if (deathmatch->value && !sv_cheats->value)
	{
		MustSetCheats(ent);
		return;
	}

	if (ent->movetype == MOVETYPE_NOCLIP)
	{
		ent->movetype = MOVETYPE_WALK;
		msg = "noclip OFF\n";
	}
	else
	{
		ent->movetype = MOVETYPE_NOCLIP;
		msg = "noclip ON\n";
	}

	safe_cprintf (ent, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
/*
// ERASER, only enable grapple if calc_nodes = 0
	if (bot_calc_nodes->value && !strcmp(s, "grapple"))
	{
		safe_cprintf (ent, PRINT_HIGH, "Grapple not available while bot_calc_nodes = 1\n");
		return;
	}
*/
	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

//RAV
	if(voosh->value)
	{
		if (Q_stricmp(gi.args(), "slugs") == 0)
		{
			safe_cprintf (ent, PRINT_HIGH, "Sorry you can't drop ammo in railwarz\n");
			return;
		}
	}

	if (Q_stricmp(gi.args(), "flag") == 0)
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Red Flag"))])
		{
			it = FindItem ("Red Flag");
			it->drop (ent, it);
			return;
		}
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Blue Flag"))])
		{
			it = FindItem ("Blue Flag");
			it->drop (ent, it);
			return;
		}
	}

	if (Q_stricmp(gi.args(), "ammo") == 0
		&& ent->client->pers.weapon
		&& ent->client->pers.weapon->ammo
		&& (!voosh->value))
	{
		it = FindItem (ent->client->pers.weapon->ammo);
		it->drop (ent, it);
		return;
	}

	s = gi.args();
	if ((Q_stricmp(s, "rune")==0)|| (Q_stricmp(s, "tech")==0))
	{
		if (!CTFWhat_Tech(ent))
		{
			runes_drop(ent);
			return;
		}
	}
//


//ZOID--special case for tech powerups
	if (Q_stricmp(gi.args(), "tech") == 0
		&& (it = CTFWhat_Tech(ent)) != NULL)
	{
		it->drop (ent, it);
		return;
	}
//ZOID

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		safe_cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		safe_cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}

/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{

	if (ent->client->menu)
	{
		PMenu_Close(ent);
		ent->client->update_chase = true;
		return;
	}

	if (ent->client->showinventory)
	{
		ent->client->showinventory = false;
		return;
	}

	if(mapvote->value && votetime > level.time && maplist->nextmap == -1)
	{
		stuffcmd(ent, "map_vote\n");
		return;
	}

	if (ctf->value)// && cl->resp.ctf_team == CTF_NOTEAM)
	{
		CTFOpenJoinMenu(ent);
		return;
	}
	else
	{
		OpenJoinMenu(ent);
		return;
	}
}

/*
=================
Cmd_InvUse_f
=================
*/
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

//ZOID
	if (ent->client->menu) {
		PMenu_Select(ent);
		return;
	}
//ZOID

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		safe_cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}

//ZOID
/*
=================
Cmd_LastWeap_f
=================
*/
void Cmd_LastWeap_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	cl->pers.lastweapon->use (ent, cl->pers.lastweapon);
}
//ZOID

/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	cl = ent->client;

	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapLast_f
=================
*/
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}

/*
=================
Cmd_InvDrop_f
=================
*/
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

  // Not available to dead or respawning players!
  if (!G_ClientInGame(ent)) return;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		safe_cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		safe_cprintf (ent,
					  PRINT_HIGH,
					  "Item %s is not droppable.\n", it->pickup_name);
		return;
	}
	it->drop (ent, it);
}


/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	// Not available to dead or respawning players!
	if (!G_ClientInGame(ent))
		return;

	//JSW
	if (nokill->value)
		return;
	//end
	
	//ZOID
	if (ent->solid == SOLID_NOT)
		return;
	//ZOID
	
	if((level.time - ent->client->respawn_time) < 5)
		return;
	
	//Hook bug fix  (duncan alerted)
	if (ent->client->hook || ent->client->ctf_grapple)
	{
		my_bprintf(PRINT_HIGH, "%s is trying to spam the server with HOOKS\n"
			"and was disconnected from the server\n",
			ent->client->pers.netname);
		stuffcmd(ent, "disconnect;error \"You have been disconnected for "
			"trying to die or switch teams while hooking, "
			"which will crash the server. "
			"Multiple attempts at this will result in a ban.\"");
		return;
	}

	if ((int)punish_suicide->value & PS_FORCESPEC)
	{
		ent->client->pers.pl_state = PL_SPECTATOR;
		ent->client->resp.spectator = 1;
		if (ent->client->menu)
			PMenu_Close(ent);
		Spectate (ent, NULL);
		return;
	}

	//JSW
	ent->client->kill = 1;
	//end
	ent->flags &= ~FL_GODMODE;
	ent->health = 0;
	meansOfDeath = MOD_SUICIDE;
	player_die (ent, ent, ent, 100000, vec3_origin);
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent,false);
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
//ZOID
	if (ent->client->menu)
		PMenu_Close(ent);
	ent->client->update_chase = true;
//ZOID
}

int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	safe_cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}

/*
//JSW
=================
Cmd_Operators_f
=================
*/
void Cmd_Operators_f (edict_t *ent)
{
	int		i, j;
	edict_t	*player;

	j = 1;
	for (i = 1; i <= maxclients->value; i++)
	{
		player = g_edicts + i;
		if (player && player->inuse && !player->bot_client)
		{
			if (player->client->pers.isop)
			{
				safe_cprintf(ent, PRINT_HIGH,
					"%d. %s  %d\n",
					j, player->client->pers.netname,
					player->client->pers.oplevel);
				j++;
			}
		}
	}
}

void MapVoteThink(qboolean passed, qboolean now)
{
	char string[1024];
	edict_t		*ent;
	edict_t		*player;
	int			i;

	if (passed)
	{
		if (now)
		{
			my_bprintf (PRINT_CHAT,
				"Immediate map change to %s \"%s\" has passed.\n",
				maplist->mapname[maplist->currentmapvote],
				maplist->mapnick[maplist->currentmapvote]);
			ent = G_Spawn ();
			ent->classname = "target_changelevel";
			ent->map = maplist->mapname[maplist->currentmapvote];
			if (Maplist_CheckFileExists(ent->map))
				BeginIntermission (ent);
			else
			{
				gi.bprintf(PRINT_CHAT,"Map %s does not exist on server, reverting to last map.\n", ent->map);
				ent->map = level.mapname;
				votetime = 0;
				BeginIntermission (ent);
			}
			mapvoteactive = false;
		}
		else
		{
			my_bprintf (PRINT_CHAT,
				"End of level map change to %s \"%s\" has passed.\n",
				maplist->mapname[maplist->currentmapvote],
				maplist->mapnick[maplist->currentmapvote]);
			maplist->nextmap = maplist->currentmapvote;
			mapvoteactive = false;
		}
	}
	else
	{
		if (mapvoteactive == false)
		{
			if (now)
			{
				votemapnow = true;
				sprintf(string,
					"An immediate mapvote has been started for %s \"%s\". Type 'YES' to vote yes.\n",
					maplist->mapname[maplist->currentmapvote],
					maplist->mapnick[maplist->currentmapvote]);
			}
			else
			{
				votemapnow = false;
				sprintf(string,
					"An end of level mapvote has been started for %s \"%s\". Type 'YES' to vote yes.\n",
					maplist->mapname[maplist->currentmapvote],
					maplist->mapnick[maplist->currentmapvote]);
			}
			convert_string(string, 0, 127, 128, string); // white -> green
			my_bprintf(PRINT_HIGH, string);
			mapvoteactive = true;
			mapvotetime = level.time + vote_timeout->value;

			//			gi.dprintf ("D. map to be voted on is %s %s\n",
			//						maplist->mapname[maplist->currentmapvote],
			//						maplist->mapnick[maplist->currentmapvote]);
		}
		else
		{
			sprintf(string,
				"Mapvote for %s \"%s\" has failed.\n",
				maplist->mapname[maplist->currentmapvote],
				maplist->mapnick[maplist->currentmapvote]);

			for (i = 1; i <= maxclients->value; i++)
			{
				player = g_edicts + i;
				if (player && player->inuse && !player->bot_client)
				{
					player->client->resp.vote = false;
				}
			}
			convert_string(string, 0, 127, 128, string); // white -> green
			my_bprintf(PRINT_HIGH, string);
			mapvoteactive = false;
			//			gi.dprintf ("E. map to be voted on is %s\n",
			//						maplist->mapname[maplist->currentmapvote]);
		}
	}
}

void Cmd_MapVote (edict_t *ent)
{
	int i;
	qboolean valid = false;
	if (mapvoteactive)
	{
		safe_cprintf(ent, PRINT_HIGH,
					 "A mapvote for %s \"%s\" is already in progress.\n",
					 maplist->mapname[maplist->currentmapvote],
					 maplist->mapnick[maplist->currentmapvote]);
		return;
	}
	if (gi.argc() < 2)
	{
		safe_cprintf(ent, PRINT_HIGH,
			"Enter a map name such as 'mapvote q2ctf1' to start a mapvote.\n");

		for (i = 0; i < maplist->nummaps; i++)
		{
			safe_cprintf(ent, PRINT_HIGH,
						 "%s - '%s'   \n",
						 maplist->mapname[i], maplist->mapnick[i]);
		}
		return;
	}
	if (level.time+(int)menutime->value-1 < votetime)
		return;

	for (i = 0; i < maplist->nummaps; i++)
	{
//		gi.dprintf ("strcmp returned %d\n",
//					strcmp(maplist->mapname[i], gi.argv(1)));
		if (strcmp(maplist->mapname[i], gi.argv(1)) == 0)
		{
//			gi.dprintf ("G. map to be voted on is %s %s\n",
//						maplist->mapname[i], maplist->mapnick[i]);
			valid = true;
			maplist->currentmapvote = i;
//			gi.dprintf ("maplist->currentmapvote index is %d, i is %d\n",
//						maplist->currentmapvote, i);
		}
	}
	if (!valid)
	{
		safe_cprintf(ent, PRINT_HIGH,
					 "That map is not available to vote on.\n");
		return;
	}
	ent->client->resp.vote = true;
	MapVoteThink(false, true);
//	gi.dprintf ("A. map to be voted on is %s %s\n",
//				maplist->mapname[maplist->currentmapvote],
//				maplist->mapnick[maplist->currentmapvote]);
}
//end

// server side skinlist command to allow users to get
// skins list from server when using clients like r1q2 that
// kill the skins command. //QW//
void Cmd_SkinList_f(edict_t *ent)
{
	int		i;
	char	*skin, *name, string[64];
	edict_t	*edict;
	
	if (!level.intermissiontime) 
	{
		// make it all look nice
		gi.cprintf (ent,
					PRINT_HIGH,
					"\nnum name             skin");
		gi.cprintf (ent,
					PRINT_HIGH,
					"\n--- ---------------- ---------------------\n");
		for (i=0, edict=g_edicts + 1 + i; i < maxclients->value; i++, edict++)
		{
			if (!edict->inuse) 
				continue;
			skin = Info_ValueForKey(edict->client->pers.userinfo, "skin");
			name = Info_ValueForKey(edict->client->pers.userinfo, "name");
			sprintf (string,"%3i %16s %s\n", i, name, skin);
			gi.cprintf (ent, PRINT_HIGH, string);
		}
		sprintf (string, "\n");
		gi.cprintf (ent, PRINT_HIGH, string);
	}
}

/*
=================
Cmd_Wave_f
=================
*/
void Cmd_Wave_f (edict_t *ent)
{
	int		i;

	i = atoi (gi.argv(1));

  // Not available to dead or respawning players!
  if (!G_ClientInGame(ent)) return;

	// can't wave when ducked
	if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
		return;

	if (ent->client->anim_priority > ANIM_WAVE)
		return;

	ent->client->anim_priority = ANIM_WAVE;

	switch (i)
	{
	case 0:
		safe_cprintf (ent, PRINT_HIGH, "flipoff\n");
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		safe_cprintf (ent, PRINT_HIGH, "salute\n");
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		safe_cprintf (ent, PRINT_HIGH, "taunt\n");
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
		break;
	case 3:
		safe_cprintf (ent, PRINT_HIGH, "wave\n");
		ent->s.frame = FRAME_wave01-1;
		ent->client->anim_end = FRAME_wave11;
		break;
	case 4:
	default:
		safe_cprintf (ent, PRINT_HIGH, "point\n");
		ent->s.frame = FRAME_point01-1;
		ent->client->anim_end = FRAME_point12;
		break;
	}
}

void Say_Op(edict_t *who, char *msg)
{
	char outmsg[1024];
	int i;
	char *p;
	edict_t *cl_ent;
	char msg2[1024];
	outmsg[0] = 0;
	
	if (*msg == '\"')
	{
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++)
	{
			*p++ = *msg;
	}

	*p = 0;

	sprintf(msg2, "<OPCHAT> %s: %s\n", who->client->pers.netname, outmsg);
	if (dedicated->value)
	{
		gi.dprintf(msg2);
	}
	if (log_chat->value)
	{
		LogChat(msg2);
	}
	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		//if (cl_ent->client->pers.isop || cl_ent->client->pers.ismop)
		if (cl_ent->client->pers.oplevel & OP_SAY)
		{
			if (!cl_ent->bot_client)
				safe_cprintf(cl_ent, PRINT_CHAT, msg2);
		}
	}
}

void Cmd_ShowVotes_f(edict_t *ent)
{
	int i;
	for (i = 0; i < maplist->nummaps; ++i)
		safe_cprintf (ent, PRINT_HIGH, "%d. %s (%d votes)\n",
		   i, maplist->mapname[i], maplist->votes[i]);
}

/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	//DB
	int		i;
	//DB
	edict_t	*other;
	char	*p;
	char	text[2048];
	char	text2[2048];
	//JSW
	qboolean action = 0;
	char *separator;
	int place;
	char *pdest;
	int  result;
	char banned[64] = "sv ban ";
	char *name;
	//end

	name = ent->client->pers.netname;

//	gi.dprintf("DEBUG: netname is .%s. name is .%s.\n",
//			   ent->client->pers.netname, name);

	//RAV
	//turn off camping if chatting
	ent->client->check_camping = false;
	//

	if(ent->is_muted)
		return;
	
//	if (level.intermissiontime && level.time > level.intermissiontime + 6.0)
  //      return;
	
	if (gi.argc () < 2 && !arg0)
		return;
	else
	{
		//JSW
		if (Q_stricmp (gi.argv(1), "/me") == 0
			|| (arg0 && (Q_stricmp (gi.argv(0), "/me") == 0)))
		{
			Com_sprintf (text, sizeof(text),
						 "%s ", ent->client->pers.netname);
			action = true;
		}
		if (!action && (strstr(gi.argv(1), "/me")))
		{
			pdest = strstr( gi.argv(1), "/me" );
			result = pdest - gi.argv(1) + 1;
			if( pdest != NULL && result < 2)
			{
				Com_sprintf (text, sizeof(text),
							 "%s ", ent->client->pers.netname);
				action = true;
			}
		}
		if (!action && ent->client->pers.saytype == 0)
		{
			if (!team)
				Com_sprintf (text, sizeof(text),
							 "%s: ", ent->client->pers.netname);
			else
				Com_sprintf (text, sizeof(text),
							 "(%s): ", ent->client->pers.netname);
		}
		else if (ent->client->pers.saytype == 1)
			Com_sprintf (text, sizeof(text),
						 "<OPCHAT> %s: ", ent->client->pers.netname);
		else if (ent->client->pers.saytype == 2)
			Com_sprintf (text, sizeof(text),
						 "Message from server op %s:\n\n",
						 ent->client->pers.netname);
		else if (ent->client->pers.saytype == 3)
		{
			Com_sprintf(buffer, sizeof buffer,
						"rcon_password \"\";rcon %s %s\n",
						rcon->string, gi.args());
			stuffcmd (ent, va(buffer));
//			Com_sprintf (text, sizeof(text), "rcon %s %s",
//						 rcon_password->value, gi.args());
			ent->client->pers.saytype = 0;
			return;
		}
		else
			gi.dprintf("Say type unknown!\n");
		//end
	}

	if (arg0 && !action)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	//JSW
	else if (action)
	{
		p = gi.args();
		if (*p == '"' || *p == ':')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		if (strlen(p) <= 3)
			return;
		if (arg0)
		{
			strcat (text, p);
		}
		else
		{
			separator = strchr(p, ' ');
			if (!separator)
				return;	// not found
			place = separator - p + 1;
			strcat (text, p + place);
		}
	}
	//end
	else
	{
		p = gi.args();
		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}
	
	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;
	
	strcat(text, "\n");
	
	//q2 crashbug fix on the exe not liking the % cmd)
	if ( strstr(text, "%" )&& !team)
	{
		safe_cprintf(ent, PRINT_HIGH,
					 "Use messagemode2 for messages with 'info' chars\n");
		return;
	}

	//log and ban anyone trying to get rcon password
	if (strstr (text, "$rcon_password"))
	{
		strcat(banned, GetIp(ent));
		gi.AddCommandString(banned);
		stuffcmd(ent, "disconnect; error \"You have been banned for attempting to get the server's rcon password. Other servers will be notified of your attempt as well.\"");
		my_bprintf(PRINT_HIGH, "%s %s has been BANNED for trying to expose the server's rcon password!\n", ent->client->pers.netname, GetIp(ent));
	}

	//JSW
	if (ent->client->pers.saytype == 1)
	{
		gi.dprintf(text);
		LogChat(text);
		for (i = 0; i < maxclients->value; i++)
		{
			other = g_edicts + 1 + i;
			if (!other->inuse)
				continue;
			//if (other->client->pers.isop || other->client->pers.ismop)
			if (other->client->pers.oplevel & OP_SAY)
			{
				if (!other->bot_client)
					safe_cprintf(other, PRINT_CHAT, "%s", text);
			}
		}
		//ent->client->pers.saytype = 0;
		return;
	}
/*	FIXMEJSW: redo this to use multiple lines, like sv msg, write it's own function
	if (ent->client->pers.saytype == 2)
	{
		convert_string(text, 0, 127, 128, text); // white -> green
		for (i = 0; i < maxclients->value; i++)
		{
			other = g_edicts + 1 + i;
			if (!other->inuse)
				continue;
			if (!other->bot_client)
					safe_centerprintf(other, "%s\n", text);
		}
		ent->client->pers.saytype = 0;
		return;
	}
*/
	//end

	if (flood_msgs->value)
	{
		if (!CheckFlood(ent))
			return;
	}
	
	if(ctf->value)
	{
		//raven adds prefix to chat message in dm/ctf
		//JSW
		if (action)
		{
			sprintf(text2, "%s", text);
		}
		//end
		else if ((ctf->value) && (ent->client->resp.ctf_team == CTF_TEAM1))
		{
			//JSW	if(ent->client->pers.isop==1)
			if(ent->client->pers.isop)//level != OP_NAMEPASS && ent->client->pers.oplevel != 0)
				sprintf(text2, "<OP_RED> %s", text);
			else
				sprintf(text2, "<RED> %s", text);
		}
		else if ((ctf->value) && (ent->client->resp.ctf_team == CTF_TEAM2))
		{
			//JSW	if(ent->client->pers.isop==1)
			if(ent->client->pers.isop)//oplevel != OP_NAMEPASS && ent->client->pers.oplevel != 0)
				sprintf(text2, "<OP_BLUE> %s", text);
			else
				sprintf(text2, "<BLUE> %s", text);
		}
		else if (ent->client->pers.pl_state == PL_SPECTATOR)
		{
			if(ent->client->pers.oplevel != OP_NAMEPASS && ent->client->pers.oplevel != 0)
				sprintf(text2, "<OP_SPEC> %s", text);
			else
				sprintf(text2, "<SPEC> %s", text);
		}
		else
		{
			if(ent->client->pers.isop)
				sprintf(text2, "<OP> %s", text);
			else
				sprintf(text2, "%s", text);
		}
	}
	else
	{
		if (ent->client->pers.pl_state == PL_SPECTATOR)
		{
			if(ent->client->pers.oplevel != OP_NAMEPASS && ent->client->pers.oplevel != 0)
				sprintf(text2, "<OP_SPEC> %s", text);
			else
				sprintf(text2, "<SPEC> %s", text);
		}
		else
		{
			if(ent->client->pers.isop)
				sprintf(text2, "<OP> %s", text);
			else
				sprintf(text2, "%s", text);
		}
	}
	
	if (dedicated->value) //JSW added flood, moved logging to allow
	{											// ops to speak even when muted
		gi.dprintf("%s", text2);
		if(log_chat->value)
		{
			LogChat(text2);
		}
	}

	if(g_filter->value) // this is for text filtering out any unwanted text (textfilter.cfg)
	{
		if (FilterText(text2))
			gi.dprintf("(^^^^filtered^^^^filtered^^^^filtered^^^^)\n");
	}
	//DB


	
	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if(other->client->overflowed)
			continue;
		if (team)
		{
			if (other->client->pers.pl_state != PL_SPECTATOR)
				// || !OnSameTeam(ent, other))
				continue;
		}
		
		//DB
			safe_cprintf(other, PRINT_CHAT, "%s", text2);
	}
}

//QW// UNUSED, replaced by Spectate()
//RAV Spectator MODE
void player_set_observer(edict_t *ent, int value)
{
	if (value)
	{
		// no weapons available
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);

		// kill the flashlight if its on
		if ( ent->flashlight )
		{
			G_FreeEdict(ent->flashlight);
			ent->flashlight = NULL;
		}
		//ZOID
		if(ctf->value)
		{
			CTFDeadDropFlag(ent);
			CTFDeadDropTech(ent);
			ent->client->resp.ctf_team = CTF_NOTEAM;
		}
		//ZOID
		// drop the rune if we have one
		runes_drop(ent);
		ent->client->resp.score = 0;
		ent->client->pers.pl_state = PL_SPECTATOR;
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->ps.gunindex = 0;
		gi.linkentity (ent);
	}
	else
	{
		if(ent->client->pers.motd == true)//RAV
		{
			//nothing
		}
		else
			if (CTFStartClient(ent))
				return;
	}
}


//3ZB
void Cmd_ZoomIn(edict_t *ent)
{
	if( ent->client->zc.autozoom )
	{
		gi.cprintf(ent,PRINT_HIGH,"autozoom has been selected.\n");
		return;
	}

//	if(	ent->client->pers.weapon != FindItem("Railgun")) return;

	if(ent->client->zc.aiming != 1 &&
	   ent->client->zc.aiming != 3) return;

	if(ent->client->zc.distance < 15 ||
	   ent->client->zc.distance > 90)
	{
		ent->client->zc.distance = 90;
		ent->client->ps.fov = 90;
	}
	
	if(ent->client->zc.distance > 15)
	{
		gi.sound (ent, CHAN_AUTO,
				  gi.soundindex("3zb/zoom.wav"), 1, ATTN_NORM, 0);
		if(ent->client->zc.distance == 90 )
			ent->client->zc.distance = 65;
		else if(ent->client->zc.distance == 65 )
			ent->client->zc.distance = 40;
		else
			ent->client->zc.distance = 15;

		ent->client->ps.fov = ent->client->zc.distance;
	}
}

void Cmd_ZoomOut(edict_t *ent)
{
	if( ent->client->zc.autozoom )
	{
		gi.cprintf(ent,PRINT_HIGH,"autozoom has been selected.\n");
		return;
	}

//	if(	ent->client->pers.weapon != FindItem("Railgun")) return;

	if(ent->client->zc.aiming != 1 &&
	   ent->client->zc.aiming != 3) return;

	if(ent->client->zc.distance < 15 ||
	   ent->client->zc.distance > 90)
	{
		ent->client->zc.distance = 90;
		ent->client->ps.fov = 90;
	}
	
	if(ent->client->zc.distance < 90)
	{
		gi.sound (ent, CHAN_AUTO,
				  gi.soundindex("3zb/zoom.wav"), 1, ATTN_NORM, 0);
		if(ent->client->zc.distance == 15 )
			ent->client->zc.distance = 40;
		else if(ent->client->zc.distance == 40 )
			ent->client->zc.distance = 65;
		else
			ent->client->zc.distance = 90;

		ent->client->ps.fov = ent->client->zc.distance;
	}
}

void Cmd_AutoZoom(edict_t *ent)
{
	if( ent->client->zc.autozoom )
	{
		gi.cprintf(ent,PRINT_HIGH,"autozoom off.\n");
		ent->client->zc.autozoom = false;
	}
	else
	{
		gi.cprintf(ent,PRINT_HIGH,"autozoom on.\n");
		ent->client->zc.autozoom = true;
	}
}

void UndoChain(edict_t *ent, int step)
{
	int	count,i;
	trace_t	rs_trace;

	if(step < 2) count = 2;
	else count = step;

	if(chedit->value && !ent->deadflag && ent == &g_edicts[1])
	{
		for(i = CurrentIndex - 1;i > 0 ;i--)
		{
			if(Route[i].state == GRS_NORMAL)
			{
				rs_trace = gi.trace(Route[i].Pt, ent->mins,
									ent->maxs,
									Route[i].Pt,
									ent,
									MASK_BOTSOLID);

				if(--count <= 0
				   && !rs_trace.allsolid
				   && !rs_trace.startsolid)
					break;
			}
		}

		gi.cprintf(ent,
				   PRINT_HIGH,
				   "backed %i %i steps.\n",
				   CurrentIndex - i,step);
		CurrentIndex = i;
		VectorCopy(Route[CurrentIndex].Pt,ent->s.origin);
		VectorCopy(Route[CurrentIndex].Pt,ent->s.old_origin);

		memset(&Route[CurrentIndex],0,sizeof(route_t));
		if(CurrentIndex > 0)
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1;
	}
}

//
/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;
	char buffer[256];
	//edict_t *item = NULL;
	
	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;
	
	if (!ent->client /*|| ent->bot_client*/)
		return;		// not fully in game yet
	
	cmd = gi.argv(0);
	
	//q2 crashbug fix on the exe not liking the % cmd)
	if ( strstr(cmd, "%" ))
		return;
	
	//RAv be sure players are connected 
	if(!ent->client->pers.in_game)
		return;
	
	//RAV bot user !
	if(ent->client->pers.pl_state == PL_CHEATBOT)
	{
		safe_cprintf(ent, PRINT_HIGH, "Bots are not welcome here\n");
		return;
	}
	//
	
	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}

	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}

	if (Q_stricmp (cmd, "say_team") == 0 || Q_stricmp (cmd, "steam") == 0)
	{
		if (ctf->value)
			CTFSay_Team(ent, gi.args());
		else if (ent->client->resp.spectator)
			Cmd_Say_f (ent, true, false);
		else
			Cmd_Say_f (ent, false, false);
		return;
	}

	//JSW
	if (Q_stricmp (cmd, "sayop") == 0 || Q_stricmp (cmd, "opsay") == 0)
	{
		//if (ent->client->pers.isop ==1 || ent->client->pers.ismop ==1)
		if (ent->client->pers.oplevel & OP_SAY)
			Say_Op (ent, gi.args());
		else
			NoAccess(ent);
		return;
	}

	if (Q_stricmp (cmd, "op_chat") == 0 || Q_stricmp (cmd, "opchat") == 0)
	{
		//if (ent->client->pers.isop ==1 || ent->client->pers.ismop ==1)
		if (ent->client->pers.oplevel & OP_SAY)
		{
			if (ent->client->pers.saytype != 1)
			{
				ent->client->pers.saytype = 1;
				stuffcmd (ent, "echo \"Op Chat enabled\";messagemode\n");
			}
			else
			{
				ent->client->pers.saytype = 0;
				stuffcmd (ent, "echo \"Op Chat disabled\"\n");
			}
		}
		else
			NoAccess(ent);
		return;
	}

/* FIXMEJSW: need to rewrite the command
	if (Q_stricmp (cmd, "opbroadcast") == 0)
	{
		if (ent->client->pers.isop ==1)
		{
			ent->client->pers.saytype = 2;
			stuffcmd (ent, "echo \"Op Broadcast enabled\";messagemode");
		}
		else
			NoAccess(ent);
		return;
	}
*/

	if (Q_stricmp (cmd, "rconmode") == 0)
	{
		//if (ent->client->pers.isop ==1 || ent->client->pers.ismop ==1)
		if (ent->client->pers.oplevel & OP_RCON)
		{
			if (ent->client->pers.saytype != 3)
			{
				ent->client->pers.saytype = 3;
				stuffcmd (ent, "echo \"RCON mode enabled\";messagemode\n");
			}
			else
			{
				ent->client->pers.saytype = 0;
				stuffcmd (ent, "echo \"RCON mode disabled\"\n");
			}
		}
		else
			NoAccess(ent);
		return;
	}

	if (Q_stricmp (cmd, "grcon") == 0)
	{
		if (ent->client->pers.oplevel & OP_RCON)
		{
			sprintf(buffer,
					"rcon_password \"\";rcon %s %s\n", rcon->string, gi.args());
			stuffcmd (ent, va(buffer));
		}
		else
			NoAccess(ent);
		return;
	}

	if (Q_stricmp (cmd, "lockserver") == 0)
	{
		//if (ent->client->pers.isop ==1 || ent->client->pers.ismop == 1)
		if (ent->client->pers.oplevel & OP_LOCKSERVER)
		{
			my_bprintf(PRINT_CHAT,
					   "Server was locked by %s\n", ent->client->pers.netname);
			serverlocked = true;
		}
		else
			NoAccess(ent);
		return;
	}

	if (Q_stricmp (cmd, "unlockserver") == 0)
	{
		//if (ent->client->pers.isop ==1 || ent->client->pers.ismop == 1)
		if (ent->client->pers.oplevel & OP_LOCKSERVER)
		{
			my_bprintf(PRINT_CHAT, "Server was unlocked by %s\n", ent->client->pers.netname);
			serverlocked = false;
		}
		else
			NoAccess(ent);
		return;
	}
	//end

	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "hscore") == 0)
	{
		Cmd_HighScore_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}
	if (level.intermissiontime)
		return;
	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "drop") == 0)
		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "give") == 0)
		Cmd_Give_f (ent);
	else if (Q_stricmp (cmd, "god") == 0)
		Cmd_God_f (ent);
	else if (Q_stricmp (cmd, "notarget") == 0)
		Cmd_Notarget_f (ent);
	else if (Q_stricmp (cmd, "noclip") == 0)
		Cmd_Noclip_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "invnext") == 0)
		SelectNextItem (ent, -1);
	else if (Q_stricmp (cmd, "invprev") == 0)
		SelectPrevItem (ent, -1);
	else if (Q_stricmp (cmd, "invnextw") == 0)
		SelectNextItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invprevw") == 0)
		SelectPrevItem (ent, IT_WEAPON);
	else if (Q_stricmp (cmd, "invnextp") == 0)
		SelectNextItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invprevp") == 0)
		SelectPrevItem (ent, IT_POWERUP);
	else if (Q_stricmp (cmd, "invuse") == 0)
		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "invdrop") == 0)
		Cmd_InvDrop_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "weaplast") == 0)
		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_strcasecmp (cmd, "skinlist") == 0)
		Cmd_SkinList_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp (cmd, "wave") == 0)
		Cmd_Wave_f (ent);
	
	//ZOID
	else if (Q_stricmp (cmd, "team") == 0 || Q_stricmp (cmd, "join") == 0)
	{
		if(locked_teams == false)
		{
			ent->client->resp.hook_wait = level.time + .5; //JSW was 5, is that number necessary??
			abandon_hook_reset (ent);
			CTFTeam_f (ent, 0);
		}
		else
			safe_cprintf(ent, PRINT_HIGH, "TEAMS ARE LOCKED !!\n");
	}
	else if (Q_stricmp(cmd, "id") == 0)
	{
		//CTFID_f (ent);
	}
	
	//ZOID
	else if (Q_strcasecmp (cmd, "join") == 0)
	{
		if(ctf->value)
			safe_cprintf(ent, PRINT_HIGH, "try team <red or blue>\n");
		else
			JoinGame(ent, NULL);
	}
	
	//QW// This is interesting. Case insensitive string comparisons
	//     using mixed case strings? Set all to lower case, changed
	//     Q_stricmp to Q_strcasecmp and changed parentheses to match
	//     what I think it's supposed to be doing and to silence clang.
	//RAVEN
	else if (Q_strcasecmp (cmd, "spec") == 0
			  || Q_strcasecmp (cmd, "spectator") == 0
			  || Q_strcasecmp (cmd, "chase") == 0
			  || Q_strcasecmp (cmd, "chasecam") == 0
			  || Q_strcasecmp (cmd, "observe") == 0
			  || (Q_strcasecmp (cmd, "observer") == 0
			 && ent->client->pers.pl_state != PL_SPECTATOR
			 && !(ent->client->pers.pl_state != PL_CHEATBOT)))
	{
		if (ent->client->resp.spectator == 1)
		{
			safe_cprintf (ent, PRINT_HIGH,
						  "You are already a spectator!\n");
			return;
		}

		if(CountConnectedClients()+2 >= (maxclients->value - (int)reserved_slots->value))
		{
			if(op_specs->value || max_specs->value)
			{
				if((max_specs->value )
				   && (CountSpecClients() >= (max_specs->value)))
				{
					safe_cprintf (ent, PRINT_HIGH,
								  "Too many spectators already\n");
					return;
				}
				if((op_specs->value)
				   && (CountSpecClients() >= (max_specs->value)))
					safe_cprintf (ent, PRINT_HIGH,
								  "Too many spectators already\n");
				return;
			}
		}
		if(ent->deadflag & DEAD_DEAD)
		{
			safe_cprintf (ent, PRINT_HIGH,
						  "You must be ALIVE to go Spectator\n");
			return;
		}
		ent->client->pers.pl_state = PL_SPECTATOR;
		ent->client->resp.spectator = 1;

		if (ent->client->menu)
			PMenu_Close(ent);

		Spectate (ent, NULL);
		CTFChaseCam(ent, NULL);
		//player_set_observer (ent, true);

		CheckPlayers();
		if (ctf->value)
			my_bprintf(PRINT_HIGH,
					   "%s has become a spectator. "
					   "(%d red, %d blue, %d spectators)\n",
					   ent->client->pers.netname,
					   ctfgame.players1,
					   ctfgame.players2, ctfgame.specs);
		else
			my_bprintf(PRINT_HIGH,
					   "%s has become a spectator. "
					   "(%d players, %d spectators)\n",
					   ent->client->pers.netname,
					   ctfgame.players_total,
					   ctfgame.specs);

	}
	
	//HOOK
	/*
	else if (Q_stricmp (cmd, "lhook") == 0)
	{
		if ((ent->solid != SOLID_NOT &&
			ent->deadflag == DEAD_NO)&&(ent->client->pers.pl_state != PL_CHEATBOT)&&
			(match_state > STATE_COUNTDOWN))
			abandon_hook_fire (ent);
	}
	else if (Q_stricmp (cmd, "lunhook") == 0)
	{
		if ((ent->client->hook)&& (match_state > STATE_COUNTDOWN))
			abandon_hook_reset(ent->client->hook);
	}
	*/
	
	//cheat detections
	else if(Q_stricmp  (cmd, "mm_fps") == 0)
	{
		if(atoi(gi.argv(1)) > maxfps->value)
		{
			stuffcmd (ent, va("set cl_maxfps %i\n", (int)maxfps->value));
			safe_cprintf (ent, PRINT_HIGH,
						  "Server Restricting FPS To %i\n",
						  (int)maxfps->value);
			return;
		}
		else if(atoi(gi.argv(1)) < minfps->value)
		{
			stuffcmd (ent, va("set cl_maxfps %i\n", (int)minfps->value));
			safe_cprintf (ent, PRINT_HIGH,
						  "Server Increasing FPS To %i\n",(int)minfps->value);
			return;
		}
	}

	else if(Q_stricmp  (cmd, "mm_delta") == 0)
	{
		if(atoi(gi.argv(1)) > 0)
		{
			stuffcmd (ent, va("cl_nodelta 0\n"));
			safe_cprintf (ent, PRINT_HIGH,
						  "You cannot use cl_nodelta here.\n");
			return;
		}
	}

	else if(Q_stricmp  (cmd, "mm_ts") == 0)
	{
		if(atoi(gi.argv(1)) != 1 && ent->client->pers.pl_state < PL_CHEATBOT)
		{
			ent->client->newweapon = NULL;
			ChangeWeapon (ent);
			ent->client->ps.gunindex = 0;
			OnBotDetection(ent, va("speed-hack"));
			return;
		}
	}

	else if (Q_stricmp (cmd, "flashlight") == 0)
	{
		if(ent->solid == SOLID_NOT
		   || ent->deadflag != DEAD_NO
		   || flashlight->value != 1)
			return;

		if (ent->flashlight)
		{
			FL_make (ent);
			gi.sound (ent, CHAN_AUTO,
					  gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			FL_make (ent);
			gi.sound (ent, CHAN_AUTO,
					  gi.soundindex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
		}
	}
	
	//VOTING
	else if (Q_stricmp (cmd, "map_vote") == 0)
	{
		if(mapvotefilled != true)
		{
			safe_cprintf (ent, PRINT_HIGH,
						  "You have to wait until end of level\n");
			return;
		}
		if(ent->client->pers.vote_times > 0)
		{
			safe_cprintf (ent, PRINT_HIGH,
						  "You have already voted for a map.\n");
			return;
		}
		if (ent->client->pers.HasVoted == false)
		{
			MapVote(ent);
		}
		else
			safe_cprintf (ent, PRINT_HIGH,
						  "You have already voted for a map.\n");
	}
	else if (Q_stricmp (cmd, "showvotes") == 0)
	{
        Cmd_ShowVotes_f (ent);
		return;
	}


	
	//ADMIN options
	else if (Q_stricmp (cmd, "m_map") == 0)
	{
		//if (ent->client->pers.isop == 1 || ent->client->pers.ismop == 1)
		if (ent->client->pers.oplevel & OP_CHANGEMAP)
		{
			cmd = gi.argv(1);
			sprintf(buffer, "\ngamemap %s\n", cmd);
			gi.AddCommandString(buffer);
		}
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "m_status") == 0)
	{
		//if (ent->client->pers.isop == 1 || ent->client->pers.ismop == 1)
		if (ent->client->pers.oplevel & OP_STATUS)
		{
			sprintf(buffer,
					"rcon_password \"\";rcon %s status\n", rcon->string);
			stuffcmd (ent, va(buffer));
		}
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "m_kick") == 0)
	{
//		if (ent->client->pers.isop == 1 || ent->client->pers.ismop == 1)
		if (ent->client->pers.oplevel & OP_KICK)
		{
			cmd = gi.argv(1);
			sprintf(buffer, "\nkick %s\n", cmd);
			gi.AddCommandString(buffer);
		}
		else
			NoAccess(ent);
	}
	//JSW
	else if (Q_stricmp (cmd, "lockteams") == 0)
	{
//		if (ent->client->pers.isop ==1 || ent->client->pers.ismop ==1)
		if (ent->client->pers.oplevel & OP_LOCKTEAMS)
		{
			locked_teams = true;
			my_bprintf(PRINT_HIGH, "Teams are LOCKED\n");
		}
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "unlockteams") == 0)
	{
//		if (ent->client->pers.isop ==1 || ent->client->pers.ismop ==1)
		if (ent->client->pers.oplevel & OP_LOCKTEAMS)
		{
			locked_teams = false;
			my_bprintf(PRINT_HIGH, "Teams are UN-LOCKED\n");
		}
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "showopfile") == 0)
	{
//		if (ent->client->pers.isop ==1)
		if (ent->client->pers.oplevel & OP_ADDOP)
			ShowFile(ent, "user_o.txt");
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "showbannedfile") == 0)
	{
//		if (ent->client->pers.isop ==1)
		if (ent->client->pers.oplevel & OP_BAN)
			ShowFile (NULL, "ip_banned.txt");
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "ban") == 0)
	{
//		if (ent->client->pers.isop ==1)
		if (ent->client->pers.oplevel & OP_BAN)
			sv_ban_ip(ent);
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "restart") == 0)
	{
//		if (ent->client->pers.isop ==1 || ent->client->pers.ismop ==1)
		if (ent->client->pers.oplevel & OP_RESTART)
			OpRestart(ent, NULL);
		else
			NoAccess(ent);
	}
	else if (Q_stricmp (cmd, "modop") == 0)
	{
		int level;
		if (ent->client->pers.oplevel & OP_MODOP)
		{
			if (IPMatch (gi.argv(1),  "*@*.*.*.*") == 1 && gi.argc() == 3)
			{
				if (atoi(gi.argv(2)) > ent->client->pers.oplevel)
					level = ent->client->pers.oplevel;
				else
					level = atoi(gi.argv(2));

				if (ModifyOpLevel(CheckOpFile (NULL, gi.argv(1), true), level))
					safe_cprintf (ent, PRINT_HIGH,
								  "%s level changed to %d\n",
								  gi.argv(1), level);
				else
					safe_cprintf (ent, PRINT_HIGH,
								  "No matching entry found.\n");
			}
			else
				safe_cprintf(ent, PRINT_HIGH,
							 "Usage: modop user@ip newlevel\n");
		}
		else
			NoAccess(ent);
	}
	else if (Q_stricmp(cmd, "addop") == 0)
	{
		int level;
		char pass[16];
		if (ent->client->pers.oplevel & OP_ADDOP)
		{
			if (IPMatch (gi.argv(1),  "*@*.*.*.*") == 1)
			{
				gi.dprintf("there are %d args\n", gi.argc());
				if (gi.argc() < 3)
					level = (int)defaultoplevel->value;
				else
					level = atoi(gi.argv(2));
				if (gi.argc() < 4)
					sprintf(pass, "nopass");
				else
					Com_sprintf(pass, sizeof(pass), gi.argv(3));

				safe_cprintf (ent, PRINT_HIGH,
					"%s added to user_o.txt with level %d and password %s.\n",
							  gi.argv(1), level, pass);
				AddOperator (gi.argv(1), level, pass);
			}
			else
				safe_cprintf(ent, PRINT_HIGH, "Usage: addop user@ip level pass\n");
		}
		else
			NoAccess(ent);
	}
	//end
	else if(Q_stricmp (cmd, "lightsoff") == 0  && lights_out->value == 1)
	{
//		if (ent->client->pers.isop==1)
		if (ent->client->pers.oplevel & OP_LIGHTS)
		{
			int i;
			for (i = 1; i <= maxclients->value; i++)
			{
				ent = g_edicts + i;
				if (ent && ent->inuse && !ent->bot_client)
				{
					gi.sound (ent, CHAN_AUTO,
						gi.soundindex ("world/lite_out.wav"),
						1, ATTN_NORM, 0);
				}
			}
			if (LIGHTS)
			{
				// 0 normal
				gi.configstring(CS_LIGHTS+0, "m");
				// 1 FLICKER (first variety)
				gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
				// 2 SLOW STRONG PULSE
				gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
				// 3 CANDLE (first variety)
				gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
				// 4 FAST STROBE
				gi.configstring(CS_LIGHTS+4, "mamamamamama");
				// 5 GENTLE PULSE 1
				gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
				// 6 FLICKER (second variety)
				gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
				// 7 CANDLE (second variety)
				gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
				// 8 CANDLE (third variety)
				gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
				// 9 SLOW STROBE (fourth variety)
				gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
				// 10 FLUORESCENT FLICKER
				gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");
				// 11 SLOW PULSE NOT FADE TO BLACK
				gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
				//JR 3/26/98
			}               
			else
			{
				// 0 normal
				gi.configstring(CS_LIGHTS+0, "b");
				// 1 FLICKER (first variety)
				gi.configstring(CS_LIGHTS+1, "a");
				// 2 SLOW STRONG PULSE
				gi.configstring(CS_LIGHTS+2, "b");
				// 3 CANDLE (first variety)
				gi.configstring(CS_LIGHTS+3, "a");
				// 4 FAST STROBE
				gi.configstring(CS_LIGHTS+4, "b");
				// 5 GENTLE PULSE 1
				gi.configstring(CS_LIGHTS+5,"a");
				// 6 FLICKER (second variety)
				gi.configstring(CS_LIGHTS+6, "b");
				// 7 CANDLE (second variety)
				gi.configstring(CS_LIGHTS+7, "a");
				// 8 CANDLE (third variety)
				gi.configstring(CS_LIGHTS+8, "b");
				// 9 SLOW STROBE (fourth variety)
				gi.configstring(CS_LIGHTS+9, "a");
				// 10 FLUORESCENT FLICKER
				gi.configstring(CS_LIGHTS+10, "b");
				// 11 SLOW PULSE NOT FADE TO BLACK
				gi.configstring(CS_LIGHTS+11, "a"); 
			}
		}
		else
			NoAccess(ent);
	}
	// lights on
	else if(Q_stricmp (cmd, "lightson") == 0 && lights_out->value == 1)
	{
//		if (ent->client->pers.isop==1 )
		if (ent->client->pers.oplevel & OP_LIGHTS)
		{
			int i;
			for (i = 1; i <= maxclients->value; i++)
			{
				ent = g_edicts + i;
				if (ent && ent->inuse && !ent->bot_client)
				{
					gi.sound (ent, CHAN_AUTO,
						gi.soundindex ("world/lite_on3.wav"),
						1, ATTN_NORM, 0);
				}
			}
			if (LIGHTS)
			{
				// 0 normal
				gi.configstring(CS_LIGHTS+0, "b");
				// 1 FLICKER (first variety)
				gi.configstring(CS_LIGHTS+1, "a");
				// 2 SLOW STRONG PULSE
				gi.configstring(CS_LIGHTS+2, "b");
				// 3 CANDLE (first variety)
				gi.configstring(CS_LIGHTS+3, "a");
				// 4 FAST STROBE
				gi.configstring(CS_LIGHTS+4, "b");
				// 5 GENTLE PULSE 1
				gi.configstring(CS_LIGHTS+5,"a");
				// 6 FLICKER (second variety)
				gi.configstring(CS_LIGHTS+6, "b");
				// 7 CANDLE (second variety)
				gi.configstring(CS_LIGHTS+7, "a");
				// 8 CANDLE (third variety)
				gi.configstring(CS_LIGHTS+8, "b");
				// 9 SLOW STROBE (fourth variety)
				gi.configstring(CS_LIGHTS+9, "a");
				// 10 FLUORESCENT FLICKER
				gi.configstring(CS_LIGHTS+10, "b");
				// 11 SLOW PULSE NOT FADE TO BLACK
				gi.configstring(CS_LIGHTS+11, "a");
			}
			else
			{
				// 0 normal
				gi.configstring(CS_LIGHTS+0, "m");
				// 1 FLICKER (first variety)
				gi.configstring(CS_LIGHTS+1, "mmnmmommommnonmmonqnmmo");
				// 2 SLOW STRONG PULSE
				gi.configstring(CS_LIGHTS+2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");
				// 3 CANDLE (first variety)
				gi.configstring(CS_LIGHTS+3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");
				// 4 FAST STROBE
				gi.configstring(CS_LIGHTS+4, "mamamamamama");
				// 5 GENTLE PULSE 1
				gi.configstring(CS_LIGHTS+5,"jklmnopqrstuvwxyzyxwvutsrqponmlkj");
				// 6 FLICKER (second variety)
				gi.configstring(CS_LIGHTS+6, "nmonqnmomnmomomno");
				// 7 CANDLE (second variety)
				gi.configstring(CS_LIGHTS+7, "mmmaaaabcdefgmmmmaaaammmaamm");
				// 8 CANDLE (third variety)
				gi.configstring(CS_LIGHTS+8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");
				// 9 SLOW STROBE (fourth variety)
				gi.configstring(CS_LIGHTS+9, "aaaaaaaazzzzzzzz");
				// 10 FLUORESCENT FLICKER
				gi.configstring(CS_LIGHTS+10, "mmamammmmammamamaaamammma");
				// 11 SLOW PULSE NOT FADE TO BLACK
				gi.configstring(CS_LIGHTS+11, "abcdefghijklmnopqrrqponmlkjihgfedcba");
				//JR 3/26/98
			}
		}
		else
			NoAccess(ent);
	}
	//3ZB
	else if (Q_stricmp (cmd, "zoomin") == 0)		//zoom
		Cmd_ZoomIn(ent);
	else if (Q_stricmp (cmd, "zoomout") == 0)
		Cmd_ZoomOut(ent);
	else if (Q_stricmp (cmd, "autozoom") == 0)
		Cmd_AutoZoom(ent);
	else if (Q_stricmp (cmd, "undo") == 0)
	{
		if(gi.argc() <= 1)
			UndoChain(ent,1);
		else
			UndoChain (ent,atoi(gi.argv(1)));
	}
	else if (Q_stricmp (cmd, "chain") == 0 && chedit->value)
	{
		gi.AddCommandString("sv savechain\n");
	}
	//
	// offhanded hook
    else if (Q_stricmp (cmd, "hook") == 0)
	{
		if (!use_hook->value)
			return;
		if ((ent->client->pers.pl_state == PL_PLAYING)
			&& (ent->solid != SOLID_NOT && ent->deadflag == DEAD_NO)&&
			(match_state == STATE_PLAYING))
			RaV_hook (ent);
	}
	else if (Q_stricmp (cmd, "unhook") == 0)
	{
		if (!use_hook->value)
			return;
		if (ent->solid != SOLID_NOT && ent->deadflag == DEAD_NO)
			RaV_unhook (ent);
	}
//JSW
	else if (Q_stricmp (cmd, "download") == 0)
	{
		gi.dprintf ("download called by %s\n",
					ent->client->pers.netname);

		if (strstr (gi.argv(1), ".txt") ||
			strstr (gi.argv(1), ".log") ||
			strstr (gi.argv(1), ".cfg"))
				my_bprintf(PRINT_HIGH,
					"%s tried to download an unauthorized file \nand was disconnected from the server.\n",
					ent->client->pers.netname);
		stuffcmd(ent,
				 "disconnect;error \"You have been disconnected for trying to download an unauthorized file. Multiple attempts at this will result in a ban from this server.\"");
		return;
	}
	else if (Q_stricmp (cmd, "listops") == 0
		  || Q_stricmp (cmd, "oplist") == 0)
	{
		Cmd_Operators_f(ent);
	}
	else if (Q_stricmp (cmd, "iddist") == 0)
	{
		if (gi.argc() < 2)
			safe_cprintf (ent, PRINT_HIGH,
						  "Your iddist is %d\n",
						  ent->client->resp.iddist);
		else
		{
			if (atoi(gi.argv(1)) > (int)max_hud_dist->value)
				ent->client->resp.iddist = (int)max_hud_dist->value;
			else
				ent->client->resp.iddist = atoi(gi.argv(1));

			safe_cprintf (ent, PRINT_HIGH,
						  "Your iddist was set to %d",
						  ent->client->resp.iddist);

			if (ent->client->resp.iddist == (int)max_hud_dist->value)
				safe_cprintf (ent, PRINT_HIGH, " (server max).\n");
			else
				safe_cprintf (ent, PRINT_HIGH, ".\n");
		}
	}
	else if (Q_stricmp (cmd, "no_hum") == 0)
	{
		if (!ent->client->resp.no_hum)
		{
			safe_cprintf(ent, PRINT_CHAT, "Railgun hum turned OFF\n");
			ent->client->resp.no_hum = 1;
		}
		else
		{
			safe_cprintf(ent, PRINT_CHAT, "Railgun hum turned ON\n");
			ent->client->resp.no_hum = 0;
		}
	}
	else if (Q_stricmp (cmd, "oplevel") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH,
					 "oplevel = %d\n", ent->client->pers.oplevel);
	}
	else if (Q_stricmp (cmd, "checkop") == 0) {
		if (ent->client->pers.oplevel & OP_LOCKSERVER)
			safe_cprintf(ent, PRINT_HIGH, "Lock server permission on!");
		else
			safe_cprintf(ent, PRINT_HIGH, "Lock server permission off!");
	}
	else if (Q_stricmp (cmd, "spree") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH,
					 "Your current spree is %d. Your longest spree is %d.\n",
					 ent->client->resp.spree, ent->client->resp.bigspree);
	}
	else if (Q_stricmp (cmd, "mapvote") == 0)
	{
		if (allow_vote->value)
			Cmd_MapVote(ent);
		else
			safe_cprintf(ent, PRINT_HIGH,
						 "Map voting is disabled on this server.\n");
	}
	else if (Q_stricmp (cmd, "yes") == 0)
	{
		if (mapvoteactive == false)
		{
			safe_cprintf(ent, PRINT_HIGH,
						 "There is no vote active.\n");
			return;
		}
		if (ent->client->resp.vote == true)
			safe_cprintf(ent, PRINT_HIGH,
						 "You have already voted YES.\n");
		else
		{
			ent->client->resp.vote = true;
			safe_cprintf(ent, PRINT_HIGH,
						 "Your vote has been changed to YES\n");
		}
	}
	else if (Q_stricmp (cmd, "no") == 0)
	{
		if (mapvoteactive == false)
		{
			safe_cprintf(ent, PRINT_HIGH,
						 "There is no vote active.\n");
			return;
		}
		if (ent->client->resp.vote == false)
			safe_cprintf(ent, PRINT_HIGH,
						 "You have already voted NO.\n");
		else
		{
			ent->client->resp.vote = false;
			safe_cprintf(ent, PRINT_HIGH,
						 "Your vote has been changed to NO\n");
		}
	}
	else if (Q_stricmp (cmd, "menus") == 0)
	{
		safe_cprintf(ent, PRINT_HIGH,
					 "pl_state is %d, menu is %d, showscores is %d.\n",
					 ent->client->pers.pl_state,
					 ent->client->menu,
					 ent->client->showscores);
	}
	else if (Q_stricmp (cmd, "permissions") == 0)
	{
		//char *commands;// = "You have the following permissions: ";
		if (ent->client->pers.isop)
		{
			safe_cprintf(ent,
						 PRINT_HIGH,
						 "You have the following permissions:\n");
			if (ent->client->pers.oplevel & OP_SPEC)
				safe_cprintf(ent, PRINT_HIGH, "OP_SPEC\n");
			if (ent->client->pers.oplevel & OP_SAY)
				safe_cprintf(ent, PRINT_HIGH, "OP_SAY\n");
			if (ent->client->pers.oplevel & OP_SWITCH)
				safe_cprintf(ent, PRINT_HIGH, "OP_SWITCH\n");
			if (ent->client->pers.oplevel & OP_CHANGEMAP)
				safe_cprintf(ent, PRINT_HIGH, "OP_CHANGEMAP\n");
			if (ent->client->pers.oplevel & OP_LOCKTEAMS)
				safe_cprintf(ent, PRINT_HIGH, "OP_LOCKTEAMS\n");
			if (ent->client->pers.oplevel & OP_RESTART)
				safe_cprintf(ent, PRINT_HIGH, "OP_RESTART\n");
			if (ent->client->pers.oplevel & OP_KICK)
				safe_cprintf(ent, PRINT_HIGH, "OP_KICK\n");
			if (ent->client->pers.oplevel & OP_STATUS)
				safe_cprintf(ent, PRINT_HIGH, "OP_STATUS\n");
			if (ent->client->pers.oplevel & OP_SILENCE)
				safe_cprintf(ent, PRINT_HIGH, "OP_SILENCE\n");
			if (ent->client->pers.oplevel & OP_LOCKSERVER)
				safe_cprintf(ent, PRINT_HIGH, "OP_LOCKSERVER\n");
			if (ent->client->pers.oplevel & OP_LISTEN)
				safe_cprintf(ent, PRINT_HIGH, "OP_LISTEN\n");
			if (ent->client->pers.oplevel & OP_SHOWBANNEDFILE)
				safe_cprintf(ent, PRINT_HIGH, "OP_SHOWBANNEDFILE\n");
			if (ent->client->pers.oplevel & OP_BAN)
				safe_cprintf(ent, PRINT_HIGH, "OP_BAN\n");
			if (ent->client->pers.oplevel & OP_LIGHTS)
				safe_cprintf(ent, PRINT_HIGH, "OP_LIGHTS\n");
			if (ent->client->pers.oplevel & OP_PROTECTED)
				safe_cprintf(ent, PRINT_HIGH, "OP_PROTECTED\n");
			if (ent->client->pers.oplevel & OP_ADDOP)
				safe_cprintf(ent, PRINT_HIGH, "OP_ADDOP\n");
			if (ent->client->pers.oplevel & OP_MODOP)
				safe_cprintf(ent, PRINT_HIGH, "OP_MODOP\n");
			if (ent->client->pers.oplevel & OP_RCON)
				safe_cprintf(ent, PRINT_HIGH, "OP_RCON\n");
		}
	}
	//	else
	else if (console_chat->value)
//end
		Cmd_Say_f (ent, false, true);
	else
		safe_cprintf(ent, PRINT_HIGH,
					 "Unrecognized command: %s\n", cmd);
}

