#include "g_local.h"

//RAV
#include "runes.h"
#include "stdlog.h"
#include "e_hook.h"
//

//3ZB
qboolean bots_moveok ( edict_t *ent,float ryaw,vec3_t pos,float dist,float *bottom);
//

void OpenJoinMenu(edict_t *ent);//RAV
char menustring[24][64];
void RavCheckTeams();
int CountConnectedClients ();
void CTFJoinTeam1(edict_t *ent, pmenu_t *p);
void CTFJoinTeam2(edict_t *ent, pmenu_t *p);
void TossClientWeapon (edict_t *self);
void ChangeNow (edict_t *ent, pmenu_t *menu);
void ChangeLater (edict_t *ent, pmenu_t *menu);

qboolean firsttechs;
gitem_t	*item_tech1, *item_tech2, *item_tech3, *item_tech4;

char *rail_statusbar =

"yb	-24 "
// health
//"xl	2 "
//"hnum "
"xl	52 "
//"pic 0 "

// red team
"yb -53 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
"xr -62 "
"num 2 18 "

//joined overlay
"if 22 "
  "yb -55 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -26 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -62 "
"num 2 20 "

"if 23 "
  "yb -28 "
  "xr -28 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yb -24 "
  "xl 79 "
  "pic 21 "
"endif "

//RAV
//TIMER
"if 28 " // if in predm timer
"xv -140 " // set the x
"yb -190 " // and y positions
"num 3 28 "
"endif "

//flagcarrier names
"xr -168 "
"yb -20 "
"stat_string 29 "
"yb -47 "
"stat_string 30 "
;

char *ctf_statusbar =
"yb	-24 "

// health
"xv	0 "
"hnum "
"xv	50 "
"pic 0 "

// ammo
"if 2 "
"	xv	100 "
"	anum "
"	xv	150 "
"	pic 2 "
"endif "

// armor
"if 4 "
"	xv	200 "
"	rnum "
"	xv	250 "
"	pic 4 "
"endif "

// selected item
"if 6 "
"	xv	296 "
"	pic 6 "
"endif "
"yb	-50 "

//  help / weapon icon 
"if 11 "
  "xv 148 "
  "pic 11 "
"endif "


// picked up item
"if 7 "
"	xv	0 "
"	pic 7 "
"	xv	26 "
"	yb	-42 "
"	stat_string 8 "
"	yb	-50 "
"endif "

//tech
"yb -137 "
"if 26 "
  "xr -26 "
  "pic 26 "
"endif "

// red team
"yb -112 "
"if 17 "
  "xr -26 "
  "pic 17 "
"endif "
"xr -62 "
"num 2 18 "

// timer
"if 9 "
  "yb -50 "
  "xv 246 "
  "num 2 10 "
  "xv 296 "
  "pic 9 "
"endif "

//joined overlay
"if 22 "
  "yb -114 "
  "xr -28 "
  "pic 22 "
"endif "

// blue team
"yb -85 "
"if 19 "
  "xr -26 "
  "pic 19 "
"endif "
"xr -62 "
"num 2 20 "
"if 23 "
  "yb -87 "
  "xr -28 "
  "pic 23 "
"endif "

// have flag graph
"if 21 "
  "yt 26 "
  "xr -24 "
  "pic 21 "
"endif "


//RAV
//TIMER
"if 28 " // if in predm timer
"xv -140 " // set the x
"yb -190 " // and y positions
"num 3 28 "
"endif "

//flagcarrier names
"xr -168 "
"yb -77 "
"stat_string 29 "
"yb -104 "
"stat_string 30 "

// rune
"if 31 "
"	xr	-108"
"	yb	-13 "
"	stat_string	31 "
"endif "
//
;

static char *tnames[] =
{
	"item_tech1",
	"item_tech2",
	"item_tech3",
	"item_tech4",
	NULL
};

/*--------------------------------------------------------------------------*/

//RAV
int CountSpecClients ()
{
	int n, count;
	edict_t *player;

	count = 0;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse || player->client->pers.pl_state != 0)
			continue;
		count++;
	}
	ctfgame.specs = count;
	return(count);
}

void CheckPlayers (void)
{
	int i;
	edict_t	*carrier;
	ctfgame.players1 = 0;
	ctfgame.players2 = 0;
	ctfgame.players3 = 0;
	ctfgame.specs = 0;
	for (i = 1; i <= maxclients->value; i++)
	{
		carrier = g_edicts + i;
		if (ctf->value)
		{
			if (carrier->inuse && carrier->client->resp.ctf_team > 0)
			{
				if(carrier->client->resp.ctf_team == 1)
					ctfgame.players1++;
				else
					ctfgame.players2++;
			}
		}
		else if (carrier->inuse && carrier->client->pers.pl_state > 0)
		{
			ctfgame.players3++;
		}
	}
	CountSpecClients();
}


/*
This function is to force spectators to join a team if the max_spec or op_spec cvars are 
used have to keep in mind that only players- reserved are to be counted
*/

qboolean Check_for_SpecLimit(edict_t *who)
{
	int force =0;
	if(CountConnectedClients()+2 < (maxclients->value - (int)reserved_slots->value))
		return false;
	if(!op_specs->value && !max_specs->value)
		return false;
	if((max_specs->value) && (CountSpecClients() >= (max_specs->value)))
		force = 1;
	if((op_specs->value) && (CountSpecClients() >= (max_specs->value)))
		force = 1;
	if(force == 1)
	{
		edict_t		*player;
		int i;
		int team1count = 0, team2count = 0;
		who->client->resp.ctf_state = CTF_STATE_START;
		for (i = 1; i <= maxclients->value; i++)
		{
			player = &g_edicts[i];
			if (!player->inuse || player->client == who->client)
				continue;
			switch (player->client->resp.ctf_team)
			{
			case CTF_TEAM1:
				team1count++;
				break;
			case CTF_TEAM2:
				team2count++;
			}
		}
		
		if (team1count < team1count)
			CTFJoinTeam1(who,NULL);
		else if (team2count < team1count)
			CTFJoinTeam2(who,NULL);
		else if (rand() & 1)
			CTFJoinTeam1(who,NULL);
		else
			CTFJoinTeam2(who,NULL);
		return true;
	}
	return false;
}

/*
=================
findradius
Returns entities that have origins within a spherical area
findradius (origin, radius)
=================
*/
static edict_t *loc_findradius (edict_t *from, vec3_t org, float rad)
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
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->s.origin[j] + (from->mins[j] + from->maxs[j])*0.5);

		if (VectorLength(eorg) > rad)
			continue;

		return from;
	}
	return NULL;
}

static void loc_buildboxpoints(vec3_t p[8], vec3_t org, vec3_t mins, vec3_t maxs)
{
	VectorAdd(org, mins, p[0]);
	VectorCopy(p[0], p[1]);
	p[1][0] -= mins[0];
	VectorCopy(p[0], p[2]);
	p[2][1] -= mins[1];
	VectorCopy(p[0], p[3]);
	p[3][0] -= mins[0];
	p[3][1] -= mins[1];
	VectorAdd(org, maxs, p[4]);
	VectorCopy(p[4], p[5]);
	p[5][0] -= maxs[0];
	VectorCopy(p[0], p[6]);
	p[6][1] -= maxs[1];
	VectorCopy(p[0], p[7]);
	p[7][0] -= maxs[0];
	p[7][1] -= maxs[1];
}

static qboolean loc_CanSee (edict_t *targ, edict_t *inflictor)
{
	trace_t	trace;
	vec3_t	targpoints[8];
	int i;
	vec3_t viewpoint;
	
	// bmodels need special checking because their origin is 0,0,0
	if (targ->movetype == MOVETYPE_PUSH)
		return false; // bmodels not supported
	loc_buildboxpoints(targpoints, targ->s.origin, targ->mins, targ->maxs);
	VectorCopy(inflictor->s.origin, viewpoint);
	viewpoint[2] += inflictor->viewheight;
	for (i = 0; i < 8; i++)
	{
		trace = gi.trace (viewpoint, vec3_origin, vec3_origin, targpoints[i], inflictor, MASK_SOLID);
		if (trace.fraction == 1.0)
			return true;
	}
	return false;
}

/*--------------------------------------------------------------------------*/
gitem_t *flag1_item;
gitem_t *flag2_item;
void botGetFlag(edict_t *ent);

void CTFInit(void)
{
	if (!flag1_item)
		flag1_item = FindItemByClassname("item_flag_team1");
	if (!flag2_item)
		flag2_item = FindItemByClassname("item_flag_team2");
	memset(&ctfgame, 0, sizeof(ctfgame));
	techspawn = false;
	runespawn = 0;
	hstime = level.time - 10;
	//RAV flag check thinker
	flagchecktime = level.time + 45;
	redflaggone,blueflaggone = false;
}

/*--------------------------------------------------------------------------*/
char *CTFTeamName(int team)
{
	switch (team)
	{
	case CTF_TEAM1:
		return "RED";
	case CTF_TEAM2:
		return "BLUE";
	}
	return "UNKNOWN";
}

char *CTFOtherTeamName(int team)
{
	switch (team)
	{
	case CTF_TEAM1:
		return "BLUE";
	case CTF_TEAM2:
		return "RED";
	}
	return "UNKNOWN";
}

int CTFOtherTeam(int team)
{
	switch (team)
	{
	case CTF_TEAM1:
		return CTF_TEAM2;
	case CTF_TEAM2:
		return CTF_TEAM1;
	}
	return -1; // invalid value
}

/*--------------------------------------------------------------------------*/
edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
float	PlayersRangeFromSpot (edict_t *spot);
//RAV
qboolean CheckModel(edict_t *ent ,char *s);
//

void CTFAssignSkin(edict_t *ent, char *s)
{
	int playernum = ent-g_edicts-1;
	char *p;
	char t[64];
	
	if (!ctf->value)
		return;
	//RAV
	if(CheckModel (ent, s))
		Com_sprintf(t, sizeof(t), "%s", s);
	//RAV
	else
		Com_sprintf(t, sizeof(t), "male/");
	
	if ((p = strrchr(t, '/')) != NULL)
	{
		p[1] = 0;
		if (strlen(t) <= 1) strcpy(t, "male/");
	}
	else
		strcpy(t, "male/");
	//
	switch (ent->client->resp.ctf_team)
	{
	case CTF_TEAM1:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM1_SKIN) );
		break;
	case CTF_TEAM2:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s%s", ent->client->pers.netname, t, CTF_TEAM2_SKIN) );
		break;
	default:
		gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );
		break;
	}
	//	gi.dprintf(ent, PRINT_HIGH, "%s has been assigned skin: %s\n", ent->client->pers.netname);
}

void CTFAssignTeam(gclient_t *who)
{
	edict_t		*player;
	int i;
	int team1count = 0, team2count = 0;
	who->resp.ctf_state = CTF_STATE_START;
	
	if (!(dmflag & DF_CTF_FORCEJOIN))
	{
		who->resp.ctf_team = CTF_NOTEAM;
		return;
	}
	
	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];
		if (!player->inuse || player->client == who)
			continue;
		switch (player->client->resp.ctf_team)
		{
		case CTF_TEAM1:
			team1count++;
			break;
		case CTF_TEAM2:
			team2count++;
		}
	}
	if (team1count < team1count)
		who->resp.ctf_team = CTF_TEAM1;
	else if (team2count < team1count)
		who->resp.ctf_team = CTF_TEAM2;
	else if (rand() & 1)
		who->resp.ctf_team = CTF_TEAM1;
	else
		who->resp.ctf_team = CTF_TEAM2;
}

/*
================
SelectCTFSpawnPoint
go to a ctf point, but NOT the two points closest
to other players
================
*/
edict_t *SelectCTFSpawnPoint (edict_t *ent)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	char	*cname;

	if (ent->client->resp.ctf_state != CTF_STATE_START)
		if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
			return SelectFarthestDeathmatchSpawnPoint ();
		else
			return SelectRandomDeathmatchSpawnPoint ();

	ent->client->resp.ctf_state = CTF_STATE_PLAYING;

	switch (ent->client->resp.ctf_team)
	{
	case CTF_TEAM1:
		cname = "info_player_team1";
		break;
	case CTF_TEAM2:
		cname = "info_player_team2";
		break;
	default:
		return SelectRandomDeathmatchSpawnPoint();
	}

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), cname)) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return SelectRandomDeathmatchSpawnPoint();

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;
	spot = NULL;

	do
	{
		spot = G_Find (spot, FOFS(classname), cname);
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}


/*------------------------------------------------------------------------*/
/*
CTFFragBonuses
Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumaltive.  You get one, they are in importance
order.
*/

void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker)
{
	int i;
	edict_t *ent;
	gitem_t *flag_item, *enemy_flag_item;
	int otherteam;
	edict_t *flag, *carrier;
	char *c;
	vec3_t v1, v2;
	
	if (!ctf->value)
		return;
	// no bonus for fragging yourself
	if (!targ->client || !attacker->client || targ == attacker)
		return;
	otherteam = CTFOtherTeam(targ->client->resp.ctf_team);
	if (otherteam < 0)
		return; // whoever died isn't on a team
	
	// same team, if the flag at base, check to he has the enemy flag
	if (targ->client->resp.ctf_team == CTF_TEAM1)
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	}
	else
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}
	// did the attacker frag the flag carrier?
	if (targ->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
	{
		attacker->client->resp.ctf_lastfraggedcarrier = level.time;
		attacker->client->resp.score += CTF_FRAG_CARRIER_BONUS;
		if (!attacker->bot_client)
			safe_cprintf(attacker, PRINT_MEDIUM, "BONUS: %d points for fragging enemy flag carrier.\n",
			(int)CTF_FRAG_CARRIER_BONUS);
		//RAV
		// Log Flag Carrier Frag - MarkDavies
        sl_LogScore( &gi, attacker->client->pers.netname, NULL, "FC Frag", NULL, CTF_FRAG_CARRIER_BONUS, level.time);
		//end
		// the the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= maxclients->value; i++)
		{
			ent = g_edicts + i;
			if (ent->inuse && ent->client->resp.ctf_team == otherteam)
				ent->client->resp.ctf_lasthurtcarrier = 0;
		}
		return;
	}
	if (targ->client->resp.ctf_lasthurtcarrier && level.time - targ->client->resp.ctf_lasthurtcarrier < CTF_CARRIER_DANGER_PROTECT_TIMEOUT &&
		!attacker->client->pers.inventory[ITEM_INDEX(flag_item)])
	{
		// attacker is on the same team as the flag carrier and
		// fragged a guy who hurt our flag carrier
		attacker->client->resp.score += CTF_CARRIER_DANGER_PROTECT_BONUS;
		my_bprintf(PRINT_MEDIUM, "%s defends %s's flag carrier against an agressive enemy\n",
			attacker->client->pers.netname, CTFTeamName(attacker->client->resp.ctf_team));
		//RAV
		// Log Flag Danger Carrier Protect Frag - MarkDavies
        sl_LogScore( &gi, attacker->client->pers.netname, NULL, "FC Def", NULL, CTF_CARRIER_DANGER_PROTECT_BONUS, level.time );
		//end
		return;
	}
	
	// flag and flag carrier area defense bonuses
	// we have to find the flag and carrier entities
	// find the flag
	switch (attacker->client->resp.ctf_team)
	{
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	default:
		return;
	}
	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname), c)) != NULL)
	{
		if (!(flag->spawnflags & DROPPED_ITEM))
			break;
	}
	if (!flag)
		return; // can't find attacker's flag
	//3ZB
	if(attacker)
	{
		VectorSubtract(targ->s.origin,attacker->s.origin,v1);
		if(VectorLength(v1) < 300 && attacker->client && !(attacker->deadflag) && (attacker->svflags & SVF_MONSTER))
		{
			attacker->client->zc.second_target = flag;
		}
	}
	//
	// find attacker's team's flag carrier
	for (i = 1; i <= maxclients->value; i++)
	{
		carrier = g_edicts + i;
		if (carrier->inuse && carrier->client->pers.inventory[ITEM_INDEX(flag_item)])
			break;
		carrier = NULL;
	}
	// ok we have the attackers flag and a pointer to the carrier
	// check to see if we are defending the base's flag
	VectorSubtract(targ->s.origin, flag->s.origin, v1);
	VectorSubtract(attacker->s.origin, flag->s.origin, v2);
	if (VectorLength(v1) < CTF_TARGET_PROTECT_RADIUS || VectorLength(v2) < CTF_TARGET_PROTECT_RADIUS ||
		loc_CanSee(flag, targ) || loc_CanSee(flag, attacker))
	{
		// we defended the base flag
		attacker->client->resp.score += CTF_FLAG_DEFENSE_BONUS;
		if (flag->solid == SOLID_NOT)
			my_bprintf(PRINT_MEDIUM, "%s defends the %s base.\n", attacker->client->pers.netname,
			CTFTeamName(attacker->client->resp.ctf_team));
		else
			my_bprintf(PRINT_MEDIUM, "%s defends the %s flag.\n", attacker->client->pers.netname,
			CTFTeamName(attacker->client->resp.ctf_team));
		//RAV
		// Log Flag Defense - MarkDavies
        sl_LogScore( &gi, attacker->client->pers.netname, NULL, "F Def", NULL, CTF_FLAG_DEFENSE_BONUS, level.time );
		//end
		return;
	}
	
	if (carrier && carrier != attacker)
	{
		VectorSubtract(targ->s.origin, carrier->s.origin, v1);
		VectorSubtract(attacker->s.origin, carrier->s.origin, v1);
		if (VectorLength(v1) < CTF_ATTACKER_PROTECT_RADIUS || VectorLength(v2) < CTF_ATTACKER_PROTECT_RADIUS ||
			loc_CanSee(carrier, targ) || loc_CanSee(carrier, attacker))
		{
			attacker->client->resp.score += CTF_CARRIER_PROTECT_BONUS;
			my_bprintf(PRINT_MEDIUM, "%s defends the %s's flag carrier.\n",
				attacker->client->pers.netname, CTFTeamName(attacker->client->resp.ctf_team));
			//RAV
			// Log Flag Carrier Protect Frag - MarkDavies
            sl_LogScore( &gi, attacker->client->pers.netname, NULL, "FC Def", NULL, CTF_CARRIER_PROTECT_BONUS, level.time );
			//end
			return;
		}
	}
}

void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker)
{
	gitem_t *flag_item;
	if (!targ->client || !attacker->client)
		return;
	if (targ->client->resp.ctf_team == CTF_TEAM1)
		flag_item = flag2_item;
	else
		flag_item = flag1_item;
	if (targ->client->pers.inventory[ITEM_INDEX(flag_item)] &&
		targ->client->resp.ctf_team != attacker->client->resp.ctf_team)
		attacker->client->resp.ctf_lasthurtcarrier = level.time;
}

/*------------------------------------------------------------------------*/
void CTFResetFlag(int ctf_team)
{
	char *c;
	edict_t *ent;

	switch (ctf_team)
	{
	case CTF_TEAM1:
		c = "item_flag_team1";
		break;
	case CTF_TEAM2:
		c = "item_flag_team2";
		break;
	default:
		return;
	}

	ent = NULL;

	while ((ent = G_Find (ent, FOFS(classname), c)) != NULL)
	{
		if (ent->spawnflags & DROPPED_ITEM)
		{
			G_FreeEdict(ent);
		}
		else
		{
			ent->svflags &= ~SVF_NOCLIENT;
			ent->solid = SOLID_TRIGGER;
			gi.linkentity(ent);
			ent->s.event = EV_ITEM_RESPAWN;
		}
	}
	
	//RAV flag tracking
	switch (ctf_team)
	{
	case CTF_TEAM1:
		VectorCopy (redflag_origin, redflagnow);
		redflaggone = false;
		break;
	case CTF_TEAM2:
		VectorCopy (blueflag_origin, blueflagnow);
		blueflaggone = false;
		break;
	default:
		return;
	}
	//
}


void CTFResetFlags(void)

{

	CTFResetFlag(CTF_TEAM1);

	CTFResetFlag(CTF_TEAM2);

}


