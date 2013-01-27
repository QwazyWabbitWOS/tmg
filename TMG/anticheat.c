#include "g_local.h"
#include <ctype.h>
#include "e_hook.h"//RAV
#include "runes.h"//RAV

qboolean getLogicalValue(char *arg);
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

    if (ent->client->pers.pl_state == 5)
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
	char path[260];

	strcpy (path, game_dir->string);
	if (Q_stricmp (path, "\0")==0) strcpy (path, "baseq2");
#ifdef _WIN32
	strcat (path, "\\");
	strcat (path, cfgdir->string);
	strcat (path, "\\");
#else
	strcat (path, "/");
	strcat (path, cfgdir->string);
	strcat (path, "/");
#endif
	strcat (path, filename);
	fd=fopen (path, t);
	return (fd);
}
void addEntry2 (char *filename, char ip[IP_LENGTH])
{
	FILE *ipfile;
	

	// add user to file
	strcat (ip, "\n");

	if (ipfile=tn_open(filename, "a"))
	{
		fputs(ip, ipfile);
		fclose (ipfile);
	}
	return;
}


char *ConvertName(char name[IP_LENGTH])
{
	int i=0, j=0, len =0;

	len = strlen(name);

	while (i <= len)
	{

		// convert BADCHARS to ' '

		for (j=0; j<len; j++)
			if (name[j] == '~')
				name[j] = 'x';
			else
			if (name[j] == '!')
				name[j] = 'x';
			else
			if (name[j] == '@')
				name[j] = 'x';
			else
			if (name[j] == '#')
				name[j] = 'x';
			else
			if (name[j] == '$')
				name[j] = 'x';
			else
			if (name[j] == '%')
				name[j] = 'x';
			else
			if (name[j] == '^')
				name[j] = 'x';
			else
			if (name[j] == '&')
				name[j] = 'x';
			else
			if (name[j] == '(')
				name[j] = 'x';
			else
			if (name[j] == ')')
				name[j] = 'x';
			else
			if (name[j] == '=')
				name[j] = 'x';
			else
			if (name[j] == '|')
				name[j] = 'x';
			else
			if (name[j] == '?')
				name[j] = 'x';
			else
			if (name[j] == '.')
				name[j] = 'x';
			else
			if (name[j] == '>')
				name[j] = 'x';
			else
			if (name[j] == '<')
				name[j] = 'x';
			else
			if (name[j] == ',')
				name[j] = 'x';
			else
			if (name[j] == '{')
				name[j] = 'x';
			else
			if (name[j] == '}')
				name[j] = 'x';
			else
			if (name[j] == '[')
				name[j] = 'x';
			else
			if (name[j] == ']')
				name[j] = 'x';
			else
			if (name[j] == ':')
				name[j] = 'x';
			else
			if (name[j] == ';')
				name[j] = 'x';
			else
			if (name[j] == '*')
				name[j] = 'x';
			else
			if (name[j] == '/')
				name[j] = 'x';
			else
			if (name[j] == '-')
				name[j] = 'x';
		i++;
	}
	return (name);
}
void BadPlayer(edict_t *ent)
{
	// Make sure ent exists!
  if (!G_EntExists(ent))
	  return;

	 if(ent->client->pers.pl_state != 5)
			return;

	 //set flags/lose gun ..ect
	    ent->client->newweapon = NULL;
		ChangeWeapon (ent);
       	ent->client->ps.gunindex = 0;
		ent->bust_time = 0;
		
		if(ent->client->hook)
		abandon_hook_reset(ent->client->hook);
		
		runes_drop(ent);

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

 if(ent->client->pers.pl_state == 5)
		return;

	ent->client->pers.pl_state = 5;
    
//crashbug fix (on name logging)
	strcpy (name ,ent->client->pers.netname);
	strcpy (name , ConvertName(name));

	if(!strcmp(name,ent->client->pers.netname) == 0)
		log =1;

	sprintf(user, "%s@%s", ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo, "ip"));
    sprintf(userip, "*@%s", Info_ValueForKey (ent->client->pers.userinfo, "ip"));
	sprintf(logged, "%s@%s@%s@%8s@%10s", ent->client->pers.netname, Info_ValueForKey (ent->client->pers.userinfo, "ip"), msg, sys_time, sys_date);
	

	
    if (botdetection & BOT_LOG)
	{
#if defined(linux)
		addEntry2 ("logs/bot_detected.txt", logged);
#else
		addEntry2 ("logs\\bot_detected.txt", logged);
#endif
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
InitBotDetection(void) {
    char fname[256];

#if defined(linux)
	sprintf(fname, "%s/%s/%s/bot_detected.log", basedir->string, game_dir->string, cfgdir->string);
#else
	sprintf(fname, "%s\\%s\\%s\\bot_detected.log", basedir->string, game_dir->string, cfgdir->string);
#endif
    
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

