
#include "g_local.h"
#include "filehand.h"
#include "bot.h"

void sv_addop (edict_t *ent);
//void sv_addmop (edict_t *ent);


//ルート修正
//ノーマルポッドは全て切り捨て
void Move_LastRouteIndex(void)
{
	int	i;

	for(i = CurrentIndex - 1 ; i >= 0;i--)
	{
		if(Route[i].state)
			break;
		else if(!Route[i].index)
			break;
	}
	if(!CurrentIndex || !Route[i].index)
		CurrentIndex = i;
	else
		CurrentIndex = i + 1;

	if(CurrentIndex < MAXNODES)
	{
		memset(&Route[CurrentIndex], 0, sizeof(route_t));
		if(CurrentIndex > 0)
			Route[CurrentIndex].index = Route[CurrentIndex - 1].index + 1;
	}
}

//分岐付きに変換処理
//static void	RouteTreepointSet(void)
//{
//	int	i;
//
//	for(i = 0;i < CurrentIndex;i++)
//	{
//		if(Route[i].state == GRS_NORMAL)
//		{
//			// something...
//		}
//	}
//}

//chainファイルのセーブ
static void SaveChain(void)
{
	char name[256];
	FILE *fpout;
	unsigned int size;

	if(!chedit->value)
	{
		gi.cprintf (NULL, PRINT_HIGH, "Not a chaining mode.\n");
		return;
	}

	sprintf(name,"%s/%s/%s/nav/%s.nav",
			basedir->string, game_dir->string,
			cfgdir->string,level.mapname);

	fpout = fopen(name,"wb");
	if(fpout == NULL)
		gi.cprintf(NULL,PRINT_HIGH,"Can't open %s\n",name);
	else
	{
		if(!ctf->value)	fwrite("3ZBRGDTM",sizeof(char),8,fpout);
		else fwrite("3ZBRGCTF",sizeof(char),8,fpout);

		fwrite(&CurrentIndex,sizeof(int),1,fpout);

		size = (unsigned int)CurrentIndex * sizeof(route_t);

		fwrite(Route,size,1,fpout);

		gi.cprintf (NULL, PRINT_HIGH,"%s Saving done.\n",name);
		fclose(fpout);
	}
}

//Spawn Command
static void SpawnCommand(int i)
{
	int	j;

	if(chedit->value)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Can't spawn.");
		return;
	}

	if(i <= 0)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Specify num of bots.");
		return;
	}

	for(j = 0; j < i; j++)
	{
		SpawnBotReserving();
	}
}

//Random Spawn Command

static void RandomSpawnCommand(int i)
{
	int	j,k,red = 0,blue = 0;

	edict_t	*e;

	if(chedit->value)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Can't spawn.");
		return;
	}

	if(i <= 0)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Specify num of bots.");
		return;
	}

	//count current teams
	for ( k = 1 ; k <= maxclients->value ; k++)
	{
		e = &g_edicts[k];
		if(e->inuse && e->client)
		{
			if(e->client->resp.ctf_team == CTF_TEAM1) red++;
			else if(e->client->resp.ctf_team == CTF_TEAM2) blue++;
		}
	}

	for(j = 0;j < i;j++)
	{
		SpawnBotReserving2(&red,&blue);
		//gi.cprintf(NULL,PRINT_HIGH,"R B %i %i\n",red,blue);
	}
}

//Remove Command
static void RemoveCommand(int i)
{
	int	j;

	if(i <= 0)
		i = 1;
		//gi.cprintf(NULL,PRINT_HIGH,"Specify num of bots.");


	for(j = 0;j < i;j++)
	{
		RemoveBot();
	}
}

//Debug Spawn Command
static void DebugSpawnCommand(int i)
{
	if(!chedit->value)
	{
		gi.cprintf(NULL,PRINT_HIGH,"Can't debug.");
		return;
	}

	//	if(targetindex) {gi.cprintf(NULL,PRINT_HIGH,"Now debugging.");return;}

	if(i < 1) i = 1;

	targetindex = i;

	SpawnBotReserving();
}

