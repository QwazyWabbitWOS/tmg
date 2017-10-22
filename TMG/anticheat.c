#include <ctype.h>
#include <stdlib.h>
#include "g_local.h"

#include "g_items.h"
#include "anticheat.h"
#include "e_hook.h"
#include "runes.h"
#include "filehand.h"

//global
int botdetection;

//RAV
proxyinfo_t *proxyinfo;
proxyinfo_t *proxyinfoBase;
oplist_t	*oplist;
oplist_t	*oplistBase;
proxyreconnectinfo_t *reconnectproxyinfo;
reconnect_info* reconnectlist;
retrylist_info* retrylist;
char moddir[MAX_QPATH];
char buffer[0x10000];
char reconnect_address[256] = { 0 };
//qboolean nameChangeFloodProtect = false;
//int nameChangeFloodProtectNum = 5;
//int nameChangeFloodProtectSec = 2;
//int nameChangeFloodProtectSilence = 10;
//char nameChangeFloodProtectMsg[256];
//
//qboolean skinChangeFloodProtect = false;
//int skinChangeFloodProtectNum = 5;
//int skinChangeFloodProtectSec = 2;
//int skinChangeFloodProtectSilence = 10;
//char skinChangeFloodProtectMsg[256];
//

//static FILE *fpBot;

/**********************************************
new bot detection
***********************************************/


/*  set sv_botdetection  <value>  31 is all features!
 1		Log bot detection to a file
 2		Kick detected bot
 4		Notify the other players of who is using a bot
 8		Include impulses as a detection method
 16		Bans user name and /or ip.
 */

void InitAnticheat(void)
{
	int i;

	//RAV
	proxyinfoBase = gi.TagMalloc ((maxclients->value + 1) * sizeof(proxyinfo_t), TAG_GAME);
	q2a_memset(proxyinfoBase, 0x0, (maxclients->value + 1) * sizeof(proxyinfo_t));
	proxyinfo = proxyinfoBase + 1;
	proxyinfo[-1].inuse = 1;

	reconnectproxyinfo = gi.TagMalloc (maxclients->value  * sizeof(proxyreconnectinfo_t), TAG_GAME);
	q2a_memset(reconnectproxyinfo, 0x0, maxclients->value * sizeof(proxyreconnectinfo_t));

	reconnectlist = (reconnect_info *)gi.TagMalloc (maxclients->value * sizeof(reconnect_info), TAG_GAME);

	retrylist = (retrylist_info *)gi.TagMalloc (maxclients->value * sizeof(retrylist_info), TAG_GAME);

	for( i = -1; i < maxclients->value; i++)
	{
		proxyinfo[i].inuse = 0;
		proxyinfo[i].clientcommand = 0;
		proxyinfo[i].stuffFile = 0;
		proxyinfo[i].impulsesgenerated = 0;
		proxyinfo[i].retries = 0;
		proxyinfo[i].rbotretries = 0;
		proxyinfo[i].charindex = 0;
		proxyinfo[i].teststr[0] = 0;
		proxyinfo[i].cl_pitchspeed = 0;
	}

	if(!lan->value)
	{
		//if server_ip is not set, stop the server
		if (strcmp(server_ip->string, "") == 0)
			gi.error ("You must set server_ip, e.g.: set server_ip 216.112.2.12:27910");
		strcat(reconnect_address, server_ip->string);
	}

	Com_sprintf(moddir, sizeof moddir, "%s", game_dir->string);
}

//void
//BotDetection(edict_t *ent, usercmd_t *ucmd)
//{
//	static gclient_t *cl;
//
//	cl = ent->client;
//
//	if (ent->client->pers.pl_state == PL_CHEATBOT)
//		return;
//
//	if (ucmd->impulse != 6)
//	{
//		if (botdetection & BOT_IMPULSE)
//		{
//			OnBotDetection(ent, va("impulse %d", ucmd->impulse));
//			return;
//		}
//	}
//}

FILE *tn_open (const char *filename, const char *mode)
{
	FILE *fd;
	char path[PATH_MAX];

	strcpy (path, game_dir->string);
	if (Q_stricmp (path, "") == 0)
		strcpy (path, "baseq2");

	strcat (path, "/");
	strcat (path, cfgdir->string);
	strcat (path, "/");
	strcat (path, filename);
	fd = fopen (path, mode);
	return (fd);
}

void AddLogEntry (char *filename, char *text)
{
	FILE *ipfile;

	ipfile = tn_open(filename, "a+");
	if (ipfile)
	{
		fputs(text, ipfile);
		fputs("\n", ipfile);
		fclose (ipfile);
	}
	else
	{
		gi.dprintf("ERROR opening %s for logging\n", filename);
	}
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

	if(ctf->value)
	{
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
	char user[80];
	char logged[MAX_INFO_STRING];
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
	strcpy (name, ConvertName(name)); // eliminate forbidden chars

	if(strcmp(name, ent->client->pers.netname))
		log = 1; // if name doesn't match after conversion

	Com_sprintf(user, sizeof user, "%s@%s",
				ent->client->pers.netname,
				Info_ValueForKey (ent->client->pers.userinfo, "ip"));

	Com_sprintf(userip, sizeof userip, "*@%s", 
				Info_ValueForKey (ent->client->pers.userinfo, "ip"));

	Com_sprintf(logged, sizeof logged, "%s@%s@%8s@%10s",
				user, msg, sys_time, sys_date);

	// When name doesn't match converted name
	if (log && botdetection & BOT_LOG)
	{
		AddLogEntry ("logs/bot_detected.txt", logged);
	}

	if (botdetection & BOT_NOTIFY)
	{
		safe_bprintf(PRINT_HIGH, "%s %s\n",
					 user,"was Busted By -=BOtCRuSher=-");
	}

	if(botdetection & BOT_BAN)
	{
		AddEntry ("ip_banned.txt", userip);
	}

	if (botdetection & BOT_KICK)
	{
		char	command [256];

		ent->movetype = MOVETYPE_NOCLIP;

		Com_sprintf (command, sizeof(command), "kick %s\n", ent->client->pers.netname);
		gi.AddCommandString(command);
	}
	return;
}

//void
//InitBotDetection(void)
//{
//	char fname[PATH_MAX];
//
//	Com_sprintf(fname, sizeof fname, "%s/%s/%s/bot_detected.log",
//			basedir->string, game_dir->string, cfgdir->string);
//
//	fpBot = fopen(fname, "a");
//}
//
////void
//EndBotDetection(void)
//{
//	if (fpBot)
//		fclose(fpBot);
//}
//
//int isBlank(char *buff1)
//{
//	while(*buff1 == ' ')
//	{
//		buff1++;
//	}
//	return !(*buff1);
//}
//
//qboolean CheckForConnection(edict_t *player)
//{
//	int i;
//	edict_t	*ent;
//
//	for (i=0 ; i<maxclients->value ; i++)
//	{
//		ent = g_edicts + 1 + i;
//		if (!ent->inuse || !ent->client)
//			continue;
//
//		if(strcmp(Info_ValueForKey (player->client->pers.userinfo, "ip[13]"),
//				  Info_ValueForKey (ent->client->pers.userinfo, "ip[13]")) == 0)
//			return true;
//	}
//
//	return false;
//}
