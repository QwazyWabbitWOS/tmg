#include <time.h>
#include "g_local.h"
#include "g_items.h"
#include "anticheat.h"
#include "filehand.h"
#include "m_player.h"
#include "e_hook.h"
#include "g_chase.h"
#include "g_cmds.h"
#include "p_client.h"
#include "timer.h"
#include "p_hud.h"
#include "filtering.h"
#include "hud.h"
#include "intro.h"
#include "bot.h"
#include "runes.h"
#include "statslog.h"

#define	OPTIMIZE_INTERVAL	0.1

//
// Gross, ugly, disgusting hack section
//

// this function is an ugly as hell hack to fix some map flaws
//
// the coop spawn spots on some maps are SNAFU.  There are coop spots
// with the wrong targetname as well as spots with no name at all
//
// we use carnal knowledge of the maps to fix the coop spot targetnames to match
// that of the nearest named single player spot

static void SP_FixCoopSpots (edict_t *self)
{
	edict_t	*spot;
	vec3_t	d;

	spot = NULL;

	while(1)
	{
		spot = G_Find(spot, FOFS(classname), "info_player_start");
		if (!spot)
			return;
		if (!spot->targetname)
			continue;
		VectorSubtract(self->s.origin, spot->s.origin, d);
		if (VectorLength(d) < 384)
		{
			if ((!self->targetname) || stricmp(self->targetname, spot->targetname) != 0)
			{
				gi.dprintf("FixCoopSpots changed %s at %s targetname from %s to %s\n", 
					self->classname, vtos(self->s.origin), self->targetname, spot->targetname);
				self->targetname = spot->targetname;
			}
			return;
		}
	}
}

// now if that one wasn't ugly enough for you then try this one on for size
// some maps don't have any coop spots at all, so we need to create them
// where they should have been

static void SP_CreateCoopSpots (edict_t *self)
{
	edict_t	*spot;

	if(stricmp(level.mapname, "security") == 0)
	{
		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 - 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 64;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		spot = G_Spawn();
		spot->classname = "info_player_coop";
		spot->s.origin[0] = 188 + 128;
		spot->s.origin[1] = -164;
		spot->s.origin[2] = 80;
		spot->targetname = "jail3";
		spot->s.angles[1] = 90;

		return;
	}
}

void SP_misc_teleporter_dest (edict_t *ent);

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if (!coop->value)
		return;
	if(stricmp(level.mapname, "security") == 0)
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_CreateCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/

void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	SP_misc_teleporter_dest (self);


}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/
void SP_info_player_coop(edict_t *self)
{
	if (!coop->value)
	{
		G_FreeEdict (self);
		return;
	}

	if((stricmp(level.mapname, "jail2") == 0)   ||
		(stricmp(level.mapname, "jail4") == 0)   ||
		(stricmp(level.mapname, "mine1") == 0)   ||
		(stricmp(level.mapname, "mine2") == 0)   ||
		(stricmp(level.mapname, "mine3") == 0)   ||
		(stricmp(level.mapname, "mine4") == 0)   ||
		(stricmp(level.mapname, "lab") == 0)     ||
		(stricmp(level.mapname, "boss1") == 0)   ||
		(stricmp(level.mapname, "fact3") == 0)   ||
		(stricmp(level.mapname, "biggun") == 0)  ||
		(stricmp(level.mapname, "space") == 0)   ||
		(stricmp(level.mapname, "command") == 0) ||
		(stricmp(level.mapname, "power2") == 0) ||
		(stricmp(level.mapname, "strike") == 0))
	{
		// invoke one of our gross, ugly, disgusting hacks
		self->think = SP_FixCoopSpots;
		self->nextthink = level.time + FRAMETIME;
	}
}


/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(edict_t *ent)
{
}


//=======================================================================


void player_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}


qboolean IsFemale (edict_t *ent)
{
	char		*info;
	// Make sure ent exists!
	if (!G_EntExists(ent)) return false;


	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "skin");
	if (info[0] == 'f' || info[0] == 'F')
		return true;
	return false;
}