//JSW
static void Svcmd_Stuff (void)
{
	char	buff[256];
	char	*stuff;
	char	*s;
	edict_t	*ent;
	int id = -1;

	if (gi.argc() < 4)
	{
		gi.dprintf ("SV Stuff: Stuff any command to a client.\nUsage: sv stuff <userid> <command to stuff>\n");
		return;
	}

	if (!Find_Player_Edict_t(gi.argv(2)))
	{
		id = atoi(gi.argv(2));
		if (Q_stricmp(gi.argv(2), "0") != 0 && id == 0)
		{
			gi.dprintf("Player not found\n");
			return;
		}
		else
			ent = g_edicts + 1 + id;
	}
	else
		ent = Find_Player_Edict_t(gi.argv(2));

	if (!G_EntExists(ent))
	{
		gi.dprintf ("Player not found\n");
		return;
	}

	s = gi.args();
	stuff = s +  strlen(gi.argv(1)) + strlen(gi.argv(2)) + 2;
	sprintf (buff, "%s\n", stuff);
	gi.dprintf ("stuffing to client: %s\n", ent->client->pers.netname);
	gi.cprintf (ent, PRINT_HIGH, "You were stuffed the following command by the server admin: %s\n", buff);
	stuffcmd(ent, buff);
}

/**
 ==========
 Svcmd_Msg - Prints out a message to all clients to get their attention.
 ==========
 */
static void Svcmd_Msg (void)
{
	int strnum = gi.argc();
	int line = 1;
	char msg [480], msg1[40], msg2[40], msg3[40], msg4[40], msg5[40];
	int i, j, k = 0, l = 0, m = 0, n = 0, o;
	edict_t *e;

	if (gi.argc() < 3)
	{
		gi.dprintf ("SV Msg: Prints a message to all clients from admin.\n");
		return;
	}

	j =  sprintf(msg1, "%s", gi.argv(2));

	for (i = 3; i < strnum + 1; i++)
	{
		switch (line)
		{
			case 1:
			{
				if (strlen(msg1) + strlen(gi.argv(i)) > 39)
				{
					k = sprintf(msg2, "%s", gi.argv(i));
					line = 2;
					break;
				}
				j += sprintf(msg1 + j, " %s", gi.argv(i));
				break;
			}
			case 2:
			{
				if (strlen(msg2) + strlen(gi.argv(i)) > 39)
				{
					l = sprintf(msg3, "%s", gi.argv(i));
					line = 3;
					break;
				}
				k += sprintf(msg2 + k, " %s", gi.argv(i));
				break;
			}
			case 3:
			{
				if (strlen(msg3) + strlen(gi.argv(i)) > 39)
				{
					m = sprintf(msg4, "%s", gi.argv(i));
					line = 4;
					break;
				}
				l += sprintf(msg3 + l, " %s", gi.argv(i));
				break;
			}
			case 4:
			{
				if (strlen(msg4) + strlen(gi.argv(i)) > 39)
				{
					n = sprintf(msg5, "%s", gi.argv(i));
					line = 5;
					break;
				}
				m += sprintf(msg4 + m, " %s", gi.argv(i));
				break;
			}
			case 5:
			{
				if (strlen(msg5) + strlen(gi.argv(i)) > 39)
				{
					gi.dprintf("Message exceeds max length, message will be truncated.\n");
					line = 6;
					break;
				}
				n += sprintf(msg5 + n, " %s", gi.argv(i));
				break;
			}
			default:
			{
				break;
			}
		}
	}

	convert_string(msg1, 0, 127, 128, msg1);
	convert_string(msg2, 0, 127, 128, msg2);
	convert_string(msg3, 0, 127, 128, msg3);
	convert_string(msg4, 0, 127, 128, msg4);
	convert_string(msg5, 0, 127, 128, msg5);

	o = sprintf(msg, "%s\n", msg1);
	if (line > 1)
		o += sprintf (msg + o, "%s\n", msg2);
	if (line > 2)
		o += sprintf (msg + o, "%s\n", msg3);
	if (line > 3)
		o += sprintf (msg + o, "%s\n", msg4);
	if (line > 4)
		o += sprintf (msg + o, "%s\n", msg5);

	for_each_player(e, i)
	{
		safe_centerprintf(e, "Message from server administrator:\n\n%s", msg);
		gi.sound (e, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NONE, 0);
	}

}

/*
 =================
 ServerCommand

 ServerCommand will be called when an "sv" command is issued.
 The game can issue gi.argc() / gi.argv() commands to get the rest
 of the parameters
 =================
 */
void ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);

	if (Q_stricmp (cmd, "ban") == 0)
		sv_ban_ip (NULL);
	//end
	else if (Q_stricmp (cmd, "savechain") == 0)
		SaveChain ();
	else if (Q_stricmp (cmd, "spb") == 0)
	{
		if(gi.argc() <= 1) SpawnCommand(1);
		else SpawnCommand (atoi(gi.argv(2)));
	}
	else if (Q_stricmp (cmd, "rspb") == 0)
	{
		if(gi.argc() <= 1) RandomSpawnCommand(1);
		else RandomSpawnCommand (atoi(gi.argv(2)));
	}
	else if (Q_stricmp (cmd, "rmb") == 0)
	{
		if(gi.argc() <= 1) RemoveCommand(1);
		else RemoveCommand (atoi(gi.argv(2)));
	}
	else if (Q_stricmp (cmd, "dsp") == 0)
	{
		if(gi.argc() <= 1) DebugSpawnCommand(1);
		else DebugSpawnCommand (atoi(gi.argv(2)));
	}
	//JSW
	else if (Q_stricmp (cmd, "msg") == 0)
		Svcmd_Msg ();
	else if (Q_stricmp (cmd, "restart") == 0)
		RestartLevel();
	else if (Q_stricmp (cmd, "lockserver") == 0)
	{
		serverlocked = true;
		gi.dprintf ("Server was LOCKED\n");
	}
	else if (Q_stricmp (cmd, "unlockserver") == 0)
	{
		serverlocked = false;
		gi.dprintf ("Server was UNLOCKED\n");
	}
	else if (Q_stricmp (cmd, "serverlock") == 0)
	{
		if (serverlocked)
			gi.dprintf ("Server is LOCKED\n");
		else
			gi.dprintf ("Server is UNLOCKED\n");
	}
	else if (Q_stricmp (cmd, "showopfile") == 0)
	{
		ShowFile (NULL, "user_o.txt");
	}
	else if (Q_stricmp (cmd, "showbannedfile") == 0)
	{
		ShowFile (NULL, "ip_banned.txt");
	}
	else if (Q_stricmp (cmd, "modop") == 0)
	{
		if (IPMatch (gi.argv(2),  "*@*.*.*.*") == 1 && gi.argc() == 4)
		{
			if (ModifyOpLevel(CheckOpFile (NULL, gi.argv(2), true), atoi(gi.argv(3))))
				gi.dprintf ("%s level changed to %s\n", gi.argv(2), gi.argv(3));
			else
				gi.dprintf ("No matching entry found.\n");
		}
		else
			gi.dprintf("Usage: modop user@ip newlevel\n");
	}
	else if (Q_stricmp(cmd, "addop") == 0)
	{
		int level;
		char pass[16];
		if (IPMatch (gi.argv(2),  "*@*.*.*.*") == 1)
		{
			if (gi.argc() < 4)
				level = (int)defaultoplevel->value;
			else
				level = atoi(gi.argv(3));
			if (gi.argc() < 5)
				sprintf(pass, "nopass");
			else
				sprintf(pass, "%s", gi.argv(4));
			if (AddOperator (gi.argv(2), level, pass) == 0)
				gi.dprintf ("%s added to user_o.txt with level %d and password %s.\n", gi.argv(2), level, pass);
			else
				gi.dprintf ("Error adding user\n");
		}
		else
			gi.dprintf("Usage: addop user@ip level password\n");
	}
	else if (Q_stricmp(cmd, "showops") == 0)
	{
		int i;
		//		gi.dprintf("entriesinopfile = %d\n", entriesinopfile);
		//		gi.dprintf("Showing oplist entries...\n\nEntry            Level\n");
		for (i=0; i<entriesinopfile; i++)
		{
			gi.dprintf("%d. %s\t%d\t%s\n", i+1, oplist[i].entry, oplist[i].level, oplist[i].namepass);
		}
		gi.dprintf("\n");
	}
	else if (Q_stricmp (cmd, "stuff") == 0)
		Svcmd_Stuff ();

	//	else if (Q_stricmp (cmd, "reloadmaps") == 0)
	//		map_mod_set_up();

	//end
	else
		gi.dprintf ("Unknown server command \"%s\"\n", cmd);
}

