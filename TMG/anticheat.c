#include "g_local.h"
#include <ctype.h>
#include <stdlib.h>

#include "anticheat.h"

//qboolean getLogicalValue(char *arg);

//global
int botdetection;

static FILE *fpBot;


/**********************************************
new bot detection
***********************************************/


/*  set sv_botdetection  <value>  31 is all features!
1           Log bot detection to a file
2           Kick detected bot
4           Notify the other players of who is using a bot
8           Include impulses as a detection method
16			Bans user name and /or ip.
*/

void
BotDetection(edict_t *ent, usercmd_t *ucmd)
{
	

	static gclient_t *cl;

    cl = ent->client;

    if (ent->client->pers.pl_state == PL_CHEATBOT)
        return;
	
   if (ucmd->impulse != 6) {
   
   	   if (botdetection & BOT_IMPULSE) {
     OnBotDetection(ent, va("impulse %d", ucmd->impulse));
            return;
	
        }
    }
}

FILE *tn_open (const char *filename, const char *t)
{
	FILE *fd;
	char path[PATH_MAX];

	strcpy (path, game_dir->string);
	if (Q_stricmp (path, "\0") == 0)
		strcpy (path, "baseq2");
	strcat (path, "/");
	strcat (path, cfgdir->string);
	strcat (path, "/");
	strcat (path, filename);
	fd=fopen (path, t);
	return (fd);
}

void addEntry2 (char *filename, char ip[IP_LENGTH])
{
	FILE *ipfile;

	// add user to file
	strcat (ip, "\n");

	if ((ipfile = tn_open(filename, "a")))
	{
		fputs(ip, ipfile);
		fclose (ipfile);
	}
	return;
}

/**
 Strip names of selected forbidden characters
 */
char *ConvertName(char *name)
{
	// Note: escapes needed for \ and "
	char *forbidden = "~!@#$%^&*()=|?,.<>[]{}:;/-";

	int i, j;

	for(i = 0; i < strlen(forbidden); i++)
	{
		for (j = 0; j < strlen(name); j++)
		{
			if(forbidden[i] == name[j])
			{
				name[j] = 'x';
			}
		}
	}
	return (name);
}

/**
 This could probably replace ConvertName
 without all its complexity and it also
 catches " ' + \ _ ` and DEL characters
 but I think space and _ were intended to
 be allowed and the others were an oversight.
 */
char *ConvertNameA(char *name)
{
	int i;

	for(i = 0; i < strlen(name); i++ )
	{
		if(!isalnum(name[i])
		   && name[i] != ' '
		   && name[i] != '_')
		{
			name[i] = 'x';
		}
	}
	return (name);
}

/*
 Impair identified cheaters and make them glow
 */
void BadPlayer(edict_t *ent)
{
	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	if(ent->client->pers.pl_state != PL_CHEATBOT)
		return;

	//set flags/lose gun ..ect
	ent->client->newweapon = NULL;
	ChangeWeapon (ent);
	ent->client->ps.gunindex = 0;
	ent->bust_time = 0;
	runes_drop(ent);

	if(ent->client->hook)
		abandon_hook_reset(ent->client->hook);

	if ( ent->flashlight )
	{
		G_FreeEdict(ent->flashlight);
		ent->flashlight = NULL;
	}

	if(ctf->value){
		CTFPlayerResetGrapple(ent);
		CTFDeadDropFlag(ent);
		CTFDeadDropTech(ent);
		ent->client->resp.ctf_team = CTF_NOTEAM;
	}
	//give the Cheater shell
	ent->s.effects |= EF_COLOR_SHELL;
	ent->s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);

}

void
OnBotDetection(edict_t *ent, char *msg)
{
	int log = 0;
	char user[32];
	char logged[80];
	char userip[80];
	char name[40];

	// Make sure ent exists!
	if (!G_EntExists(ent))
		return;

	if(ent->client->pers.pl_state == PL_CHEATBOT)
		return;

	ent->client->pers.pl_state = PL_CHEATBOT;

	//crashbug fix (on name logging)
	strcpy (name, ent->client->pers.netname);
	strcpy (name, ConvertName(name));

	if(!strcmp(name, ent->client->pers.netname) == 0)
		log = 1;

	sprintf(user, "%s@%s",
			ent->client->pers.netname,
			Info_ValueForKey (ent->client->pers.userinfo, "ip"));

	sprintf(userip, "*@%s", Info_ValueForKey (ent->client->pers.userinfo, "ip"));

	sprintf(logged, "%s@%s@%s@%8s@%10s",
			ent->client->pers.netname,
			Info_ValueForKey (ent->client->pers.userinfo, "ip"),
			msg, sys_time, sys_date);

	//QW// when name doesn't match converted name
	if (log && botdetection & BOT_LOG)
	{
		addEntry2 ("logs/bot_detected.txt", logged);
	}

	if (botdetection & BOT_NOTIFY) {
		safe_bprintf(PRINT_HIGH, "%s %s\n",
					 user,"was Busted By -=BOtCRuSher=-");
	}

	if(botdetection & BOT_BAN){
		addEntry ("ip_banned.txt", userip);
	}


	if (botdetection & BOT_KICK) {
		char	command [256];

		ent->movetype = MOVETYPE_NOCLIP;

		Com_sprintf (command, sizeof(command), "kick %s\n", ent->client->pers.netname);
		gi.AddCommandString(command);
	}
	return;
}

void
InitBotDetection(void)
{
	char fname[PATH_MAX];

	sprintf(fname, "%s/%s/%s/bot_detected.log",
			basedir->string, game_dir->string, cfgdir->string);

	fpBot = fopen(fname, "a");
}

void
EndBotDetection(void)
{
    if (fpBot)
        fclose(fpBot);
}

int isBlank(char *buff1)
{
  while(*buff1 == ' ')
  {
    buff1++;
  }

  return !(*buff1);
}

qboolean CheckForConnection(edict_t *player)
{
	int i;
	edict_t	*ent;
	
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = g_edicts + 1 + i;
		if (!ent->inuse || !ent->client)
			continue;

if(strcmp(Info_ValueForKey (player->client->pers.userinfo, "ip[13]"),
   Info_ValueForKey (ent->client->pers.userinfo, "ip[13]")) == 0)		
	return true;
	}

return false;
}