qboolean CTFPickup_Flag(edict_t *ent, edict_t *other)
{
	int ctf_team;
	int i;
	edict_t *player;
	gitem_t *flag_item, *enemy_flag_item;
	//JSW
	float	held_time;
	char	heldtime[64];
	//end
	
	// figure out what team this flag is
	if (strcmp(ent->classname, "item_flag_team1") == 0)
		ctf_team = CTF_TEAM1;
	else if (strcmp(ent->classname, "item_flag_team2") == 0)
		ctf_team = CTF_TEAM2;
	else
	{
		if (!ent->bot_client)
			safe_cprintf(ent, PRINT_HIGH, "Don't know what team the flag is on.\n");
		return false;
	}
	
	// same team, if the flag at base, check to he has the enemy flag
	if (ctf_team == CTF_TEAM1)
	{
		flag_item = flag1_item;
		enemy_flag_item = flag2_item;
	}
	else
	{
		flag_item = flag2_item;
		enemy_flag_item = flag1_item;
	}
	
	if (ctf_team == other->client->resp.ctf_team)
	{
		if (!(ent->spawnflags & DROPPED_ITEM))
		{
			//RAV
			//if teams are not even do not allow capture!
			if ((ctf_team == 1 && !notfairBLUE) || (ctf_team == 2 && !notfairRED))
			{
				// the flag is at home base.  if the player has the enemy
				// flag, he's just won!
				//JSW - Other no longer has flag
				other->hasflag = 0;
				held_time = level.time - other->client->resp.ctf_flagsince;
				sprintf(heldtime, "(held %1.1f seconds)", held_time);
				//end
				if (other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)])
				{
					my_bprintf(PRINT_HIGH, "%s captured the %s flag! %s\n",		//JSW added heldtime
						other->client->pers.netname, CTFOtherTeamName(ctf_team), heldtime);
					//RAV
					// Log Flag Capture - MarkDavies
					sl_LogScore( &gi, other->client->pers.netname, NULL, "F Capture",
						NULL, CTF_CAPTURE_BONUS, level.time );
					//end
					other->client->pers.inventory[ITEM_INDEX(enemy_flag_item)] = 0;
					ctfgame.last_flag_capture = level.time;
					ctfgame.last_capture_team = ctf_team;
					if (ctf_team == CTF_TEAM1)
						ctfgame.team1++;
					else
						ctfgame.team2++;
					gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagcap.wav"), 1, ATTN_NONE, 0);
					// other gets another 10 frag bonus
					other->client->resp.score += CTF_CAPTURE_BONUS;
					// Ok, let's do the player loop, hand out the bonuses
					for (i = 1; i <= maxclients->value; i++)
					{
						player = &g_edicts[i];
						if (!player->inuse)
							continue;
						if (player->client->resp.ctf_team != other->client->resp.ctf_team)
							player->client->resp.ctf_lasthurtcarrier = -5;
						else if (player->client->resp.ctf_team == other->client->resp.ctf_team)
						{
							if (player != other)
							{	//RAV
								player->client->resp.score += CTF_TEAM_BONUS;
								// Log Flag Capture Team Score - MarkDavies
								sl_LogScore( &gi, player->client->pers.netname,
									NULL, "Team Score", NULL, CTF_TEAM_BONUS, level.time );
							}
							//end
							// award extra points for capture assists
							if (player->client->resp.ctf_lastreturnedflag + CTF_RETURN_FLAG_ASSIST_TIMEOUT > level.time)
							{
								my_bprintf(PRINT_HIGH, "%s gets an assist for returning the flag!\n", player->client->pers.netname);
								player->client->resp.score += CTF_RETURN_FLAG_ASSIST_BONUS;
								//RAV
								// Log Flag Capture Team Score - MarkDavies
								sl_LogScore( &gi, player->client->pers.netname, NULL, "F Return Assist",
                                         NULL, CTF_RETURN_FLAG_ASSIST_BONUS, level.time );
                            //end
							}
							
							if (player->client->resp.ctf_lastfraggedcarrier + CTF_FRAG_CARRIER_ASSIST_TIMEOUT > level.time)
							{
								my_bprintf(PRINT_HIGH, "%s gets an assist for fragging the flag carrier!\n", player->client->pers.netname);
								player->client->resp.score += CTF_FRAG_CARRIER_ASSIST_BONUS;
								//RAV
								// Log Flag Capture Team Score - MarkDavies
								sl_LogScore( &gi, player->client->pers.netname, NULL,
									"FC Frag Assist", NULL, CTF_FRAG_CARRIER_ASSIST_BONUS, level.time );
								//end
							}
						}
					}
					
				
					CTFResetFlags();
					return false;
				}
			}//RAV

			return false; // its at home base already
		}
		// hey, its not home.  return it by teleporting it back
		my_bprintf(PRINT_HIGH, "%s returned the %s flag!\n", other->client->pers.netname, CTFTeamName(ctf_team));
		//RAV
		// Log Flag Recover - MarkDavies
        sl_LogScore( &gi, other->client->pers.netname, NULL,
			"F Return", NULL, CTF_RECOVERY_BONUS, level.time );
		//end
		other->client->resp.score += CTF_RECOVERY_BONUS;
		other->client->resp.ctf_lastreturnedflag = level.time;
		gi.sound (ent, CHAN_RELIABLE+CHAN_NO_PHS_ADD+CHAN_VOICE, gi.soundindex("ctf/flagret.wav"), 1, ATTN_NONE, 0);
		//CTFResetFlag will remove this entity!  We must return false
		CTFResetFlag(ctf_team);
		return false;
	}
	
	// hey, its not our flag, pick it up
	my_bprintf(PRINT_HIGH, "%s got the %s flag!\n",
		other->client->pers.netname, CTFTeamName(ctf_team));
	other->client->resp.score += CTF_FLAG_BONUS;
	//RAV
	// Log Flag Pickup - MarkDavies
    sl_LogScore( &gi, other->client->pers.netname, NULL, "F Pickup", NULL, CTF_FLAG_BONUS, level.time );
	//end
	
	other->client->pers.inventory[ITEM_INDEX(flag_item)] = 1;
	other->client->resp.ctf_flagsince = level.time;

	//JSW - Set flag carrier var
	other->hasflag = 1;
	//end
	
	// pick up the flag
	// if it's not a dropped flag, we just make is disappear
	// if it's dropped, it will be removed by the pickup caller
	if (!(ent->spawnflags & DROPPED_ITEM))
	{
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	
	return true;
}


void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)

{

	//owner (who dropped us) can't touch for two secs

	if ((other == ent->owner) && (ent->timestamp > level.time - 1.0))
		return;

	ent->timestamp = level.time;
	Touch_Item (ent, other, plane, surf);

}


static void CTFDropFlagThink(edict_t *ent)
{
	if ((ent->timestamp + CTF_AUTO_FLAG_RETURN_TIMEOUT < level.time) || level.intermissiontime || ((mapvote->value) && (level.time+(int)menutime->value-1 < votetime)))//raven
	{
		// auto return the flag
		// reset flag will remove ourselves
		if (strcmp(ent->classname, "item_flag_team1") == 0)
		{
			CTFResetFlag(CTF_TEAM1);
			safe_bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(CTF_TEAM1));
		}
		else if (strcmp(ent->classname, "item_flag_team2") == 0)
		{
			CTFResetFlag(CTF_TEAM2);
			safe_bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(CTF_TEAM2));
		}
	}
	ent->nextthink = level.time + 0.2;
}


// Called from PlayerDie, to drop the flag from a dying player

void CTFDeadDropFlag(edict_t *self)
{
	edict_t *dropped = NULL;
	
	if (!ctf->value)
		return;

	if (!flag1_item || !flag2_item)
		CTFInit();
	
	if (self->client->pers.inventory[ITEM_INDEX(flag1_item)])
	{
		dropped = Drop_Item(self, flag1_item);
		self->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM1));
	}
	else if (self->client->pers.inventory[ITEM_INDEX(flag2_item)])
	{
		dropped = Drop_Item(self, flag2_item);
		self->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
		my_bprintf(PRINT_HIGH, "%s lost the %s flag!\n",
			self->client->pers.netname, CTFTeamName(CTF_TEAM2));
	}
	
	if (dropped)
	{
		dropped->think = CTFDropFlagThink;
		dropped->timestamp = level.time;
		dropped->nextthink = level.time + 0.2;
		dropped->touch = CTFDropFlagTouch;
	}

	//JSW - clear flag carrier var
	self->hasflag = 0;
}

void CTFDrop_Flag(edict_t *ent, gitem_t *item)

{
	edict_t *dropped = NULL;
	
	if (!ctf->value)
		return;

	if (!flag1_item || !flag2_item)
		CTFInit();
	
	if (allow_flagdrop->value)
	{
		if (ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
		{
			dropped = Drop_Item(ent, flag1_item);
			ent->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
			my_bprintf(PRINT_HIGH, "%s dropped the %s flag!\n",
				ent->client->pers.netname, CTFTeamName(CTF_TEAM1));
		}
		else if (ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
		{
			dropped = Drop_Item(ent, flag2_item);
			ent->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
			my_bprintf(PRINT_HIGH, "%s dropped the %s flag!\n",
				ent->client->pers.netname, CTFTeamName(CTF_TEAM2));
		}
		
		if (dropped)
		{
			dropped->owner = ent;
			dropped->nextthink = level.time + 0.2;
			dropped->think = CTFDropFlagThink;
			dropped->timestamp = level.time;
			dropped->touch = CTFDropFlagTouch;
		}

		//JSW - clear flag carrier var
		ent->hasflag = 0;
		return;
	}

	if (rand() & 1) 

	{

		if (!ent->bot_client)

			safe_cprintf(ent, PRINT_HIGH, "Only lusers drop flags.\n");

	}

	else

	{

		if (!ent->bot_client)

			safe_cprintf(ent, PRINT_HIGH, "Winners don't drop flags.\n");

	}

//	return;

}


float	team1_rushbase_time, team2_rushbase_time;	// used by RUSHBASE command

float	team1_defendbase_time, team2_defendbase_time;


static void CTFFlagThink(edict_t *ent)


{


	if (ent->solid != SOLID_NOT)


		ent->s.frame = 173 + (((ent->s.frame - 173) + 1) % 16);


	ent->nextthink = level.time + FRAMETIME;


}

//3ZB
void droptofloor (edict_t *ent);
void SpawnItem3 (edict_t *ent, gitem_t *item);
void ChainPodThink (edict_t *ent);
qboolean ChkTFlg(void);//���Z�b�g�A�b�v�ς݁H
void SetBotFlag1(edict_t *ent);	//�`�[��1�̊�
void SetBotFlag2(edict_t *ent);  //�`�[��2�̊�

//


void CTFFlagSetup (edict_t *ent)

{

	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else //if(ent->item->world_model)		//3ZB
		gi.setmodel (ent, ent->item->world_model);
//	else ent->s.modelindex = 0;			
	//


	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);
	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("CTFFlagSetup: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);
	gi.linkentity (ent);	
	ent->nextthink = level.time + FRAMETIME;
	ent->think = CTFFlagThink;
	return;
}


void CTFEffects(edict_t *player)

{

	player->s.effects &= ~(EF_FLAG1 | EF_FLAG2);

	if (player->health > 0) {

		if (player->client->pers.inventory[ITEM_INDEX(flag1_item)]) {

			player->s.effects |= EF_FLAG1;

		}

		if (player->client->pers.inventory[ITEM_INDEX(flag2_item)]) {

			player->s.effects |= EF_FLAG2;

		}

	}


	if (player->client->pers.inventory[ITEM_INDEX(flag1_item)])

		player->s.modelindex3 = gi.modelindex("players/male/flag1.md2");

	else if (player->client->pers.inventory[ITEM_INDEX(flag2_item)])

		player->s.modelindex3 = gi.modelindex("players/male/flag2.md2");

	else

		player->s.modelindex3 = 0;

}


// called when we enter the intermission

void CTFCalcScores(void)

{

	int i;


	ctfgame.total1 = ctfgame.total2 = 0;

	for (i = 0; i < maxclients->value; i++) {

		if (!g_edicts[i+1].inuse)

			continue;

		if (game.clients[i].resp.ctf_team == CTF_TEAM1)

			ctfgame.total1 += game.clients[i].resp.score;

		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)

			ctfgame.total2 += game.clients[i].resp.score;

	}

}

//JSW
void ResetCaps (void)
{
	ctfgame.team1 = 0;
	ctfgame.team2 = 0;
}
//end

/*
void CTFID_f (edict_t *ent)
{

	if (ent->client->resp.id_state) {

		if (!ent->bot_client)

			safe_cprintf(ent, PRINT_HIGH, "Disabling player identication display.\n");

		ent->client->resp.id_state = false;

	} else {

		if (!ent->bot_client)

			safe_cprintf(ent, PRINT_HIGH, "Activating player identication display.\n");

		ent->client->resp.id_state = true;

	}

}
*/

/*
static void CTFSetIDView(edict_t *ent)

{

	vec3_t	forward, dir;

	trace_t	tr;

	edict_t	*who, *best;

	float	bd = 0, d;

	int i;


//	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;


	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	VectorScale(forward, 1024, forward);

	VectorAdd(ent->s.origin, forward, forward);

	tr = gi.trace(ent->s.origin, NULL, NULL, forward, ent, MASK_SOLID);

	if (tr.fraction < 1 && tr.ent && tr.ent->client) {

		ent->client->ps.stats[STAT_CTF_ID_VIEW] = 

			CS_PLAYERSKINS + (ent - g_edicts - 1);

		return;

	}


	AngleVectors(ent->client->v_angle, forward, NULL, NULL);

	best = NULL;

	for (i = 1; i <= maxclients->value; i++) {

		who = g_edicts + i;

		if (!who->inuse)

			continue;

		VectorSubtract(who->s.origin, ent->s.origin, dir);

		VectorNormalize(dir);

		d = DotProduct(forward, dir);

		if (d > bd && loc_CanSee(ent, who)) {

			bd = d;

			best = who;

		}

	}

//	if (bd > 0.90)

	//	ent->client->ps.stats[STAT_CTF_ID_VIEW] = 

	//		CS_PLAYERSKINS + (best - g_edicts - 1);

}
*/

/*
void SetCTFStats(edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2;
	edict_t *e;

	//T-MEK
	int rfc;  //red flag carrier
	int bfc;  //blue flag carrier
	rfc = 0;
	bfc = 0;
	//T-MEK

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("ctfsb2");

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (ctfgame.team1 > ctfgame.team2)
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.team2 > ctfgame.team1)
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.total2 > ctfgame.total1) 
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else { // tie game!
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		}
	}

	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	while (tnames[i]) {
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)]) {
			ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
			break;
		}
		i++;
	}

	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = gi.imageindex ("i_ctf1");
	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p1 = gi.imageindex ("i_ctf1d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)]) {
					// enemy has it
					p1 = gi.imageindex ("i_ctf1t");
					bfc=i;  //T-MEK
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p1 = gi.imageindex ("i_ctf1d"); // must be dropped
	}
	p2 = gi.imageindex ("i_ctf2");
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL) {
		if (e->solid == SOLID_NOT) {
			int i;

			// not at base
			// check if on player
			p2 = gi.imageindex ("i_ctf2d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)]) {
					// enemy has it
					p2 = gi.imageindex ("i_ctf2t");
					rfc = i; //T-MEK
					break;
				}
		} else if (e->spawnflags & DROPPED_ITEM)
			p2 = gi.imageindex ("i_ctf2d"); // must be dropped
	}


	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;

	if (ctfgame.last_flag_capture && level.time - ctfgame.last_flag_capture < 5) {
		if (ctfgame.last_capture_team == CTF_TEAM1)
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		else
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
	}

	ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.team1;
	ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.team2;


	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");



	//T-MEK
	if (rfc != 0 )
		rfc = rfc + (CS_PLAYERNAMESRIGHT - 1);
	else
		rfc = CS_EMPTYSTRING;

	if (bfc != 0 )
		bfc = bfc + (CS_PLAYERNAMESRIGHT - 1);
	else
		bfc = CS_EMPTYSTRING;

	
	ent->client->ps.stats[STAT_CTF_RED_FLAG_CARRIER] = rfc;
	ent->client->ps.stats[STAT_CTF_BLUE_FLAG_CARRIER] = bfc;


}
*/

void SetCTFStats(edict_t *ent)
{
	gitem_t *tech;
	int i;
	int p1, p2;
	edict_t *e;
	int rfc;  //red flag carrier
	int bfc;  //blue flag carrier
	rfc = 0;
	bfc = 0;

	// logo headers for the frag display
	ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = gi.imageindex ("ctfsb1");
	ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = gi.imageindex ("ctfsb2");

	// if during intermission, we must blink the team header of the winning team
	if (level.intermissiontime && (level.framenum & 8)) { // blink 1/8th second
		// note that ctfgame.total[12] is set when we go to intermission
		if (ctfgame.team1 > ctfgame.team2)
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.team2 > ctfgame.team1)
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else if (ctfgame.total1 > ctfgame.total2) // frag tie breaker
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
		else if (ctfgame.total2 > ctfgame.total1) 
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		else
		{ // tie game!
			ent->client->ps.stats[STAT_CTF_TEAM1_HEADER] = 0;
			ent->client->ps.stats[STAT_CTF_TEAM2_HEADER] = 0;
		}
	}
	// tech icon
	i = 0;
	ent->client->ps.stats[STAT_CTF_TECH] = 0;
	while (tnames[i])
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL &&
			ent->client->pers.inventory[ITEM_INDEX(tech)])
		{
			ent->client->ps.stats[STAT_CTF_TECH] = gi.imageindex(tech->icon);
			break;
		}
		i++;
	}
	// figure out what icon to display for team logos
	// three states:
	//   flag at base
	//   flag taken
	//   flag dropped
	p1 = gi.imageindex ("i_ctf1");
	e = G_Find(NULL, FOFS(classname), "item_flag_team1");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;
			// not at base
			// check if on player
			p1 = gi.imageindex ("i_ctf1d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag1_item)])
				{
					// enemy has it
					p1 = gi.imageindex ("i_ctf1t");
					bfc = i; //
					break;
				}
		}
		else if (e->spawnflags & DROPPED_ITEM)
			p1 = gi.imageindex ("i_ctf1d"); // must be dropped
	}
	p2 = gi.imageindex ("i_ctf2");
	e = G_Find(NULL, FOFS(classname), "item_flag_team2");
	if (e != NULL)
	{
		if (e->solid == SOLID_NOT)
		{
			int i;
			// not at base
			// check if on player
			p2 = gi.imageindex ("i_ctf2d"); // default to dropped
			for (i = 1; i <= maxclients->value; i++)
				if (g_edicts[i].inuse &&
					g_edicts[i].client->pers.inventory[ITEM_INDEX(flag2_item)])
				{
					// enemy has it
					p2 = gi.imageindex ("i_ctf2t");
					rfc = i; //
					break;
				}
		}
		else if (e->spawnflags & DROPPED_ITEM)
			p2 = gi.imageindex ("i_ctf2d"); // must be dropped
	}
	ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
	ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
	if (ctfgame.last_flag_capture && level.time - ctfgame.last_flag_capture < 5)
	{
		if (ctfgame.last_capture_team == CTF_TEAM1)
		{
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = p1;
			else
				ent->client->ps.stats[STAT_CTF_TEAM1_PIC] = 0;
		}
		else
		{
			if (level.framenum & 8)
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = p2;
			else
				ent->client->ps.stats[STAT_CTF_TEAM2_PIC] = 0;
		}
	}
	ent->client->ps.stats[STAT_CTF_TEAM1_CAPS] = ctfgame.team1;
	ent->client->ps.stats[STAT_CTF_TEAM2_CAPS] = ctfgame.team2;
	ent->client->ps.stats[STAT_CTF_FLAG_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1 &&
		ent->client->pers.inventory[ITEM_INDEX(flag2_item)] && (level.framenum & 8))
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf2");
	else if (ent->client->resp.ctf_team == CTF_TEAM2 &&
		ent->client->pers.inventory[ITEM_INDEX(flag1_item)] && (level.framenum & 8))
		ent->client->ps.stats[STAT_CTF_FLAG_PIC] = gi.imageindex ("i_ctf1");
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = 0;
	ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM1_PIC] = gi.imageindex ("i_ctfj");
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		ent->client->ps.stats[STAT_CTF_JOINED_TEAM2_PIC] = gi.imageindex ("i_ctfj");
	//ent->client->ps.stats[STAT_CTF_ID_VIEW] = 0;
	//if (ent->client->resp.id_state)
	//	CTFSetIDView(ent);
	if (rfc != 0 )
		rfc = rfc + (CS_PLAYERNAMESRIGHT - 1);
	else
		rfc = CS_EMPTYSTRING;
	if (bfc != 0 )
		bfc = bfc + (CS_PLAYERNAMESRIGHT - 1);
	else
		bfc = CS_EMPTYSTRING;
	if(show_carrier->value)
	{
		ent->client->ps.stats[STAT_CTF_RED_FLAG_CARRIER] = rfc;
		ent->client->ps.stats[STAT_CTF_BLUE_FLAG_CARRIER] = bfc;
	}
	else
	{
		ent->client->ps.stats[STAT_CTF_RED_FLAG_CARRIER] = CS_EMPTYSTRING;
		ent->client->ps.stats[STAT_CTF_BLUE_FLAG_CARRIER] = CS_EMPTYSTRING;
	}
}

/*------------------------------------------------------------------------*/