qboolean IsNeutral (edict_t *ent)
{
	char    *info;
	// Make sure ent exists!
	if (!G_EntExists(ent)) return false;


	if (!ent->client)
		return false;

	info = Info_ValueForKey (ent->client->pers.userinfo, "gender");
	if (info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	return false;
}

void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int mod, i;
	char *message[16];
	char *message2[16];
	qboolean ff;
	ff = meansOfDeath & MOD_FRIENDLY_FIRE;
	mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;

	// If no victim then no obit!
	if (!G_EntExists(self))
		return;

	if (attacker->client != self->client && attacker->client != NULL) 
		StatsLog("OBIT: %s\\%s\\%d\\%d\\%d\\%d\\%.1f\n", //a player fragged another player
		self->client->pers.netname, //victim
		attacker->client->pers.netname, //attacker
		self->client->ping, 
		attacker->client->ping, 
		mod, //means of death
		/*hitLocation*/NULL,
		level.time); 
	else 
		StatsLog("KILL: %s\\%d\\%d\\%d\\%.1f\n",	// a player killed himself
		self->client->pers.netname, 
		self->client->ping, 
		mod, 
		/*hitLocation*/NULL,
		level.time); 

	for (i = 0; i < 16; i++)
	{
		message[i] = NULL;
		message2[i] = "";
	} //end for

	switch (mod)
	{
	case MOD_SUICIDE:
		message[0] = "commits suicide";
		message[1] = "takes the easy way out";
		if (IsNeutral(self))
		{
			message[2] = "has fragged itself";
			message[3] = "took it's own life";
		}
		else if (IsFemale(self))
		{
			message[2] = "has fragged herself";
			message[3] = "took her own life";
		} //end if
		else
		{
			message[2] = "has fragged himself";
			message[3] = "took his own life";
		} //end else
		message[4] = "can be scraped off the pavement";
		break;
	case MOD_FALLING:
		message[0] = "cratered";
		message[1] = "discovers the effects of gravity";
		break;
	case MOD_CRUSH:
		message[0] = "was squished";
		message[1] = "was squeezed like a ripe grape";
		message[2] = "turned to juice";
		break;
	case MOD_WATER:
		message[0] = "sank like a rock";
		message[1] = "tried unsuccesfully to breathe water";
		message[2] = "tried to imitate a fish";
		message[3] = "must learn when to breathe";
		message[5] = "needs to learn how to swim";
		message[6] = "took a long walk off a short pier";
		message[7] = "might want to use a rebreather next time";
		if (IsNeutral(self))
		{
			message[4] = "thought it didn't need a rebreather";
		} //end if
		else if (IsFemale(self))
		{
			message[4] = "thought she didn't need a rebreather";
		} //end if
		else
		{
			message[4] = "thought he didn't need a rebreather";
		} //end else
		break;
	case MOD_SLIME:
		message[0] = "melted";
		message[1] = "was dissolved";
		message[2] = "sucked slime";
		message[3] = "found an alternative way to die";
		message[4] = "needs more slime-resistance";
		message[5] = "might try on an environmental suit next time";
		break;
	case MOD_LAVA:
		message[0] = "does a back flip into the lava";
		message[1] = "was fried to a crisp";
		message[2] = "thought that lava was water";
		message[3] = "turned into a real hothead";
		message[4] = "thought lava was 'funny water'";
		message[5] = "tried to hide in the lava";
		if (IsNeutral(self))
		{
			message[6] = "thought it was fire resistant";
			message[7] = "tried to emulate the demigod";
			message[8] = "needs to rebind it's 'strafe' keys";
		} //end if
		else if (IsFemale(self))
		{
			message[6] = "thought she was fire resistant";
			message[7] = "tried to emulate the goddess Pele";
			message[8] = "needs to rebind her 'strafe' keys";
		} //end if
		else
		{
			message[6] = "thought he was fire resistant";
			message[7] = "tried to emulate the god of hell-fire";
			message[8] = "needs to rebind his 'strafe' keys";
		} //end else
		break;
	case MOD_EXPLOSIVE:
	case MOD_BARREL:
		message[0] = "blew up";
		break;
	case MOD_EXIT:
		message[0] = "found a way out";
		message[1] = "had enough for today";
		message[2] = "exit, stage left";
		message[3] = "has returned to real life(tm)";
		break;
	case MOD_TARGET_LASER:
		message[0] = "saw the light";
		break;
	case MOD_TARGET_BLASTER:
		message[0] = "got blasted";
		break;
	case MOD_BOMB:
	case MOD_SPLASH:
	case MOD_TRIGGER_HURT:
		message[0] = "was in the wrong place";
		message[1] = "got lost";
		message[2] = "shouldn't have been there";
		break;
	}
	if (attacker == self)
	{
		switch (mod)
		{
		case MOD_HELD_GRENADE:
			message[0] = "tried to put the pin back in";
			message[2] = "got the red and blue wires mixed up";
			if (IsNeutral(self))
			{
				message[1] = "held it's grenade too long";
				message[3] = "tried to disassemble it's own grenade";
			} //end if
			else if (IsFemale(self))
			{
				message[1] = "held her grenade too long";
				message[3] = "tried to disassemble her own grenade";
			} //end if
			else
			{
				message[1] = "held his grenade too long";
				message[3] = "tried to disassemble his own grenade";
			} //end else
			break;
		case MOD_HG_SPLASH:
			//T-MEK
			/*				if (IsFemale(self))
			{
			message[0] = "tripped on her own lasermine";
			message[1] = "didn't know that tripwire was hers";
			message[2] = "needs to watch her step";
			}
			else if (IsNeutral(self))
			{
			message[0] = "tripped on its own lasermine";
			message[1] = "didn't know that tripwire was its own";
			message[2] = "needs to watch its step";
			}
			else
			{
			message[0] = "tripped on his own lasermine";
			message[1] = "didn't know that tripwire was his";
			message[2] = "needs to watch his step";
			}
			break;
			//T-MEK
			*/
			if (IsNeutral(self))
			{
				message[0] = "tripped on it's own grenade";
				message[1] = "stepped on it's own pineapple";
			}
			else if (IsFemale(self))
			{
				message[0] = "tripped on her own grenade";
				message[1] = "stepped on her own pineapple";
			} //end if
			else
			{
				message[0] = "tripped on his own grenade";
				message[1] = "stepped on his own pineapple";
			} //end else
			break;

		case MOD_G_SPLASH:
			message[2] = "tried to grenade-jump unsuccessfully";
			message[3] = "tried to play football with a grenade";
			message[4] = "shouldn't mess around with explosives";
			if (IsNeutral(self))
			{
				message[0] = "tripped on it's own grenade";
				message[1] = "stepped on it's own pineapple";
			}
			else if (IsFemale(self))
			{
				message[0] = "tripped on her own grenade";
				message[1] = "stepped on her own pineapple";
			} //end if
			else
			{
				message[0] = "tripped on his own grenade";
				message[1] = "stepped on his own pineapple";
			} //end else
			break;
		case MOD_R_SPLASH:
			message[2] = "had the rocket launcher backwards";
			message[4] = "thought up a novel new way to fly";
			if (IsNeutral(self))
			{
				message[0] = "blew itself up";
				message[1] = "thought it was Werner von Braun";
				message[3] = "thought it had more health";
				message[5] = "found it's own rocketlauncher's trigger";
				message[6] = "thought it had more armor on";
				message[7] = "blew itself to kingdom come";
			} //end if
			else if (IsFemale(self))
			{
				message[0] = "blew herself up";
				message[1] = "thought she was Werner von Braun";
				message[3] = "thought she had more health";
				message[5] = "found her own rocketlauncher's trigger";
				message[6] = "thought she had more armor on";
				message[7] = "blew herself to kingdom come";
			} //end if	
			else
			{
				message[0] = "blew himself up";
				message[1] = "thought he was Werner von Braun";
				message[3] = "thought he had more health";
				message[5] = "found his own rocketlauncher's trigger";
				message[6] = "thought he had more armor on";
				message[7] = "blew himself to kingdom come";
			} //end else
			break;
		case MOD_BFG_BLAST:
			message[0] = "should have used a smaller gun";
			message[1] = "shouldn't play with big guns";
			message[2] = "doesn't know how to work the BFG";
			message[3] = "has trouble using big guns";
			message[4] = "can't distinguish which end is which with the BFG";
			message[5] = "should try to avoid using the BFG near obstacles";
			message[6] = "tried to BFG-jump unsuccesfully";
			break;
		default:
			DbgPrintf("%s %s has MOD %d %f %f %f\n", __func__, self->client->pers.netname, mod,
				self->s.origin[0], self->s.origin[1], self->s.origin[2]);
			message[1] = "commited suicide";
			message[2] = "went the way of the dodo";
			message[3] = "thought 'kill' was a funny console command";
			message[4] = "wanted one frag less";
			if (IsNeutral(self))
			{
				message[0] = "killed itself";
				message[5] = "thought it had one too many frags";
			} //end if
			else if (IsFemale(self))
			{
				message[0] = "killed herself";
				message[5] = "thought she had one too many frags";
			} //end if
			else
			{
				message[0] = "killed himself";
				message[5] = "thought he had one too many frags";
			} //end else
			break;
		} //end switch
	} //end if
	if (message[0])
	{
		for (i = 0; i < 16; i++)
		{
			if (!message[i]) break;
		} //end for
		i = random() * (float) i;
		safe_bprintf (PRINT_MEDIUM, "%s %s.\n", self->client->pers.netname, message[i]);
		if(match_state == STATE_PLAYING)
		{
			//JSW
			if (((int)punish_suicide->value & PS_RESETSCORE) && self->client->kill == 1)
			{
				safe_cprintf(self, PRINT_HIGH, "Your score has been reset to 0 due to intentional suicide\n");
				self->client->resp.score = 0;
				self->client->resp.frags = 0;
			}
			else
			{//end
				self->client->resp.score--;
				//			self->client->resp.frags--;
			}
			self->client->resp.suicide++;
			self->client->resp.spree = 0;
		}
		self->enemy = NULL;
		return;
	} //end if

	self->enemy = attacker;
	if (attacker && attacker->client)
	{
		if (mod == MOD_BLASTER && extrasounds->value)
			gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/humiliation.wav"), 1, ATTN_NORM, 0);
		switch (mod)
		{
		case MOD_BLASTER:
			message[0] = "(quakeweenie) was massacred by";
			message2[0] = " (quakegod)!!!";
			message[1] = "was killed with the wimpy blaster by";
			message[2] = "died a wimp's death by";
			message[3] = "can't even avoid a blaster from";
			message[4] = "was blasted by";
			break;
		case MOD_SHOTGUN:
			message[1] = "was gunned down by";
			if (IsNeutral(self))
			{
				message[0] = "found itself on the wrong end of";
				message2[0] = "'s gun";
			} //end if
			else if (IsFemale(self))
			{
				message[0] = "found herself on the wrong end of";
				message2[0] = "'s gun";
			} //end if
			else
			{
				message[0] = "found himself on the wrong end of";
				message2[0] = "'s gun";
			} //end else
			break;
		case MOD_SSHOTGUN:
			message[0] = "was blown away by";
			message2[0] = "'s super shotgun";
			if (IsNeutral(self))
			{
				message[0] = "had it ears cleaned out by";
				message2[0] = "'s super shotgun";
			} //end if
			else if (IsFemale(self))
			{
				message[0] = "had her ears cleaned out by";
				message2[0] = "'s super shotgun";
			} //end if
			else
			{
				message[0] = "had his ears cleaned out by";
				message2[0] = "'s super shotgun";
			} //end else
			message[3] = "was put full of buckshot by";
			break;
		case MOD_MACHINEGUN:
			message[0] = "was machinegunned by";
			message[1] = "was filled with lead by";
			message[2] = "was put full of lead by";
			message[3] = "was pumped full of lead by";
			message[4] = "ate lead dished out by";
			message[5] = "eats lead from";
			message[6] = "bites the bullet from";
			break;
		case MOD_CHAINGUN:
			message[0] = "was cut in half by";
			message2[0] = "'s chaingun";
			message[2] = "was turned into a strainer by";
			message[3] = "was put full of holes by";
			message[4] = "couldn't avoid death by painless from";
			if (IsNeutral(self))
			{
				message[1] = "was put so full of lead by";
				message2[1] = " you can call it a pencil";
			} //end if
			else if (IsFemale(self))
			{
				message[1] = "was put so full of lead by";
				message2[1] = " you can call her a pencil";
			} //end if
			else
			{
				message[1] = "was put so full of lead by";
				message2[1] = " you can call him a pencil";
			} //end else
			break;
		case MOD_GRENADE:
			message[0] = "was popped by";
			message2[0] = "'s grenade";
			message[1] = "caught";
			message2[1] = "'s grenade in the head";
			message[2] = "tried to headbutt the grenade of";
			break;
		case MOD_G_SPLASH:
			message[0] = "was shredded by";
			message2[0] = "'s shrapnel";
			break;
		case MOD_ROCKET:
			message[0] = "ate";
			message2[0] = "'s rocket";
			message[1] = "sucked on";
			message2[1] = "'s boomstick";
			message[2] = "tried to play 'dodge the missile' with";
			message[3] = "tried the 'patriot move' on the rocket from";
			message[4] = "had a rocket stuffed down the throat by";
			message[5] = "got a rocket up the tailpipe by";
			message[6] = "tried to headbutt";
			message2[6] = "'s rocket";
			break;
		case MOD_R_SPLASH:
			message[0] = "almost dodged";
			message2[0] = "'s rocket";
			message[1] = "was spread around the place by";
			message[2] = "was gibbed by";
			message[3] = "has been blown to smithereens by";
			message[4] = "was blown to itsie bitsie tiny pieces by";
			break;
		case MOD_HYPERBLASTER:
			message[0] = "was melted by";
			message2[0] = "'s hyperblaster";
			message[1] = "was used by";
			message2[1] = " for target practice";
			message[2] = "was hyperblasted by";
			message[3] = "was pumped full of bolts by";
			message[4] = "couldn't outrun the hyperblaster from";
			break;
		case MOD_RAILGUN:
			message[0] = "was railed by";
			message[2] = "played 'catch the slug' with";
			message[4] = "bites the slug from";
			message[5] = "caught the slug from";
			if (IsNeutral(self))
			{
				message[1] = "got a slug put through it by";
				message[3] = "was corkscrewed through it's head by";
				message[6] = "had it's body pierced with a slug from";
				message[7] = "had it's brains blown out by";
			} //end if
			else if (IsFemale(self))
			{
				message[1] = "got a slug put through her by";
				message[3] = "was corkscrewed through her head by";
				message[6] = "had her body pierced with a slug from";
				message[7] = "had her brains blown out by";
			} //end if
			else
			{
				message[1] = "got a slug put through him by";
				message[3] = "was corkscrewed through his head by";
				message[6] = "had his body pierced with a slug from";
				message[7] = "had his brains blown out by";
			} //end else
			break;
		case MOD_BFG_LASER:
			message[0] = "saw the pretty lights from";
			message2[0] = "'s BFG";
			message[1] = "was diced by the BFG from";
			break;
		case MOD_BFG_BLAST:
			message[0] = "was disintegrated by";
			message2[0] = "'s BFG blast";
			message[1] = "was flatched with the green light by";
			message2[1] = "";
			break;
		case MOD_BFG_EFFECT:
			message[0] = "couldn't hide from";
			message2[0] = "'s BFG";
			message[1] = "tried to soak up green energy from";
			message2[1] = "'s BFG";
			message[2] = "was energized with 50 cells by";
			message[3] = "doesn't know when to run from";
			message[4] = "'saw the light' from";
			break;
		case MOD_HANDGRENADE:
			message[0] = "caught";
			message2[0] = "'s handgrenade";
			message[1] = "should watch more carefully for handgrenades from";
			message[2] = "caught";
			message2[2] = "'s handgrenade in the head";
			message[3] = "tried to headbutt the handgrenade of";
			break;
		case MOD_HG_SPLASH:
			message[0] = "didn't see";
			message2[0] = "'s handgrenade";
			break;
		case MOD_HELD_GRENADE:
			message[0] = "feels";
			message2[0] = "'s pain";
			break;
		case MOD_TELEFRAG:
			message[0] = "tried to invade";
			message2[0] = "'s personal space";
			message[1] = "is less telefrag aware than";
			message[2] = "should appreciate scotty more like";
			break;
		case MOD_GRAPPLE:
			message[0] = "was caught by";
			message2[0] = "'s grapple";
			break;
		}

		if (message[0])
		{
			for (i = 0; i < 16; i++)
			{
				if (!message[i]) 
					break;
			}

			i = (random()-0.01) * (float) i;
			safe_bprintf (PRINT_MEDIUM,"%s %s %s%s\n", self->client->pers.netname, message[i], attacker->client->pers.netname, message2[i]);
			//BOTS
			if(bot_insult->value)
			{
				TauntVictim(attacker,self);
				InsultVictim(attacker,self);
			}
			if (ff)
			{
				safe_bprintf (PRINT_CHAT, 
					"%s Killed His own TeamMate!! %s and LOST A FRAG!!\n", 
					attacker->client->pers.netname, 
					self->client->pers.netname);
				if(match_state == STATE_PLAYING)
				{
					attacker->client->resp.frags++;
					attacker->client->resp.score--;
					attacker->client->resp.spree++;
					if (attacker->client->resp.spree > attacker->client->resp.bigspree)
						attacker->client->resp.bigspree = attacker->client->resp.spree;
				}
			}
			else
				if(match_state == STATE_PLAYING)
				{
					attacker->client->resp.score++;
					attacker->client->resp.frags++;
					attacker->client->resp.spree++;
					if (attacker->client->resp.spree > attacker->client->resp.bigspree)
						attacker->client->resp.bigspree = attacker->client->resp.spree;
					if (!attacker->bot_client && extrasounds->value)
					{
						switch (attacker->client->resp.spree)
						{
						case 5:
							if (rand() & 1)
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/dominatingf.wav"), 1, ATTN_NORM, 0);
							else
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/dominating.wav"), 1, ATTN_NORM, 0);
							safe_bprintf(PRINT_HIGH, "%s is beginning a killing streak!\n", attacker->client->pers.netname);
							break;
						case 10:
							if (rand() & 1)
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/killingspree.wav"), 1, ATTN_NORM, 0);
							else
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/killingspreef.wav"), 1, ATTN_NORM, 0);
							safe_bprintf(PRINT_HIGH, "%s is on a killing streak!\n", attacker->client->pers.netname);
							break;
						case 15:
							if (rand() & 1)
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/rampage.wav"), 1, ATTN_NORM, 0);
							else
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/rampagef.wav"), 1, ATTN_NORM, 0);
							safe_bprintf(PRINT_HIGH, "%s is on a rampage!\n", attacker->client->pers.netname);
							break;
						case 20:
							if (rand() & 1)
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/godlike.wav"), 1, ATTN_NORM, 0);
							else
								gi.sound (attacker, CHAN_AUTO, gi.soundindex ("misc/godlikef.wav"), 1, ATTN_NORM, 0);
							safe_bprintf(PRINT_HIGH, "%s is GOD LIKE!\n", attacker->client->pers.netname);
							break;
						default:
							break;
						}
					}
				}
				return;
		}
	}

	safe_bprintf (PRINT_MEDIUM,"%s died.\n", self->client->pers.netname);
	if(match_state == STATE_PLAYING)
	{
		self->client->resp.score--;
		self->client->resp.spree = 0;
		//self->client->resp.frags--;
	}
}


void QuadTimeout (edict_t *ent)
{
	edict_t	*e;
	int	i;

	//gi.dprintf("A quad damage has expired!\n");
	for (i = 1; i <= maxclients->value; i++)
	{
		if ((e = &g_edicts[i]) && e && e->inuse && !e->bot_client)
		{
			//safe_centerprintf(e, "A quad damage has expired!\n");
			gi.sound (e, CHAN_AUTO, gi.soundindex ("items/quadexp.wav"), 1, ATTN_NONE, 0);
		}
	}
	G_FreeEdict(ent);
}

void TossClientWeapon (edict_t *self)
{
	gitem_t		*item;
	edict_t		*drop;
	qboolean	quad;
	float		spread;

	// Make sure ent exists!
	if (!G_EntExists(self)) return;

	if (!deathmatch->value)
		return;

	item = self->client->pers.weapon;
	if (! self->client->pers.inventory[self->client->ammo_index] )
		item = NULL;
	if (item && (strcmp (item->pickup_name, "Blaster") == 0))
		item = NULL;

	//dont try to loose the grapple
	if (item && (strcmp (item->pickup_name, "Grapple") == 0))
		item = NULL;

	if (!((int)(dmflags->value) & DF_QUAD_DROP))
		quad = false;
	else
		quad = (self->client->quad_framenum > (level.framenum + 10));

	if (item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if (item)
	{
		self->client->v_angle[YAW] -= spread;
		drop = Drop_Item (self, item);
		self->client->v_angle[YAW] += spread;
		drop->spawnflags = DROPPED_PLAYER_ITEM;
	}

	if (quad)
	{
		self->client->v_angle[YAW] += spread;
		drop = Drop_Item (self, FindItemByClassname ("item_quad"));
		self->client->v_angle[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->client->quad_framenum - level.framenum) * FRAMETIME;
		if ((int)quad_notify->value & QUAD_NOTIFY_EXPIRE)
			drop->think = QuadTimeout;
		else
			drop->think = G_FreeEdict;
	}
}

/*
==================
LookAtKiller
==================
*/
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	vec3_t		dir;

	// Make sure ent exists!
	if (!G_EntExists(self)) 
		return;

	if (attacker && attacker != world && attacker != self)
	{
		VectorSubtract (attacker->s.origin, self->s.origin, dir);
	}
	else if (inflictor && inflictor != world && inflictor != self)
	{
		VectorSubtract (inflictor->s.origin, self->s.origin, dir);
	}
	else
	{
		self->client->killer_yaw = self->s.angles[YAW];
		return;
	}

	self->client->killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
}

// Fast die, no gibs, no death frames.
static void player_die_fast (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	// if no-one died, then exit..
	if (!G_EntExists(self))
		return;

	if(ctf->value)
	{
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
	}

	// drop the rune if we have one
	runes_drop(self);

	if ( self->flashlight )
	{	
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
	}

	VectorClear (self->avelocity);

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	self->s.modelindex3 = 0;	// remove linked ctf flag

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	self->s.sound = 0;
	self->client->weapon_sound = 0;
	self->maxs[2] = -8;
	self->svflags |= SVF_DEADMONSTER;

	self->deadflag = DEAD_DEAD;
	gi.linkentity (self);

	self->client->pers.pl_state = PL_PLAYING;
}

