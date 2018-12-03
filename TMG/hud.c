
#include "g_local.h"
#include "hud.h"
#include "g_items.h"
#include "timer.h"
#include "g_chase.h"
#include "runes.h"

// globals
int timer;
int vote_state;
int vote_pro;
int vote_con;
int newdmflags;
edict_t *votestarter;
edict_t *votetarget;

void GetDate(void)
{
	int day, month, year;
	struct tm *ltime;  
	time_t gmtime;     

	time(&gmtime);
	ltime = localtime(&gmtime);

	month = ltime->tm_mon + 1;
	day = ltime->tm_mday;
	year = ltime->tm_year + 1900;
	Com_sprintf(sys_date, sizeof sys_date, 
		"%02i-%02i-%4i", month, day, year);
}

/*
 Format the local time string for use in the HUD.
 Use AM/PM or 24 hour time depending on the cvar tmgclock, (12 or 24). 
 Cvar server_time controls display of local or UTC modes. 
 Set server_time to 0 for no time in HUD, 1 for local time, 2 for GMT.
 Variable ampm contains "", "AM", "PM" or "UTC" per each mode.
*/
static qboolean initialized = 0;
void GetTime(void)
{
	char ampm[4] = "";
	char buf[32] = "";
	int min, hour, sec;
	struct tm *ltime;  
	time_t gmt_time;     

	time(&gmt_time); // read current time
	if (server_time->value == 1)
		ltime = localtime(&gmt_time);

	else // if server_time != 0 or 1 we want UTC
	{
		ltime = gmtime(&gmt_time);
		Com_sprintf(ampm, sizeof ampm, "UTC");
	}

	min = ltime->tm_min;
	sec = ltime->tm_sec;
	hour = ltime->tm_hour;

	if (server_time->value == 1 && tmgclock->value != 24)
	{
		if (hour >= 12)
		{
			hour -= 12;
			Com_sprintf(ampm, sizeof ampm, "PM");
		}
		else
			Com_sprintf(ampm, sizeof ampm, "AM");
	}

	Com_sprintf (buf, sizeof buf,
				 "%02i:%02i:%02i %s", hour, min, sec, ampm);

	if (Q_stricmp(sys_time, buf) != 0) 
	{
		strcpy(sys_time, buf);
		if (initialized)
		{
			gi.configstring (CS_SYSTIME, sys_time);
			initialized = 1;
		}
	}
}

/************this displays 5 digit min:sec************/
void TimeLeft(void)
{
	int min;
	int sec;
	char buf[32];
	long seconds_left;

	// how long left ?
	seconds_left = ceil(match_state_end - level.time);
	min = (int) seconds_left / 60;
	sec = (int) seconds_left - (min) * 60;

	// this slows the update rate to once every 0.5 seconds
	timer++; 
	if((timer < 5 ) || (level.intermissiontime))
		return;
	
	timer = 0;
	if (sec < 0)
		Com_sprintf (buf, sizeof(buf), "00:00");
	else
		Com_sprintf (buf, sizeof(buf), "%02i:%02i", min, sec);

	if (strcmp(match_time_left, buf) != 0)
	{
		strcpy(match_time_left, buf);
		gi.configstring (CS_TIMELEFT, match_time_left);
	}
	//track server time
	if(server_time->value)
		GetTime();
}

int CountConnectedClients (void)
{
	int n, count;
	edict_t *player;

	count = 0;
	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];
		if (!player->inuse)
			continue;
		else
			count++;
	}
	return count;
}

static char *tn_id (edict_t *ent)
{
	int j = 0;
	static char stats[200];
	vec3_t  start, forward, end, v;
	trace_t tr;

	Com_sprintf(stats, sizeof(stats), "");
	VectorCopy(ent->s.origin, start);
	start[2] += ent->viewheight;
	AngleVectors(ent->client->v_angle, forward, NULL, NULL);
	VectorMA(start, 8192, forward, end);
	tr = gi.trace(start, NULL, NULL, end, ent, MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA);

	if ((Q_stricmp (tr.ent->classname, "player") == 0 && tr.ent->inuse))
	{
		_VectorSubtract (ent->s.origin, tr.ent->s.origin, v);
		j += sprintf(stats + j,
			"xv 80 yb -68 string \"Viewing %s\" ", 
			tr.ent->client->pers.netname );
	}
	return (stats); 
}