/*QUAKED info_player_team1 (1 0 0) (-16 -16 -24) (16 16 32)

potential team1 spawning position for ctf games

*/

void SP_info_player_team1(edict_t *self)

{

}


/*QUAKED info_player_team2 (0 0 1) (-16 -16 -24) (16 16 32)

potential team2 spawning position for ctf games

*/

void SP_info_player_team2(edict_t *self)

{

}



//*------------------------------------------------------------------------*/
/* GRAPPLE																  */
/*------------------------------------------------------------------------*/

// ent is player
void CTFPlayerResetGrapple(edict_t *ent)
{
	int i;
	edict_t	*e;
	vec3_t v,vv;

//PON-CTF
	if(chedit->value && ent == &g_edicts[1] && ent->client->ctf_grapple)
	{
		e = (edict_t*)ent->client->ctf_grapple;
		VectorCopy(e->s.origin,vv);

		for(i = 1;(CurrentIndex - i) > 0;i++)
		{
			if(Route[CurrentIndex - i].state == GRS_GRAPHOOK) break;
			else if(Route[CurrentIndex - i].state == GRS_GRAPSHOT) break;
		}
		if(Route[CurrentIndex - i].state == GRS_GRAPHOOK)
		{
			Route[CurrentIndex].state = GRS_GRAPRELEASE;
			VectorCopy(ent->s.origin,Route[CurrentIndex].Pt);
			VectorSubtract(ent->s.origin,vv,v);
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
	}
//PON-CTF

	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
	ent->s.sound = 0;
}

// self is grapple, not player
void CTFResetGrapple(edict_t *self)
{
	

	if(self->owner == NULL)
	{
		G_FreeEdict(self);
		return;
	}
	self->s.sound = 0;

	self->owner->hooktime = 0;
	
	if (self->owner->client->ctf_grapple) {
		float volume = 1.0;
		gclient_t *cl;

		if (self->owner->client->silencer_shots)
			volume = 0.2;

		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grreset.wav"), volume, ATTN_NORM, 0);
		cl = self->owner->client;
		cl->ctf_grapple = NULL;
		cl->ctf_grapplereleasetime = level.time;
		cl->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
		cl->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	
		//clean up the laser entity
		if (self->laser) 
		G_FreeEdict(self->laser);

		
		G_FreeEdict(self);
	}
}

void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	short	i;
	float volume = 1.0;

	if (other == self->owner)
		return;

	if (self->owner->client->ctf_grapplestate != CTF_GRAPPLE_STATE_FLY)
		return;

	if (surf && (surf->flags & SURF_SKY)&& (!hook_sky->value))
	{
		CTFResetGrapple(self);
		return;
	}

	VectorCopy(vec3_origin, self->velocity);

//	PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	if (other->takedamage)
	{
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, grapple_damage->value  , grapple_damage->value  , 0, MOD_GRAPPLE);
		if(reset_hook->value)
		{
			CTFResetGrapple(self);
			return;
		}
	}

	self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_PULL; // we're on hook
	self->enemy = other;

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
			gi.bprintf(PRINT_HIGH,"Grapple has been hooked.Last %i pod(s).\n",MAXNODES - CurrentIndex);
			memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
		}
	}
//PON-CTF

	self->solid = SOLID_NOT;

	if (self->owner->client->silencer_shots)
		volume = 0.2;

	gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grpull.wav"), volume, ATTN_NORM, 0);
	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhit.wav"), volume, ATTN_NORM, 0);

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_SPARKS);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
//========================================================
//==========================================================
// This is the same as function P_ProjectSource in the source 
// except it projects in reverse...
//==========================================================
void P_ProjectSource_Reverse(gclient_t *client,
vec3_t point,
vec3_t distance,
vec3_t forward,
vec3_t right,
vec3_t result)
{
vec3_t dist; 

VectorCopy(distance, dist);
if (client->pers.hand == RIGHT_HANDED)
dist[1] *= -1; // Left Hand already defaulted
else if (client->pers.hand == CENTER_HANDED)
dist[1]= 0;
G_ProjectSource(point, dist, forward, right, result);
} 


// draw beam between grapple and self
void CTFGrappleDrawCable(edict_t *self)
{
	vec3_t	offset, start, end, f, r;
	vec3_t	dir;
	float	distance;
	float	x;

	if(1//!(self->owner->svflags & SVF_MONSTER)
)
	{
		AngleVectors (self->owner->client->v_angle, f, r, NULL);
		VectorSet(offset, 16, -16, self->owner->viewheight-8);
//		if(hook_offhand->value)
		P_ProjectSource_Reverse (self->owner->client, self->owner->s.origin, offset, f, r, start);
//		else
//		P_ProjectSource (self->owner->client, self->owner->s.origin, offset, f, r, start);
	}
	else
	{
		x = self->owner->s.angles[YAW] ;
		x = x * M_PI * 2 / 360;
		start[0] = self->owner->s.origin[0] + cos(x) * 16;
		start[1] = self->owner->s.origin[1] + sin(x) * 16;
		if(self->owner->maxs[2] >=32) start[2] = self->owner->s.origin[2]+16;
		else start[2] = self->owner->s.origin[2]-12;
	}

	VectorSubtract(start, self->owner->s.origin, offset);

	VectorSubtract (start, self->s.origin, dir);
	distance = VectorLength(dir);

	// don't draw cable if close
	if (distance < 64)
		return;

#if 0
	if (distance > 256)
		return;

	// check for min/max pitch
	vectoangles (dir, angles);
	if (angles[0] < -180)
		angles[0] += 360;
	if (fabs(angles[0]) > 45)
		return;

	trace_t	tr; //!!

	tr = gi.trace (start, NULL, NULL, self->s.origin, self, MASK_SHOT);
	if (tr.ent != self) {
		CTFResetGrapple(self);
		return;
	}
#endif

	// adjust start for beam origin being in middle of a segment
	VectorMA (start, 8, f, start);

	VectorCopy (self->s.origin, end);


	// adjust end z for end spot since the monster is currently dead
//	end[2] = self->absmin[2] + self->size[2] / 2;

if (use_grapple->value)
{
	gi.WriteByte (svc_temp_entity);
//#if 1 //def USE_GRAPPLE_CABLE
	gi.WriteByte (TE_GRAPPLE_CABLE);
	gi.WriteShort (self->owner - g_edicts);
	gi.WritePosition (self->owner->s.origin);
	gi.WritePosition (end);
	gi.WritePosition (offset);
//#else
//	gi.WriteByte (TE_MEDIC_CABLE_ATTACK);
//	gi.WriteShort (self - g_edicts);
//	gi.WritePosition (end);
//	gi.WritePosition (start);
//#endif
	gi.multicast (self->s.origin, MULTICAST_PVS);
}
  }

void hackLift(edict_t *player) ;
void SV_AddGravity (edict_t *ent);

/////////////////////////////////////////////////////////////////////////////////////////


// pull the player toward the grapple
void CTFGrapplePull(edict_t *self)
{
	vec3_t hookdir, v;
	float vlen;

	if(self->owner == NULL)
	{
		CTFResetGrapple(self);
		return;
	}

	//timer in action ?
		if(hook_maxtime->value && self->owner->hooktime !=0
			&& self->owner->hooktime <= level.time){
			CTFResetGrapple(self);
			return;
		}

	if (self->enemy) {
		if (self->enemy->solid == SOLID_NOT) {
			CTFResetGrapple(self);
			return;
		}
		if (self->enemy->solid == SOLID_BBOX) {
			VectorScale(self->enemy->size, 0.5, v);
			VectorAdd(v, self->enemy->s.origin, v);
			VectorAdd(v, self->enemy->mins, self->s.origin);
			gi.linkentity (self);
		} else
			VectorCopy(self->enemy->velocity, self->velocity);
		if (self->enemy->takedamage &&
			!CheckTeamDamage (self->enemy, self->owner)) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			T_Damage (self->enemy, self, self->owner, self->velocity, self->s.origin, vec3_origin, 1, 1, 0, MOD_GRAPPLE);
		//	gi.sound (self, CHAN_WEAPON, gi.soundindex("weapons/grapple/grhurt.wav"), volume, ATTN_NORM, 0);
		}
		if (self->enemy->deadflag) { // he died
			CTFResetGrapple(self);
			return;
		}
	}

	CTFGrappleDrawCable(self);

//	hackLift(self);
	
	if (self->owner->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		// pull player toward grapple
		// this causes icky stuff with prediction, we need to extend
		// the prediction layer to include two new fields in the player
		// move stuff: a point and a velocity.  The client should add
		// that velociy in the direction of the point
		vec3_t forward, up;

		if(!(self->owner->bot_client))
		{
			AngleVectors (self->owner->client->v_angle, forward, NULL, up);
			VectorCopy(self->owner->s.origin, v);
			v[2] += self->owner->viewheight;
			VectorSubtract (self->s.origin, v, hookdir);
		}
	else
	{
			VectorCopy(self->owner->s.origin, v);
			if(self->owner->maxs[2] >=32) v[2] += 16;
			else v[2] -= 12;
			VectorSubtract (self->s.origin, v, hookdir);
		}


		vlen = VectorLength(hookdir);

		if (self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL &&
			vlen < 64) {
			float volume = 1.0;

			if (self->owner->client->silencer_shots)
				volume = 0.2;

			self->owner->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION;
		gi.sound (self->owner, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grhang.wav"), volume, ATTN_NORM, 0);
			self->owner->client->ctf_grapplestate = CTF_GRAPPLE_STATE_HANG;
		}

		VectorNormalize (hookdir);
		if(self->owner->bot_client)
		VectorScale(hookdir, 750, hookdir);
		else
		VectorScale(hookdir, CTF_GRAPPLE_PULL_SPEED, hookdir);
		VectorCopy(hookdir, self->owner->velocity);
		SV_AddGravity(self->owner);
	}
}


void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect)
{
	edict_t	*grapple;
	trace_t	tr;

	VectorNormalize (dir);

	grapple = G_Spawn();
	VectorCopy (start, grapple->s.origin);
	VectorCopy (start, grapple->s.old_origin);
	vectoangles (dir, grapple->s.angles);
	VectorScale (dir, speed, grapple->velocity);
	grapple->movetype = MOVETYPE_FLYMISSILE;
	grapple->clipmask = MASK_SHOT;
	grapple->solid = SOLID_BBOX;
	grapple->s.effects |= effect;
	VectorClear (grapple->mins);
	VectorClear (grapple->maxs);
//	grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
//	grapple->s.sound = gi.soundindex ("misc/lasfly.wav");
	grapple->owner = self;
	grapple->touch = CTFGrappleTouch;
//	grapple->nextthink = level.time + FRAMETIME;
//	grapple->think = CTFGrappleThink;
	grapple->dmg = damage;
	self->client->ctf_grapple = grapple;
	self->client->ctf_grapplestate = CTF_GRAPPLE_STATE_FLY; // we're firing, not on hook
	// start up the laser
	if (use_grapple->value)
	{
		grapple->s.modelindex = gi.modelindex ("models/weapons/grapple/hook/tris.md2");
	}
	else
	{
		grapple->laser = abandon_hook_laser_start(grapple);
	}

	
	gi.linkentity (grapple);

	//start the hook reset timer 
	self->hooktime = level.time +(int)hook_maxtime->value;

	tr = gi.trace (self->s.origin, NULL, NULL, grapple->s.origin, grapple, MASK_SHOT);
	if (tr.fraction < 1.0)
	{
		VectorMA (grapple->s.origin, -10, dir, grapple->s.origin);
		grapple->touch (grapple, tr.ent, NULL, NULL);
	}
	hackLift(self);
//PON-CTF
	if(chedit->value && self == &g_edicts[1])
	{
		Route[CurrentIndex].state = GRS_GRAPSHOT;
		VectorCopy(self->s.origin,Route[CurrentIndex].Pt);

		if(++CurrentIndex < MAXNODES)
		{
			gi.bprintf(PRINT_HIGH,"Hook has been fired.Last %i pod(s).\n",MAXNODES - CurrentIndex);
			memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
		}
	}
//PON-CTF

}


void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect)
{
	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	offset;
	float volume = 1.0;

	if (ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY)
		return; // it's already out

	AngleVectors (ent->client->v_angle, forward, right, NULL);
/*
	if(ent->bot_client){
	VectorSet(offset, 24, -16, ent->viewheight-8+2);
	}
	else*/
	VectorSet(offset, 24, 16, ent->viewheight-8+2);

//	VectorAdd (offset, g_offset, offset);
//	if(hook_offhand->value)
//	P_ProjectSource_Reverse (ent->client, ent->s.origin, offset, forward, right, start);
//	else
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	if (ent->client->silencer_shots)
		volume = 0.2;

//	gi.positioned_sound(ent->s.origin, ent, CHAN_WEAPON, gi.soundindex("flyer/Flyatck2.wav"), 1, ATTN_NORM, 0);

	gi.sound (ent, CHAN_RELIABLE+CHAN_WEAPON, gi.soundindex("weapons/grapple/grfire.wav"), volume, ATTN_NORM, 0);
	CTFFireGrapple (ent, start, forward, damage, CTF_GRAPPLE_SPEED, effect);
/* 
#if 0
	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_BLASTER);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
#endif

	PlayerNoise(ent, start, PNOISE_WEAPON);
	*/
}


void CTFWeapon_Grapple_Fire (edict_t *ent)
{
	int		damage;

	damage = 10;
	CTFGrappleFire (ent, vec3_origin, damage, 0);
	ent->client->ps.gunframe++;
}

void CTFWeapon_Grapple (edict_t *ent)
{
	static int	pause_frames[]	= {10, 18, 27, 0};
	static int	fire_frames[]	= {6, 0};
	int prevstate,i;
	vec3_t	v,vv;
	edict_t *e;

	// if the the attack button is still down, stay in the firing frame
	if ((ent->client->buttons & BUTTON_ATTACK) &&
		ent->client->weaponstate == WEAPON_FIRING &&
		ent->client->ctf_grapple)
		ent->client->ps.gunframe = 9;

	if (!(ent->client->buttons & BUTTON_ATTACK) &&
		ent->client->ctf_grapple)
	{
		i = ent->client->ctf_grapplestate;
		e = (edict_t*)ent->client->ctf_grapple;
		VectorCopy(e->s.origin,vv);
		CTFResetGrapple(ent->client->ctf_grapple);
		if (ent->client->weaponstate == WEAPON_FIRING)
			ent->client->weaponstate = WEAPON_READY;

		if(ent->svflags & SVF_MONSTER)
		{
			if(ent->waterlevel
				|| i == CTF_GRAPPLE_STATE_HANG ) VectorClear(ent->velocity);
			//fix the moving speed
			if(i == CTF_GRAPPLE_STATE_PULL)
			{
				v[0] = ent->velocity[0];
				v[1] = ent->velocity[1];
				v[2] = 0;

				ent->client->zc.moveyaw = Get_yaw(v);
				ent->moveinfo.speed = (VectorLength(v) * FRAMETIME) / MOVE_SPD_RUN;
				ent->velocity[0] = 0;
				ent->velocity[1] = 0;
			}
			else if(i == CTF_GRAPPLE_STATE_HANG )
			{
				ent->moveinfo.speed = 0;
				ent->velocity[0] = 0;
				ent->velocity[1] = 0;
			}
		}
//PON-CTF
		if(chedit->value && ent == &g_edicts[1])
		{
			for(i = 0;(CurrentIndex - i) > 0;i++)
			{
				if(Route[CurrentIndex - i].state == GRS_GRAPHOOK) break;
				else if(Route[CurrentIndex - i].state == GRS_GRAPSHOT) break;
			}
			if(Route[CurrentIndex - i].state == GRS_GRAPHOOK)
			{
				Route[CurrentIndex].state = GRS_GRAPRELEASE;
				VectorCopy(ent->s.origin,Route[CurrentIndex].Pt);
				VectorSubtract(ent->s.origin,vv,v);
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
					gi.bprintf(PRINT_HIGH,"Hook has been released.Last %i pod(s).\n",MAXNODES - CurrentIndex);
					memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
					Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
				}
			}
		}
//PON-CTF

	}


	if (ent->client->newweapon &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY &&
		ent->client->weaponstate == WEAPON_FIRING) {
		// he wants to change weapons while grappled
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = 32;
	}

	prevstate = ent->client->weaponstate;
	Weapon_Generic (ent, 5, 9, 31, 36, pause_frames, fire_frames,
		CTFWeapon_Grapple_Fire);


	if(ent->svflags & SVF_MONSTER)
	{
		return;
	}

	// if we just switched back to grapple, immediately go to fire frame
	if (prevstate == WEAPON_ACTIVATING &&
		ent->client->weaponstate == WEAPON_READY &&
		ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) {
		if (!(ent->client->buttons & BUTTON_ATTACK))
			ent->client->ps.gunframe = 9;
		else
			ent->client->ps.gunframe = 5;
		ent->client->weaponstate = WEAPON_FIRING;
	}
}





/////////////////////////////////////////////////////////////////////////////////////////

//RaVeN aded offhand hook for map chaining 
void RaV_hook (edict_t *ent)
{
		if (ent->client->pers.weapon != FindItem ("Grapple"))
				CTFGrappleFire (ent, vec3_origin, 10, 0);
		else 
			safe_cprintf (ent, PRINT_HIGH, "Offhand grapple not available.\n");
	
	return;
}

void RaV_unhook (edict_t *ent)
{
	CTFPlayerResetGrapple(ent);
		return;
}




qboolean hook_cond_reset(edict_t *self);