/*
==================
player_die
==================
*/
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	// if no-one died, then exit..
	if (!G_EntExists(self)) 
		return;

	VectorClear (self->avelocity);

	// drop the rune if we have one
	runes_drop(self);

	if ( self->flashlight )
	{	
		G_FreeEdict(self->flashlight);
		self->flashlight = NULL;
	}

	self->takedamage = DAMAGE_YES;
	self->movetype = MOVETYPE_TOSS;

	self->s.modelindex2 = 0;	// remove linked weapon model
	//ZOID
	self->s.modelindex3 = 0;	// remove linked ctf flag
	//ZOID

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;

	self->s.sound = 0;
	self->client->weapon_sound = 0;

	self->maxs[2] = -8;

	//	self->solid = SOLID_NOT;
	self->svflags |= SVF_DEADMONSTER;

	if (!self->deadflag)
	{
		self->client->respawn_time = level.time + 1.0;
		LookAtKiller (self, inflictor, attacker);
		self->client->ps.pmove.pm_type = PM_DEAD;
		ClientObituary (self, inflictor, attacker);
		//ZOID
		if (!OnSameTeam(self, attacker)) //JSW
			CTFFragBonuses(self, inflictor, attacker);

		//ZOID

		if(voosh->value == 0)
			TossClientWeapon (self);

		if(match_state == STATE_PLAYING)
			self->client->resp.deaths++;

		self->client->resp.spree = 0;
		//		

		//ZOID
		CTFPlayerResetGrapple(self);
		CTFDeadDropFlag(self);
		CTFDeadDropTech(self);
		//ZOID

		if (deathmatch->value && ctf_deathscores->value && !self->bot_client)
			Cmd_Help_f (self);		// show scores
	}

	// remove powerups
	self->client->quad_framenum = 0;
	self->client->invincible_framenum = 0;
	self->client->breather_framenum = 0;
	self->client->enviro_framenum = 0;

	// clear inventory
	memset(self->client->pers.inventory, 0, sizeof(self->client->pers.inventory));

	// minimal gibs if railserver
	if (self->health < -40 || voosh->value)
	{	// gib
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n = 0; n < 3; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);

		// An empty name here caused GAME WARNING in SV_FindIndex
		// so throw another gib model instead.
		//gi.setmodel(self, "models/objects/gibs/sm_meat/tris.md2");
		gi.setmodel(self, "");
		self->takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if (!self->deadflag)
		{
			static int i;

			i = (i + 1) % 3;
			// start a death animation
			self->client->anim_priority = ANIM_DEATH;
			if (self->client->ps.pmove.pm_flags & PMF_DUCKED)
			{
				self->s.frame = FRAME_crdeath1-1;
				self->client->anim_end = FRAME_crdeath5;
			}
			else switch (i)
			{
			case 0:
				self->s.frame = FRAME_death101-1;
				self->client->anim_end = FRAME_death106;
				break;
			case 1:
				self->s.frame = FRAME_death201-1;
				self->client->anim_end = FRAME_death206;
				break;
			case 2:
				self->s.frame = FRAME_death301-1;
				self->client->anim_end = FRAME_death308;
				break;
			}
			gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}
	self->deadflag = DEAD_DEAD;

	//routing last index move
	if(chedit->value && self == &g_edicts[1]) 
		Move_LastRouteIndex();

	gi.linkentity (self);
}

//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	gitem_t		*item;

	int plstate, mapvoted, oplevel;
	char namepass[16];
	//
	qboolean motd,ig;
	int	motd_seen;
	char userip[24], namecg[18], skinn[24];

	// Save certain current player state variables
	oplevel = client->pers.oplevel;
	strcpy(namepass, client->pers.namepass);
	plstate = client->pers.pl_state;
	motd = client->pers.motd;
	motd_seen = client->pers.motd_seen;
	ig = client->pers.in_game;
	strcpy (userip,client->pers.ip);
	strcpy (namecg,client->pers.name_change);
	strcpy (skinn,client->pers.skin_change);
	mapvoted = client->pers.vote_times;

	// Zero this region
	memset (&client->pers, 0, sizeof(client->pers));

	// Restore the saved state
	client->pers.pl_state = plstate;
	client->pers.oplevel = oplevel;
	strcpy(client->pers.namepass, namepass);
	client->pers.motd = motd;
	client->pers.motd_seen = motd_seen;
	client->pers.in_game = ig;
	strcpy (client->pers.ip, userip);
	strcpy (client->pers.name_change, namecg);
	strcpy (client->pers.skin_change, skinn);
	client->pers.vote_times = mapvoted;

	if (voosh->value)
	{
		item = FindItem("Slugs");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1000;

		item = FindItem("Railgun");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
	}
	else
	{
		item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 2;

		/********************************************
		start  ammo
		********************************************/
		if (sa_shells->value)
		{
			item = FindItem("Shells");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = sa_shells->value;

		}


		if (sa_bullets->value)
		{
			item = FindItem("Bullets");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = sa_bullets->value;

		}

		if (sa_grenades->value)
		{
			item = FindItem("Grenades");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = sa_grenades->value;

		}

		if (sa_rockets->value)
		{
			item = FindItem("Rockets");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = sa_rockets->value;

		}

		if (sa_cells->value)
		{
			item = FindItem("Cells");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = sa_cells->value;
		}

		if (sa_slugs->value)
		{
			item = FindItem("Slugs");
			client->pers.selected_item = ITEM_INDEX(item);
			client->pers.inventory[client->pers.selected_item] = sa_slugs->value;

		}
	}
	client->pers.weapon = item;
	client->pers.lastweapon = item;

	item = FindItem("Grapple");

	if (use_grapple->value)
	{
		client->pers.inventory[ITEM_INDEX(item)] = 1;
	}
	else
	{
		client->pers.inventory[ITEM_INDEX(item)] = 0;
	}

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.max_bullets	= 200;
	client->pers.max_shells		= 100;
	client->pers.max_rockets	= 50;
	client->pers.max_grenades	= 50;
	client->pers.max_cells		= 200;
	client->pers.max_slugs		= 50;

	client->pers.connected = true;
}

void InitClientResp (gclient_t *client)
{
	int ctf_team = client->resp.ctf_team;
	int iddst = client->resp.iddist;

	memset (&client->resp, 0, sizeof(client->resp));

	if(clear_teams->value)
	{
		client->resp.ctf_team = CTF_NOTEAM;
		client->pers.pl_state = PL_SPECTATOR;
	}
	else
		client->resp.ctf_team = ctf_team;

	client->resp.iddist = iddst;
	client->resp.enterframe = level.framenum;
	client->resp.startframe = level.newframenum;
	client->resp.coop_respawn = client->pers;

	if (ctf->value && (client->resp.ctf_team < CTF_TEAM1))
		CTFAssignTeam(client);
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.powerArmorActive = (ent->flags & FL_POWER_ARMOR);
		if (coop->value)
			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	// Make sure ent exists!
	if (!G_EntExists(ent)) 
		return;

	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	if (ent->client->pers.powerArmorActive)
		ent->flags |= FL_POWER_ARMOR;
	if (coop->value)
		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

SelectSpawnPoint

=======================================================================
*/
/*RAV
================
PlayersRangeFromSpotRAV

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpotRAV (edict_t *spot, edict_t *ent)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 0; n < maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse|| !player->client || player == ent ||
			player->client->pers.pl_state != PL_PLAYING)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}
/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 0; n < maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t  *spot, *spot1, *spot2;
	int    count = 0;
	int    selection;
	float  range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
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
		return NULL;

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
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);
	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t  *bestspot;
	float  bestdistance, bestplayerdistance;
	edict_t  *spot;

	if(debug_spawn->value)
		DbgPrintf("%s\n", __FUNCTION__);

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;

		}
	}

	if (bestspot)
	{
		if(debug_spawn->value)
			DbgPrintf("Bestspot %s returning %s \nat %f %f %f\n", __func__, bestspot->classname,
			bestspot->s.origin[0], bestspot->s.origin[1], bestspot->s.origin[2]); 
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown

	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	if(debug_spawn->value)
		DbgPrintf("Telefrag %s returning %s \nat %f %f %f\n", __func__, spot->classname,
		spot->s.origin[0], spot->s.origin[1], spot->s.origin[2]); 

	return spot;
}

/*
================
SpawnNearFlag
JSW
================
*/
edict_t *SpawnNearFlag (edict_t *ent)
{
	edict_t  *bestspot;
	float  bestdistance, bestplayerdistance;
	edict_t  *spot;
	char	*teamspawn;

	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	if (ent->client->resp.ctf_team == CTF_TEAM1)
		teamspawn = "info_player_team1";
	else if (ent->client->resp.ctf_team == CTF_TEAM2)
		teamspawn = "info_player_team2";
	else
		teamspawn = "info_player_deathmatch";
	while ((spot = G_Find (spot, FOFS(classname), teamspawn)) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);
		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}
	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), teamspawn);

	if(debug_spawn->value)	
		DbgPrintf("8888 %s returning %s for %s\nat %f %f %f\n", __func__, 
		spot->classname, ent->client->pers.netname,
		spot->s.origin[0], spot->s.origin[1], spot->s.origin[2]); 

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	edict_t *spot;

	if(debug_spawn->value)
		DbgPrintf("%s\n", __FUNCTION__);

	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
	{
		spot = SelectFarthestDeathmatchSpawnPoint ();
		return spot;
	}
	else
	{
		spot = SelectRandomDeathmatchSpawnPoint ();
		return spot;
	}
}


edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}
	return spot;
}

/*
===========
SelectSpawnPointRAV

Chooses a player start, deathmatch start, coop start, etc
============
*/
//void 
qboolean SelectSpawnPointRAV (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t  *spot = NULL;

	if(debug_spawn->value)
		DbgPrintf("%s\n", __FUNCTION__);

	//ZOID
	if (ctf->value && ent->client->kill) //JSW
		spot = SpawnNearFlag(ent);
	else if (!ctf->value)
		spot = SelectDeathmatchSpawnPoint ();
	else
		spot = SelectCTFSpawnPoint(ent);
	//ZOID



	// find a single player start spot
	if(!spot) 
	{
		//warning!!!
		gi.dprintf("Warning: failed to find deathmatch spawn point\n");

		while((spot = G_Find(spot, FOFS(classname), "info_player_start")) != NULL) {
			if(!game.spawnpoint[0] && !spot->targetname)
				break;

			if(!game.spawnpoint[0] || !spot->targetname)
				continue;

			if(!Q_stricmp(game.spawnpoint, spot->targetname))
				break;
		}

		if(!spot) 
		{
			if(!game.spawnpoint[0]) {	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find(spot, FOFS(classname), "info_player_start");
			}

			if(!spot)
				gi.error("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}


	//*******************************************//
	//return false if  we are playing and someone is standing on a spawn pad.

	if(PlayersRangeFromSpotRAV(spot, ent) < 60.0
		&& ent->client->pers.pl_state == PL_PLAYING)
		return(false);
	//*********************************************//
	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;

	VectorCopy (spot->s.angles, angles);

	return(true);
}
/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if(debug_spawn->value)
		DbgPrintf("%s %s\n", __FUNCTION__, ent->client->pers.netname);

	if (deathmatch->value)
		//ZOID
		if (ctf->value)
			spot = SelectCTFSpawnPoint(ent);
		else
			//ZOID
			spot = SelectDeathmatchSpawnPoint ();
	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);

	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}

//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void Body_droptofloor(edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-24);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	ent->s.origin[2] += 32;

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);

	VectorCopy(tr.endpos, ent->s.origin);

	gi.linkentity(ent);

	if (tr.ent)
		ent->nextthink = level.time + 0.1;
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int	n;

	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		self->s.origin[2] -= 48;
		ThrowClientHead (self, damage);
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);
	gi.unlinkentity (body);
	return;

	//	body->s = ent->s;
	//	body->s.number = body - g_edicts;
	//
	//	body->svflags = ent->svflags;
	//	VectorCopy (ent->mins, body->mins);
	//	VectorCopy (ent->maxs, body->maxs);
	//	VectorCopy (ent->absmin, body->absmin);
	//	VectorCopy (ent->absmax, body->absmax);
	//	VectorCopy (ent->size, body->size);
	//	body->solid = ent->solid;
	//	body->clipmask = ent->clipmask;
	//	body->owner = ent->owner;
	//	body->movetype = ent->movetype;
	//
	//	body->die = body_die;
	//	body->takedamage = DAMAGE_YES;
	//
	//	gi.linkentity (body);

	//	body->nextthink = level.time + 0.2;
	//	body->think = Body_droptofloor;
}