/*
char *tn_vote_msg (edict_t *ent)
{
	static char stats[200];
	static char line1[64], line2[64];
	int xv1, xv2;

	if (Q_stricmp(vote_cmd, "map") == 0)
		Com_sprintf (line1, sizeof line1, "%s : change map to %s.", 
			votestarter->client->pers.netname, vote_entry);
	else
	if (Q_stricmp(vote_cmd,"cycle")==0)
		Com_sprintf (line1, sizeof line1, "%s : change cycle to %s.", 
			votestarter->client->pers.netname, vote_entry);
	else
	if (Q_stricmp(vote_cmd,"promote")==0)
		Com_sprintf (line1, sizeof line1, "%s : promote %s.", 
			votestarter->client->pers.netname, 
			votetarget->client->pers.netname );
	else
	if (Q_stricmp(vote_cmd,"demote")==0)
		Com_sprintf (line1, sizeof line1, "%s : demote %s.", 
			votestarter->client->pers.netname, 
			votetarget->client->pers.netname );
	Com_sprintf (line2, sizeof line2, 
		"Agree? Go to Console and type (yes/no)");
 
	xv1 = 160-(strlen(line1)*4);
	xv2 = 160-(strlen(line2)*4);

	Com_sprintf (stats, sizeof stats, 
		"xv %d yb -90 string2 \"%s\" xv %d yb -180 string \"%s\" ", 
		xv1, line1, xv2, line2);

	return (stats);
}

char *tn_votewait (edict_t *ent)
{
	static char stats[200];
	static char line1[64];
	int xv1;
	Com_sprintf (line1, sizeof line1, 
		"Waiting for vote results...");

	xv1 = 160-(strlen(line1)*4);
	Com_sprintf (stats, sizeof stats, 
		"xv %d yb -90 string2 \"%s\" ", xv1, line1);

	return (stats);
}
*/

/* 
	Calculate player's frags per hour
	and clamp it to three digits.
	Store result in player's client data.
*/
void CalcFPH(edict_t *ent)
{
	int fph = 0;
	float framesperhour = 3600.0f / FRAMETIME;
	gclient_t *cl = ent->client;
	int interval = level.framenum - cl->resp.enterframe;
	fph = cl->resp.frags / (interval / framesperhour);

	if(fph >= 999)
		fph = 999;
	if(fph <= -99)
		fph = -99;
	if(fph >= 0)
		cl->resp.fph = fph;
}

/* 
	Calculate player's frags per minute
	and clamp it to three digits.
	Store result in player's client data.
*/
void CalcFPM(edict_t *ent)
{
	int fpm = 0;
	float framespermin = 60.0f / FRAMETIME;
	gclient_t *cl = ent->client;
	int interval = level.framenum - cl->resp.enterframe;
	fpm = cl->resp.frags / (interval / framespermin);

	if(fpm >= 999)
		fpm = 999;
	if(fpm <= -99)
		fpm = -99;
	if(fpm >= 0)
		cl->resp.fpm = fpm;
} 

static int rav_getrank(edict_t *ent)
{
	int total,i,j,k,score;
	edict_t *cl_ent;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];

	total = 0;
	for (i=0; i < game.maxclients; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;

		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}
	for (i=0 ; i<total ; i++)
	{
		cl_ent = g_edicts + 1 + sorted[i];
		if (cl_ent == ent)
		{
			return  i+1;
		}
	}
	return 0;
}

static int rav_getdied(gclient_t* cl)
{
	int dead = 0;

	dead = cl->resp.deaths;

	if(dead >= 999)
		dead = 999;
	if(dead <= -99)
		dead = -99;

	return dead;
}