void CTFTeam_f (edict_t *ent, int desired_team)
{
	char *t, *s;
	//int desired_team;
	if (!ctf->value)
		return;
	
	if (ent->client->resp.teamswitch > level.time)
		return;

	ent->client->resp.teamswitch = level.time + 3;
	
	if (ent->classname[0] == 'c')
	{	// in CAM mode
		safe_cprintf(ent, PRINT_HIGH, "You are in CAM mode. You must reconnect or restart the game to rejoin the action.\n");
		return;
	}
	
	t = gi.args();
	if (Q_stricmp(t, "red") == 0)
		desired_team = CTF_TEAM1;
	else if (Q_stricmp(t, "blue") == 0)
		desired_team = CTF_TEAM2;

	
	if (!desired_team)
	{
		if (!ent->bot_client)
			safe_cprintf(ent, PRINT_HIGH, "You are on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}
	
	if (desired_team != CTF_TEAM1 && desired_team != CTF_TEAM2)
	{
		if (!ent->bot_client)
			safe_cprintf(ent, PRINT_HIGH, "Unknown team %s.\n", t);
		return;
	}
	
	if (ent->client->resp.ctf_team == desired_team)
	{
		if (!ent->bot_client)
			safe_cprintf(ent, PRINT_HIGH, "You are already on the %s team.\n",
			CTFTeamName(ent->client->resp.ctf_team));
		return;
	}

	if(notfairRED && desired_team == CTF_TEAM1)
	{
		safe_cprintf(ent, PRINT_HIGH, "You cannot join that team at this time (unfair teams.)\n");
		return;
	}

	if(notfairBLUE && desired_team == CTF_TEAM2)
	{
		safe_cprintf(ent, PRINT_HIGH, "You cannot join that team at this time (unfair teams.)\n");
		return;
	}

	CheckPlayers();

	////
	if(!ent->bot_client)
	{
		////RAV
		if(chedit->value)
		{//for node making !!!
			stuffcmd(ent, "alias +hook \"cmd hook\"\n");
			stuffcmd(ent, "alias -hook \"cmd unhook\"\n");
			stuffcmd(ent, "alias +use +hook ; alias -use -hook\n");
		}
		else //normal hook ent/
			stuffcmd(ent, "alias +hook +use; alias -hook -use \n");
		ent->spec_warned = false;
		ent->speccheck = false;
		//set up the no delta value
		stuffcmd(ent, "set cl_notelta $cl_nodelta u\n");
	}
	ent->svflags = 0;
	ent->flags &= ~FL_GODMODE;
	ent->client->resp.ctf_team = desired_team;
	ent->client->resp.ctf_state = CTF_STATE_START;
	s = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	//skin change is allowed here
	ent->client->skintime = level.time -1;
	CTFAssignSkin(ent, s);
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
	if (ent->solid == SOLID_NOT || ent->client->resp.spectator !=0)
	{ // spectator
		PutClientInServer (ent);
		// add a teleportation effect
		ent->s.event = EV_PLAYER_TELEPORT;
		// hold in place briefly
		ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		ent->client->ps.pmove.pm_time = 14;
		CheckPlayers();
		my_bprintf(PRINT_HIGH, "%s joined the %s team. (%d red, %d blue, %d spectators)\n", ent->client->pers.netname,
			CTFTeamName(desired_team), ctfgame.players1, ctfgame.players2, ctfgame.specs);
		ent->client->pers.pl_state = 1;//playing
		ent->client->resp.spectator = 0;
		return;
	}
	ent->health = 0;
//	player_die (ent, ent, ent, 100000, vec3_origin);
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
	if (!voosh->value)
		TossClientWeapon(ent);
	if (ent->client && ent->client->ctf_grapple)
		CTFResetGrapple(ent->client->ctf_grapple);
	CTFPlayerResetGrapple(ent);
	// drop the rune if we have one
	runes_drop(ent);
	if ( ent->flashlight )
	{
		G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
    }
	//if on the hook drop it also 
	if (ent->client->hook || ent->client->ctf_grapple)
	{
		my_bprintf(PRINT_HIGH, "%s is trying to spam the server with HOOKS\n and was disconnected from this server\n",
			ent->client->pers.netname);
		stuffcmd(ent ,"disconnect;error \"You have been disconnected for trying to die or switch teams while hooking, which will crash the server. Multiple attempts at this will result in a ban.\"");
		return;
	}
	ent->client->resp.score = 0;
	CheckPlayers();
	my_bprintf(PRINT_HIGH, "%s changed to the %s team. (%d red, %d blue, %d spectators)\n",
		ent->client->pers.netname, CTFTeamName(desired_team), ctfgame.players1, ctfgame.players2, ctfgame.specs);
	ent->client->pers.pl_state = 1;//playing
	ent->client->resp.spectator = 0;
	//skin change is allowed here
	ent->client->skintime = level.time -1;
	// if hook should be dropped, just return
	if (ent->client->hook || ent->client->ctf_grapple)
	{
		my_bprintf(PRINT_HIGH, "%s is trying to spam the server with HOOKS\n and was disconnected from the server\n", ent->client->pers.netname);
		stuffcmd(ent ,"disconnect;error \"You have been disconnected for trying to die or switch teams while hooking, which will crash the server. Multiple attempts at this will result in a ban.\"");
		return;
	}
	
	// don't even bother waiting for death frames
	ent->deadflag = DEAD_DEAD;
	respawn (ent,false);
}

/*
==================
CTFScoreboardMessage
==================
*/
void CTFScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		len;
	int		i, j, k, n;
	int		sorted[2][MAX_CLIENTS];
	int		sortedscores[2][MAX_CLIENTS];
	int		score, total[2], totalscore[2];
	int		last[2];
	gclient_t	*cl;
	edict_t		*cl_ent;
	int team;
	int maxsize = 1000;

	//RAV Highscores
    FILE    *motd_file;
    char    filename[32];
    char    line[80];
    int		stringlength;

	i =  sprintf(filename, "./");
    i += sprintf(filename + i, game_dir->string);
    i += sprintf(filename + i, "/hs/%s_hs.txt", level.mapname);
    if ((highscores->value)	&& (show_hs == true) &&	(motd_file = fopen(filename, "r")))
	{
		string[0] = 0;
		stringlength = strlen(string);
		i = 0;
		while ( fgets(line, 80, motd_file) )
		{
			//JSW - if the highscore contains today's date, make it green
			if (strstr (line, sys_date))
				convert_string(line, 0, 127, 128, line); // white -> green
			//end
			Com_sprintf (entry, sizeof(entry), "xv 2 yv %i string \"%s\" ",	i*8 + 24, line);
			j = strlen(entry);
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
			i++;
		}
		
		// be good now ! ... close the file
		fclose(motd_file);
		j = strlen(entry);
		if (stringlength + j < 1024)
		{
			strcpy (string + stringlength, entry);
			stringlength += j;
		}
	}
	else
	{
		//end highscores
		// sort the clients by team and score
		total[0] = total[1] = 0;
		last[0] = last[1] = 0;
		totalscore[0] = totalscore[1] = 0;
		for (i=0 ; i<game.maxclients ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (!cl_ent->inuse)
				continue;
			if (game.clients[i].resp.ctf_team == CTF_TEAM1)
				team = 0;
			else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
				team = 1;
			else
				continue; // unknown team?
			score = game.clients[i].resp.score;
			for (j=0 ; j<total[team] ; j++)
			{
				if (score > sortedscores[team][j])
					break;
			}
			for (k=total[team] ; k>j ; k--)
			{
				sorted[team][k] = sorted[team][k-1];
				sortedscores[team][k] = sortedscores[team][k-1];
			}
			sorted[team][j] = i;
			sortedscores[team][j] = score;
			totalscore[team] += score;
			total[team]++;
		}
		
		// print level name and exit rules
		// add the clients in sorted order
		*string = 0;
		len = 0;
		// team one
		sprintf(string, "if 24 xv 8 yv 8 pic 24 endif "
			"xv 40 yv 28 string \"%4d/%-3d\" "
			"xv 98 yv 12 num 2 18 "
			"if 25 xv 168 yv 8 pic 25 endif "
			"xv 200 yv 28 string \"%4d/%-3d\" "
			"xv 256 yv 12 num 2 20 ", totalscore[0], total[0],
			totalscore[1], total[1]);
		len = strlen(string);
		for (i=0 ; i<16 ; i++)
		{
			if (i >= total[0] && i >= total[1])
				break; // we're done
#if 0 //ndef NEW_SCORE
			// set up y
			sprintf(entry, "yv %d ", 42 + i * 8);
			if (maxsize - len > strlen(entry))
			{
				strcat(string, entry);
				len = strlen(string);
			}
#else
			*entry = 0;
#endif
			// left side
			if (i < total[0])
			{
				cl = &game.clients[sorted[0][i]];
				cl_ent = g_edicts + 1 + sorted[0][i];
#if 0 //ndef NEW_SCORE
				sprintf(entry+strlen(entry),
					"xv 0 %s \"%3d %3d %-12.12s\" ",
					(cl_ent == ent) ? "string2" : "string",
					cl->resp.score, (cl->ping > 999) ? 999 : cl->ping, 
					cl->pers.netname);
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
					strcat(entry, "xv 56 picn sbfctf2 ");
#else
				sprintf(entry+strlen(entry),
					"ctf 0 %d %d %d %d ",
					42 + i * 8,
					sorted[0][i],
					cl->resp.score,
					cl->ping);
				
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag2_item)])
					sprintf(entry + strlen(entry), "xv 56 yv %d picn sbfctf2 ",	42 + i * 8);
#endif
				
				if (maxsize - len > strlen(entry))
				{
					strcat(string, entry);
					len = strlen(string);
					last[0] = i;
				}
			}
			
			// right side
			if (i < total[1])
			{
				cl = &game.clients[sorted[1][i]];
				cl_ent = g_edicts + 1 + sorted[1][i];
#if 0 //ndef NEW_SCORE
				sprintf(entry+strlen(entry),
					"xv 160 %s \"%3d %3d %-12.12s\" ",
					(cl_ent == ent) ? "string2" : "string",
					cl->resp.score, 
					(cl->ping > 999) ? 999 : cl->ping, 
					cl->pers.netname);
				
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
					strcat(entry, "xv 216 picn sbfctf1 ");
#else
				sprintf(entry+strlen(entry),
					"ctf 160 %d %d %d %d ",
					42 + i * 8,
					sorted[1][i],
					cl->resp.score,
					cl->ping);
				if (cl_ent->client->pers.inventory[ITEM_INDEX(flag1_item)])
					sprintf(entry + strlen(entry), "xv 216 yv %d picn sbfctf1 ",
					42 + i * 8);
#endif
				if (maxsize - len > strlen(entry))
				{
					strcat(string, entry);
					len = strlen(string);
					last[1] = i;
				}
			}
		}
		
		// put in spectators if we have enough room
		if (last[0] > last[1])
			j = last[0];
		else
			j = last[1];
		j = (j + 2) * 8 + 42;
		k = n = 0;
		if (maxsize - len > 50)
		{
			for (i = 0; i < maxclients->value; i++)
			{
				cl_ent = g_edicts + 1 + i;
				cl = &game.clients[i];
				if (!cl_ent->inuse || cl_ent->solid != SOLID_NOT || cl_ent->client->resp.ctf_team != CTF_NOTEAM)
					continue;
				if (!k)
				{
					k = 1;
					sprintf(entry, "xv 0 yv %d string2 \"Spectators\" ", j);
					strcat(string, entry);
					len = strlen(string);
					j += 8;
				}
				
				sprintf(entry+strlen(entry), "ctf %d %d %d %d %d ",
					(n & 1) ? 160 : 0, // x
					j, // y
					i, // playernum
					cl->resp.score,
					cl->ping > 999 ? 999 : cl->ping);
				if (maxsize - len > strlen(entry))
				{
					strcat(string, entry);
					len = strlen(string);
				}
				if (n & 1)
					j += 8;
				n++;
			}
		}
		
		if (total[0] - last[0] > 1) // couldn't fit everyone
			sprintf(string + strlen(string), "xv 8 yv %d string \"..and %d more\" ",
			42 + (last[0]+1)*8, total[0] - last[0] - 1);
		if (total[1] - last[1] > 1) // couldn't fit everyone
			sprintf(string + strlen(string), "xv 168 yv %d string \"..and %d more\" ",
			42 + (last[1]+1)*8, total[1] - last[1] - 1);
	}//highscores
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}

/*------------------------------------------------------------------------*/

/* TECH																	  */

/*------------------------------------------------------------------------*/


void CTFHasTech(edict_t *who)
{
	if (level.time - who->client->ctf_lasttechmsg > 2)
	{
		if (!who->bot_client)
			safe_centerprintf(who, "You already have a TECH powerup.");
		who->client->ctf_lasttechmsg = level.time;
	}
}


gitem_t *CTFWhat_Tech(edict_t *ent)
{

	gitem_t *tech;

	int i;


	i = 0;

	while (tnames[i]) {

		if ((tech = FindItemByClassname(tnames[i])) != NULL &&

			ent->client->pers.inventory[ITEM_INDEX(tech)]) {

			return tech;

		}

		i++;

	}

	return NULL;

}

qboolean CTFPickup_Tech (edict_t *ent, edict_t *other)
{
	gitem_t *tech;
	int i;

	i = 0;

	if (rune_has_a_rune(other))
	{
		CTFHasTech(other);
		return false;
	}

	if(match_state < STATE_PLAYING)
	{
		return false;
	}

	while (tnames[i])
	{
//		tech = FindItemByClassname(tnames[i]);
		if ((tech = FindItemByClassname(tnames[i])) != NULL && other->client->pers.inventory[ITEM_INDEX(tech)])
//		if (other->client->pers.inventory[ITEM_INDEX(tech)])
		{
			CTFHasTech(other);
			return false; // has this one
		}
		i++;
	}

	
	// client only gets one tech
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	other->client->ctf_regentime = level.time;
	return true;
}

//static


 void SpawnTech(gitem_t *item, edict_t *spot);


static edict_t *FindTechSpawn(void)

{

	edict_t *spot = NULL;

	int i = rand() % 16;


	while (i--)

		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");

	if (!spot)

		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");

	return spot;

}

static edict_t *FindTechSpawnCTF(int whichteam)
{
	edict_t *spot = NULL;
	int i = rand() % 16;
	char team[20];

	//gi.dprintf("whichteam is %d\n", whichteam);
	if (whichteam == 1)
	{
		if (teamtechs->value == 1)
			sprintf (team, "info_player_team1");
		else if (teamtechs->value == 2)
			sprintf(team, "item_flag_team1");
		else
			gi.dprintf("ERROR spawning techs! Check your teamtechs CVAR!\n");
//		gi.dprintf("red spawn chosen\n");
	}
	else
	{
		if (teamtechs->value == 1)
			sprintf (team, "info_player_team2");
		else if (teamtechs->value == 2)
			sprintf(team, "item_flag_team2");
		else
			gi.dprintf("ERROR spawning techs! Check your teamtechs CVAR!\n");
//		gi.dprintf("blue spawn chosen\n");
	}

	while (i--)
		spot = G_Find (spot, FOFS(classname), team);

	if (!spot)
		spot = G_Find (spot, FOFS(classname), team);

	return spot;
}

static void TechThink(edict_t *tech)
{
	edict_t *spot;

	if ((spot = FindTechSpawn()) != NULL)
	{
		SpawnTech(tech->item, spot);
		G_FreeEdict(tech);
	}
	else
	{
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
}

//RaVeN


static void TechMakeTouchable(edict_t *tech)


{


	tech->think = TechThink;


	tech->touch = Touch_Item;


	tech->nextthink = level.time + CTF_TECH_TIMEOUT;


}

void CTFDrop_Tech(edict_t *ent, gitem_t *item)
{
	edict_t *tech;
	tech = Drop_Item(ent, item);
	tech->nextthink = level.time + CTF_TECH_TIMEOUT;
	if (!pickup_tech->value)
	{
		tech->nextthink = level.time + CTF_TECH_TIMEOUT;
		tech->think = TechThink;
	}
	else
	{
		tech->nextthink = level.time + 1.0;
		tech->think = TechMakeTouchable;
	}
	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;
}


void CTFDeadDropTech(edict_t *ent)

{

	gitem_t *tech;

	edict_t *dropped;

	int i;


	if (!ctf->value)

		return;


	i = 0;

	while (tnames[i]) {

		if ((tech = FindItemByClassname(tnames[i])) != NULL &&

			ent->client->pers.inventory[ITEM_INDEX(tech)]) {

			dropped = Drop_Item(ent, tech);

			// hack the velocity to make it bounce random

			dropped->velocity[0] = (rand() % 600) - 300;

			dropped->velocity[1] = (rand() % 600) - 300;

			dropped->nextthink = level.time + CTF_TECH_TIMEOUT;

			dropped->think = TechThink;

			dropped->owner = NULL;

			ent->client->pers.inventory[ITEM_INDEX(tech)] = 0;

		}

		i++;

	}

}


//static

gitem_t *titems[4] = {NULL, NULL, NULL, NULL};
void SpawnTech(gitem_t *item, edict_t *spot)
{
	edict_t	*ent;
	vec3_t	forward, right;
	vec3_t  angles;
	
	//if (!ctf->value)
	//	return;
	
	ent = G_Spawn();
	ent->classname = item->classname;
	ent->item = item;
	ent->spawnflags = DROPPED_ITEM;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	
	//RAV
	//ban runes
	if ((((int)(runeflags->value) & 1) && (Q_stricmp(ent->classname, "item_rune_strength")==0)) ||
		(((int)(runeflags->value) & 2) && (Q_stricmp(ent->classname, "item_rune_resist")==0)) ||
		(((int)(runeflags->value) & 4) && (Q_stricmp(ent->classname, "item_rune_haste")==0)) ||
		(((int)(runeflags->value) & 8) && (Q_stricmp(ent->classname, "item_rune_regen")==0)) ||
		(((int)(runeflags->value) & 16) && (Q_stricmp(ent->classname, "item_rune_jump")==0)) ||
		(((int)(runeflags->value) & 32) && (Q_stricmp(ent->classname, "item_rune_liquid")==0)) ||
		(((int)(runeflags->value) & 64) && (Q_stricmp(ent->classname, "item_rune_invis")==0)) ||
		(((int)(runeflags->value) & 128) && (Q_stricmp(ent->classname, "item_rune_vamp")==0)) ||
		(((int)(runeflags->value) & 256) && (Q_stricmp(ent->classname, "item_rune_speed")==0)))
	{
		G_FreeEdict(ent);
		return;
	}
	
	if (strcmp(ent->classname, "item_rune_strength") == 0)
	{
		ent->s.renderfx |= RF_SHELL_RED;
	}
	if (strcmp(ent->classname, "item_rune_resist")==0)
	{
		ent->s.renderfx |= RF_SHELL_BLUE;
	}
	if (strcmp(ent->classname, "item_rune_haste")==0)
	{
		ent->s.renderfx |= RF_SHELL_DOUBLE;
	}
	if (strcmp(ent->classname, "item_rune_regen")==0)
	{
		ent->s.renderfx |= RF_SHELL_GREEN;
	}
		if (strcmp(ent->classname, "item_rune_jump") == 0)
	{
		ent->s.renderfx |= RF_SHELL_BLUE | RF_SHELL_DOUBLE;
	}
	if (strcmp(ent->classname, "item_rune_liquid")==0)
	{
		ent->s.renderfx |= RF_SHELL_GREEN | RF_SHELL_BLUE;
	}
	if (strcmp(ent->classname, "item_rune_invis")==0)
	{
		ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_HALF_DAM;
	}
	if (strcmp(ent->classname, "item_rune_vamp")==0)
	{
		ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE;
	}
	if (strcmp(ent->classname, "item_rune_speed")==0)
	{
		ent->s.renderfx |= RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_HALF_DAM;
	}
	VectorSet (ent->mins, -15, -15, -15);
	VectorSet (ent->maxs, 15, 15, 15);
	gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;
	ent->touch = Touch_Item;
	ent->owner = ent;
	angles[0] = 0;
	angles[1] = rand() % 360;
	angles[2] = 0;
	AngleVectors (angles, forward, right, NULL);
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 16;
	VectorScale (forward, 100, ent->velocity);
	if (teamtechs->value && firsttechs == true)
		ent->velocity[2] = 700;
	else
		ent->velocity[2] = 300;
	ent->nextthink = level.time + CTF_TECH_TIMEOUT;
	ent->think = TechThink;
	gi.linkentity (ent);
	
	// add to the tech lookup list
	{
		int i=0;
		while (titems[i])
			i++;
		if (i < 3)
		{
			titems[i] = item;
		}
	}
}

static void SpawnTechs(edict_t *ent)
{
	gitem_t *tech;
	edict_t *spot;
	int i;
	int blue = 0;
	int red = 0;
	int team;
	i = 0;
	firsttechs = true;

	while (tnames[i])
	{
		if ((rand()%10 & 1) && (red < 2))
		{
			team = 1;
			red++;
			//gi.dprintf("red spawn chosen\n");
		}
		else if (blue < 2)
		{
			team = 2;
			blue++;
			//gi.dprintf("blue spawn chosen\n");
		}
		else
		{
			team = 1;
			red++;
			//gi.dprintf("red spawn chosen\n");
		}

		if (teamtechs->value)
		{
			if ((tech = FindItemByClassname(tnames[i])) != NULL && (spot = FindTechSpawnCTF(team)) != NULL)
				SpawnTech(tech, spot);
		}
		else
		{
			if ((tech = FindItemByClassname(tnames[i])) != NULL && (spot = FindTechSpawn()) != NULL)
				SpawnTech(tech, spot);
		}

		i++;
	}
	firsttechs = false;
}


// frees the passed edict!

void CTFRespawnTech(edict_t *ent)

{

	edict_t *spot;


	if ((spot = FindTechSpawn()) != NULL)

		SpawnTech(ent->item, spot);


	G_FreeEdict(ent);

}


void CTFSetupTechSpawn(void)
{
	edict_t *ent;

	if (!ctf->value || techspawn || (dmflag & DF_CTF_NO_TECH))
		return;

	ent = G_Spawn();
	ent->nextthink = level.time + warmup_time->value + 4;
	ent->think = SpawnTechs;
	techspawn = true;

}


int CTFApplyResistance(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech1");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		// make noise
	   	gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech1.wav"), volume, ATTN_NORM, 0);
		return dmg / 2;
	}
	return dmg;
}

int CTFApplyStrength(edict_t *ent, int dmg)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (dmg && tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)]) {
		return dmg * 2;
	}
	return dmg;
}

qboolean CTFApplyStrengthSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech2");
	if (tech && ent->client && ent->client->pers.inventory[ITEM_INDEX(tech)])
	{
		if (ent->client->ctf_techsndtime < level.time)
		{
			ent->client->ctf_techsndtime = level.time + 1;
			if (ent->client->quad_framenum > level.framenum)
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2x.wav"), volume, ATTN_NORM, 0);
			else
				gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech2.wav"), volume, ATTN_NORM, 0);
		}
		return true;
	}
	return false;
}


qboolean CTFApplyHaste(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}

void CTFApplyHasteSound(edict_t *ent)
{
	static gitem_t *tech = NULL;
	float volume = 1.0;

	if (ent->client && ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech3");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)] &&
		ent->client->ctf_techsndtime < level.time) {
		ent->client->ctf_techsndtime = level.time + 1;
		gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech3.wav"), volume, ATTN_NORM, 0);
	}
}

void CTFApplyRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;
	qboolean noise = false;
	gclient_t *client;
	int index;
	float volume = 1.0;

	client = ent->client;
	if (!client)
		return;

	if (ent->client->silencer_shots)
		volume = 0.2;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && client->pers.inventory[ITEM_INDEX(tech)]) {
		if (client->ctf_regentime < level.time) {
			client->ctf_regentime = level.time;
			if (ent->health < 150) {
				ent->health += 5;
				if (ent->health > 150)
					ent->health = 150;
				client->ctf_regentime += 0.5;
				noise = true;
			}
			index = ArmorIndex (ent);
			if (index && client->pers.inventory[index] < 150) {
				client->pers.inventory[index] += 5;
				if (client->pers.inventory[index] > 150)
					client->pers.inventory[index] = 150;
				client->ctf_regentime += 0.5;
				noise = true;
			}
		}
		if (noise && ent->client->ctf_techsndtime < level.time) {
			ent->client->ctf_techsndtime = level.time + 1;
			gi.sound(ent, CHAN_VOICE, gi.soundindex("ctf/tech4.wav"), volume, ATTN_NORM, 0);
		}
	}
}

qboolean CTFHasRegeneration(edict_t *ent)
{
	static gitem_t *tech = NULL;

	if (!tech)
		tech = FindItemByClassname("item_tech4");
	if (tech && ent->client &&
		ent->client->pers.inventory[ITEM_INDEX(tech)])
		return true;
	return false;
}
/*

======================================================================


SAY_TEAM


======================================================================

*/


// This array is in 'importance order', it indicates what items are

// more important when reporting their names.

struct {

	char *classname;

	int priority;

} loc_names[] = 

{

	{	"item_flag_team1",			1 },

	{	"item_flag_team2",			1 },

	{	"item_quad",				2 }, 

	{	"item_invulnerability",		2 },

	{	"weapon_bfg",				3 },

	{	"weapon_railgun",			4 },

	{	"weapon_rocketlauncher",	4 },

	{	"weapon_hyperblaster",		4 },

	{	"weapon_chaingun",			4 },

	{	"weapon_grenadelauncher",	4 },

	{	"weapon_machinegun",		4 },

	{	"weapon_supershotgun",		4 },

	{	"weapon_shotgun",			4 },

	{	"item_power_screen",		5 },

	{	"item_power_shield",		5 },

	{	"item_armor_body",			6 },

	{	"item_armor_combat",		6 },

	{	"item_armor_jacket",		6 },

	{	"item_silencer",			7 },

	{	"item_breather",			7 },

	{	"item_enviro",				7 },

	{	"item_adrenaline",			7 },

	{	"item_bandolier",			8 },

	{	"item_pack",				8 },

	{ NULL, 0 }

};



void CTFSay_Team_Location(edict_t *who, char *buf)
{
	edict_t *what = NULL;
	edict_t *hot = NULL;
	float hotdist = 999999, newdist;
	vec3_t v;
	int hotindex = 999;
	int i;
	gitem_t *item;
	int nearteam = -1;
	edict_t *flag1, *flag2;
	qboolean hotsee = false;
	qboolean cansee;
	
	while ((what = loc_findradius(what, who->s.origin, 1024)) != NULL)
	{
		// find what in loc_classnames
		for (i = 0; loc_names[i].classname; i++)
		{
			if (strcmp(what->classname, loc_names[i].classname) == 0)
				break;
		}
		if (!loc_names[i].classname)
			continue;
		
		// something we can see get priority over something we can't
		cansee = loc_CanSee(what, who);
		if (cansee && !hotsee)
		{
			hotsee = true;
			hotindex = loc_names[i].priority;
			hot = what;
			VectorSubtract(what->s.origin, who->s.origin, v);
			hotdist = VectorLength(v);
			continue;
		}
		// if we can't see this, but we have something we can see, skip it
		if (hotsee && !cansee)
			continue;
		if (hotsee && hotindex < loc_names[i].priority)
			continue;
		VectorSubtract(what->s.origin, who->s.origin, v);
		newdist = VectorLength(v);
		if (newdist < hotdist || (cansee && loc_names[i].priority < hotindex))
		{
			hot = what;
			hotdist = newdist;
			hotindex = i;
			hotsee = loc_CanSee(hot, who);
		}
	}
	
	if (!hot)
	{
		strcpy(buf, "nowhere");
		return;
	}
	// we now have the closest item
	// see if there's more than one in the map, if so
	// we need to determine what team is closest
	what = NULL;
	while ((what = G_Find(what, FOFS(classname), hot->classname)) != NULL)
	{
		if (what == hot)
			continue;
		// if we are here, there is more than one, find out if hot
		// is closer to red flag or blue flag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL && 
			(flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL)
		{
			VectorSubtract(hot->s.origin, flag1->s.origin, v);
			hotdist = VectorLength(v);
			VectorSubtract(hot->s.origin, flag2->s.origin, v);
			newdist = VectorLength(v);
			if (hotdist < newdist)
				nearteam = CTF_TEAM1;
			else if (hotdist > newdist)
				nearteam = CTF_TEAM2;
		}
		break;
	}
	if ((item = FindItemByClassname(hot->classname)) == NULL)
	{
		strcpy(buf, "nowhere");
		return;
	}
	// in water?
	if (who->waterlevel)
		strcpy(buf, "in the water ");
	else
		*buf = 0;
	// near or above
	VectorSubtract(who->s.origin, hot->s.origin, v);
	if (fabs(v[2]) > fabs(v[0]) && fabs(v[2]) > fabs(v[1]))
		if (v[2] > 0)
			strcat(buf, "above ");
		else
			strcat(buf, "below ");
		else
			strcat(buf, "near ");
		if (nearteam == CTF_TEAM1)
			strcat(buf, "the red ");
		else if (nearteam == CTF_TEAM2)
			strcat(buf, "the blue ");
		else
			strcat(buf, "the ");
		strcat(buf, item->pickup_name);
}

static void CTFSay_Team_Armor(edict_t *who, char *buf)

{

	gitem_t		*item;

	int			index, cells;

	int			power_armor_type;


	*buf = 0;


	power_armor_type = PowerArmorType (who);

	if (power_armor_type)

	{

		cells = who->client->pers.inventory[ITEM_INDEX(FindItem ("cells"))];

		if (cells)

			sprintf(buf+strlen(buf), "%s with %i cells ",

				(power_armor_type == POWER_ARMOR_SCREEN) ?

				"Power Screen" : "Power Shield", cells);

	}


	index = ArmorIndex (who);

	if (index)

	{

		item = GetItemByIndex (index);

		if (item) {

			if (*buf)

				strcat(buf, "and ");

			sprintf(buf+strlen(buf), "%i units of %s",

				who->client->pers.inventory[index], item->pickup_name);

		}

	}


	if (!*buf)

		strcpy(buf, "no armor");

}


static void CTFSay_Team_Health(edict_t *who, char *buf)

{

	if (who->health <= 0)

		strcpy(buf, "dead");

	else

		sprintf(buf, "%i health", who->health);

}


static void CTFSay_Team_Tech(edict_t *who, char *buf)

{

	gitem_t *tech,*rune;

	int i;


	// see if the player has a tech powerup

	i = 0;



//RAV


    //rune addition


if (rune_has_a_rune(who))


{


for (i=RUNE_FIRST; i<=RUNE_LAST; i++)


 {


	if (rune_has_rune(who, i))


	{


if ((rune = FindItem(rune_namefornum[i])) != NULL &&


	who->client->pers.inventory[ITEM_INDEX(rune)])


 sprintf(buf, "the %s", rune->pickup_name);


 return;


		}


	}


}


//

	while (tnames[i]) {

		if ((tech = FindItemByClassname(tnames[i])) != NULL &&

			who->client->pers.inventory[ITEM_INDEX(tech)]) {

			sprintf(buf, "the %s", tech->pickup_name);

			return;

		}

		i++;

	}

	strcpy(buf, "no powerup");

}


static void CTFSay_Team_Weapon(edict_t *who, char *buf)

{

	if (who->client->pers.weapon)

		strcpy(buf, who->client->pers.weapon->pickup_name);

	else

		strcpy(buf, "none");

}


static void CTFSay_Team_Sight(edict_t *who, char *buf)

{

	int i;

	edict_t *targ;

	int n = 0;

	char s[1024];

	char s2[1024];


	*s = *s2 = 0;

	for (i = 1; i <= maxclients->value; i++) {

		targ = g_edicts + i;

		if (!targ->inuse || targ == who || targ->client->resp.spectator || !loc_CanSee(targ, who))
			continue;

		if (*s2) {

			if (strlen(s) + strlen(s2) + 3 < sizeof(s)) {

				if (n)

					strcat(s, ", ");

				strcat(s, s2);

				*s2 = 0;

			}

			n++;

		}

		strcpy(s2, targ->client->pers.netname);

	}

	if (*s2) {

		if (strlen(s) + strlen(s2) + 6 < sizeof(s)) {

			if (n)

				strcat(s, " and ");

			strcat(s, s2);

		}

		strcpy(buf, s);

	} else

		strcpy(buf, "no one");

}