void respawn (edict_t *self, qboolean spawn)
{
	if(!spawn)
		CopyToBodyQue (self);
	if(self->bot_client)
		PutBotInServer(self);
	else
		PutClientInServer (self);
	// add a teleportation effect
	self->s.event = EV_PLAYER_TELEPORT;
	// hold in place briefly
	self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
	self->client->ps.pmove.pm_time = 14;
	self->client->respawn_time = level.time;
	return;
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin;
	vec3_t	spawn_angles;
	gclient_t	*client;
	int		i;
	zgcl_t	zgcl;
	gitem_t	*item;
	gitem_t	*ammo;
	client_persistent_t	saved;
	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	//

	//	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	//********************************************************  
	//	else
	//no telefragging !
	if(!SelectSpawnPointRAV (ent, spawn_origin, spawn_angles))
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->clipmask = 0;
		ent->client->pers.pl_state = PL_NEEDSPAWN;//force to try again
		ent->client->ps.pmove.pm_type = PM_FREEZE;
		return;
	}
	//***********************************************************

	index = ent-g_edicts-1;
	client = ent->client;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memcpy (&zgcl, &client->zc, sizeof(zgcl_t));
	memset (client, 0, sizeof(*client));
	memcpy (&client->zc, &zgcl, sizeof(zgcl_t));

	client->pers = saved;
	client->resp = resp;

	if (client->pers.health <= 0)
		InitClientPersistant(client);

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];

	//RAV
	if(match_state < STATE_WARMUP)
		ent->takedamage = DAMAGE_NO;
	else
		ent->takedamage = DAMAGE_AIM;

	ent->movetype = MOVETYPE_WALK;
	ent->viewheight = 22;
	ent->inuse = true;
	ent->classname = "player";
	ent->mass = 200;
	ent->solid = SOLID_BBOX;
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->model = "players/male/tris.md2";
	ent->pain = player_pain;
	ent->die = player_die;

	if(ent->bot_client)
		ent->client->pers.pl_state = PL_PLAYING;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));
	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;
	//ZOID
	client->ps.pmove.pm_flags &= ~PMF_NO_PREDICTION;
	//ZOID

	//JSW	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	if (deathmatch->value && (dmflag & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}
	client->ps.gunindex = gi.modelindex(client->pers.weapon->view_model);
	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 255;		// will use the skin specified model
	//ent->s.modelindex2 = 255;		// custom gun model
	ShowGun(ent);

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;			// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	//ZOID
	if(ent->client->pers.motd == true)//RAV
	{
		ent->client->pers.pl_state = PL_SPECTATOR;
	}
	else
	{
		if (ent->client->pers.pl_state == 0 && ent->client->resp.ctf_team < 1 && !Check_for_SpecLimit(ent))
			return;
		if (CTFStartClient(ent) /*|| ent->client->resp.enterframe + 5 > level.framenum*/)
			if(!ent->bot_client)
				return;
	}
	//ZOID
	//RAV
	//  start weapons & respawn protection
	if ((resp_protect->value > 0) && (match_state > STATE_COUNTDOWN)
		&& (ent->client->pers.pl_state == PL_PLAYING))
	{
		client->respawn_framenum = level.framenum + resp_protect->value*10;
	}

	if ((int)(start_weapons->value) & 1)
	{
		item = FindItem("Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 2)
	{
		item = FindItem("Super Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 4)
	{
		item = FindItem("Machinegun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 8)
	{
		item = FindItem("Chaingun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 16)
	{
		item = FindItem("Grenade Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 32)
	{
		item = FindItem("Rocket Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 64)
	{
		item = FindItem("Hyperblaster");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 128)
	{
		item = FindItem("Railgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 256)
	{
		item = FindItem("BFG10K");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);

		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}

	// Start Items
	if ((int)(start_items->value) & 1)
	{
		edict_t *it_ent;

		item = FindItem("Body Armor");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 2)
	{
		edict_t *it_ent;

		item = FindItem("Combat Armor");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);

		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 4)
	{
		edict_t *it_ent;

		item = FindItem("Jacket Armor");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 8)
	{
		edict_t *it_ent;

		item = FindItem("Power Screen");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 16)
	{
		edict_t *it_ent;

		item = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 32)
	{
		edict_t *it_ent;

		item = FindItem("Quad Damage");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 64)
	{
		edict_t *it_ent;

		item = FindItem("Invulnerability");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 128)
	{
		edict_t *it_ent;

		item = FindItem("Silencer");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 256)
	{
		edict_t *it_ent;

		item = FindItem("Rebreather");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 512)
	{
		edict_t *it_ent;

		item = FindItem("Environment Suit");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 1024)
	{
		edict_t *it_ent;

		item = FindItem("Bandolier");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 2048)
	{
		edict_t *it_ent;

		item = FindItem("Ammo Pack");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 4096)
	{
		edict_t *it_ent;

		item = FindItem("Health");
		it_ent = G_Spawn();
		SP_item_health_mega (it_ent);
		it_ent->classname = item->classname;
		Touch_Item (it_ent, ent, NULL, NULL);
	}

	// End
	//

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	if (!ent->map)
		ent->map = G_CopyString(ent->client->pers.netname);

	// force the current weapon up
	client->newweapon = client->pers.weapon;
	ChangeWeapon (ent);

	ent->client->pers.db_id = true;
	ent->client->pers.db_hud = true;
	ent->client->hudtime = level.framenum + 3;
	ent->client->checkframe = level.framenum+40;
	ent->client->checkpingtime = level.time + 25;

	if (debug_spawn->value)
		DbgPrintf("%s %s entered time: %.1f\n", __func__, ent->client->pers.netname, level.time);

	//JSW
	ent->client->kill = 0;	//Clear kill
	//end

}

//RAV
/*****************SPECTATOR/LOGIN MODE*********/
void Connect (edict_t *ent)
{

	if (ent->bot_client)
		return;
	ent->client->newweapon = NULL;
	ChangeWeapon (ent);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->movetype = MOVETYPE_NOCLIP;
	ent->solid = SOLID_NOT;
	ent->svflags |= SVF_NOCLIENT;
	ent->client->ps.gunindex = 0;
	gi.linkentity (ent);
	ent->client->pers.db_hud = true;
	ent->client->pers.motd = true;
	ent->client->pers.in_game = false;
}
//end

// Store the message of the day in memory.
char *gMOTD = ((char *)-1); // initialized at startup as bad pointer
cvar_t *motdfile;

void ClientPrintMOTD (edict_t *ent)
{
	FILE *in;
	char motdPath[MAX_QPATH + 1];
	int c;
	int motdBytes;
	char *here;
	char *s1 = MOD" "MOD_VERSION"\n\n";
	char s2[64];
	char *s3 = "Bind a key to +hook for Hook\n\n";
	int len1, len2, len3;
	char *p2 = s2;

	strcat (s2, "Welcome to ");
	strncat(s2, hostname->string, sizeof s2 - strlen(s2) - 2);
	strcat (s2, "\n\n");

	len1 = strlen(s1);
	len2 = strlen(s2);
	len3 = strlen(s3);

	// Generate the path to the MOTD file.
	sprintf (motdPath, "./%s/%s", game_dir->string, motdfile->string);

	// Open the file.
	motdBytes = 0;
	in = fopen (motdPath, "rt");
	if (in != NULL)
	{
		// Count the number of bytes in the file.
		while ((c = fgetc (in)), c != EOF)
			motdBytes++;
	}
	else
		gi.dprintf("Unable to open MOTD file at %s.\n", motdPath);

	motdBytes += len1 + len2 + len3;

	// Make space for that many bytes.
	gMOTD = gi.TagMalloc (motdBytes + 1, TAG_GAME);
	gi.dprintf("Allocating %i bytes for MOTD\n", motdBytes + 1);
	here = gMOTD; //extra pointer for writing into gMOTD

	//Combine the strings into a banner block
	while (len1)
	{
		memcpy(here, s1++, 1);
		here++;
		motdBytes--;
		len1--;
	}

	while (len2)
	{
		memcpy(here, p2++, 1);
		here++;
		motdBytes--;
		len2--;
	}

	while (len3)
	{
		memcpy(here, s3++, 1);
		here++;
		motdBytes--;
		len3--;
	}

	highlight_text(gMOTD, gMOTD);

	// Now append the MOTD file.  Null-terminate the string.
	if (in)
	{
		rewind (in);
		while ((c = fgetc (in)), c != EOF)
		{
			*here = c;
			here++;
			motdBytes--;
		}
		fclose(in);
	}

	*here = '\0';

	// If anything went wrong, warn the console.
	if (motdBytes != 0)
		gi.dprintf ("MOTD error: off by %d bytes", motdBytes);

	// Print the message.
	if (!ent->bot_client)
		gi.centerprintf (ent, "%s", gMOTD);
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	char text[80];
	char *name;

	G_InitEdict (ent);

	//jsw
	CheckPlayers();
	name = ent->client->pers.netname;
	highlight_text(name, name);

	if (ctf->value)
		sprintf (text,
		"%s entered the game (%d red, %d blue, %d spectators)\n",
		name, ctfgame.players1, ctfgame.players2, ctfgame.specs);
	else
		sprintf (text,
		"%s entered the game (%d players, %d spectators)\n",
		name, ctfgame.players_total, ctfgame.specs);

	my_bprintf (PRINT_HIGH, text);

	InitClientResp (ent->client);

	//raven gzspace bug chase	
	ent->display = 0;
	// zgcl clear
	//	memset (&ent->client->zc,0,sizeof(zgcl_t));
	if(ent->client->pers.pl_state < PL_PLAYING )
		Connect(ent);
	else
	{
		gi.dprintf ("%s re-joined server\n", ent->client->pers.netname);
		ent->client->pers.db_hud = true;
	}


	// locate ent at a spawn point
	PutClientInServer (ent);

	// send effect
	/*RAV no need for this non sense!
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	*/
	ClientPrintMOTD(ent);

	if(use_bots->value)
		gi.centerprintf(ent,ClientMessage);
	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;
	//RAV
	int client;

	client = getEntOffset(ent) - 1;

	if(client <= maxclients->value && !ent->bot_client) 
	{
		// setup ent to be valid...
		ent->client->ps.fov = 10;
		proxyinfo[client].inuse = 0;
		//	proxyinfo[client].retries = 0;
		proxyinfo[client].rbotretries = 0;
		proxyinfo[client].charindex = 0;
		proxyinfo[client].teststr[0] = 0;
		proxyinfo[client].impulsesgenerated = 0;

	}
	//
	ent->client = game.clients + (ent - g_edicts - 1);



	ent->client->pers.inventory[ITEM_INDEX(FindItem("Grapple"))] = 0;
	ent->client->pers.HasVoted = false;
	ent->client->resp.vote = false;


	if (deathmatch->value)
	{

		ClientBeginDeathmatch (ent);

		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
		ent->client->pers.db_hud = true;
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}
	else
	{
		// send effect if in a multiplayer game
		if (game.maxclients > 1)
		{
			gi.WriteByte (svc_muzzleflash);
			gi.WriteShort (ent-g_edicts);
			gi.WriteByte (MZ_LOGIN);
			gi.multicast (ent->s.origin, MULTICAST_PVS);
			CheckPlayers();
			if (ctf->value)
				my_bprintf (PRINT_HIGH, "%s entered the game (%d red, %d blue, %d spectators)\n", ent->client->pers.netname, ctfgame.players1, ctfgame.players2, ctfgame.specs);
			else
				my_bprintf (PRINT_HIGH, "%s entered the game (%d players, %d spectators)\n", ent->client->pers.netname, ctfgame.players_total, ctfgame.specs);
		}
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

void maxrate_think(edict_t *self)
{
	safe_cprintf( self->owner, PRINT_HIGH, "Server restricting rate to %i\n", (int)maxrate->value );
	G_FreeEdict(self);
}

void nodelta_think(edict_t *self)
{
	safe_cprintf( self->owner, PRINT_HIGH, "You cannot hide your ping here\n");
	G_FreeEdict(self);
}

//RAV

qboolean Check_tag (edict_t *ent, char *namecheck)
{
	if ((!strstr(clan_name->string, " ")) && (!strstr(clan_pass->string, " ")))
		if ( strstr(namecheck, clan_name->string ))
			return false;
	return true;
}


/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s, *value;
	int		playernum;
	int rate, delta;//RAV
	char player[40];
	qboolean emptyname = 0;
	int i;
	char	*namepass;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	namepass = Info_ValueForKey (userinfo, "namepass");
	if (strlen(namepass) < 4)
	{
		namepass = "nopass";
	}
	strcpy(ent->client->pers.namepass, namepass);

	if (strstr("badinfo", s))
	{
		StuffCmd(ent,"disconnect\n");
		return;
	}

	//RAV
	if(g_filter->value)
		FilterText(s);
	//

	// catch an all-spaces name
	for (i = 0; i < strlen(s); i++)
	{
		if (s[i] != ' ')
		{
			emptyname = false;
			break;
		}
		else
			emptyname = true;
	}

	// fix empty or blank names
	if(strcmp ("", s) == 0 || emptyname)
	{
		StuffCmd (ent, va("name ImARetard\n")); // force client name change
		return;
	}

	//JSW - Make all player names all white, remove any green text
	white_text(s, s);
	//end
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	//clan name protection
	if(ent->client->pers.pl_state == PL_PLAYING)
	{
		if(!Check_tag(ent,ent->client->pers.netname))
		{
			value = Info_ValueForKey (userinfo, "clan");
			if (strcmp(clan_pass->string, value) != 0)
			{
				safe_bprintf (PRINT_HIGH, "%s's tried to use a Protected Clan name\n", ent->client->pers.netname);
				ent->command = 4;
				return;
			}
		}
	}
	//

	if(!ent->client->pers.name_set)
	{
		strcpy (ent->client->pers.name_change, ent->client->pers.netname);
		ent->client->pers.name_set = true;
	}
	else
	{
		//name has changed
		if ( strcmp(ent->client->pers.name_change, ent->client->pers.netname)!= 0)
		{
			if (!CheckNameProtect(Info_ValueForKey(userinfo, "name"), namepass))
			{
				//value = Info_ValueForKey (userinfo, "namepass");
				//	if (strcmp(Info_ValueForKey (userinfo, "namepass"), ent->client->pers.namepass) != 0)
				//	{
				safe_cprintf (ent, PRINT_HIGH, "That name is protected\n");
				ent->command = 2;
				return;
				//	}
			}
			if(ent->client->nametime > level.time)
			{
				safe_cprintf (ent, PRINT_HIGH, "Name Flooding is not permitted here\n ");
				ent->command = 2;
				return;
			}
			else
			{
				safe_bprintf (PRINT_HIGH, "%s's name was changed to %s \n", ent->client->pers.name_change, ent->client->pers.netname);
				strcpy (ent->client->pers.name_change, ent->client->pers.netname);
				ent->client->nametime = level.time+5;
			}
		}
	}

	//JSW
	//	StuffCmd(ent, "mm_delta $cl_nodelta\n");

	///* JSW - handled above^^^^^
	if(cl_check->value)
	{
		// check for 0 pingers
		if(!(ent->bot_client) && ent->client->pers.pl_state != PL_SPECTATOR)
		{   
			s = Info_ValueForKey (userinfo, "cl_nodelta");
			if (s)
			{
				delta = atoi(s);
				if (delta == 1)
				{
					edict_t *thinker;
					thinker = G_Spawn();
					thinker->think = nodelta_think;
					thinker->nextthink = level.time + 2 + random()*2;
					thinker->owner = ent;
					Info_SetValueForKey( userinfo, "cl_nodelta", va("%i", 0 ));
				}
			}
		}
	}
	//*/

	if(cl_check->value)
	{
		if(ent->client->pers.pl_state == PL_PLAYING)
		{
			//Pitchspeed Cheaters
			int newps = q2a_atoi(Info_ValueForKey(userinfo, "cl_pitchspeed"));
			int newas = q2a_atoi(Info_ValueForKey(userinfo, "cl_anglespeedkey"));
			int newglmono = q2a_atoi(Info_ValueForKey(userinfo, "gl_monolightmap"));
			if(newps == 0)
			{
				StuffCmd(ent, "set cl_pitchspeed $cl_pitchspeed u\n");
			}
			else if(ent->client->pers.pitchspeed == 0)
			{
				ent->client->pers.pitchspeed = newps;
			}
			else if(ent->client->pers.pitchspeed != newps)
			{
				//gi.bprintf (PRINT_HIGH, "%s changed pitchspeed to %d and was Warned\n", ent->client->pers.netname, newps);
				ent->command = 7;
				newps = ent->client->pers.pitchspeed;
				safe_cprintf (ent, PRINT_HIGH, "Pitch and Angle Speed changing is not permitted here\n ");
				return;
			}

			//anglespeed cheaters
			if(newas == 0)
			{
				StuffCmd(ent, "set cl_anglespeedkey $cl_anglespeedkey u\n");
			}
			else if(ent->client->pers.anglespeed == 0)
			{
				ent->client->pers.anglespeed = newas;
			}
			else if(ent->client->pers.anglespeed != newas)
			{
				gi.bprintf (PRINT_HIGH, "%s changed anglespeed to %d and was Warned\n", ent->client->pers.netname, newas);
				// StuffCmd (ent, va("set cl_anglespeedkey %i u\n", ent->client->pers.anglespeed));
				ent->command = 6;
				newas = ent->client->pers.anglespeed;
				safe_cprintf (ent, PRINT_HIGH, "Pitch and Angle Speed changing is not permitted here\n ");
				return;
			}

			//gl_monolightmap check
			if(newglmono == 1)
			{
				StuffCmd(ent, "set gl_monolightmap 0 u\n");
			}
			else if(ent->client->pers.glmonolightmap == 1)
			{
				ent->client->pers.glmonolightmap = newglmono;
			}
			else if(ent->client->pers.glmonolightmap != newglmono)
			{
				gi.bprintf (PRINT_HIGH, "%s tried to use gl_monolightmap and was Warned\n", ent->client->pers.netname);
				// StuffCmd (ent, va("set cl_anglespeedkey %i u\n", ent->client->pers.anglespeed));
				ent->command = 9;
				newglmono = ent->client->pers.glmonolightmap;
				safe_cprintf (ent, PRINT_HIGH, "Pitch and Angle Speed changing is not permitted here\n ");
				return;
			}

		}
	}
	//
	// check maxrate
	if(!(ent->bot_client) && ent->client->pers.pl_state != PL_SPECTATOR)
	{
		s = Info_ValueForKey (userinfo, "rate");
		if (s)
		{
			rate = atoi(s);
			if (rate > (int)maxrate->value)
			{
				edict_t *thinker;
				thinker = G_Spawn();
				thinker->think = maxrate_think;
				thinker->nextthink = level.time + 2 + random()*2;
				thinker->owner = ent;
				Info_SetValueForKey( userinfo, "rate", va("%i", (int)maxrate->value) );
				StuffCmd(ent, va("rate %i\n", (int)maxrate->value));
			}
		}
	}

	//RAV  ths allows a "loophole" for the reconnect feature for local lan play
	s = Info_ValueForKey (userinfo, "loop");
	if (strlen(s) )
		ent->flags |= FL_SPECIAL;

	if(!ent->bot_client)
	{
		s = Info_ValueForKey (userinfo, "ip");
		if (strlen(s))
			if(entryInFile ("safe_ip.txt", s))
				ent->flags |= FL_SPECIAL;
	}

	//RAV new bot detection routine for new ratbot wannbe
	//
	if(!ent->bot_client && ent->client->pers.pl_state == PL_PLAYING)
	{
		s = Info_ValueForKey (userinfo, prox->string);
		if (!strlen(s))
		{
			//BOT !
		}
	}
	//

	// set skin
	s = Info_ValueForKey (userinfo, "skin");
	playernum = ent - g_edicts - 1;
	// combine name and skin into a configstring

	//RAV
	//Check for bad skins here
	if(CheckModel (ent, s))
		gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\%s", ent->client->pers.netname, s) );
	else
		gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\male/grunt", ent->client->pers.netname) );
	//RAV

	//ZOID
	if (ctf->value)
		CTFAssignSkin(ent, s);
	else
		//ZOID
		gi.configstring (CS_PLAYERSKINS + playernum, va("%s\\%s", ent->client->pers.netname, s) );

	gi.configstring (CS_GENERAL + playernum, va("%15s", ent->client->pers.netname) );

	//Skin changers delight
	if(ent->client->pers.pl_state == PL_PLAYING && ent->client->resp.ctf_team > 0)
	{
		if(!ent->client->pers.skin_set)
		{
			strcpy (ent->client->pers.skin_change, s);
			ent->client->pers.skin_set = true;
		}
		else
		{
			//skin has changed
			if ( strcmp(ent->client->pers.skin_change, s)!= 0)
			{
				if(ent->client->skintime > level.time)
				{
					safe_cprintf (ent, PRINT_HIGH, "Skin Change Flooding is not permitted here\n ");
					//	StuffCmd (ent, va("skin %s\n",ent->client->pers.skin_change));
					ent->command = 3;
					return;
				}
				else
				{
					//	safe_bprintf (PRINT_HIGH, "%s's skin was changed to %s \n", ent->client->pers.name_change, s);
					strcpy (ent->client->pers.skin_change, s);
					ent->client->skintime = level.time + 5;
				}
			}
		}
	}
	//

	// fov
	//JSW	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	if (deathmatch->value && (dmflag & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	//RAV operators
	strcpy(player, "/0");
	strcpy(player, ent->client->pers.name_change);
	strcat(player, "@");
	strcat(player, ent->client->pers.ip);

	//JSW
	CheckOpFile(ent, player, true);
	//end

	// save off the userinfo in case we want to check something later
	strncpy(ent->client->pers.userinfo, 
		userinfo, sizeof(ent->client->pers.userinfo) - 1);
}

/*
RaVeNs remove edict/on forced reconnection 
*/
void RavenDisconnect (edict_t *ent)
{
	int		playernum;

	// Safety check...
	if (!G_EntExists(ent))
		return;

	if (!ent->client)
		return;

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;
	playernum = ent - g_edicts - 1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


char *GetPort(edict_t *ent,char *ip)
{
	static char modif[40];
	char entry[32], ipp1[3],ipp2[3],ipp3[3],ipp4[3],ipp5[5];
	int ec, j;

	sprintf(entry, "%sX", ip);

	j = 0;
	ec = 0;

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

	ec++;
	j = 0;
	while  (!strchr("X", entry[ec]))
	{
		sprintf (&ipp5[j], "%c", entry[ec]);
		j++;
		ec++;
	}
	sprintf (modif, "%s",ipp5);

	return (modif);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value, *namecheck;
	char *s;
	int client;
	qboolean passed = false;
	qboolean is_bot = false;
	char *name;
	char *ip;
	char player[30];
	int i;
	qboolean emptyname;

	if (!ent->client)
	{
		gi.dprintf ("Client Connection Refused due to entity not having a client\n");
		return false;
	}

	client = getEntOffset(ent) - 1;
	ent->bust_time = 0;

	//JSW - Check to see if server is locked
	// but allow operators
	name = Info_ValueForKey (userinfo, "name");
	ip = Info_ValueForKey (userinfo, "ip");

	if (developer->value)
		gi.dprintf ("ClientConnect called by %s@%s\n", name, ip);

	if (ip == NULL)
	{
		gi.dprintf ("Client Connection Refused due to NULL IP\n");
		return false;
	}

	emptyname = false; //QW// benefit of the doubt
	for (i = 0; i < strlen(name); i++)
	{
		if (name[i] != ' ')
		{
			emptyname = false;
		}
		else
			emptyname = true;
	}
	if (emptyname == true) //QW// || strlen(name) < 1)
	{
		gi.dprintf ("Client connection refused due to no name.\n");
		Info_SetValueForKey(userinfo, "rejmsg", "You do not have a name set.\n");
		return false;
	}


	strcpy(player, "/0");
	strcpy(player, name);
	strcat(player, "@");
	strcat(player, ip);

	strcpy(ent->client->pers.namepass, Info_ValueForKey(userinfo, "namepass"));
	CheckOpFile(ent, player, true);

	if (serverlocked && !(ent->client->pers.oplevel & OP_LOCKSERVER))
	{
		gi.dprintf("Player rejected due to locked server: %s\n", player);
		Info_SetValueForKey(userinfo, "rejmsg", "Server is locked\n");
		return false;
	}
	//end

	//RAV
	//check for reserved slots and password
	if(client > maxclients->value- 1 -(int)reserved_slots->value )
	{
		value = Info_ValueForKey (userinfo, "rpassword");
		if (strcmp(reserved_password->string, value) != 0 && !(ent->client->pers.oplevel & OP_PROTECTED))
		{
			gi.dprintf("Player rejected due to full server: %s\n", player);
			if((int)reserved_slots->value)
				Info_SetValueForKey(userinfo, "rejmsg", "Client Slots are Full!\nEnter Password for Reserved Slot\n");
			else
				Info_SetValueForKey(userinfo, "rejmsg", "Server Is FULL\n");
			return false;
		}
		passed = true;
	}
	//

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	//RAV
	//checks banned file / but if player is OP it will pass
	if(!ent->bot_client)
	{
		if ((checkAllowed (userinfo)!=0) && !(ent->client->pers.oplevel & OP_PROTECTED))
		{
			gi.dprintf("Player rejected due to matching entry in ip_banned.txt: %s\n", player);
			Info_SetValueForKey(userinfo, "rejmsg", "You've been BANNED contact admin !!.");
			//RavenDisconnect (ent);
			return false;
		}
	}
	//

	// check for a password
	if (!ent->bot_client)
	{
		namecheck = Info_ValueForKey (userinfo, "name");
		value = Info_ValueForKey (userinfo, "namepass");
		if (strlen(value) < 4)
		{
			Info_SetValueForKey(userinfo, "namepass", "nopass");
			value = "nopass";
		}
		if (!CheckNameProtect(namecheck, value))
		{
			if (strlen(Info_ValueForKey(userinfo, "namepass")) < 4)
			{
				gi.dprintf("namepass was not long enough, check to make sure it is greater than 4 chars.\n");
			}
			Info_SetValueForKey(userinfo, "rejmsg", "This name is protected\n");
			return false;
		}

		value = Info_ValueForKey (userinfo, "password");
		if (strcmp(passwd->string, value) != 0)
			//RAV
			if(!passed)
			{
				gi.dprintf("Player rejected due to invalid password: %s\n", player);
				Info_SetValueForKey(userinfo, "rejmsg", "Password Problem\n");
				//	RavenDisconnect (ent);
				return false;
			}

			if(! Check_tag(ent,namecheck))
			{
				value = Info_ValueForKey (userinfo, "clan");
				if (strcmp(clan_pass->string, value) != 0)
				{
					gi.dprintf("Player rejected due to invalid clan name: %s\n", player);
					Info_SetValueForKey(userinfo, "rejmsg", "You do not have the right to wear this tag\n");
					return false;
				}
			}
			proxyinfo[client].clientcommand = 0;
			proxyinfo[client].retries = 0;
			proxyinfo[client].rbotretries = 0;
			proxyinfo[client].charindex = 0;
			proxyinfo[client].ipaddress[0] = 0;
			//proxyinfo[client].name[0] = 0;
			proxyinfo[client].skin[0] = 0;
			proxyinfo[client].ipaddressBinary[0] = 0;
			proxyinfo[client].ipaddressBinary[1] = 0;
			proxyinfo[client].ipaddressBinary[2] = 0;
			proxyinfo[client].ipaddressBinary[3] = 0;

			if(client < maxclients->value )
			{
				// check for malformed or illegal info strings
				if (!Info_Validate(userinfo))
				{
					q2a_strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
				}
				q2a_strcpy(proxyinfo[client].userinfo, userinfo);
				s = Info_ValueForKey (userinfo, "skin");
				q2a_strncpy (proxyinfo[client].skin, s, sizeof(proxyinfo[client].skin)-1);
				q2a_strncpy (proxyinfo[client].userinfo, userinfo, sizeof(proxyinfo[client].userinfo) - 1);
				// they can connect
				ent->client = game.clients + (ent - g_edicts - 1);
				//disable the reconnection  on a LAN !!!
				if(!lan->value)
				{
					//need to set the addy on first connect, check for same on 2nd and let play if same, if not force 3rd try (bot)
					//then bust them, be sure after 2nd to kill the stored ip so 3rd try will bust them.
					if(strcmp(proxyinfo[client].firstport,"") == 0)
					{
						q2a_strcpy(proxyinfo[client].firstport, GetPort(ent,Info_ValueForKey(userinfo, "ip")));
						//reconnect now
						ent->command = 1;
						ent->pausetime = level.time +5;
						return true;
					}
					else
					{
						q2a_strcpy(proxyinfo[client].port, GetPort(ent,Info_ValueForKey(userinfo, "ip")));
						//if port is the same //not a bot //if differant bust me !!!
						if(strcmp(proxyinfo[client].firstport,proxyinfo[client].port) == 0
							&& (!(strcmp(proxyinfo[client].firstport,"") == 0)))
						{
							//reset the detection port
							q2a_strcpy(proxyinfo[client].firstport,"");
							ent->bust_time = 0;
						}
						else
						{
							//BOT
							//reset the detection port
							if(!(strcmp(proxyinfo[client].firstport,"") == 0))
								ent->bust_time = level.time +rndnum(30,69);
							q2a_strcpy(proxyinfo[client].firstport,"");
						}
					}
				}
			}
	}
	//

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		//ZOID -- force team join
		ent->client->resp.ctf_team = -1;
		//ZOID
		is_bot = ent->bot_client;			// make sure bot's join a team
		InitClientResp (ent->client);
		is_bot = false;
		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	// do real client specific stuff
	ent->client->pers.in_game = false;
	ent->client->pers.pl_state = PL_SPECTATOR; //spec
	ent->client->pers.oplevel = 0;
	ent->client->pers.motd = true;
	ent->client->pers.in_game = false;

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected from %s\n", ent->client->pers.netname, Info_ValueForKey (userinfo, "ip"));

	strcpy (ent->client->pers.ip, Info_ValueForKey (userinfo, "ip"));
	ent->client->pers.connected = true;
	if (log_connect->value)
		LogConnect(ent, true);

	Spectate(ent, NULL);

	StatsLog("CONN: %s\\%.1f\n", ent->client->pers.netname, level.time);

	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;
	char text[80];
	char *name;

	// Safety check...
	if (!G_EntExists(ent))
		return;

	if (!ent->client)
		return;

	if (log_connect->value)
		LogConnect(ent, false);

	//ZOID
	CTFDeadDropFlag(ent);
	CTFDeadDropTech(ent);
	//ZOID
	//RAV
	// drop the rune if we have one
	runes_drop(ent);

	StatsLog("QUIT: %s\\%.1f\n", ent->client->pers.netname, level.time);

	//clear out his name 
	ent->client->pers.name_set = 0;
	strcpy (ent->client->pers.ip, " ");
	strcpy (ent->client->pers.name_change, " ");
	strcpy (ent->client->pers.skin_change, " ");
	ent->client->pers.vote_times = 0;

	name = ent->client->pers.netname;
	highlight_text(name, name);

	if ( ent->flashlight )
	{	
		G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
	}
	//
	// send effect
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGOUT);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;

	CheckPlayers();
	if (ctf->value)
		sprintf (text, "%s has left the server. (%d red, %d blue, %d spectators remaining)\n", name, ctfgame.players1, ctfgame.players2, ctfgame.specs);
	else
		sprintf (text, "%s has left the server. (%d players, %d spectators remaining)\n", name, ctfgame.players_total, ctfgame.specs);
	my_bprintf (PRINT_HIGH, text);
	if (ctfgame.players1 + ctfgame.players2 + ctfgame.players_total + ctfgame.specs == 0)
		locked_teams = false;

	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
static
	trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}

void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

//RAV
void MV (edict_t * ent)
{
	AngleVectors (ent->client->v_angle, v_forward, v_right, v_up);
	return;
}

static qboolean is_jump;
static qboolean is_invis;
static qboolean is_speed;

static float framerate (usercmd_t *cmd)
{
	return ((float)cmd->msec / 1000);
}

void Get_Position ( edict_t *ent, vec3_t position )
{
	float yaw,pitch;

	yaw = ent->s.angles[YAW];
	pitch = ent->s.angles[PITCH];

	yaw = yaw * M_PI * 2 / 360;
	pitch = pitch * M_PI * 2 / 360;

	position[0] = cos(yaw) * cos(pitch);
	position[1] = sin(yaw) * cos(pitch);
	position[2] = -sin(pitch);
}

void ChainPodThink (edict_t *ent)
{
	if(ent->owner == NULL )return;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_LASER);
	gi.WritePosition (ent->s.origin);
	gi.WritePosition (ent->owner->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if(ent->target_ent != NULL)
	{
		if(Q_stricmp (ent->target_ent->classname, "item_flag_team2") == 0)
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_LASER);
			gi.WritePosition (ent->s.origin);
			gi.WritePosition (ent->target_ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
		}
	}
	ent->nextthink = level.time + FRAMETIME * 10;
}

qboolean TraceX (edict_t *ent,vec3_t p2)
{
	trace_t		rs_trace;
	vec3_t		v1,v2;

	int			contents;


	contents = CONTENTS_SOLID | CONTENTS_WINDOW;

	if(!(ent->bot_client))
	{
		if(ent->client->zc.waterstate)
		{
			VectorCopy(ent->mins,v1);
			VectorCopy(ent->maxs,v2);

			/*			v1[0] -= 4;
			v1[1] -= 4;
			v2[0] += 4;
			v2[1] += 4;*/
		}
		else if(!(ent->client->ps.pmove.pm_flags & PMF_DUCKED))
		{
			VectorSet(v1,-16,-16,-4);
			VectorSet(v2,16,16,32);
		}
		else
		{
			//			VectorCopy(ent->mins,v1);
			//			VectorCopy(ent->maxs,v2);
			VectorSet(v1,-4,-4,-4);
			VectorSet(v2,4,4,4);
		}
	}
	else
	{
		VectorSet(v1,0,0,0);
		VectorSet(v2,0,0,0);
		contents |= CONTENTS_LAVA | CONTENTS_SLIME;
	}

	rs_trace = gi.trace (ent->s.origin, v1, v2, p2, ent, contents);
	if(rs_trace.fraction == 1.0 && !rs_trace.allsolid && !rs_trace.startsolid ) return true;

	if(ent->client->zc.route_trace && rs_trace.ent && (ent->bot_client))
	{
		//if(!rs_trace.ent->targetname)
		if(!Q_stricmp(rs_trace.ent->classname, "func_door"))
		{
			if(rs_trace.ent->moveinfo.state == PSTATE_UP) return true;
			else return false;
		}
		//		if(!Q_stricmp(rs_trace.ent->classname, "func_train")) return true;
	}

	return false;
}
//

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
	edict_t	*other;
	int		i, j, k,l,oldwaterstate;
	byte	impulse;
	pmove_t	pm;
	vec3_t	min,max,v,vv;
	float x;
	trace_t		rs_trace;

	static	edict_t	*old_ground;
	static	qboolean	wasground;

	impulse = ucmd->impulse;

	if(impulse == 1)
		gi.bprintf(PRINT_HIGH,"%f\n",ent->s.origin[2]);


	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	//RAV
	is_jump = rune_has_rune(ent, RUNE_JUMP);
	is_invis = rune_has_rune(ent, RUNE_INVIS);
	is_speed = rune_has_rune(ent, RUNE_SPEED);
	//JSW
	/*	if (ent->client->resp.bonus == true)
	is_invis = true;
	*/	//end

	if(ent->stealth > level.time)
		is_invis = true;

	//QW// Flood protection CheckFlood()
	if (level.time < ent->client->flood_locktill)
		ent->is_blocked = true; // show in hud
	else
		ent->is_blocked = false;

	//
	//--------------------------------------------------------------------------------------
	//Target check
	if(ent->client->zc.first_target)
	{
		if(!ent->client->zc.first_target->inuse) ent->client->zc.first_target = NULL;
		else if(!ent->client->zc.first_target->deadflag) ent->client->zc.first_target = NULL;
	}

	//--------------------------------------------------------------------------------------
	//get JumpMax
	if(JumpMax == 0)
	{
		x = VEL_BOT_JUMP - ent->gravity * sv_gravity->value * FRAMETIME;
		JumpMax = 0;
		while(1)
		{
			JumpMax += x * FRAMETIME;
			x -= ent->gravity * sv_gravity->value * FRAMETIME;
			if( x < 0 ) break;
		}
	}

	//route nodeput
	j = 0;
	if(ent->client->ctf_grapple && ent->client->ctf_grapplestate > CTF_GRAPPLE_STATE_FLY) j = 1;

	if(!j && chedit->value && CurrentIndex < MAXNODES && !ent->deadflag && ent == &g_edicts[1])
	{
		if(targetindex > 0)
		{
			if(ent->target_ent == NULL) return;
			other = ent->target_ent;

			if(!TraceX(ent,other->s.origin))
			{
				k = 0;
				i = other->client->zc.routeindex;
				while(1)
				{
					if(i + 1 >= CurrentIndex)
					{
						j = Route[i + 1].state;
						if(j == GRS_ONTRAIN) if(Route[i + 1].ent->trainteam) break;
						Get_RouteOrigin(i + 1,v);
						if(!TraceX(ent,other->s.origin))
						{
							break;
						}
					}
					else break;
					i++;
				}
				Get_RouteOrigin(i + 1,v);
				VectorCopy(v,ent->s.origin);
			}

			VectorSubtract(other->s.origin,ent->s.origin,vv);
			ent->client->ps.viewangles[YAW] = Get_yaw(vv);
			ent->client->v_angle[YAW] = ent->client->ps.viewangles[YAW];

			ent->client->ps.viewangles[PITCH] = Get_pitch(vv);
			ent->client->v_angle[PITCH] = ent->client->ps.viewangles[PITCH];
			ent->viewheight = 22;

			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION ;
			ent->client->ps.pmove.pm_flags = PM_FREEZE;
			gi.linkentity(ent);

			return;
		}

		oldwaterstate = ent->client->zc.waterstate;
		Get_WaterState(ent);
		i = false;
		l = GRS_NORMAL;
		if(CurrentIndex > 0) Get_RouteOrigin(CurrentIndex - 1,v);
		if(!Route[CurrentIndex].index)
		{
			VectorCopy(ent->s.origin,v);
			old_ground = ent->groundentity;
			//gi.bprintf(PRINT_HIGH,"1\n");
			if(ent->groundentity)
				i = true;
		}
		else if(!TraceX(ent,v) /*&& ent->groundentity*/)
		{
			VectorCopy(ent->s.old_origin,v);
			//gi.bprintf(PRINT_HIGH,"2\n");
			i = 3;
			if(/* DISABLES CODE */ (0)/*ent->groundentity*/)
			{
				if(ent->groundentity->classname[0] == 'f') i = false;
			}
		}
		else if(ent->client->zc.waterstate != oldwaterstate)
		{
			i = true;
			if(ent->groundentity )
			{
				if(!Q_stricmp(ent->groundentity->classname, "func_train")
					|| !Q_stricmp(ent->groundentity->classname, "func_plat")
					|| !Q_stricmp(ent->groundentity->classname, "func_door")) i = false;
			}

			if(ent->client->zc.waterstate > oldwaterstate) VectorCopy(ent->s.origin,v);
			else VectorCopy(ent->s.old_origin,v);
			//gi.bprintf(PRINT_HIGH,"5\n");
		}
		else if(fabs(v[2] - ent->s.origin[2]) > 20)
		{
			if(ent->groundentity && ent->waterlevel < 2)
			{
				k = true;
				if(k)
				{
					VectorCopy(ent->s.origin,v);
					//gi.bprintf(PRINT_HIGH,"3\n");
					i = true;
				}
			}

		}
		else if(((/*ent->velocity[2] > 10 &&*/ !ent->groundentity && wasground == true)
			|| (/*ent->velocity[2] < -0.5 &&*/ ent->groundentity && wasground == false))
			&& Route[CurrentIndex - 1].state <= GRS_ITEMS)
		{
			j = false;
			k = true;//false;
			VectorCopy(ent->s.old_origin,v);
			v[2] -= 2;
			rs_trace = gi.trace(ent->s.old_origin, 
				ent->mins, ent->maxs, v, ent, MASK_PLAYERSOLID);
			if(rs_trace.fraction != 1.0) j = true;

			if(old_ground)
			{
				if(!Q_stricmp(old_ground->classname, "func_train")
					|| !Q_stricmp(old_ground->classname, "func_plat")
					|| !Q_stricmp(old_ground->classname, "func_door")) k = false;
			}
			if(!ent->groundentity /*&& j*/&& wasground == true && k)
			{
				VectorCopy(ent->s.old_origin,v);
				//gi.bprintf(PRINT_HIGH,"6\n");
				i = true;
			}
			else if(ent->groundentity /*&& !j*/&& wasground == false && k)
			{
				//VectorSubtract(ent->s.origin)

				VectorCopy(ent->s.origin,v);
				//gi.bprintf(PRINT_HIGH,"7\n");
				i = true;
			}

		}
		else if(Route[CurrentIndex-1].index > 1)
		{
			k = true;
			if(/* DISABLES CODE */ (0)/*old_ground*/)
			{
				if(!Q_stricmp(old_ground->classname, "func_train")
					|| !Q_stricmp(old_ground->classname, "func_plat")
					|| !Q_stricmp(old_ground->classname, "func_door")) k = false;
			}
			Get_RouteOrigin(CurrentIndex - 1,min);
			Get_RouteOrigin(CurrentIndex - 2,max);
			VectorSubtract(min,max,v);
			x = Get_yaw(v);
			VectorSubtract(ent->s.origin,/*Route[CurrentIndex-1].Pt*/ent->s.old_origin,v);
			if(VectorLength(v) > 0 && Get_vec_yaw(v,x) > 45 && k )
			{
				VectorCopy(ent->s.old_origin,v);
				//gi.bprintf(PRINT_HIGH,"8\n");
				i = true;
			}
		}

		if(ent->groundentity)
		{
			if(ent->groundentity != old_ground)
			{
				other = old_ground;
				old_ground = ent->groundentity;
				if(!Q_stricmp(old_ground->classname, "func_plat"))
				{
					if(old_ground->union_ent)
					{
						if(old_ground->union_ent->inuse && old_ground->union_ent->classname[0] == 'R')
						{
							//gi.bprintf(PRINT_HIGH,"plat put\n");
							VectorCopy(old_ground->monsterinfo.last_sighting,v);
							l = GRS_ONPLAT;
							i = 2;
						}
					}
				}
				else if(!Q_stricmp(old_ground->classname, "func_train"))
				{
					if(old_ground->union_ent)
					{
						if(old_ground->union_ent->inuse && old_ground->union_ent->classname[0] == 'R')
						{
							VectorCopy(old_ground->monsterinfo.last_sighting,v);
							l = GRS_ONTRAIN;
							i = 2;
						}
					}
				}
				else if(!Q_stricmp(old_ground->classname, "func_door"))
				{
					k = false;
					if(old_ground->targetname && old_ground->union_ent)
					{
						if(TraceX(ent,old_ground->union_ent->s.origin)
							&& fabs(ent->s.origin[2] - old_ground->union_ent->s.origin[2]) < JumpMax)
						{
							VectorCopy(old_ground->monsterinfo.last_sighting,v);
							l = GRS_ONDOOR;
							i = 2;
						}
						else k = true;
					}
					else k = true;
					if(k && i)
					{
						i = 2;
						old_ground = other;
					}
				}
			}
		}
		if(old_ground)
		{
			if(old_ground->classname[0] == 'f' && i != 2)
			{
				if(!Q_stricmp(old_ground->classname, "func_train")
					|| !Q_stricmp(old_ground->classname, "func_plat")
					|| !Q_stricmp(old_ground->classname, "func_door")) i = false;
			}
		}

		if(Route[CurrentIndex-1].index > 0 && i == true)
		{
			Get_RouteOrigin(CurrentIndex - 1,max);
			VectorSubtract(max,v,vv);
			if(VectorLength(vv) <= 32 ) i = false;
		}

		if(l == GRS_ONTRAIN || l == GRS_ONPLAT || l == GRS_ONDOOR)
		{
			if(Route[CurrentIndex - 1].ent == old_ground) i = false;
		}

		if(i)
		{
			if(l == GRS_NORMAL && ent->groundentity)
			{
				if(!Q_stricmp(old_ground->classname, "func_rotating"))
				{
					l = GRS_ONROTATE;
					//gi.bprintf(PRINT_HIGH,"On Rotate\n");
				}
			}

			VectorCopy(v,Route[CurrentIndex].Pt);
			Route[CurrentIndex].state = l;
			if(l > GRS_ITEMS && l <= GRS_ONTRAIN) Route[CurrentIndex].ent = old_ground;
			else if(l == GRS_ONDOOR) Route[CurrentIndex].ent = old_ground;

			if(l == GRS_ONTRAIN && old_ground->trainteam && old_ground->target_ent)
			{
				if(!Q_stricmp(old_ground->target_ent->classname,"path_corner"))
					VectorCopy(old_ground->target_ent->s.origin,Route[CurrentIndex].Tcourner);

				//gi.bprintf(PRINT_HIGH,"get chain\n");
			}
			//when normal or items
			if(++CurrentIndex < MAXNODES)
			{
				gi.bprintf(PRINT_HIGH,"Last %i pod(s).\n",MAXNODES - CurrentIndex);
				memset(&Route[CurrentIndex],0,sizeof(route_t)); //initialize
				Route[CurrentIndex].index = Route[CurrentIndex - 1].index +1;
			}
		}
		//VectorCopy(ent->s.origin,old_origin);
		if(ent->groundentity != NULL) wasground = true;
		else wasground = false;
	}



	//--------------------------------------
	level.current_entity = ent;
	client = ent->client;

	//RAV
	// this stops respawn protection if you shoot !!
	if((ent->client->respawn_framenum > level.framenum) &&
		(ucmd->buttons & BUTTON_ATTACK) && (!ent->deadflag) )
		client->respawn_framenum = level.framenum;


	if (client->hook_on && client->hook && ent->client->resp.hook_wait < level.time)
		abandon_hook_service(client->hook);

	if (client->hook_on && VectorLength(ent->velocity) < 1) 
		client->ps.pmove.gravity = 0;

	if(!ent->client->pers.pl_state)
		client->ps.pmove.pm_type = PM_SPECTATOR;

	//



	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after fifteen seconds
		if (level.time > level.intermissiontime + 15.0 //RAV (5) for Highscores 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}
	//new runes
	//speed

	if(is_speed)
	{
		float vel = 0, mul = 0;
		MV (ent);
		if (ucmd->forwardmove > 0)
		{
			ent->velocity[0] += 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_forward[1] * framerate(ucmd);
		}
		else if (ucmd->forwardmove < 0)
		{
			ent->velocity[0] -= 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_forward[1] * framerate(ucmd);
		}
		if (ucmd->sidemove > 0)
		{
			ent->velocity[0] += 1200 * v_right[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_right[1] * framerate(ucmd);
		}
		else if (ucmd->sidemove < 0)
		{
			ent->velocity[0] -= 1200 * v_right[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_right[1] * framerate(ucmd);
		}
		vel = sqrt (ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);
		if (vel > 550)
		{
			mul = 550 / vel;
			ent->velocity[0] *= mul;
			ent->velocity[1] *= mul;
		}
	}
	//end

	// SUPER JUMP

	if (is_jump)
	{
		float vel = 0, mul = 0;

		MV (ent);
		if (ucmd->forwardmove > 0)
		{
			ent->velocity[0] += 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_forward[1] * framerate(ucmd);
		}
		else if (ucmd->forwardmove < 0)
		{
			ent->velocity[0] -= 1200 * v_forward[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_forward[1] * framerate(ucmd);
		}

		if (ucmd->sidemove > 0)
		{
			ent->velocity[0] += 1200 * v_right[0] * framerate(ucmd);
			ent->velocity[1] += 1200 * v_right[1] * framerate(ucmd);
		}
		else if (ucmd->sidemove < 0)
		{
			ent->velocity[0] -= 1200 * v_right[0] * framerate(ucmd);
			ent->velocity[1] -= 1200 * v_right[1] * framerate(ucmd);
		}

		if (ucmd->upmove >= 10 && ent->groundentity)
		{
			ent->velocity[2] += 600;
		}


		vel = sqrt (ent->velocity[0]*ent->velocity[0] + ent->velocity[1]*ent->velocity[1]);

		if (vel > 550)
		{
			mul = 550 / vel;
			ent->velocity[0] *= mul;
			ent->velocity[1] *= mul;
		}

	}

	//stealth
	if(is_invis)
	{
		ent->svflags |= SVF_NOCLIENT;
		if (ucmd->forwardmove  ||  ucmd->sidemove || ucmd->upmove
			|| ucmd->buttons & BUTTON_ATTACK)
			ent->svflags &= ~SVF_NOCLIENT;

		ent->client->was_stealth = level.time + 2;
	}
	else
		if(ent->client->pers.pl_state == PL_PLAYING)
			ent->svflags &= ~SVF_NOCLIENT;
	//end


	//RAV
	if(speed_check->value && ent->client->resp.enterframe+300 > level.framenum)
	{
		if ((ucmd->msec >= (int)speed_msec->value)&&
			(ent->client->resp.enterframe+1000 < level.framenum))
		{
			if((ucmd->msec == (int)speed_bust->value) && (ent->client->pers.pl_state == PL_PLAYING))
				ent->client->resp.speed_cheat ++;

			//lets slow them down a bit (will appear to be laggy for the cheater)
			ucmd->msec = (int)speed_set->value;

			if((ent->client->resp.speed_cheat > 100) && (ent->client->pers.pl_state < PL_CHEATBOT)) //100 times over the speed limit!
			{
				OnBotDetection(ent, va("speed-hack"));
				ent->client->pers.pl_state = PL_CHEATBOT;
				return;
			}
		}
	}
	//gi.dprintf("f: %i, s: %i, u: %i\n", ucmd->forwardmove, ucmd->sidemove, ucmd->upmove);
	//gi.dprintf("%i, %i\n", ucmd->buttons, client->ps.pmove.pm_flags);

	pm_passent = ent;
	//RAV
	//chasecam

	if(ent->client->pers.pl_state == PL_SPECTATOR
		&& ent->client->pers.motd == false 
		&& match_state == STATE_PLAYING)
	{
		if (!client->buttons && 
			(ucmd->buttons & BUTTON_ATTACK)){
				//fire button
				if(ent->client->resp.spectator == 0)
					ent->client->resp.spectator = 1;
				if(!ent->client->pers.db_hud)
					ent->client->pers.db_hud = true;

				SwitchModeChaseCam(ent);
		}

	}
	//
	//ZOID
	if (ent->client->chase_target) 
	{
		client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
		client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
		client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

		//RAV chasecam upgrade

		// this buttons stuff is copied from further below in ClientThink...
		client->oldbuttons = client->buttons;
		client->buttons = ucmd->buttons;
		client->latched_buttons |= client->buttons & ~client->oldbuttons;
		return;
		//
	}
	//ZOID

	// set up for pmove
	memset (&pm, 0, sizeof(pm));


	if (ent->movetype == MOVETYPE_NOCLIP)
		client->ps.pmove.pm_type = PM_SPECTATOR;
	else if (ent->s.modelindex != 255)
		client->ps.pmove.pm_type = PM_GIB;
	else if (ent->deadflag)
		client->ps.pmove.pm_type = PM_DEAD;
	else
		client->ps.pmove.pm_type = PM_NORMAL;

	client->ps.pmove.gravity = sv_gravity->value;
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
		//		gi.dprintf ("pmove changed!\n");
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

	if (ent->groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		gi.sound(ent, CHAN_VOICE, gi.soundindex("*jump1.wav"), 1, ATTN_NORM, 0);
		PlayerNoise(ent, ent->s.origin, PNOISE_SELF);
	}

	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	if (ent->deadflag)
	{
		client->ps.viewangles[ROLL] = 40;
		client->ps.viewangles[PITCH] = -15;
		client->ps.viewangles[YAW] = client->killer_yaw;
	}
	else
	{
		VectorCopy (pm.viewangles, client->v_angle);
		VectorCopy (pm.viewangles, client->ps.viewangles);
	}


	//ZOID
	if (client->ctf_grapple)
		CTFGrapplePull(client->ctf_grapple);
	//ZOID

	gi.linkentity (ent);

	if (ent->movetype != MOVETYPE_NOCLIP)
		G_TouchTriggers (ent);

	// touch other objects
	for (i=0 ; i<pm.numtouch ; i++)
	{
		other = pm.touchents[i];
		for (j=0 ; j<i ; j++)
			if (pm.touchents[j] == other)
				break;
		if (j != i)
			continue;	// duplicated
		if (!other->touch)
			continue;
		other->touch (other, ent, NULL, NULL);
	}

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;

	// save light level the player is standing on for
	// monster sighting AI
	ent->light_level = ucmd->lightlevel;

	// fire weapon from final position if needed
	if (client->latched_buttons & BUTTON_ATTACK
		//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
		//ZOID
		)
	{
		if (!client->weapon_thunk)
		{
			client->weapon_thunk = true;
			Think_Weapon (ent);
		}
	}

	//ZOID
	//regen tech
	CTFApplyRegeneration(ent);
	//ZOID

	//ZOID

	for (i = 1; i <= maxclients->value; i++) {
		other = g_edicts + i;
		if (other->inuse && other->client->chase_target == ent)
			UpdateChaseCam(other);
	}
	//RAV
	if (ent->client->chase_target != NULL)
		UpdateChaseCam(ent);
	//ZOID
	if(ent->client->resp.shots != 0)
	{
		ent->client->resp.eff = 100 * ent->client->resp.frags / ent->client->resp.shots;
	}

	// =========================================================
	// =========================================================

	//RAV  hook stuff
	//if(!chedit->value) //use old hook for normal usage
	//{
	if (ent->client->buttons & BUTTON_USE
		&& !ent->deadflag && !(client->ps.pmove.pm_type == PM_SPECTATOR))			
	{
		abandon_hook_fire (ent);//FIRE HOOK 
	}
	if (Ended_Grappling (client) && 
		!ent->deadflag && client->hook)
	{
		abandon_hook_reset(ent->client->hook);
	}

	//}

}

/*
GET_PING_AVG
this function gets an average ping for 30 sec  to reduce overflowing og=f the server
*/
qboolean GET_AVG_PING(edict_t *ent)
{
	int ping1, ping2, ping3, ping4 = 0;

	ping1 = ent->client->ping1;
	ping2 = ent->client->ping2;
	ping3 = ent->client->ping3;
	ping4 = ent->client->ping4;

	if(ping1 == 0)
	{
		ent->client->ping1 = ent->client->ping;
		ent->client->checkpingtime = level.time + 1;
		return true;
	}
	else if(ping2 == 0)
	{
		ent->client->ping2 = ent->client->ping;
		ent->client->checkpingtime = level.time + 1;
		return true;
	}
	else if(ping3 == 0)
	{
		ent->client->ping3 = ent->client->ping;
		ent->client->checkpingtime = level.time + 1;
		return true;
	}
	else if(ping4 == 0)
	{
		ent->client->ping4 = ent->client->ping;
		ent->client->checkpingtime = level.time + 1;
		return true;
	}

	if(ping1 + ping2 + ping3 + ping4 /4 > (int)lag_ping->value)
	{
		ent->client->checkpingtime = level.time + 5;
		return false;
	}

	//reset all pings 
	ent->client->ping1 = 0;
	ent->client->ping2 = 0;
	ent->client->ping3 = 0;
	ent->client->ping4 = 0;
	ent->client->checkpingtime = level.time + 10;
	return true;
}

void SendStatusBar(edict_t *ent, char *bar)
{
	gi.WriteByte (0x0D); //configstring
	gi.WriteShort(5); //status bar index
	gi.WriteString (bar);
	gi.unicast (ent, false);

}

/*|\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
SPEC_CHECK
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
void Spec_Check (edict_t *ent)
{

	int kick;

	if (ent->client->pers.oplevel & OP_SPEC)
		return;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	if(ent->spec_time > level.time || match_state != STATE_PLAYING || ent->bot_client)
		return;

	if (CountConnectedClients() < maxclients->value - 1 - (int)reserved_slots->value)
	{
		ent->spec_time = level.time + 30;
		ent->spec_warned = false;
		ent->speccheck = false;
		return;
	}
	else if(!ent->speccheck)
	{
		ent->spec_time = level.time + 10;
		ent->speccheck = true;
	}

	if(ent->spec_time == level.time && !ent->spec_warned)
	{
		safe_centerprintf (ent, "Spectators not allowed\nif server is full\nYou have 10 seconds to Join\n");

		ent->spec_kick = level.time + 10;
		ent->spec_warned = true;
		return;
	}

	if(ent->spec_warned && (ent->spec_kick > level.time))
	{
		kick = ent->spec_kick - level.time;
		safe_centerprintf (ent, "Spectators not allowed\nif server is full\nYou have %d seconds to Join\n",(kick));
		return;
	}

	if(ent->spec_warned && (ent->spec_kick == level.time))
	{
		safe_centerprintf (ent, "Spectators not allowed\nYou have been kicked\n");
		StuffCmd (ent, "disconnect\n");
		return;
	}
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/

extern char *dm_statusbar;
extern char *raildm_statusbar;

void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	//RAV
	char song[80];
	int clts = ent-g_edicts-1;

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	if (level.intermissiontime)
		return;

	//RAV motd
	if(!ent || !ent->client)
		return;

	//fixes any over max health issues
	if (ent->health > max_Vhealth->value)
		ent->health = max_Vhealth->value;

	//RATBOT
	//sorry bout yer luck dude
	if((!ent->flags) & FL_SPECIAL && ent->bust_time != 0 && !ent->client->resp.spectator &&
		ent->bust_time < level.time && ent->client->pers.pl_state < PL_CHEATBOT)
	{
		OnBotDetection(ent, va("ProxyBot"));
		ent->client->pers.pl_state = PL_CHEATBOT;
		return;
	}
	//

	if(ent->command == 1 && ent->pausetime < level.time )
	{
		//raven gzspace bug chase
		ent->display = 0;
		//	ent->inuse = false;
		if(ent->flags & FL_SPECIAL)
			StuffCmd (ent, va("reconnect\n"));
		else
		{
			sprintf(buffer, "\nconnect %s\n", reconnect_address);
			ent->bust_time = 0;
			StuffCmd (ent, va("%s\n",buffer));
		}
		ent->command = 0;
		ent->pausetime = 0;
		return;
	}

	if(ent->reset_time > 0 && ent->reset_time == level.time)
	{
		ent->tries = 0;
		q2a_strcpy(proxyinfo[clts].conaddress, "0");
		ent->reset_time = 0;
	}

	//RAVENS cl_additions
	if(ent->command > 1)
	{
		if(ent->commandtimeout >= level.time)
		{
			//do nothing  (debugg)
		}
		else
		{
			ent->commandtimeout = level.time + 5;
			//fixes bad names and cl_ stuff
			if (ent->command == 2)
			{
				//name
				StuffCmd (ent, va("name %s\n", ent->client->pers.name_change));
				ent->command = 0;
			}
			else if (ent->command == 3)
			{
				//skin
				StuffCmd (ent, va("skin %s\n",ent->client->pers.skin_change));
				ent->command = 0;
			}
			else if (ent->command == 4)
			{
				//Clan name
				StuffCmd (ent, va("disconnect\n"));
				ent->command = 0;
			}
			else if (ent->command == 5)
			{
				//Anglespeed
				StuffCmd (ent, va("set cl_anglespeedkey %i u\n", 
					ent->client->pers.anglespeed));
				ent->command = 0;
			}
			else if (ent->command == 6)
			{
				//Anglespeed
				StuffCmd (ent, va("set cl_anglespeedkey %i u\n",
					ent->client->pers.anglespeed));
				ent->command = 0;
			}
			else if (ent->command == 7)
			{
				//Anglespeed
				StuffCmd (ent, va("set cl_pitchspeed %i u\n",
					ent->client->pers.pitchspeed));
				ent->command = 0;
			}
			else if (ent->command== 8)
			{
				//Anglespeed
				StuffCmd (ent, va("set cl_nodelta 0 u\n" ));
				ent->command = 0;
			}
			else if (ent->command == 9)
			{
				StuffCmd (ent, va("set gl_monolightmap 0 u\n" ));
				ent->command = 0;
			}
		}
	}

	if(ent->client->pers.motd == true)
	{
		ent->movetype = MOVETYPE_NOCLIP;
		ent->solid = SOLID_NOT;
		ent->svflags |= SVF_NOCLIENT;
		ent->client->resp.ctf_team = CTF_NOTEAM;
		ent->client->ps.gunindex = 0;
		ent->client->pers.pl_state = PL_SPECTATOR;
		gi.linkentity (ent);

		//remove Hud and statusbar
		//SendStatusBar (ent, "\0");
		//raven gzspace bug chase
		ent->display = 0;

		if (ent->client->buttons & BUTTON_ANY)
		{
			if(ent->client->resp.enterframe + 33 > level.framenum)
				return;
			if (ent->client->buttons & BUTTON_ATTACK)
				return;
			ent->client->pers.motd_seen = true;
			ent->client->pers.in_game = true;
			ent->client->pers.motd = false;
			ent->client->pers.pl_state = PL_SPECTATOR;
			ent->reset_time = level.time + 10;
			respawn(ent, true);

			//raven gzspace bug chase
			ent->display = 1;

			//play the intro song if used
			if(wavs->value && !ent->bot_client)
			{
				Com_sprintf(song, sizeof(song), songtoplay->string);
				//gi.sound (ent, CHAN_AUTO, gi.soundindex (song), 1, ATTN_NORM, 0);
				StuffCmd(ent, va("play %s\n", song));
			}
		}
	}

	//

	//RAV chasecam!
	if(ent->client->pers.pl_state == PL_SPECTATOR)
	{
		if (ent->client->chase_mode == CHASE_FREECAM)
			ent->client->ps.pmove.pm_type = PM_SPECTATOR;
		else
			ent->client->ps.pmove.pm_type = PM_FREEZE;	
	}

	//RAV
	//GZ_SPACE  overflow check

	//if(ent->client->checkpingtime == level.time && !GET_AVG_PING(ent))
	//	ent->client->overflowed = true;
	//	else
	//	ent->client->overflowed = false;

	//Check For Cheaters
	//if (!ent->bot_client)
	//	StuffCmd(ent, "mm_fps $cl_maxfps\n");//max fps
	//if(ent->client->checkframe == level.framenum && !ent->bot_client)
	//{
	//	//StuffCmd(ent, "mm_fps $cl_maxfps\n");//max fps
	//	StuffCmd(ent, "mm_ts $timescale\n");//timescale hack
	//	ent->client->checkframe = level.framenum+40;
	//}

	//RAV	anti camper mod
	if (camper_check->value && ent->client->check_camping && !ent->bot_client)
	{
		CheckForCamping (ent);
	}
	//

	//RAV
	if((ent->movetype == MOVETYPE_NOCLIP) && ent->bot_client)
		ent->movetype = MOVETYPE_STEP;
	if(ent->client->pers.pl_state == PL_PLAYING)
	{
		if((ent->movetype == MOVETYPE_NOCLIP ||	ent->solid & SOLID_NOT
			|| ent->svflags & SVF_NOCLIENT || (!ent->takedamage) & DAMAGE_YES)
			&& (ent->health > 5 ) /* && !ent->bot_client*/ )
		{
			//gi.dprintf ("%s was made whole by TMG\n",ent->client->pers.netname);
			if(ent->inuse && !ent->deadflag && !is_invis && ent->client->was_stealth < level.time)
			{
				if(ent->client->needschecked)
				{
					//make them respawn
					gi.dprintf ("%s was made whole by TMG\n",ent->client->pers.netname);
					ent->client->pers.pl_state = PL_NEEDSPAWN;
					ent->client->needschecked = false;
				}
				else
				{
					ent->client->needschecked = true;
				}
			}
		}
	}

	//this section dictates player state of the game
	if(level.warmup && !ent->bot_client) //warmup mode
	{
		if(ent->client->pers.pl_state == PL_PLAYING)//playing in game
			ent->client->pers.pl_state = PL_WARMUP;//set to warmup
	}
	//if not playing or spec
	if(ent->client->pers.pl_state > PL_PLAYING)
	{
		//set the playerstate (warmup, playing ect)
		switch (ent->client->pers.pl_state)
		{
		case PL_NEEDSPAWN:	//player needs respawned
			// don't even bother waiting for death frames
			ent->client->pers.pl_state = PL_PLAYING;
			ent->deadflag = DEAD_DEAD;
			player_die_fast (ent, ent, ent, 100000, vec3_origin);
			if(ctf->value)
				ent->client->resp.ctf_state = CTF_STATE_START;
			respawn(ent, true);
			break;
		case PL_WARMUP:
			//limbo mode while in level countdown
			CTFPlayerResetGrapple(ent);
			abandon_hook_reset(ent->client->hook);
			ent->client->newweapon = NULL;
			ChangeWeapon (ent);
			ent->client->ps.gunindex = 0;
			gi.linkentity (ent);
			break;
		case PL_CHEATBOT:	//busted bot
			BadPlayer(ent);
			break;
		default:
			//bot
			break;
		}
	}

	//raven spectator check !
	if(ent->client->pers.pl_state == PL_SPECTATOR  && !ent->client->pers.motd && spec_check->value)
		Spec_Check (ent);

	//RAV add a small check to be sure all playing players are damagable
	if(ctf->value && ent->client->resp.ctf_team < 1 && ent->client->pers.pl_state != PL_CHEATBOT &&!ent->bot_client)
		ent->client->pers.pl_state = PL_SPECTATOR;
	if(ent->client->resp.spectator)
	{
		ent->client->pers.pl_state = PL_SPECTATOR;
	}

	//RAV check for invisible player with a weapon(bugfix 3/31/01)Kai reported
	if((ent->client->pers.pl_state == PL_SPECTATOR || ent->client->resp.spectator)
		&&(ent->client->ps.gunindex != 0))
	{
		StuffCmd (ent, va("spec\n"));
	}

	//RAV flag tracking
	if(ctf->value)
	{
		if(ent->client->pers.inventory[ITEM_INDEX(flag1_item)])//red flag
			VectorCopy (ent->s.origin, redflagnow);
		if(ent->client->pers.inventory[ITEM_INDEX(flag2_item)])//blue flag
			VectorCopy (ent->s.origin, blueflagnow);
	}
	//

	client = ent->client;
	// run weapon animations if it hasn't been done by a ucmd_t
	if (!client->weapon_thunk
		//ZOID
		&& ent->movetype != MOVETYPE_NOCLIP
		//ZOID
		)
		Think_Weapon (ent);
	client->weapon_thunk = false;

	//RAV
	if (rune_has_rune(ent, RUNE_REGEN))
	{
		client->regen_acc += (float) RUNE_REGEN_PER_SEC / 10.0;
		if (ent->health < 150)
		{
			ent->health += (int) client->regen_acc;
			if (ent->health > 150)
				ent->health = 150;
			if (ent->pain_debounce_time < level.time)
			{
				gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/comp_up.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 2;
			}
		}
		if (ArmorIndex(ent) && client->pers.inventory[ArmorIndex(ent)] < 100)
		{
			client->pers.inventory[ArmorIndex(ent)] += (int) client->regen_acc;
			if (client->pers.inventory[ArmorIndex(ent)] > 100)
				client->pers.inventory[ArmorIndex(ent)] = 100;
			if (ent->pain_debounce_time < level.time)
			{
				gi.sound(ent, CHAN_ITEM, gi.soundindex("items/pkup.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 2;
			}
		}
		client->regen_acc -= (int) client->regen_acc;
	}
	//
	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;
			if ( ( client->latched_buttons & buttonMask )
				|| (deathmatch->value && (dmflag & DF_FORCE_RESPAWN)))
			{
				respawn(ent, false);
				client->latched_buttons = 0;
			}
		}
		return;
	}


	client->latched_buttons = 0;
}