char *rav_gettech(edict_t *ent)
{
	gitem_t *rune;
	int i;

	//rune addition
	if(runes->value && rune_has_a_rune(ent))
	{
		for (i=RUNE_FIRST; i<=RUNE_LAST; i++)
		{
			if (rune_has_rune(ent, i))
			{
				if ((rune = FindItem(rune_namefornum[i])) != NULL
					&& ent->client->pers.inventory[ITEM_INDEX(rune)])
					return(rune->pickup_name);
			}
		}
	}
	return ("No Rune");
}

// ===================================================================
// Unified HUD initialization by QwazyWabbit
// ===================================================================

// cursor positioning
//	xl <value>		x-left side
//	xr <value>		x-right side
//	yb <value>		y-bottom
//	yt <value>		y-top
//	xv <value>		x-value
//	yv <value>		y-value

// drawing
//	statpic <name>
//	pic <stat>
//	num <fieldwidth> <stat>
//	string <stat>

// control
//	if <stat>
//	ifeq <stat> <value>
//	ifbit <stat> <value>
//	endif

// //QW//
// In all cases <stat> is the integer representing the status message item
// to be presented. I've used manifest constants to keep the messages
// properly enumerated across the game modes and they are defined in 
// q_shared.h. The code originally had these scattered about in
// different places but it mostly used magic numbers in the messages and that just
// made it hard to figure out exactly what was happening. They defined the constants
// and then didn't use them. Later authors were overlapping them in their
// mods or pulling in overlapping stat items from other mods and this creates
// HUD conflicts. ENUM or #DEFINE, then use them. Please.

// Positioning:
// Position 0,0 is the center of the screen with x going negative to the right.
// xv, yv is relative to this origin.
// xl is plus counts from origin left justified.
// xr is minus counts from origin right justified.
// yb is minus counts from bottom. (yb -10 is bottom line)
// yt is plus counts from 0 at top.

/* //QW//
A standard HUD character (conchar) is 8 screen units wide. The xl and xr origins are with
respect to the left and right borders. Add/subract 2 units to keep a little space between
the edge and the chars. For example, the string "FPH" is 3 chars wide, 3 * 8 = 24 units. 
If using xr, add 2 and negate, giving -26 from the right edge as the start of the 
string on the screen, "Range" is (-1)(5 * 8 + 2) = -42.
Using xl, just use an origin of 2 to space a string 2 units from the border so it looks nice.
*/

// //QW//
// The big HUD characters for the counters are 16 units wide but 
// their origin is already offset by 2.
// They are positioned at 0 on the left and at (n * 16)-2 on the right
// when n is the number of digits you want to display.
//
// Printing big chars on the left is problematic, they are right-justfied inside their
// block so printing them on the left will gap them from the edge when the value doesn't
// fill the full range of digits. Negative signs will probably be clipped if you don't
// allow an extra space for them. 
// You should layout for signed values on the right side or midline where negative
// values aren't a problem. 
// Layout for unsigned on the left if you don't mind having right-justfied 
// blocks there. Allow space only for the number of digits you expect to
// display to keep the HUD packet as small as possible.

// I think I first saw an integrated HUD as a function in the CTC mod.
// I took it a little further by integrating the game modes and collecting
// the stat constants in one place.  

/* //QW//
Big HUD chars are 24 units tall and conchars are 8 for a total of 32 plus
vertical spacing of 3 units to make it 35 units for a per-line increment
on the big chars. I use a 25 unit vertical offset for the conchars label
below it. This seems to give a nice uniform leading between lines.
*/

// ===================================================================