void CTFSay_Team(edict_t *who, char *msg)
{
	char outmsg[1024];
	char buf[1024];
	int i;
	char *p;
	edict_t *cl_ent;
	char msg2[1024];
	outmsg[0] = 0;


	
	if(who->is_muted)
		return;

	if (flood_team->value)
	{
		if (!CheckFlood(who))
			return;
	}

	if (*msg == '\"')
	{
		msg[strlen(msg) - 1] = 0;
		msg++;
	}

	for (p = outmsg; *msg && (p - outmsg) < sizeof(outmsg) - 1; msg++)
	{
		if (*msg == '%')
		{
			switch (*++msg)
			{
			case 'l' :
			case 'L' :
				CTFSay_Team_Location(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'a' :
			case 'A' :
				CTFSay_Team_Armor(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'h' :
			case 'H' :
				CTFSay_Team_Health(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 't' :
			case 'T' :
				CTFSay_Team_Tech(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'w' :
			case 'W' :
				CTFSay_Team_Weapon(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			case 'n' :
			case 'N' :
				CTFSay_Team_Sight(who, buf);
				strcpy(p, buf);
				p += strlen(buf);
				break;
			default :
				*p++ = *msg;
			}
		}
		else
			*p++ = *msg;
	}

	*p = 0;

	sprintf(msg2, "(%s): %s\n", who->client->pers.netname, outmsg);

	if (log_chat->value)
	{
		LogChat(msg2);
	}
	if (dedicated->value)
	{
		gi.dprintf(msg2);
	}

	for (i = 0; i < maxclients->value; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		if (cl_ent->client->resp.ctf_team == who->client->resp.ctf_team)
		{
			if (!cl_ent->bot_client)
				safe_cprintf(cl_ent, PRINT_CHAT, msg2);
		}
	}
}

/*-----------------------------------------------------------------------*/

/*QUAKED misc_ctf_banner (1 .5 0) (-4 -64 0) (4 64 248) TEAM2

The origin is the bottom of the banner.

The banner is 248 tall.

*/

static void misc_ctf_banner_think (edict_t *ent)

{

	ent->s.frame = (ent->s.frame + 1) % 16;

	ent->nextthink = level.time + FRAMETIME;

}


void SP_misc_ctf_banner (edict_t *ent)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_NOT;

	ent->s.modelindex = gi.modelindex ("models/ctf/banner/tris.md2");

	if (ent->spawnflags & 1) // team2

		ent->s.skinnum = 1;


	ent->s.frame = rand() % 16;

	gi.linkentity (ent);


	ent->think = misc_ctf_banner_think;

	ent->nextthink = level.time + FRAMETIME;

}


/*QUAKED misc_ctf_small_banner (1 .5 0) (-4 -32 0) (4 32 124) TEAM2

The origin is the bottom of the banner.

The banner is 124 tall.

*/

void SP_misc_ctf_small_banner (edict_t *ent)

{

	ent->movetype = MOVETYPE_NONE;

	ent->solid = SOLID_NOT;

	ent->s.modelindex = gi.modelindex ("models/ctf/banner/small.md2");

	if (ent->spawnflags & 1) // team2

		ent->s.skinnum = 1;


	ent->s.frame = rand() % 16;

	gi.linkentity (ent);


	ent->think = misc_ctf_banner_think;

	ent->nextthink = level.time + FRAMETIME;

}



/*-----------------------------------------------------------------------*/
void player_die1 (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

void CTFJoinTeam(edict_t *ent, int desired_team)
{
//	char *s;
	
	PMenu_Close(ent);

	if (!ctf->value)
		return;

	CTFTeam_f(ent, desired_team);
		return;
}


void CTFJoinTeam1(edict_t *ent, pmenu_t *p)

{

	CTFJoinTeam(ent, CTF_TEAM1);

}


void CTFJoinTeam2(edict_t *ent, pmenu_t *p)

{

	CTFJoinTeam(ent, CTF_TEAM2);
}

//RAV


void JoinGame(edict_t *ent, pmenu_t *menu)
{
	if(ent->client->pers.pl_state > 0)
	{
		safe_centerprintf(ent,"You are already Playing");
		return;
	}
	ent->svflags &= ~SVF_NOCLIENT;
	// add a teleportation effect
	ent->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	ent->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	ent->client->ps.pmove.pm_time = 14;
	ent->client->pers.pl_state = 3;//playing/needs spawned
	ent->client->resp.spectator = false;
	PutClientInServer (ent);
	CheckPlayers();
	my_bprintf(PRINT_HIGH, "%s Has a DeathWish. (%d players, %d spectators)\n",	ent->client->pers.netname, ctfgame.players3, ctfgame.specs);
	
	if(!ent->bot_client)
	{
		////RAV
		stuffcmd(ent, "alias +hook \"cmd hook\"\n");
		stuffcmd(ent, "alias -hook \"cmd unhook\"\n");
		stuffcmd(ent, "alias +hook +use; alias -hook -use \n");
		ent->spec_warned = false;
		ent->speccheck = false;
		//set up the no delta value
		stuffcmd(ent, "set cl_notelta $cl_nodelta u\n");
	}
	PMenu_Close(ent);
}



//RAV aDD spectator mode


void Spec(edict_t *ent, pmenu_t *menu)
{
	//drop techs and such
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
	
	// drop the rune if we have one
	runes_drop(ent);
	if ( ent->flashlight )
	{
		G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
    }
	
	ent->client->resp.score = 0;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->pers.pl_state = 0;
	ent->client->pers.db_hud = true;
	ent->client->ps.gunindex = 0;
	if(ctf->value)
		ent->client->resp.ctf_team = CTF_NOTEAM;
	gi.linkentity (ent);
}



void CTFChaseCam(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;
	qboolean found = false;
	
	if (ent->client->chase_target)
	{
		ent->client->chase_target = NULL;
		Spec(ent, NULL);
		PMenu_Close(ent);
		ent->client->pers.pl_state = 0;
		return;
	}
	
	for (i = 1; i <= maxclients->value; i++)
	{
		e = g_edicts + i;
		if (e->inuse && e->solid != SOLID_NOT && ent != e)
		{
			ent->client->chase_target = e;
			ent->client->update_chase = true;
			//RAV
			Spec(ent ,NULL);
			found = true;
			my_bprintf(PRINT_HIGH, "%s Moved to Spectator.\n", ent->client->pers.netname);
			PMenu_Close(ent);
			ent->client->pers.pl_state = 0;
			break;
		}
	}
	//RAV  close menu !!
	if(!found)
		safe_cprintf(ent, PRINT_HIGH, "No target to chase\n" );
	ent->client->pers.pl_state = 0;
	//
}

//RAV
void RAVspec(edict_t *ent, pmenu_t *p)
{


	PMenu_Close(ent);


	stuffcmd (ent, va("spec\n"));


}


//

void CTFReturnToMain(edict_t *ent, pmenu_t *p)

{

	PMenu_Close(ent);


	if(ctf->value)

	CTFOpenJoinMenu(ent);


	else


	OpenJoinMenu(ent);

}

#define MAX_MENU_MAPS	14

void CTFCredits(edict_t *ent, pmenu_t *p);


void DeathmatchScoreboard (edict_t *ent);


void CTFShowScores(edict_t *ent, pmenu_t *p)

{

	PMenu_Close(ent);


	ent->client->showscores = true;

	ent->client->showinventory = false;

	DeathmatchScoreboard (ent);

}

void OPMenu(edict_t *ent, pmenu_t *p);


void KickMe(edict_t *ent, pmenu_t *p);



//RAV


void LightsOn(edict_t *ent, pmenu_t *menu)


{


		PMenu_Close(ent);


	stuffcmd (ent, va("lightson\n"));

}


void LightsOff(edict_t *ent, pmenu_t *menu)


{


		PMenu_Close(ent);


	stuffcmd (ent, va("lightsoff\n"));


}

void VoteYes (edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	if (mapvoteactive == false)
	{
		safe_centerprintf(ent, "There is no vote active.\n");
		return;
	}
	if (ent->client->resp.vote == true)
		safe_centerprintf(ent, "You have already voted YES.\n");
	else
	{
		ent->client->resp.vote = true;
		safe_centerprintf(ent, "Your vote has been changed to YES\n");
	}
}

void VoteNo (edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	if (mapvoteactive == false)
	{
		safe_centerprintf(ent, "There is no vote active.\n");
		return;
	}
	if (ent->client->resp.vote == false)
		safe_centerprintf(ent, "You have already voted NO.\n");
	else
	{
		ent->client->resp.vote = false;
		safe_centerprintf(ent, "Your vote has been changed to NO\n");
	}
}

	pmenu_t creditsmenu[] = {

	{ "*Quake II",						PMENU_ALIGN_CENTER,  0, NULL },

	{ "*ThreeWave Capture the Flag",	PMENU_ALIGN_CENTER,  0, NULL },

	{ "*Programming",					PMENU_ALIGN_CENTER,  0, NULL }, 

	{ "Dave 'Zoid' Kirsch",				PMENU_ALIGN_CENTER,  0, NULL },

	{ "*TMG", 				        	PMENU_ALIGN_CENTER,  0, NULL },

	{ "Doug 'RaVeN' Buckley",			PMENU_ALIGN_CENTER,  0, NULL },

	{ "ACME Clan",							PMENU_ALIGN_CENTER,  0, NULL },

	{ "Yowza'Duncan",							PMENU_ALIGN_CENTER,  0, NULL },

	{ "PhaZe",				PMENU_ALIGN_CENTER,  0, NULL },

	{ "Josh 'greider' Waggoner",					PMENU_ALIGN_CENTER,  0, NULL },

	{ "Ponpoko for base bot code",				PMENU_ALIGN_CENTER,  0, NULL },

	{ "$lasher for testing",			PMENU_ALIGN_CENTER,  0, NULL },

	{ "*Sound",							PMENU_ALIGN_CENTER,  0, NULL },

	{ "Tom 'Bjorn' Klok",				PMENU_ALIGN_CENTER,  0, NULL },

	{ "*Original CTF Art Design",		PMENU_ALIGN_CENTER,  0, NULL },

	{ "Brian 'Whaleboy' Cozzens",		PMENU_ALIGN_CENTER,  0, NULL },

	{ NULL,								PMENU_ALIGN_CENTER,  0, NULL },

	{ "Return to Main Menu",			PMENU_ALIGN_LEFT,    0, CTFReturnToMain }

};

//RAV

//

pmenu_t joinmenu[] = {

	{ "*Quake II - TMG_MOD",			PMENU_ALIGN_CENTER,  0,  NULL },

	{ "www.railwarz.com",		PMENU_ALIGN_CENTER,  0,  NULL },

	{ NULL,					PMENU_ALIGN_CENTER,  0,  NULL },
	
	{ NULL,					PMENU_ALIGN_CENTER,  0,  NULL }, //JSW Added

	{ "Join Red Team",		PMENU_ALIGN_LEFT,  0,  CTFJoinTeam1 },

	{ NULL,					PMENU_ALIGN_LEFT,  0, NULL },

	{ "Join Blue Team",		PMENU_ALIGN_LEFT,  0,  CTFJoinTeam2 },

	{ NULL,					PMENU_ALIGN_LEFT,  0,  NULL },

	{ "Spectate",		PMENU_ALIGN_LEFT,  0,  RAVspec },

	{ "Vote Menu",					PMENU_ALIGN_LEFT,  0,  VoteMap },

	{ "Op Menu",			PMENU_ALIGN_LEFT,  0,  OPMenu },

	{ "Credits",			PMENU_ALIGN_LEFT,  0,  CTFCredits },

	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT, 0,  NULL },

	{ "ENTER to select",	PMENU_ALIGN_LEFT, 0,  NULL },

	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT,  0,  NULL },

	{ "(TAB to Return)",	PMENU_ALIGN_LEFT,  0,  NULL },

	{ "*v. " MOD_VERSION,	PMENU_ALIGN_LEFT,  0,  NULL },

};

//RAV


pmenu_t joindm[] = {


	{ "*Quake II",			PMENU_ALIGN_CENTER,  0,  NULL },


	{ NULL,				PMENU_ALIGN_CENTER,   0, NULL },


	{ "*TMG_MOD",		PMENU_ALIGN_CENTER,  0,  NULL },


	{ NULL,					PMENU_ALIGN_CENTER,  0,  NULL },


	{ "Join Game",  		PMENU_ALIGN_LEFT,   0, JoinGame },


	{ NULL,					PMENU_ALIGN_LEFT,  0,  NULL },


	{ "Vote Menu",					PMENU_ALIGN_LEFT,  0,  VoteMap },



	{ "Op Menu",			PMENU_ALIGN_LEFT,  0,  OPMenu },


	{ "Spectate",		PMENU_ALIGN_LEFT,  0, RAVspec },


	{ "Credits",			PMENU_ALIGN_LEFT,   0, CTFCredits },


	{ NULL,					PMENU_ALIGN_LEFT,   0, NULL },


	{ "Use [ and ] to move cursor",	PMENU_ALIGN_LEFT,   0, NULL },


	{ "ENTER to select",	PMENU_ALIGN_LEFT,  0,  NULL },


	{ "ESC to Exit Menu",	PMENU_ALIGN_LEFT,  0,  NULL },


	{ "(TAB to Return)",	PMENU_ALIGN_LEFT,   0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT,   0, NULL },


	{ "*v. " MOD_VERSION,	PMENU_ALIGN_LEFT,   0, NULL },


};


pmenu_t tmgmapvote[] = {

    { "*Please Vote For Next Map:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t kicknbanmenu[] = {


    { "*Kick/Ban Player Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t banmenu[] = {


    { "*Ban Player Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};



pmenu_t kickmenu[] = {


    { "*Kick Player Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t silencemenu[] = {


    { "*Silence Player Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t unsilencemenu[] = {


    { "*Un-Silence Player Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t playerlistmenu[] = {


    { "*Player List Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t opmapmenu[] = {
	{ "*Choose a Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t opmapmenu2[] = {
	{ "*Choose a Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t opmapmenu3[] = {
	{ "*Choose a Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t votemapmenu[] = {
	{ "*Choose a Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t votemapmenu2[] = {
	{ "*Choose a Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t votemapmenu3[] = {
	{ "*Choose a Map:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t noworlatermenu[] = {

	{ NULL,			PMENU_ALIGN_CENTER,  0,  NULL },
	{ NULL,				PMENU_ALIGN_CENTER,   0, NULL },
	{ "*Change map now",		PMENU_ALIGN_LEFT,  0,  ChangeNow },
	{ NULL,					PMENU_ALIGN_CENTER,  0,  NULL },
	{ "*Set as next map",  		PMENU_ALIGN_LEFT,   0, ChangeLater },
};

pmenu_t yesnomenu[] = {

	{ "*Vote to change map to",			PMENU_ALIGN_CENTER,  0,  NULL },
	{ NULL,				PMENU_ALIGN_CENTER,   0, NULL },
	{ NULL,				PMENU_ALIGN_CENTER,   0, NULL },
	{ NULL,				PMENU_ALIGN_CENTER,   0, NULL },
	{ "*Vote Yes",		PMENU_ALIGN_LEFT,  0,  VoteYes },
	{ NULL,					PMENU_ALIGN_CENTER,  0,  NULL },
	{ "*Vote No",  		PMENU_ALIGN_LEFT,   0, VoteNo },
};


pmenu_t specmenu[] = {
	{ "*Force Player to Spec:", PMENU_ALIGN_CENTER, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
};

pmenu_t switchmenu[] = {


    { "*Transfer a Player:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};


pmenu_t promotemenu[] = {


    { "*Add an Operator:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


};



pmenu_t lightsmenu[] = {


    { "*Lights Menu:", PMENU_ALIGN_CENTER, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ "Lights ON",					PMENU_ALIGN_LEFT, 0, LightsOn },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },


    { "Lights OFF",					PMENU_ALIGN_LEFT, 0, LightsOff },


};

pmenu_t opmenu[] = {


	{ "**TMG_MOD",			PMENU_ALIGN_CENTER,  0,  NULL },

	{ "*Operator Menu",	PMENU_ALIGN_CENTER,  0,  NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },

    { NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ "*(TAB to Return)",	PMENU_ALIGN_LEFT,  0,  NULL },

};

pmenu_t opmenuP[] = {


	{ "**TMG_MOD",			PMENU_ALIGN_CENTER,  0,  NULL },

	{ "*Operator Player Menu",	PMENU_ALIGN_CENTER,  0,  NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },

	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ NULL,					PMENU_ALIGN_LEFT, 0, NULL },
	{ "*(TAB to Return)",	PMENU_ALIGN_LEFT,  0,  NULL },



};


void OpRestart(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	RestartLevel();
}
void ChangeNow (edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	MapVoteThink(false, true);
	yesnomenu[2].text = "*right now";
}

void ChangeLater (edict_t *ent, pmenu_t *menu)
{
	PMenu_Close(ent);
	MapVoteThink(false, false);
	yesnomenu[2].text = "*at end of level";
}

void VoteChangeMap(edict_t *ent, pmenu_t *p)
{
	if (mapvoteactive == false)
	{
		maplist->currentmapvote = p->arg;
		PMenu_Close(ent);
		ent->client->resp.vote = true;
		PMenu_Open(ent, noworlatermenu, -1, sizeof(noworlatermenu) / sizeof(pmenu_t), true, false);
	}
	else
	{
		PMenu_Close(ent);
		safe_centerprintf(ent, "Mapvote already in progress.\n");
	}
}

void VoteMapNames3()
{
	int i;
	int pos;
	pos = 1;
	
	for (i = 28; i < maplist->nummaps && pos < MAX_MENU_MAPS+1; ++i)
	{
		sprintf(menustring[pos],"%d. %s - %s", i+1, maplist->mapname[i], maplist->mapnick[i]);
		//sprintf(menustring[pos],"%s: %-19.19s", maplist->mapname[i], maplist->mapnick[i]);
		menustring[pos][45] = '\0';
		votemapmenu3[pos].text = menustring[pos];
		votemapmenu3[pos].SelectFunc = VoteChangeMap;
		votemapmenu3[pos].arg = i;
		++pos;
	}
	
	sprintf(menustring[pos],"..Back");
	votemapmenu3[pos].text = menustring[pos];
	votemapmenu3[pos].SelectFunc = VoteMap;
	votemapmenu3[pos].arg = i;
	++pos;

	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS+1)
	{
		votemapmenu3[pos].text = "";
		votemapmenu3[pos].SelectFunc = NULL;
		votemapmenu3[pos].arg = 0;
		++pos;
	}
}

void VoteMap3(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	VoteMapNames3();
	PMenu_Open(ent, votemapmenu3, -1, sizeof(votemapmenu3) / sizeof(pmenu_t), true, false);
}

void VoteMapNames2()
{
	int i;
	int pos;
	pos = 1;
	
	for (i = 14; i < maplist->nummaps && pos < MAX_MENU_MAPS+1; ++i)
	{
		sprintf(menustring[pos],"%d. %s - %s", i+1, maplist->mapname[i], maplist->mapnick[i]);
		//sprintf(menustring[pos],"%s: %-19.19s", maplist->mapname[i], maplist->mapnick[i]);
		menustring[pos][45] = '\0';
		votemapmenu2[pos].text = menustring[pos];
		votemapmenu2[pos].SelectFunc = VoteChangeMap;
		votemapmenu2[pos].arg = i;
		++pos;
	}
	
	if (maplist->nummaps >= MAX_MENU_MAPS*2)
	{
		sprintf(menustring[pos],"More Maps..");
		votemapmenu2[pos].text = menustring[pos];
		votemapmenu2[pos].SelectFunc = VoteMap3;
		votemapmenu2[pos].arg = i;
		++pos;
	}
	else
	{
		sprintf(menustring[pos],"..Back");
		votemapmenu2[pos].text = menustring[pos];
		votemapmenu2[pos].SelectFunc = VoteMap;
		votemapmenu2[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS+1)
	{
		votemapmenu2[pos].text = "";
		votemapmenu2[pos].SelectFunc = NULL;
		votemapmenu2[pos].arg = 0;
		++pos;
	}
}

void VoteMap2(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	VoteMapNames2();
	PMenu_Open(ent, votemapmenu2, -1, sizeof(votemapmenu2) / sizeof(pmenu_t), true, false);
}

void VoteMapNames()
{
	int i;
	int pos;
	pos = 1;
	
	for (i = 0; i < maplist->nummaps && pos < MAX_MENU_MAPS+1; ++i)
	{
		sprintf(menustring[pos],"%d. %s - %s", pos, maplist->mapname[i], maplist->mapnick[i]);
		//sprintf(menustring[pos],"%s: %-19.19s", maplist->mapname[i], maplist->mapnick[i]);
		//strncpy(menustring[pos], string, 24);
		menustring[pos][45] = '\0';
		votemapmenu[pos].text = menustring[pos];
		votemapmenu[pos].SelectFunc = VoteChangeMap;
		votemapmenu[pos].arg = i;
		++pos;
	}
	
	if (maplist->nummaps >= MAX_MENU_MAPS)
	{
		sprintf(menustring[pos],"More Maps..");
		votemapmenu[pos].text = menustring[pos];
		votemapmenu[pos].SelectFunc = VoteMap2;
		//votemapmenu[pos].arg = i;
	}

	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS)
	{
		votemapmenu[pos].text = "";
		votemapmenu[pos].SelectFunc = NULL;
		votemapmenu[pos].arg = 0;
		++pos;
	}
}


void VoteMap(edict_t *ent, pmenu_t *p)
{
	static char string[64];

	if (ent->client->menu)
		PMenu_Close(ent);

	if (!allow_vote->value)
	{
		safe_centerprintf(ent, "Map voting is disabled on this server\n");
		return;
	}

	if (mapvoteactive == true)
	{
		string[0] = '*';
		sprintf(string+1, "%s - %s", maplist->mapname[maplist->currentmapvote], maplist->mapnick[maplist->currentmapvote]);
		yesnomenu[1].text = string;
		PMenu_Open(ent, yesnomenu, -1, sizeof(yesnomenu) / sizeof(pmenu_t), true, false);
	}
	else
	{
		VoteMapNames();
		PMenu_Open(ent, votemapmenu, -1, sizeof(votemapmenu) / sizeof(pmenu_t), true, false);
	}
}

char *GetIpOp(edict_t *ent)
{
	static char modif[40];
	char entry[64], namep[20], ipp1[3],ipp2[3],ipp3[3],ipp4[3];
	int ec, j;
	
	if (!G_EntExists(ent))
		return (0);
	
	printf(entry, "%s@%s", ent->client->pers.netname, ent->client->pers.ip);
	j=0;
	ec = 0;
	while (!strchr("@", entry[ec]))
	{
		sprintf (&namep[j], "%c", entry[ec]);
		j++;
		ec++;
	}
	ec++;
	j = 0;
	while  (!strchr(".", entry[ec]))
	{
		sprintf (&ipp1[j], "%c", entry[ec]);
		j++;
		ec++;
	}
	ec++;
	j = 0;
	while  (!strchr(".", entry[ec]))
	{
		sprintf (&ipp2[j], "%c", entry[ec]);
		j++;
		ec++;
	}
	ec++;
	j = 0;
	while  (!strchr(".", entry[ec]))
	{
		sprintf (&ipp3[j], "%c", entry[ec]);
		j++;
		ec++;
	}
	ec++;
	j = 0;
	while  (!strchr(":", entry[ec]))
	{
		sprintf (&ipp4[j], "%c", entry[ec]);
		j++;
		ec++;
	}
	sprintf (modif, "%s@%s.%s.%s.*",ent->client->pers.netname , ipp1, ipp2, ipp3);
	return (modif);
}

void List_Op(edict_t *ent)
{
	int i;
	int pos;
	edict_t *e;

	pos = 1;
	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)
	{
		e = g_edicts + 1 + i;
		if (!G_EntExists(e))
			continue;
		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);
		promotemenu[pos].text = menustring[pos];
		if(e->client->pers.isop ==1)
			promotemenu[pos].SelectFunc = NULL;
		else
			promotemenu[pos].SelectFunc = OpMe;
		promotemenu[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < maxclients->value+1)
	{
		promotemenu[pos].text = "";
		promotemenu[pos].SelectFunc = NULL;
		promotemenu[pos].arg = 0;
		++pos;
	}
}

void OpPlayer(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	List_Op(ent);
	PMenu_Open(ent, promotemenu, -1, sizeof(promotemenu) / sizeof(pmenu_t), true, false);
}

void OpMe(edict_t *ent, pmenu_t *p)
{
	int i;
	char entry[40];
	edict_t *e;

	i = p->arg;
	e = g_edicts + 1 + i;

	PMenu_Close(ent);

	if (!G_EntExists(e))
		return;

	sprintf(entry, "addop %s %d nopass", GetIp(e), (int)defaultoplevel->value);
	stuffcmd (ent, entry);
}

void OpLockServer(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	
	if(serverlocked)
	{
		serverlocked = false;
		my_bprintf(PRINT_HIGH, "Server is UN-LOCKED\n");
	}
	else
	{
		serverlocked = true;
		my_bprintf(PRINT_HIGH, "Server is LOCKED\n");
	}
}


void OpLock(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	
	if(locked_teams)
	{
		locked_teams = false;
		my_bprintf(PRINT_HIGH, "Teams are UN-LOCKED\n");
	}
	else
	{
		locked_teams = true;
		my_bprintf(PRINT_HIGH, "Teams are LOCKED\n");
	}
}



void List_Switch(edict_t *ent)
{
	int i;
	int pos;
	edict_t *e;
	pos = 1;
	
	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)
	{
		e = g_edicts + 1 + i;
		if (!G_EntExists(e))
			continue;
		if(e->client->resp.ctf_team < 1)
			sprintf(menustring[pos],"%d. %s <SPEC>", pos, e->client->pers.netname);
		else
		{
			if(e->client->resp.ctf_team == 2)
				sprintf(menustring[pos],"%d. %s <Blue>", pos, e->client->pers.netname);
			else
				sprintf(menustring[pos],"%d. %s <Red>", pos, e->client->pers.netname);
		}
		switchmenu[pos].text = menustring[pos];
		//		if(e->client->resp.ctf_team < 1 || e->client->pers.isop ==1)
		if(e->client->resp.ctf_team < 1 || e->client->pers.oplevel & OP_PROTECTED)
			switchmenu[pos].SelectFunc = NULL;
		else
			switchmenu[pos].SelectFunc = SwitchMe;
		switchmenu[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < maxclients->value+1)
	{
		switchmenu[pos].text = "";
		switchmenu[pos].SelectFunc = NULL;
		switchmenu[pos].arg = 0;
		++pos;
	}
}

void SwitchPlayer(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	List_Switch(ent);
	PMenu_Open(ent, switchmenu, -1, sizeof(switchmenu) / sizeof(pmenu_t), true, false);
}



void SwitchMe(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;

	i = p->arg;
	e = g_edicts + 1 + i;

	PMenu_Close(ent);

	if (!G_EntExists(e))
		return;

	if (e->client->hook)
	{
		safe_cprintf (ent, PRINT_CHAT, "Player switch attempted while player was hooking\n");
		return;
	}

	if(e->client->resp.ctf_team == 1)//red
	{
		if(e->bot_client)
			CTFJoinTeam(e, CTF_TEAM2);
		else
			stuffcmd (e, va("team blue\n"));
	}
	else
	{
		if(e->bot_client)
			CTFJoinTeam(e, CTF_TEAM1);
		else
			stuffcmd (e, va("team red\n"));
	}
}

void SpecMe(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;
	i = p->arg;
	e = g_edicts + 1 + i;
	PMenu_Close(ent);
	if (!G_EntExists(e))
		return;
	if (e->bot_client)
	{
		safe_cprintf (ent, PRINT_CHAT, "Player spec force attempted on bot client\n");
		return;
	}
	if (e->client->hook)
	{
		safe_cprintf (ent, PRINT_CHAT, "Player spec force attempted while player was hooking\n");
		return;
	}
	stuffcmd (e, va("spec\n"));
}
void List_Spec(edict_t *ent)
{
	int i;
	int pos;

	edict_t *e;
	pos = 1;

	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)
	{
		e = g_edicts + 1 + i;
		if (!G_EntExists(e))
			continue;
		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);
		specmenu[pos].text = menustring[pos];
		if(e->client->pers.oplevel & OP_PROTECTED || e->is_muted)
			specmenu[pos].SelectFunc = NULL;
		else
			specmenu[pos].SelectFunc = SpecMe;
		specmenu[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < maxclients->value+1)
	{
		specmenu[pos].text = "";
		specmenu[pos].SelectFunc = NULL;
		specmenu[pos].arg = 0;
		++pos;
	}
}

void SpecPlayer(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	List_Spec(ent);
	PMenu_Open(ent, specmenu, -1, sizeof(specmenu) / sizeof(pmenu_t), true, false);
}

void List_UnSilence(edict_t *ent)
{
	int i;
	int pos;

	edict_t *e;
	pos = 1;

	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)
	{
		e = g_edicts + 1 + i;
		if (!G_EntExists(e))
			continue;
		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);
		unsilencemenu[pos].text = menustring[pos];
		if(!e->is_muted)
			unsilencemenu[pos].SelectFunc = NULL;
		else
			unsilencemenu[pos].SelectFunc = UnSilenceMe;
		unsilencemenu[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < maxclients->value+1)
	{
		unsilencemenu[pos].text = "";
		unsilencemenu[pos].SelectFunc = NULL;
		unsilencemenu[pos].arg = 0;
		++pos;
	}
}

void UnSilencePlayer(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	List_UnSilence(ent);


	PMenu_Open(ent, unsilencemenu, -1, sizeof(unsilencemenu) / sizeof(pmenu_t), true, false);


}


void UnSilenceMe(edict_t *ent, pmenu_t *p)


{


	int i;


	edict_t *e;


	i = p->arg;



	e = g_edicts + 1 + i;


		


	PMenu_Close(ent);


		if (!G_EntExists(e))


		return;


	e->is_muted = false;


}



//Player lister


void List_Silence(edict_t *ent)
{
	int i;
	int pos;

	edict_t *e;
	pos = 1;

	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)
	{
		e = g_edicts + 1 + i;
		if (!G_EntExists(e))
			continue;
		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);
		silencemenu[pos].text = menustring[pos];
		if(e->client->pers.oplevel & OP_PROTECTED || e->is_muted)
			silencemenu[pos].SelectFunc = NULL;
		else
			silencemenu[pos].SelectFunc = SilenceMe;
		silencemenu[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < maxclients->value+1)
	{
		silencemenu[pos].text = "";
		silencemenu[pos].SelectFunc = NULL;
		silencemenu[pos].arg = 0;
		++pos;
	}
}



void SilencePlayer(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu) PMenu_Close(ent);
		List_Silence(ent);
	PMenu_Open(ent, silencemenu, -1, sizeof(silencemenu) / sizeof(pmenu_t), true, false);
}


void SilenceMe(edict_t *ent, pmenu_t *p)
{
	int i;
	edict_t *e;
	i = p->arg;
	e = g_edicts + 1 + i;

	


	PMenu_Close(ent);



	if (!G_EntExists(e))


		return;


	e->is_muted = true;


}






//GET IP's strings for banning 


char *GetIp(edict_t *ent)


{


	static char modif[40];


	char entry[32], namep[20], ipp1[3],ipp2[3],ipp3[3],ipp4[3];


	int ec, j;


		


 sprintf(entry, "%s@%s", ent->client->pers.netname, ent->client->pers.ip);



	j=0;


	ec = 0;


	while (!strchr("@", entry[ec]))


	{


		sprintf (&namep[j], "%c", entry[ec]);


		j++;


		ec++;


	}


	ec++;


	j = 0;


	while  (!strchr(".", entry[ec]))


	{


		sprintf (&ipp1[j], "%c", entry[ec]);


		j++;


		ec++;


	}


	ec++;


	j = 0;


	while  (!strchr(".", entry[ec]))


	{


		sprintf (&ipp2[j], "%c", entry[ec]);


		j++;


		ec++;


	}


	ec++;


	j = 0;


	while  (!strchr(".", entry[ec]))


	{


		sprintf (&ipp3[j], "%c", entry[ec]);


		j++;


		ec++;


	}


	ec++;


	j = 0;


	while  (!strchr(":", entry[ec]))


	{


		sprintf (&ipp4[j], "%c", entry[ec]);


		j++;


		ec++;


	}



	sprintf (modif, "*@%s.%s.%s.*",  ipp1, ipp2, ipp3);



	return (modif);


}




void LightsMenu(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	PMenu_Open(ent, lightsmenu, -1, sizeof(lightsmenu) / sizeof(pmenu_t), true, false);


}


void PlayerMenu(edict_t *ent, pmenu_t *p)


{
	UpdatePlayerMenu(ent);
	
	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, opmenuP, -1, sizeof(opmenuP) / sizeof(pmenu_t), true, false);


}



//Player lister


void List_KickBan(edict_t *ent)


{


	int i;


	int pos;



	edict_t *e;


	


	


	pos = 1;


	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)


	{


		e = g_edicts + 1 + i;



	if (!G_EntExists(e))


		continue;



		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);


		kicknbanmenu[pos].text = menustring[pos];


//		if(e->client->pers.isop ==1)
		if(e->client->pers.oplevel & OP_BAN)


		kicknbanmenu[pos].SelectFunc = NULL;


		else


		kicknbanmenu[pos].SelectFunc = KicknBanMe;


		kicknbanmenu[pos].arg = i;


		++pos;


	}




	//Clear the rest of the menu


	while (pos < maxclients->value+1)


	{


		kicknbanmenu[pos].text = "";


		kicknbanmenu[pos].SelectFunc = NULL;


		kicknbanmenu[pos].arg = 0;


		++pos;


	}



}



void KicknBanPlayer(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	List_KickBan(ent);


	PMenu_Open(ent, kicknbanmenu, -1, sizeof(kicknbanmenu) / sizeof(pmenu_t), true, false);


}


void KicknBanMe(edict_t *ent, pmenu_t *p)


{


	int i;


	char cmd[80], cmd1[80];


	edict_t *e;


	i = p->arg;



	e = g_edicts + 1 + i;


		


	PMenu_Close(ent);



	if (!G_EntExists(e))


		return;



	sprintf(cmd, "%s\n",  GetIp(e));


   	addEntry("ip_banned.txt", cmd);



	sprintf(cmd1, "\nkick %d\n",  i);


   	gi.AddCommandString(cmd1);


}




void BanMe(edict_t *ent, pmenu_t *p)
{
	int i;
	char cmd[80];
	edict_t *e;
	i = p->arg;

	e = g_edicts + 1 + i;
	PMenu_Close(ent);
	if (!G_EntExists(e))
		return;

	sprintf(cmd, "%s\n",  GetIp(e));
   	addEntry("ip_banned.txt", cmd);
}



//Player lister


void List_Ban(edict_t *ent)


{


	int i;


	int pos;



	edict_t *e;


	


	


	pos = 1;


	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)


	{


		e = g_edicts + 1 + i;



	if (!G_EntExists(e))


		continue;


		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);


		banmenu[pos].text = menustring[pos];


//		if(e->client->pers.isop ==1)
		if(e->client->pers.oplevel & OP_BAN)


		banmenu[pos].SelectFunc = NULL;


		else


		banmenu[pos].SelectFunc = BanMe;


		banmenu[pos].arg = i;


		++pos;


	}




	//Clear the rest of the menu


	while (pos < maxclients->value+1)


	{


		banmenu[pos].text = "";


		banmenu[pos].SelectFunc = NULL;


		banmenu[pos].arg = 0;


		++pos;


	}



}



void BanPlayer(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	List_Ban(ent);


	PMenu_Open(ent, banmenu, -1, sizeof(banmenu) / sizeof(pmenu_t), true, false);


}




void OPEndDMLevel (int mapindex, edict_t *cl);


void OpChangeMap(edict_t *ent, pmenu_t *p)
{
	int i;

	i = p->arg;
	PMenu_Close(ent);
	OPEndDMLevel (i, ent);
}

void OpMapNames3()
{
	int i;
	int pos;
	pos = 1;
	
	for (i = 28; i < maplist->nummaps && pos < MAX_MENU_MAPS+1; ++i)
	{
		sprintf(menustring[pos],"%d. %s - %s", i+1, maplist->mapname[i], maplist->mapnick[i]);
		//sprintf(menustring[pos],"%s: %-19.19s", maplist->mapname[i], maplist->mapnick[i]);
		menustring[pos][45] = '\0';
		opmapmenu3[pos].text = menustring[pos];
		opmapmenu3[pos].SelectFunc = OpChangeMap;
		opmapmenu3[pos].arg = i;
		++pos;
	}
	
	sprintf(menustring[pos],"..Back");
	opmapmenu3[pos].text = menustring[pos];
	opmapmenu3[pos].SelectFunc = OpMap;
	opmapmenu3[pos].arg = i;
	++pos;

	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS+1)
	{
		opmapmenu3[pos].text = "";
		opmapmenu3[pos].SelectFunc = NULL;
		opmapmenu3[pos].arg = 0;
		++pos;
	}
}

void OpMap3(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	OpMapNames3();
	PMenu_Open(ent, opmapmenu3, -1, sizeof(opmapmenu3) / sizeof(pmenu_t), true, false);
}

void OpMapNames2()
{
	int i;
	int pos;
	pos = 1;
	
	for (i = 14; i < maplist->nummaps && pos < MAX_MENU_MAPS+1; ++i)
	{
		sprintf(menustring[pos],"%d. %s - %s", i+1, maplist->mapname[i], maplist->mapnick[i]);
//		sprintf(menustring[pos],"%s: %-19.19s", maplist->mapname[i], maplist->mapnick[i]);
		menustring[pos][45] = '\0';
		opmapmenu2[pos].text = menustring[pos];
		opmapmenu2[pos].SelectFunc = OpChangeMap;
		opmapmenu2[pos].arg = i;
		++pos;
	}
	
	if (maplist->nummaps >= MAX_MENU_MAPS*2)
	{
		sprintf(menustring[pos],"More Maps..");
		opmapmenu2[pos].text = menustring[pos];
		opmapmenu2[pos].SelectFunc = OpMap3;
		opmapmenu2[pos].arg = i;
		++pos;
	}
	else
	{
		sprintf(menustring[pos],"..Back");
		opmapmenu2[pos].text = menustring[pos];
		opmapmenu2[pos].SelectFunc = OpMap;
		opmapmenu2[pos].arg = i;
		++pos;
	}
	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS+1)
	{
		opmapmenu2[pos].text = "";
		opmapmenu2[pos].SelectFunc = NULL;
		opmapmenu2[pos].arg = 0;
		++pos;
	}
}

void OpMap2(edict_t *ent, pmenu_t *p)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	OpMapNames2();
	PMenu_Open(ent, opmapmenu2, -1, sizeof(opmapmenu2) / sizeof(pmenu_t), true, false);
}

void OpMapNames()
{
	int i;
	int pos;
	pos = 1;
	
	for (i = 0; i < maplist->nummaps && pos < MAX_MENU_MAPS+1; ++i)
	{
		//sprintf(menustring[pos],"%s: %-19.19s", maplist->mapname[i], maplist->mapnick[i]);
		sprintf(menustring[pos],"%d. %s - %s", i+1, maplist->mapname[i], maplist->mapnick[i]);
		menustring[pos][45] = '\0';
		opmapmenu[pos].text = menustring[pos];
		opmapmenu[pos].SelectFunc = OpChangeMap;
		opmapmenu[pos].arg = i;
		++pos;
	}
	
	if (maplist->nummaps >= MAX_MENU_MAPS)
	{
		sprintf(menustring[pos],"More Maps..");
		opmapmenu[pos].text = menustring[pos];
		opmapmenu[pos].SelectFunc = OpMap2;
		opmapmenu[pos].arg = i;
	}

	//Clear the rest of the menu
	while (pos < MAX_MENU_MAPS+1)
	{
		opmapmenu[pos].text = "";
		opmapmenu[pos].SelectFunc = NULL;
		opmapmenu[pos].arg = 0;
		++pos;
	}
}


void OpMap(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	OpMapNames();


	PMenu_Open(ent, opmapmenu, -1, sizeof(opmapmenu) / sizeof(pmenu_t), true, false);


}

void KickMe(edict_t *ent, pmenu_t *p)


{


	int i;


	char cmd[80];



	i = p->arg;



	PMenu_Close(ent);



	sprintf(cmd, "\nkick %d\n",  i);


   	gi.AddCommandString(cmd);



	


}


//Player lister


void List_Kick(edict_t *ent)


{


	int i;


	int pos;



	edict_t *e;


	


	


	pos = 1;


	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)


	{


		e = g_edicts + 1 + i;



	if (!G_EntExists(e))


		continue;



		sprintf(menustring[pos],"%d. %s", pos, e->client->pers.netname);


		kickmenu[pos].text = menustring[pos];


//		if(e->client->pers.isop ==1)
		if(e->client->pers.oplevel & OP_KICK)


		kickmenu[pos].SelectFunc = NULL;


		else


		kickmenu[pos].SelectFunc = KickMe;


		kickmenu[pos].arg = i;


		++pos;


	}




	//Clear the rest of the menu


	while (pos < maxclients->value+1)


	{


		kickmenu[pos].text = "";


		kickmenu[pos].SelectFunc = NULL;


		kickmenu[pos].arg = 0;


		++pos;


	}



}



void KickPlayer(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	List_Kick(ent);


	PMenu_Open(ent, kickmenu, -1, sizeof(kickmenu) / sizeof(pmenu_t), true, false);


}

//JSW
void UpdateOpMenu(edict_t *ent)
{
	char lock_unlock[20];
	int pos, startpos = 2, entries = 1;
	char str[256];
	char seps[]= "\t";
	char *token;

	if (locked_teams)
		strcpy(lock_unlock, "Unlock Teams");
	else
		strcpy(lock_unlock, "Lock Teams");

	strcpy(str, "");
	if (ent->client->pers.oplevel & OP_CHANGEMAP)
		strcat (str, "Change Map\t");
	if (ent->client->pers.oplevel & OP_LIGHTS)
		strcat (str, "Lights Control\t");
	if (ent->client->pers.oplevel & OP_LOCKTEAMS) {
		if (locked_teams)
			strcat (str, "Unlock Teams\t");
		else
			strcat (str, "Lock Teams\t");
	}
	if (ent->client->pers.oplevel & OP_LOCKSERVER)
		strcat (str, "Lock Server\t");
	if (ent->client->pers.oplevel & OP_RESTART)
		strcat (str, "Restart Level\t");

	if (ent->client->pers.oplevel & OP_PLAYERCONTROL)
	{
		opmenu[2].text = "Player Menu";
		opmenu[2].SelectFunc = PlayerMenu;
		startpos = 3;
		entries = 2;
	}
	
	token = strtok (str, seps);
	pos = startpos;
	while (token != NULL)
	{
		sprintf(menustring[pos], token);
		token = strtok (NULL, seps);
		entries++;
		pos++;
	}

	for (pos = startpos; pos < entries+1; pos++)
	{
		opmenu[pos].text = menustring[pos];
		if (Q_stricmp(opmenu[pos].text, "Change Map") == 0)
			opmenu[pos].SelectFunc = OpMap;
		else if (Q_stricmp(opmenu[pos].text, "Lights Control") == 0)
			opmenu[pos].SelectFunc = LightsMenu;
		else if (Q_stricmp(opmenu[pos].text, lock_unlock) == 0)
			opmenu[pos].SelectFunc = OpLock;
		else if (Q_stricmp(opmenu[pos].text, "Lock Server") == 0)
			opmenu[pos].SelectFunc = OpLockServer;
		else if (Q_stricmp(opmenu[pos].text, "Restart Level") == 0)
			opmenu[pos].SelectFunc = OpRestart;
		else
			opmenu[pos].SelectFunc = NULL;
	}
	
	//Clear the rest of the menu
	while (pos < 9)
	{
		opmenu[pos].text = NULL;
		opmenu[pos].SelectFunc = NULL;
		++pos;
	}
}

void UpdatePlayerMenu(edict_t *ent)
{
	int pos = 2, entries = 1;
	char str[256];
	char seps[]= "\t";
	char *token;

	strcpy(str, "");
	if (ent->client->pers.oplevel & OP_STATUS)
		strcat (str, "Player List\t");
	if (ent->client->pers.oplevel & OP_SILENCE)
		strcat (str, "Silence Player\tUnsilence Player\t");
	if (ent->client->pers.oplevel & OP_SWITCH)
		strcat (str, "Transfer Player\tForce Player To Spec\t");
	if (ent->client->pers.oplevel & OP_KICK)
		strcat (str, "Kick Player\t");
	if (ent->client->pers.oplevel & OP_BAN)
		strcat (str, "Ban Player\tKick & Ban Player\t");
	if (ent->client->pers.oplevel & OP_ADDOP)
		strcat (str, "Give Ops\t");

	token = strtok (str, seps);
	while (token != NULL)
	{
		sprintf(menustring[pos], token);
		token = strtok (NULL, seps);
		entries++;
		pos++;
	}

	for (pos = 2; pos < entries+1; pos++)
	{
		opmenuP[pos].text = menustring[pos];
		if (Q_stricmp(opmenuP[pos].text, "Player List") == 0)
			opmenuP[pos].SelectFunc = ListPlayers;
		else if (Q_stricmp(opmenuP[pos].text, "Silence Player") == 0)
			opmenuP[pos].SelectFunc = SilencePlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Unsilence Player") == 0)
			opmenuP[pos].SelectFunc = UnSilencePlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Transfer Player") == 0)
			opmenuP[pos].SelectFunc = SwitchPlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Force Player To Spec") == 0)
			opmenuP[pos].SelectFunc = SpecPlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Kick Player") == 0)
			opmenuP[pos].SelectFunc = KickPlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Ban Player") == 0)
			opmenuP[pos].SelectFunc = BanPlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Kick & Ban Player") == 0)
			opmenuP[pos].SelectFunc = KicknBanPlayer;
		else if (Q_stricmp(opmenuP[pos].text, "Give Ops") == 0)
			opmenuP[pos].SelectFunc = OpPlayer;
		else
			opmenuP[pos].SelectFunc = NULL;
	}
}
//end

//Player lister
void List_Players(edict_t *ent)
{
	int i;
	int pos;
	edict_t *e;
	
	pos = 1;
	for (i = 0; i < maxclients->value && pos < 1+CountConnectedClients (); ++i)
	{
		e = g_edicts + 1 + i;
		
		if (!G_EntExists(e))
			continue;
		
		sprintf(menustring[pos],"%d. %s %s", pos, e->client->pers.netname ,Info_ValueForKey (e->client->pers.userinfo, "ip"));
		playerlistmenu[pos].text = menustring[pos];
		playerlistmenu[pos].SelectFunc = NULL;
		playerlistmenu[pos].arg = i;
		++pos;
	}
	
	//Clear the rest of the menu
	while (pos < maxclients->value+1)
	{
		playerlistmenu[pos].text = "";
		playerlistmenu[pos].SelectFunc = NULL;
		playerlistmenu[pos].arg = 0;
		++pos;
	}
}

void ListPlayers(edict_t *ent, pmenu_t *p)


{


	if (ent->client->menu) PMenu_Close(ent);


	//list the players


	List_Players(ent);


	PMenu_Open(ent, playerlistmenu, -1, sizeof(playerlistmenu) / sizeof(pmenu_t), true, false);


}




void OPMenu(edict_t *ent, pmenu_t *p)
{
	//JSW	if((ent->client->pers.isop !=1)&&(ent->client->pers.ismop !=1))
	if(ent->client->pers.oplevel == 0)
		return;

	if (ent->client->menu)
		PMenu_Close(ent);
	
	UpdateOpMenu(ent);
	
	if(ent->client->pers.isop == 1)
		PMenu_Open(ent, opmenu, -1, sizeof(opmenu) / sizeof(pmenu_t), true, false);
}

void FillMapNames()
{
	int i,j,k,l,m;
	int pos;
	pos = 1;

	//Random 3 maps to choose from 
	if (maplist->nummaps < 3)
	{
		return;
	}

	//avoid the same map in the list twice !!!!
	j=0;
	k=l=m=0;

	while (j<3)
	{
		i = (int) (random() * maplist->nummaps);
		
		if(k==0)
		{
			k=i;
		}
		else if(l==0)
		{
			if(i==k || i==m)
			{
				j=1;
				continue;
			}
			else
				l=i;
		}
			
		else if(m==0)
		{
			if(i==l ||i==k)
			{
				m=0;
				j=2;
				continue;
			}
			else
				m=i;
		}

		sprintf(menustring[pos],"%d. %s - %s", pos, maplist->mapname[i],maplist->mapnick[i]);
		tmgmapvote[pos].text = menustring[pos];
		tmgmapvote[pos].SelectFunc = PickMap;
		tmgmapvote[pos].arg = i;
		++pos;
		j++;
	}
	
	//tell the server console the next voting options !!
	gi.dprintf ("\nMap Vote options are\n1: %s \"%s\"\n2: %s \"%s\"\n3: %s \"%s\"\n\n",
		maplist->mapname[k],maplist->mapnick[k],maplist->mapname[l],maplist->mapnick[l],
		maplist->mapname[m],maplist->mapnick[m]);
	
	//Clear the rest of the menu
//	while (pos < 38)
//	{
//		tmgmapvote[pos].text = "";
//		tmgmapvote[pos].SelectFunc = NULL;
//		tmgmapvote[pos].arg = 0;
//		++pos;
//	}
	
	mapvotefilled = true;
}

void MapVote(edict_t *ent)
{
	if (ent->client->menu)
		PMenu_Close(ent);
	PMenu_Open(ent, tmgmapvote, -1, sizeof(tmgmapvote) / sizeof(pmenu_t), true, false);
}


void PickMap(edict_t *ent, pmenu_t *p)
{
	int i;
	char *name;
	char text[80];

	i = p->arg;
	PMenu_Close(ent);
	if (i >= 0 && i < maplist->nummaps)
	{
		maplist->votes[i]++;
		ent->client->pers.HasVoted = true;
		name = ent->client->pers.netname;
		convert_string(name, 0, 127, 128, name);
		sprintf (text, "%s voted for %s \"%s\"\n", name, maplist->mapname[i],maplist->mapnick[i]);
		my_bprintf (PRINT_HIGH, text);
		ent->client->pers.vote_times ++;
	}
	else
		gi.dprintf("Bad Menu Item #%d\n",i);
}



void Locked(edict_t *ent, pmenu_t *p)


{


PMenu_Close(ent);


safe_centerprintf (ent, "TEAMS ARE <LOCKED> !!!\n");


}

//

int CTFUpdateJoinMenu(edict_t *ent)
{
	static char levelname[32];
	static char team1players[32];
	static char team2players[32];
	int num1, num2, i;

	//JSW
	//if (!ent->client->pers.isop && !ent->client->pers.ismop)
	if (ent->client->pers.isop)// != OP_NAMEPASS && ent->client->pers.oplevel != 0)
	{
		joinmenu[10].text = "Op Menu";
		joinmenu[10].SelectFunc = OPMenu;
	}
	else
	{
		joinmenu[10].text = NULL;
		joinmenu[10].SelectFunc = NULL;
	}
	//end

	if (!locked_teams)
	{
		if(notfairBLUE)
		{
			joinmenu[4].text = "*UNFAIR teams";
			joinmenu[4].SelectFunc = NULL;
		}
		else
		{
			joinmenu[4].text = "Join Red Team";
			joinmenu[4].SelectFunc = CTFJoinTeam1;
		}
		if(notfairRED)
		{
			joinmenu[6].text = "*UNFAIR teams";
			joinmenu[6].SelectFunc = NULL;
		}
		else
		{
			joinmenu[6].text = "Join Blue Team";
			joinmenu[6].SelectFunc = CTFJoinTeam2;
		}
	}
	else
	{
		joinmenu[4].text = "Teams are Locked";
		joinmenu[4].SelectFunc = Locked;
		joinmenu[6].text = "Teams are Locked";
		joinmenu[6].SelectFunc = Locked;
	}

	if (ctf_forcejoin->string && *ctf_forcejoin->string)
	{
		if (stricmp(ctf_forcejoin->string, "red") == 0)
		{
			joinmenu[6].text = NULL;
			joinmenu[6].SelectFunc = NULL;
		}
		else if (stricmp(ctf_forcejoin->string, "blue") == 0)
		{
			joinmenu[4].text = NULL;
			joinmenu[4].SelectFunc = NULL;
		}
	}
	
	if (ent->client->chase_target)
		joinmenu[8].text = "Spectate";
	else
		joinmenu[8].text = "Spectate";
	
	levelname[0] = '*';
	
	if (g_edicts[0].message)
		strncpy(levelname+1, g_edicts[0].message, sizeof(levelname) - 2);
	else
		strncpy(levelname+1, level.mapname, sizeof(levelname) - 2);
	
	levelname[sizeof(levelname) - 1] = 0;
	num1 = num2 = 0;
	
	for (i = 0; i < maxclients->value; i++)
	{
		if (!g_edicts[i+1].inuse)
			continue;
		if (game.clients[i].resp.ctf_team == CTF_TEAM1)
			num1++;
		else if (game.clients[i].resp.ctf_team == CTF_TEAM2)
			num2++;
	}
	
	sprintf(team1players, "  (%d players)", num1);
	sprintf(team2players, "  (%d players)", num2);
	joinmenu[2].text = levelname;
	
	if (joinmenu[4].text)
		joinmenu[5].text = team1players;
	else
		joinmenu[5].text = NULL;
	
	if (joinmenu[6].text)
		joinmenu[7].text = team2players;
	else
		joinmenu[7].text = NULL;
	
	if (num1 > num2)
		return CTF_TEAM2; //JSW fixed, was team 1
	else if (num2 > num1)
		return CTF_TEAM1;
	//JSW Added
	//if teams are equal then put player on team with worse score
	if (ctfgame.team1 > ctfgame.team2)  
		return CTF_TEAM2;
	else if (ctfgame.team2 > ctfgame.team1)
		return CTF_TEAM1;
	//if teams and score are equal put on random team
	//end
	return (rand() & 1) ? CTF_TEAM1 : CTF_TEAM2;
}

//RAV


void OpenJoinMenu(edict_t *ent)


{


	PMenu_Open(ent, joindm, 0, sizeof(joindm) / sizeof(pmenu_t), true, true);


}


//

void CTFOpenJoinMenu(edict_t *ent)

{

	int team;


	team = CTFUpdateJoinMenu(ent);

	if (ent->client->chase_target)

		team = 8;

	else if (team == CTF_TEAM1)

		team = 4;

	else

		team = 6;

	PMenu_Open(ent, joinmenu, team, sizeof(joinmenu) / sizeof(pmenu_t), true, true);

}


void CTFCredits(edict_t *ent, pmenu_t *p)

{

	PMenu_Close(ent);

	PMenu_Open(ent, creditsmenu, -1, sizeof(creditsmenu) / sizeof(pmenu_t), true, true);

}


qboolean CTFStartClient(edict_t *ent)

{


	//RAV

	if(ent->client->pers.pl_state > 0 || ent->bot_client)


		return false;




	if(ctf->value){


	if (ent->client->resp.ctf_team != CTF_NOTEAM)


		return false;



	if (!(dmflag & DF_CTF_FORCEJOIN)) {

		// start as 'observer'

		ent->movetype = MOVETYPE_NOCLIP;

		ent->solid = SOLID_NOT;

		ent->svflags |= SVF_NOCLIENT;

		ent->client->resp.ctf_team = CTF_NOTEAM;

		ent->client->ps.gunindex = 0;


		gi.linkentity (ent);


		ent->client->pers.pl_state = 0;


		ent->client->resp.spectator = 0;

	//	CTFOpenJoinMenu(ent);


		safe_centerprintf(ent, "Hit 'TAB' or 'inven' for Menu\n");

		return true;

	}


}


else


{


		ent->movetype = MOVETYPE_NOCLIP;


		ent->solid = SOLID_NOT;


		ent->svflags |= SVF_NOCLIENT;


		ent->client->pers.pl_state = 0;


		ent->client->ps.gunindex = 0;


		ent->client->resp.spectator = 0;


		gi.linkentity (ent);


	//	OpenJoinMenu(ent);


		safe_centerprintf(ent, "Hit 'TAB' or 'inven' for Menu\n");


		return true;


}

	return false;

}


//RAV

void RavCheckTeams()
{
	if(ctfgame.players1 != ctfgame.players2)
	{
		if(ctfgame.players2 > ctfgame.players1+2)
		{
			bluetime = level.time +10;
			return;
		}
		if(ctfgame.players2+2 < ctfgame.players1)
		{
			redtime = level.time +10;
			return;
		}
		bluetime = 0;
		redtime = 0;
		return;
	}
	bluetime = 0;
	redtime = 0;
}


//RAV	
qboolean CTFCheckRules(void)
{
	//RAV added a lost flag check to check for flags
	edict_t *flag1, *flag2, *carrier;
	int i;
	qboolean red = false;
	qboolean blue = false;
	
	if(flagchecktime == level.time)
	{
		flag1,flag2 = NULL;
/*		//redflag
		if ((flag1 = G_Find(NULL, FOFS(classname), "item_flag_team1")) != NULL)
		{
			//if flag is at home skip
			if (redflagnow[1]  ==  redflag_origin[1])
			{
				red = true;
			}
			else
				red = false;
			if(!red)
				for (i = 1; i <= maxclients->value; i++)
				{
					carrier = g_edicts + i;
					if (carrier->inuse && carrier->client->pers.inventory[ITEM_INDEX(flag1_item)])
					{
						red = true;
						break;
					}
				}
		}
		
		//no flag: now check for time to respawn it
		if (!red)
		{
			if(!red && redflaggone)
			{
				if(redflagtime < level.time)
					//set up spawn timer for 25 sec
					redflagtime = level.time+ 25;
			}
			else
			{
				redflagtime = level.time+ 10;
				redflaggone = true;	
			}
			if(redflaggone && (redflagtime <= level.time || match_state == STATE_WARMUP))
			{
				//spawn a new flag
				CTFResetFlag(CTF_TEAM1);
				my_bprintf(PRINT_HIGH, "A. The %s flag has returned!\n", CTFTeamName(CTF_TEAM1));
				redflaggone = false;
			}
		}
		
		//blueflag
		if ((flag2 = G_Find(NULL, FOFS(classname), "item_flag_team2")) != NULL)
		{
			//if flag is at home return
			if (blueflagnow[0]  ==  blueflag_origin[0])
			{
				blue = true;
			}
			else
				blue = false;
			if(!blue)
				for (i = 1; i <= maxclients->value; i++)
				{
					carrier = g_edicts + i;
					if (carrier->inuse && carrier->client->pers.inventory[ITEM_INDEX(flag2_item)])
					{
						blue = true;
						break;
					}
				}
		}

		//no flag: now check for time to respawn it
		if (!blue)
		{
			if(!blue && blueflaggone)
			{
				//set up spawn timer for 25 sec
				if(blueflagtime < level.time)
					blueflagtime = level.time+ 25;
			}
			else
			{
				blueflagtime = level.time+ 15;
				blueflaggone = true;
			}
			if(blueflaggone && (blueflagtime <= level.time || match_state == STATE_WARMUP))
			{
				//spawn a new flag
				CTFResetFlag(CTF_TEAM2);
				my_bprintf(PRINT_HIGH, "The %s flag has returned!\n", CTFTeamName(CTF_TEAM2));
				blueflaggone = false;
			}
		}
*/		
		//if flag is totally gone respawn it
		if (!G_Find(NULL, FOFS(classname), "item_flag_team1"))
		{
			//spawn a new flag
			CTFResetFlag(CTF_TEAM1);
		}
		if (!G_Find(NULL, FOFS(classname), "item_flag_team2"))
		{	
			//spawn a new flag
			CTFResetFlag(CTF_TEAM2);
		}

		//JSW - check for duplicates
		for (i = 1; i <= maxclients->value; i++)
		{
			carrier = g_edicts + i;
			if (carrier->inuse && carrier->client->pers.inventory[ITEM_INDEX(flag1_item)] && !redflaggone && red && match_state == STATE_WARMUP)
			{
				carrier->client->pers.inventory[ITEM_INDEX(flag1_item)] = 0;
				gi.dprintf ("Duplicate RED flag found!\n");
			}
			if (carrier->inuse && carrier->client->pers.inventory[ITEM_INDEX(flag2_item)] && !blueflaggone && blue && match_state == STATE_WARMUP)
			{
				gi.dprintf ("Duplicate BLUE flag found!\n");
				carrier->client->pers.inventory[ITEM_INDEX(flag2_item)] = 0;
			}
		}
		//end
		
		//set next check time 
		flagchecktime = level.time + 1;
	}
	
	//RAV even teams
	if(even_teams->value)
	{
		int wining =0;
		
		//get scores and compare to see if we need to go further
		if(ctfgame.team1 != ctfgame.team2)
		{
			if(ctfgame.team1 > ctfgame.team2)
				wining = 1;//red
			else
				wining = 2;//blue
			switch(wining)
			{
			case 1:
				if(ctfgame.players1 >= ctfgame.players2+even_teams->value)
					notfairBLUE = true;
				else
					notfairBLUE = false;
				break;
			case 2:
				if(ctfgame.players2 >= ctfgame.players1+even_teams->value)
					notfairRED = true;
				else
					notfairRED = false;
				break;
			default :
				notfairBLUE = false;
				notfairRED = false;
				break;
			}
		}
		else
		{
			notfairBLUE = false;
			notfairRED = false;
		}
	}
	
	//END RAV
	if (mercylimit->value && (ctfgame.team1 >= ctfgame.team2+mercylimit->value
		|| ctfgame.team2 >= ctfgame.team1+mercylimit->value))
	{
		my_bprintf (PRINT_HIGH, "Mercylimit hit.\n");
		return true;
	}
	if (capturelimit->value && (ctfgame.team1 >= capturelimit->value
		|| ctfgame.team2 >= capturelimit->value)) 
	{
		my_bprintf (PRINT_HIGH, "Capturelimit hit.\n");
		return true;
	}
	
	return false;
}


/*--------------------------------------------------------------------------

 * just here to help old map conversions

 *--------------------------------------------------------------------------*/


static void old_teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{

	edict_t		*dest;

	int			i;

	vec3_t		forward;


	if (!other->client)

		return;

	dest = G_Find (NULL, FOFS(targetname), self->target);

	if (!dest)

	{

		gi.dprintf ("Couldn't find destination\n");

		return;

	}


//ZOID

	CTFPlayerResetGrapple(other);

//ZOID


	// unlink to make sure it can't possibly interfere with KillBox

	gi.unlinkentity (other);


	VectorCopy (dest->s.origin, other->s.origin);

	VectorCopy (dest->s.origin, other->s.old_origin);

//	other->s.origin[2] += 10;


	// clear the velocity and hold them in place briefly

	VectorClear (other->velocity);

	other->client->ps.pmove.pm_time = 160>>3;		// hold time

	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;


	// draw the teleport splash at source and on the player

	self->enemy->s.event = EV_PLAYER_TELEPORT;

	other->s.event = EV_PLAYER_TELEPORT;


	// set angles

	for (i=0 ; i<3 ; i++)

		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);


	other->s.angles[PITCH] = 0;

	other->s.angles[YAW] = dest->s.angles[YAW];

	other->s.angles[ROLL] = 0;

	VectorCopy (dest->s.angles, other->client->ps.viewangles);

	VectorCopy (dest->s.angles, other->client->v_angle);


	// give a little forward velocity

	AngleVectors (other->client->v_angle, forward, NULL, NULL);

	VectorScale(forward, 200, other->velocity);


	// kill anything at the destination

	if (!KillBox (other))

	{

	}


	gi.linkentity (other);

}


/*QUAKED trigger_teleport (0.5 0.5 0.5) ?

Players touching this will be teleported

*/

void SP_trigger_teleport (edict_t *ent)

{

	edict_t *s;

	int i;


	if (!ent->target)

	{

		gi.dprintf ("teleporter without a target.\n");

		G_FreeEdict (ent);

		return;

	}


	ent->svflags |= SVF_NOCLIENT;

	ent->solid = SOLID_TRIGGER;

	ent->touch = old_teleporter_touch;

	gi.setmodel (ent, ent->model);

	gi.linkentity (ent);


	// noise maker and splash effect dude

	s = G_Spawn();

	ent->enemy = s;

	for (i = 0; i < 3; i++)

		s->s.origin[i] = ent->mins[i] + (ent->maxs[i] - ent->mins[i])/2;

	s->s.sound = gi.soundindex ("world/hum1.wav");

	gi.linkentity(s);

	

}


/*QUAKED info_teleport_destination (0.5 0.5 0.5) (-16 -16 -24) (16 16 32)

Point trigger_teleports at these.

*/

void SP_info_teleport_destination (edict_t *ent)

{

	ent->s.origin[2] += 16;

}
//3ZB
//PON
void SpawnExtra(vec3_t position,char *classname);
void ED_CallSpawn (edict_t *ent);

//------------------------
//
//	Route setup
//
//	Not only for CTF but also DM
//
//------------------------
void CTFSetupNavSpawn(void)
{
	FILE	*fpout;
	char	name[256];
	char	code[8];
	char	SRCcode[8];
	int	i,j;
	vec3_t	v;
	edict_t		*other;

	unsigned int size;

//PONKO
	spawncycle = level.time + FRAMETIME * 100;
//PONKO
	//���[�g������
	CurrentIndex = 0;
	memset(Route,0,sizeof(Route));
	memset(code,0,8);

#if defined(linux)
	if(!ctf->value)
		sprintf(name,"./%s/%s/nav/%s.nav",game_dir->string, cfgdir->string, level.mapname);
	else
		sprintf(name,"./%s/%s/nav/%s.nav",game_dir->string, cfgdir->string,level.mapname);
#else
	if(!ctf->value)
		sprintf(name,".\\%s\\%s\\nav\\%s.nav",game_dir->string, cfgdir->string, level.mapname);
	else
		sprintf(name,".\\%s\\%s\\nav\\%s.nav",game_dir->string, cfgdir->string, level.mapname);
#endif

	fpout = fopen(name,"rb");
	if(fpout == NULL)
	{
		if(!ctf->value)
			gi.dprintf("Chaining: file %s.nav not found.\n",level.mapname);
		else
			gi.dprintf("Chaining: file %s.nav not found.\n",level.mapname);
	}
	else
	{
		fread(code,sizeof(char),8,fpout);

		if(!ctf->value) strncpy(SRCcode,"3ZBRGDTM",8);
		else strncpy(SRCcode,"3ZBRGCTF",8);

		if(strncmp(code,SRCcode,8))
		{
			CurrentIndex = 0;
			gi.dprintf("Chaining: %s.nav is not a chaining file.\n",level.mapname);
			fclose(fpout);
			return;
		}
		gi.dprintf("Chaining: %s.nav found.\n",level.mapname);
		fread(&CurrentIndex,sizeof(int),1,fpout);

		size = (unsigned int)CurrentIndex * sizeof(route_t);
		fread(Route,size,1,fpout);

		for(i = 0;i < CurrentIndex;i++)
		{
if(Route[i].state == GRS_TELEPORT)
gi.dprintf("GRS_TELEPORT\n");
			if((Route[i].state > GRS_TELEPORT/*GRS_ONROTATE*/ && Route[i].state <= GRS_PUSHBUTTON)
				|| Route[i].state == GRS_REDFLAG || Route[i].state == GRS_BLUEFLAG)
			{
				other = &g_edicts[(int)maxclients->value+1];
				for ( j=maxclients->value+1 ; j<globals.num_edicts ; j++, other++)
				{
					if (other->inuse)
					{
						if(Route[i].state == GRS_ONPLAT
							|| Route[i].state == GRS_ONTRAIN
							|| Route[i].state == GRS_PUSHBUTTON
							|| Route[i].state == GRS_ONDOOR)
						{
							VectorAdd(other->s.origin,other->mins,v);
							if(VectorCompare (Route[i].Pt,v/*other->monsterinfo.last_sighting*/))
							{
								//onplat
								if(Route[i].state == GRS_ONPLAT
									&& !Q_stricmp(other->classname, "func_plat"))
								{
//gi.dprintf("assingned %s\n",other->classname);
									Route[i].ent = other;
									break;
								}
								//train
								else if(Route[i].state == GRS_ONTRAIN
									&& !Q_stricmp(other->classname, "func_train"))
								{
//gi.dprintf("assingned %s\n",other->classname);
									Route[i].ent = other;
									break;
								}
								//button
								else if(Route[i].state == GRS_PUSHBUTTON
									&& !Q_stricmp(other->classname, "func_button"))
								{
//gi.dprintf("assingned %s\n",other->classname);
									Route[i].ent = other;
									break;
								}
								//door
								else if(Route[i].state == GRS_ONDOOR
									&& !Q_stricmp(other->classname, "func_door"))
								{
//gi.dprintf("assingned %s\n",other->classname);
									Route[i].ent = other;
									break;
								}
							}
						}
						else if(Route[i].state == GRS_ITEMS
							|| Route[i].state == GRS_REDFLAG || Route[i].state == GRS_BLUEFLAG)
						{
//else gi.dprintf("CYAU!!!!!!!\n");
							if(VectorCompare (Route[i].Pt,other->monsterinfo.last_sighting/*->s.origin*/))
							{
								//onplat
								if(1/*other->classname[0] == 'w' || other->classname[0] == 'i'*/)
								{
//if(Route[i].state == GRS_REDFLAG || Route[i].state == GRS_BLUEFLAG)
//gi.dprintf("HATA ATTA!!!!!!!\n");
//gi.dprintf("assingned %s\n",other->classname);
									Route[i].ent = other;
									break;
								}
							}
							else
							{
//if(Route[i].state == GRS_REDFLAG || Route[i].state == GRS_BLUEFLAG)
//gi.dprintf("HATA Dame!!!!!!!\n");
							}
						}
					}
				}
				if(j >= globals.num_edicts && (Route[i].state == GRS_ITEMS || Route[i].state == GRS_REDFLAG || Route[i].state == GRS_BLUEFLAG)) gi.dprintf("kicked item\n");
				if(j >= globals.num_edicts) Route[i].state = GRS_NORMAL;
			}
		}
		gi.dprintf("Chaining: Total %i chaining pod assigned.\n",CurrentIndex);
		fclose(fpout);
	}
	return;
}

void SpawnExtra(vec3_t position,char *classname)
{
	edict_t		*it_ent;

	it_ent = G_Spawn();

	it_ent->classname = classname;
	VectorCopy(position,it_ent->s.origin);
	ED_CallSpawn(it_ent);

	if(ctf->value && chedit->value)
	{
		it_ent->moveinfo.speed = -1;
		it_ent->s.effects |= EF_QUAD;
	}
}

void CTFJobAssign (void)
{
	int			i;
	int			defend1,defend2;		//�f�B�t�F���_�[����
	int			mate1,mate2;		//�`�[�����C�g����
	gclient_t	*client;
	edict_t		*e;
	edict_t		*defei1,*defei2;		//���
	edict_t		*geti1,*geti2;		//���

	defend1 = 0;
	defend2 = 0;
	mate1 = 0;
	mate2 = 0;
	defei1 = NULL;
	defei2 = NULL;
	geti1 = NULL;
	geti2 = NULL;

	e = &g_edicts[(int)maxclients->value];
	for ( i = maxclients->value ; i >= 1  ; i--, e--)
	{
		if (e->inuse)
		{
			client = e->client;
			if(client->zc.ctfstate == CTFS_NONE) client->zc.ctfstate = CTFS_DEFENDER;
//if(client->zc.ctfstate == CTFS_CARRIER)
//gi.bprintf(PRINT_HIGH,"I am carrierY!!\n");
			if(e->client->resp.ctf_team == CTF_TEAM1)
			{
				mate1++;
				if( e->client->pers.inventory[ITEM_INDEX(FindItem("Blue Flag"))])
				{
					client->zc.ctfstate = CTFS_CARRIER;
				}
				if(1/*e->svflags & SVF_MONSTER*/)
				{

					if( client->zc.ctfstate == CTFS_OFFENCER && random()>0.7) defei1 = e;
					else if( client->zc.ctfstate == CTFS_DEFENDER)
					{
						if(random()>0.7) geti1 = e;
						defend1++;
					}
					else if( client->zc.ctfstate == CTFS_CARRIER ) defend1++;
				}
			}
			else if(e->client->resp.ctf_team == CTF_TEAM2)
			{
				mate2++;
				if( e->client->pers.inventory[ITEM_INDEX(FindItem("Red Flag"))])
				{
					client->zc.ctfstate = CTFS_CARRIER;
				}
				if(1/*e->svflags & SVF_MONSTER*/)
				{
					if( client->zc.ctfstate == CTFS_OFFENCER && random()>0.8) defei2 = e;
					else if( client->zc.ctfstate == CTFS_DEFENDER)
					{
						if(random()>0.7) geti2 = e;
						defend2++;
					}
					else if( client->zc.ctfstate == CTFS_CARRIER ) defend2++;
				}
			}
		}
	}

	if(defend1 < mate1 / 3 && mate1 >= 2)
	{
		if(defei1 != NULL) defei1->client->zc.ctfstate = CTFS_DEFENDER;
	}
	else if(defend1 > mate1 / 3  )
	{
		if(geti1 != NULL) geti1->client->zc.ctfstate = CTFS_OFFENCER;
	}
	if(defend2 < mate2 / 3 && mate2 >= 2)
	{
		if(defei2 != NULL) defei2->client->zc.ctfstate = CTFS_DEFENDER;
	}
	else if(defend2 > mate2 / 3 )
	{
		if(geti2 != NULL) geti2->client->zc.ctfstate = CTFS_OFFENCER;
	}
///	gi.bprintf(PRINT_HIGH,"Called!!!!\n");
}

//