char *ShowHud (edict_t *ent)
{
	static char layout[1300];
	int j = 0;
	gclient_t	*cl;
	int	score;
	int ping;
	int fph;
	//int fpm; // unused
	int frags, captures, died, num_ppl, rank;
	int shots, eff;
	int bigspree;
	size_t	s;  //status bar string length

	// chasecam status plate
	char rectangle[] =  "\22\23\23\23\23\23\23\23\23\23\23\23\23\24\n"
						"\25\26\26\26\26\26\26\26\26\26\26\26\26\27\n"
						"\30\31\31\31\31\31\31\31\31\31\31\31\31\32";
	
	char hudhelp[] = "Press TAB for Menu\n"
					"Hit [ or ] for player select\n"
					"Fire to change view\n";
	
	num_ppl = CountConnectedClients();
	cl = ent->client;

	Com_sprintf (layout, sizeof(layout), "");

	if (!ent || !ent->client || ent->bot_client)
		return 0;

	if (level.intermissiontime)
		return 0;

	// Begin spectator HUD
	if ((ent->client->resp.spectator != PL_SPECTATOR || 
		ent->client->pers.pl_state == PL_SPECTATOR) && 
		(ent->client->pers.motd == false))
	{
		if (ent->client->chase_mode == CHASE_FREECAM)
		{
			j += sprintf (layout+j, "xv 0 yb -90 cstring \"%s\" ", rectangle);
			j += sprintf (layout+j, "xv 0 yb -94 cstring \"\nFreeCam\" ");
			if(ent->client->chase_target != NULL)
				j += sprintf (layout+j, "%s", va ("xv 0 yb -78 cstring \"%s\" ", 
							ent->client->chase_target->client->pers.netname ));
			j += sprintf (layout+j, "xv 0 yb -55 cstring \"%s\" ", hudhelp);
			ent->client->pers.db_id = true;
		}
		else if (ent->client->chase_mode == CHASE_FLOATCAM)
		{
			if(ent->client->chase_target == NULL)
			{
				ent->client->chase_mode = CHASE_FREECAM;
				ent->client->ps.pmove.pm_flags &= PMF_NO_PREDICTION;
				ent->client->ps.pmove.pm_type = PM_SPECTATOR;
			}
			else
			{
				j += sprintf (layout+j, "xv 0 yb -90 cstring \"%s\" ", rectangle);
				j += sprintf (layout+j, "xv 0 yb -94 cstring \"\nFloatCam\" ");
				j += sprintf (layout+j, "%s", va ("xv 0 yb -78 cstring \"%s\" ",
							ent->client->chase_target->client->pers.netname ));
				j += sprintf (layout+j, "xv 0 yb -55 cstring \"%s\" ", hudhelp);
			}
		}
		else if(ent->client->chase_mode == CHASE_EYECAM)
		{
			if(ent->client->chase_target == NULL)
			{
				ent->client->chase_mode = CHASE_FREECAM;
				ent->client->ps.pmove.pm_flags &= PMF_NO_PREDICTION;
				ent->client->ps.pmove.pm_type = PM_SPECTATOR;
			}
			else
			{
				j += sprintf (layout+j, "xv 0 yb -90 cstring \"%s\" ", rectangle);
				j += sprintf (layout+j, "xv 0 yb -94 cstring \"\nEyECam\" ");
				j += sprintf (layout+j, "%s", va ("xv 0 yb -78 cstring \"%s\" ",
							ent->client->chase_target->client->pers.netname ));
				j += sprintf (layout+j, "xv 0 yb -55 cstring \"%s\" ", hudhelp);
			}
		}
		else if(ent->client->chase_mode == CHASE_CHASECAM)
		{
			if(ent->client->chase_target == NULL)
			{
				ent->client->chase_mode = CHASE_FREECAM;
				ent->client->ps.pmove.pm_flags &= PMF_NO_PREDICTION;
				ent->client->ps.pmove.pm_type = PM_SPECTATOR;
			}
			else
			{
				j += sprintf (layout+j, "xv 0 yb -90 cstring \"%s\" ", rectangle);
				j += sprintf (layout+j, "xv 0 yb -94 cstring \"\nChaseCam\" ");
				j += sprintf (layout+j, "%s", va ("xv 0 yb -78 cstring \"%s\" ", 
							ent->client->chase_target->client->pers.netname ));
				j += sprintf (layout+j, "xv 0 yb -55 cstring \"%s\" ", hudhelp);
			}
		}
		else
			ent->client->chase_mode = CHASE_FREECAM;

		if(ent->client->chase_target != NULL)	// chasing a player
		{
			// Raven 12-31-1999
			// Revised by QwazyWabbit 2016
			score = cl->chase_target->client->resp.score;
			frags = cl->chase_target->client->resp.frags;
			captures = cl->chase_target->client->resp.captures;
			shots = cl->chase_target->client->resp.shots;
			eff = cl->chase_target->client->resp.eff;
			ping = cl->chase_target->client->ping;
			died = rav_getdied(cl->chase_target->client);
			fph = cl->chase_target->client->resp.fph;
			//QW// fpm is unused for now.
			//fpm = cl->chase_target->client->resp.fpm;
			rank = rav_getrank(cl->chase_target);
			bigspree = cl->chase_target->client->resp.spree;
			//score
			j += sprintf (layout+j, "xr -50 yt 2 string2 \"Score\" ");
			j += sprintf (layout+j, "xr -50 yt 10 string \"%i\" ", score);
			//ping
			j += sprintf (layout+j, "xr -50 yt 18 string2 \"Ping\" ");
			j += sprintf (layout+j, "xr -50 yt 26 string \"%i\" ", ping);
			//fph
			j += sprintf (layout+j, "xr -50 yt 34 string2 \"FPH\" ");
			j += sprintf (layout+j, "xr -50 yt 42 string \"%i\" ", fph);
			//Efficiency
			j += sprintf (layout+j, "xr -50 yt 50 string2 \"Acc\" ");
			j += sprintf (layout+j, "xr -50 yt 58 string \"%i%%%%\" ", eff);
			j += sprintf (layout+j, "xr -50 yt 66 string \"%i/%i\" ", frags, shots);
			//rank
			j += sprintf (layout+j, "xr -50 yt 74 string2 \"Rank\" ");
			j += sprintf (layout+j, "xr -50 yt 82 string \"%i/%i \" ", rank, num_ppl);
			//time
			j += sprintf (layout+j, "xr -50 yt 90 string2 \"Time\" ");
			j += sprintf (layout+j, "xr -50 yt 98 string \"%s \" ", match_time_left);
			//Deaths
			j += sprintf (layout+j, "xr -50 yt 106 string2 \"Deaths\" ");
			j += sprintf (layout+j, "xr -50 yt 114 string \"%i \" ", died);

			j += sprintf (layout+j, "xr -50 yt 122 string2 \"Frags\" ");
			j += sprintf (layout+j, "xr -50 yt 130 string \"%i \" ", frags);
			
			j += sprintf (layout+j, "xr -50 yt 138 string2 \"Spree\" ");
			j += sprintf (layout+j, "xr -50 yt 146 string \"%i \" ", bigspree);

			if (ctf->value)
			{
				j += sprintf (layout+j, "xr -50 yt 154 string2 \"Caps\" ");
				j += sprintf (layout+j, "xr -50 yt 162 string \"%i \" ", captures);
			}
		}
	} // End spectator HUD

	// All players see this
	if (match_state < STATE_PLAYING)
	{
		if(match_state == STATE_WARMUP)
			j += sprintf (layout+j, "xl 22 yb -200 string2 \"WarmUp Mode\" ");
		if(match_state == STATE_COUNTDOWN)
			j += sprintf (layout+j, "xl 22 yb -200 string2 \"Seconds Until Level Starts\" ");
	}

	// Begin player HUD
	if(ent->client->pers.pl_state != PL_SPECTATOR)
	{
		//parse out the info
		score = cl->resp.score;
		frags = cl->resp.frags;
		captures = cl->resp.captures;
		shots = cl->resp.shots;
		eff = cl->resp.eff;
		ping = cl->ping;
		died = rav_getdied(cl);
		rank = rav_getrank(ent);
		bigspree = cl->resp.spree;

		//score
		j += sprintf (layout+j, "xr -50 yt 2 string2 \"Score\" ");
		j += sprintf (layout+j, "xr -50 yt 10 string \"%i\" ", score);
		//ping
		j += sprintf (layout+j, "xr -50 yt 18 string2 \"Ping\" ");
		j += sprintf (layout+j, "xr -50 yt 26 string \"%i\" ", ping);
		//fph
		j += sprintf (layout+j, "xr -50 yt 34 string2 \"FPH\" ");
		j += sprintf (layout+j, "xr -50 yt 42 string \"%i\" ", cl->resp.fph);
		//Efficiency
		j += sprintf (layout+j, "xr -50 yt 50 string2 \"Acc\" ");
		j += sprintf (layout+j, "xr -50 yt 58 string \"%i%%%%\" ", eff);
		j += sprintf (layout+j, "xr -50 yt 66 string \"%i/%i\" ", frags, shots);
		//rank
		j += sprintf (layout+j, "xr -50 yt 74 string2 \"Rank\" ");
		j += sprintf (layout+j, "xr -50 yt 82 string \"%i/%i\" ", rank, num_ppl);
		//time
		j += sprintf (layout+j, "xr -50 yt 90 string2 \"Time\" ");
		j += sprintf (layout+j, "xr -50 yt 98 string \"%s \" ", match_time_left);
		//Deaths
		j += sprintf (layout+j, "xr -50 yt 106 string2 \"Deaths\" ");
		j += sprintf (layout+j, "xr -50 yt 114 string \"%i \" ", died);

		j += sprintf (layout+j, "xr -50 yt 122 string2 \"Frags\" ");
		j += sprintf (layout+j, "xr -50 yt 130 string \"%i \" ", frags);

		j += sprintf (layout+j, "xr -50 yt 138 string2 \"Spree\" ");
		j += sprintf (layout+j, "xr -50 yt 146 string \"%i \" ", bigspree);

		if (ctf->value)
		{
			j += sprintf (layout+j, "xr -50 yt 154 string2 \"Caps\" ");
			j += sprintf (layout+j, "xr -50 yt 162 string \"%i \" ", captures);
		}

		//only show if hud is active
		//stuff on lower left hand area
		if(ent->client->pers.db_hud)
		{
			/* Very bottom left corner */
			j += sprintf (layout+j, "xl 2 yb -10 string2 \"%s %s\" ", MOD, MOD_VERSION);

			//JSW
			if (ent->client->pers.oplevel)
				j += sprintf (layout+j, "xl 2 yb -20 string2 \"Operator Level %d\" ", ent->client->pers.oplevel);

			if(server_time->value)
			{
				j += sprintf (layout+j, "xl 2 yb -90 string2 \" Server Time\" ");
				j += sprintf (layout+j, "xl 2 yb -80 string \" %s\" ", sys_time);
			}

			//QW// Reserve area in HUD "xl 0 yb -40 thru -70"
			//QW// for player messages in r1q2 clients

			if (ent->is_muted)
				j += sprintf (layout+j, "xl 120 yb -90 string2 \"You are Muted\" ");
			if (ent->is_blocked)
				j += sprintf (layout+j, "xl 120 yb -80 string2 \"You are Flood-blocked\" ");
			if (notfairBLUE)
				j += sprintf (layout+j, "xl 22 yb -200 string2 \"UNFAIR: Red Has Too many Players\" ");
			if(notfairRED)
				j += sprintf (layout+j, "xl 22 yb -200 string2 \"UNFAIR: Blue  Has Too many Players\" ");
			if(redtime >level.time || bluetime >level.time)
				j += sprintf (layout+j, "xl 340 yb -220 string \"Even Teams Please!\" ");
		}
	}	// End player HUD

	/* ALL HUDS */
	// show player location just below crosshair if "location" is toggled on
	if (ent->client->resp.locationactive)
		j += sprintf(layout+j, "xv 60 yv 140 string2 \"%4.0f %4.0f %4.0f Angle %3.0f\" ", 
			ent->s.origin[0], ent->s.origin[1], ent->s.origin[2],
			ent->client->ps.viewangles[1]);

	//id disabling added 12-14-99 raven
	if ((!ent->client->chase_target) && (ent->client->pers.db_id))
		j += sprintf (layout+j, "%s", tn_id (ent));

	s = strlen(layout);
	if ( s > sizeof layout)
	{
		gi.dprintf("%s: Statusbar too big %d\n", __func__, s); //to the log
		layout[sizeof layout - 1] = 0;
	}

	//DbgPrintf(layout);
	//DbgPrintf("\n");
	return (layout);
}

