
#include "g_local.h"
#include "filehand.h"
#include "anticheat.h"

oplist_t	*oplist;
oplist_t	*oplistBase;

int entriesinopfile;

/**
Display the operators and their access levels
currently held in the server.
*/
void ShowOps(void)
{
	gi.dprintf("OP  %-24s      %-5s        %-9s\n", "Entry", "Level","Password");
	gi.dprintf("--- ----------------------  ------------------ --------\n");
	for (int i = 0; i < entriesinopfile; i++) {
		gi.dprintf("%d.  %-24s      %-6d       %s\n",
			i + 1, oplist[i].entry, oplist[i].level, oplist[i].namepass);
	}
	gi.dprintf("\n");
}

/**
 Used to decide if whitelist entry is less general
 than the blacklist entry. If true then checkAllowed
 decides to let them in.
 */
qboolean lessGeneral(char *line, char *comp)
{
	if (strcspn (line, "*") > strcspn(comp, "*"))
		return true;
	else
		return false;
}

#define ALLOWED 0
#define BANNED 1

/**
 Matches client IP to whitelist and blacklist files.
 Returns 1 if not allowed to connect.
 Returns 0 if he's a goodguy.
 */
int checkAllowed (char *userinfo)
{
	FILE *ipfile;
	char line[MAX_QPATH], ip[MAX_QPATH], aline[MAX_QPATH];
	int status = ALLOWED; // assume the best of everyone.
	char* loopback = "127.0.0.1";

	// let loopback match.
	if (Q_strnicmp(Info_ValueForKey (userinfo, "ip"), loopback, strlen(loopback)) == 0)
		return ALLOWED;

	Com_sprintf(ip, sizeof ip, "%s@%s",
		Info_ValueForKey (userinfo, "name"),
		Info_ValueForKey (userinfo, "ip"));

	ipfile = tn_open("ip_allowed.txt", "r", false);
	if (ipfile)
	{
		while ((fgets(aline, MAX_QPATH, ipfile)) && (status == 1))
		{
			if (!strchr("#", *aline))
			{
				if (IPMatch(ip, aline) == 1)
					status = ALLOWED;
			}
		}
		fclose(ipfile);
	}
	else
		status = ALLOWED; // No allowed file, admit all.

	ipfile = tn_open("ip_banned.txt", "r", false);
	if (ipfile)
	{
		while (fgets(line, MAX_QPATH, ipfile) && status == 0)
		{
			if (!strchr("#", *line))
			{
				if (IPMatch(ip, line) == 1)
					status = BANNED;
			}
		}
		fclose(ipfile);
	}

	if (status == BANNED)
	{
		ipfile = tn_open("ip_allowed.txt","r", false);
		if (ipfile)
		{
			while ((fgets(aline, MAX_QPATH, ipfile)) && (status == 1))
			{
				if (!strchr("#", *aline))
				{
					if (IPMatch(ip, aline) == 1 && lessGeneral(aline,line))
					{
						gi.dprintf ("*** IPControl - Letting %s in:\n"
							"less general than %s.\n", aline, line);
						// gi.dprintf ("%s, %s <->%s\n", ip, aline, line);
						status = ALLOWED;
					}
				}
			}
			fclose(ipfile);
		}
	}
	return status; // 1 if client is banned, otherwise 0.
}

/**
 Concatenate name and ip address of
 designated client, chopping off 
 the port from the ip address.
 Return pointer to resulting string.
*/
char *NameAndIp (edict_t *ent)
{
	static char ip[MAX_QPATH];
	char *c;

	Com_sprintf(ip, sizeof ip, "%s@%s",
		ent->client->pers.netname,
		Info_ValueForKey (ent->client->pers.userinfo, "ip"));

	c = strchr(ip, ':');
	if(c)
		*c = '\0';
	return ip;
}

// IP checking function
// Extended to also include name (raven@194.165.241.59, case sensitive)
//
int IPMatch (char *clientIP, char *maskIP)
{
	int match = 1;
	qboolean OK = true;
	qboolean psd = false;
	char *cp = clientIP;
	char *mp = maskIP;
	char *chop;

	// Remove \n
	for (chop = mp; *chop; chop++)
		if (*chop == '\n')
			*chop = '\0';

	if (*mp == '\0')
	{
		OK = false;
		match = 0;
	}

	// Name
	while (OK && !psd)
	{
		if (strchr("*", *mp)) // asterisk; go to '@'
		{
			while ((!strchr("@", *mp)) && (!strchr("\0", *mp)) && (!strchr("\n", *mp))) mp++;
			while ((!strchr("@", *cp)) && (!strchr("\0", *cp)) && (!strchr("\n", *cp))) cp++;
			if (strchr("\n", *mp))
				OK = false;
		}
		if (OK)
		{
			if (strchr("@", *cp))
			{
				psd = true;
				if (!strchr("@", *mp))
					match = 0;
			}
			else
			{
				if ((*cp != *mp) && (!strchr ("?", *mp)))
				{
					match = 0;
					/*safe_bprintf(PRINT_HIGH, "(%c <> %c)\n", *cp, *mp);*/
				}
				cp++;
				mp++;

				if ((strchr("\0", *mp)) && (!strchr("\0", *cp)) && (!strchr(":", *cp))) { match=0; }
				if ((strchr("\0", *cp)) && ((!strchr("\0", *mp)) || (!strchr("\n", *mp)))) match=0;
				if (match != 1)
					OK = false;
			}
		}
	}

	// IP
	while (OK)
	{
		if (strchr("*", *mp)) // asterisk; go to '@' or next '.'
		{
			while ((!strchr(".", *mp)) && (!strchr("\0", *mp)) && (!strchr("\n", *mp))) mp++;
			while ((!strchr(".", *cp)) && (!strchr("\0", *cp)) && (!strchr("\n", *cp))) cp++;
			if (strchr("\n", *mp))
				OK = false;
		}
		if (OK)
		{
			if ((*cp != *mp) && (!strchr ("?", *mp)))
			{
				match = 0;
				/*safe_bprintf(PRINT_HIGH,"(%c <> %c)\n",*cp, *mp);*/
			}
			cp++;
			mp++;
			if ((strchr("\0", *mp)) && (!strchr("\0", *cp)) && (!strchr(":", *cp))) { match = 0; }
			if ((strchr("\0", *cp)) && ((!strchr("\0", *mp)) || (!strchr("\n", *mp)))) match = 0;
			if (match != 1) OK = false;
		}
	}
	return (match); // 1 if IP's match, otherwise 0.
}

qboolean entryInFile (char *filename, char ip[MAX_QPATH])
{
	FILE *thefile;
	char line[MAX_QPATH];
	qboolean inFile;

	inFile = false;

	thefile = tn_open(filename, "r", true);
	if (thefile)
	{
		while (fgets(line, MAX_QPATH, thefile))
		{
			if (IPMatch(ip, line))
			{
				inFile = true;
			}
		}
		fclose (thefile);
	}
	return inFile;
}

/**
 QwazyWabbit says:
 This is one of those "does everything" functions.

 Called from InitGame to initialize the oplist:
	CheckOpFile(NULL, "*@*.*.*.*", false);

 It has 3 phases where it passes over the oplist file tokens.
 1. Fill the oplist entry with the operators name and IP.
 2. Fill the oplist entry with the operator permissions.
 3. Fill the oplist entry with the operators password.
 Repeat for all lines in the operator file to a max
 of 64 operator entries. (See oplistBase allocation.)
 However, this function seems to not care how many
 entries are available in the oplist array. Returns 0
 on completion.
 
 It takes a non-null entity pointer and a client IP string and
 matches the name and ip range to the entry in the OP file. 
 If a match is found it alters the ent->client->pers.oplevel
 for that client and returns 1.
  
 If returnindex is 1, it returns the index to their 
 place in the file if the file exists and if they're in it.
 If returnindex is 0, it returns 1 if found, 0 if not found,
 -1 on error. Jeezus.

 For all other cases it sets ent->client->pers.oplevel = 0 and 
 returns 0 for not found.

 Returns -1 if oplist file could not be opened.
 */
int CheckOpFile (edict_t *ent, char* ip, qboolean returnindex)
{
	FILE *opfile;
	char line[MAX_QPATH];
	qboolean inFile;
	int i = 0;
	int flagged = -1;
	int a;
	char *result = NULL;
	inFile = false;

	opfile = tn_open("user_o.txt", "r", true);
	if (!opfile)
	{
		return -1;
	}
	else
	{
		do
		{
			if (!feof(opfile))
			{
				while(fgets(line, MAX_QPATH, opfile) != NULL )
				{
					oplist[i].flagged = 0;
					a = 1;
					for (result = strtok(line," \t\n"); result != NULL; result = strtok(NULL," \t\n"))
					{
						if (a == 1)
						{
							strcpy(oplist[i].entry, result);
						}
						else if (a == 2 && atoi(result))
						{
							oplist[i].level = atoi(result);
						}
						else if (a == 3)
						{
							strcpy(oplist[i].namepass, result);
						}

						int ipmatch = IPMatch(ip, oplist[i].entry);
						if (ipmatch)
						{
							if (ent != NULL) // If we're authenticating the client
							{
								if (Q_strnicmp(oplist[i].namepass, ent->client->pers.namepass, strlen(ent->client->pers.namepass)) == 0)
								{
									inFile = true;
									oplist[i].flagged = true;
									flagged = i;
								}
							}
							else // We're making a modification to an operator in the list and have a match.
							{
								inFile = true;
								oplist[i].flagged = true;
								flagged = i;
							}
						}
						a++;
					}
					i++;
					entriesinopfile = i;
				}
			}
			else
				result = NULL;
		} while (result != NULL);

		fclose (opfile);
		if (ent == NULL)
		{
			if (inFile == true)
			{
				if (returnindex == true)
					return flagged;
				else
					return 1;
			}
			else
			{
				if (returnindex == true)
					return -1;
				else
					return 0;
			}
		}
	}

	if (flagged < 0)
	{
		ent->client->pers.oplevel = 0;
	}
	else
	{
		ent->client->pers.oplevel = oplist[flagged].level;
		strcpy(ent->client->pers.namepass, oplist[flagged].namepass);
	}
	if (debug_ops->value && flagged != -1)
		DbgPrintf ("Player %s matches entry %s, level = %d\n",
		ent->client->pers.netname, oplist[flagged].entry, oplist[flagged].level);

	return flagged;
}

qboolean CheckNameProtect (char name[MAX_QPATH], char namepass[MAX_QPATH])
{

	FILE *opfile;
	char line[MAX_QPATH];
	qboolean inFile;
	int i = 0;
	int a;
	char *result = NULL;
	qboolean namepassMatches;

	inFile = false;
	namepassMatches = false;
	i = 0;

	opfile = tn_open("user_o.txt", "r", true);
	if (!opfile)
		return true;
	else
	{
		do
		{
			if (!feof(opfile))
			{
				while(fgets(line, MAX_QPATH, opfile) != NULL )
				{
					oplist[i].flagged = 0;
					a = 1;
					for (result = strtok(line," @\t\n");result != NULL; result = strtok(NULL," \t\n"))
					{
						if (a == 1)
						{
							strcpy(oplist[i].name, result);
						}
						else if (a == 2)
						{
							strcpy(oplist[i].ip, result);
						}
						else if (a == 3 && atoi(result))
						{
							oplist[i].level = atoi(result);
						}
						else if (a == 4)
						{
							strcpy(oplist[i].namepass, result);
						}

						if (Q_stricmp(oplist[i].name, name) == 0 && (oplist[i].level & OP_NAMEPASS))
						{
							inFile = true;
							if (debug_ops->value)
								DbgPrintf("name .%s. matched entry .%s. and is in file.\n"
									"Q_stricmp returns value %d, "
									"namepass in oplist is .%s., "
									"client pass is .%s.\n", name,
									oplist[i].name,
									Q_stricmp(oplist[i].namepass, namepass),
									oplist[i].namepass, namepass);
							if (Q_strnicmp(oplist[i].namepass, namepass, strlen(namepass)) == 0)
							{
								namepassMatches = true;
							}
						}
						a++;
					}
					i++;
				}
			}
			else
				result = NULL;
		}

		while (result != NULL);
		fclose (opfile);

		if (inFile)
		{
			if (namepassMatches)
			{
				//gi.dprintf("name and password match!\n");
				return true;
			}
			else
			{
				//gi.dprintf("name is protected and didn't match password!\n");
				return false;
			}
		}
		else
		{
			//gi.dprintf("name was not in op file!\n");
			return true;
		}
	}
}

/**
 Change the operator level of designated player entry
 and write the data to the operator file.
 Returns true on success, otherwise false.
 */
qboolean ModifyOpLevel (int entry, int newlevel)
{
	FILE *opfile;
	int i = 0;
	char line[MAX_QPATH];

	if (entry < 0)
		return false;

	oplist[entry].level = newlevel; // Set the new level in the oplist.
	opfile = tn_open("user_o.txt", "w", true);
	if (opfile)
	{
		for (i = 0; i < entriesinopfile; i++)
		{
			Com_sprintf (line, sizeof line, "%s %d %s\n", oplist[i].entry, oplist[i].level, oplist[i].namepass);
			fputs(line, opfile);
		}
		fclose(opfile);
		return true;
	}
	return false;
}

/**
 Add operator access level for named user @ IP address.
 If user_o.txt file doesn't exist, create it.
*/
int AddOperator (char entry[MAX_QPATH], int op_lev, char pass[16])
{
	FILE *opfile;
	char line[MAX_QPATH];

	opfile = tn_open("user_o.txt", "a+", true);
	if (opfile)
	{
		Com_sprintf (line, sizeof line, "%s %d %s\n", entry, op_lev, pass);
		fputs(line, opfile);
		fclose(opfile);
		CheckOpFile(NULL, "*@*.*.*.*", false);
		return 0;
	}
	return 1;
}

/**
 Add IP of client to the file if he's not
 already in it. If file doesn't exist, create one.
 */
void AddEntry (char *filename, char *text)
{
	FILE *ipfile;

	// First, check to see that client isn't already in the file
	if (entryInFile (filename, text))
		return;

	ipfile = tn_open(filename, "a+", true);
	if (ipfile)
	{
		fputs(text, ipfile);
		fputs("\n", ipfile);
		fclose (ipfile);
	}
}

// Ban the specified player: ban user@ip
void sv_ban_ip (edict_t *ent)
{
	char banned[MAX_QPATH];

	if ((ent == NULL) || (ent->client->pers.oplevel & OP_BAN))
	{
		if (ent==NULL)
			strcpy (banned, gi.argv(2));
		else
			strcpy (banned, gi.argv(1));

		if (IPMatch (banned,  "*@*.*.*.*") == 1)
		{
			safe_cprintf (ent, PRINT_HIGH, "%s is banned.\n", banned);
			gi.dprintf ("%s is banned.\n", banned);
			AddEntry ("ip_banned.txt", banned);
		}
		else
		{
			safe_cprintf (ent, PRINT_HIGH, "Usage: ban user@ip %s\n", banned);
			gi.dprintf ("Usage: ban user@ip %s\n", banned);
		}
	}
	else
		safe_cprintf (ent, PRINT_HIGH, "You must be an operator to ban users.\n");
}

//JSW
/**
 Show the content of the operators list file
 to the client who requested it.
 */
void ShowFile(edict_t *ent, char *filename)
{
	FILE	*file;
	int c;	// variable to hold temporary file data

	file = tn_open(filename, "r", true);
	if (file == NULL)
	{	// file did not open
		if (ent == NULL)
			gi.dprintf("Could not load file %s.\n", filename);
		else
			gi.cprintf (ent, PRINT_HIGH,
			"Could not load file %s.\n", filename);
		return;
	}
	else
		// file did load
		if (ent == NULL)
			gi.dprintf("\nLoading %s...\n"
			"\n------------------------------\n\n", filename);
		else
			gi.cprintf(ent, PRINT_HIGH, "\nLoading %s...\n"
			"\n------------------------------\n\n", filename);

	while ((c = fgetc(file)) != EOF)
	{
		if (ent == NULL)
			gi.dprintf("%c", c);
		else
			gi.cprintf(ent, PRINT_HIGH, "%c", c);	// output 'c'
	}

	if (ent == NULL)
		gi.dprintf(
		"\n------------------------------\n"
		"\nEnd of %s\n\n", filename);
	else
		gi.cprintf(ent, PRINT_HIGH,
		"\n------------------------------\n"
		"\nEnd of %s\n\n", filename);
	fclose(file);
}

void LogConnect (edict_t *ent, qboolean connect)
{
	FILE *file;
	char file_name[MAX_QPATH];
	char client[256];

	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/logs/connect.log",
			basedir->string, game_dir->string, cfgdir->string);

	if (!ent)
	{
		file = fopen(file_name, "a");
		if (file)
			fclose (file);
		else
		{
			gi.dprintf("Error opening/creating %s!\n", file_name);
			gi.dprintf("Please make sure you have a logs folder in your cfg directory.\n");
		}
		return;
	}

	if (ent->bot_client)
		return;

	Com_sprintf(client, sizeof client,
		"%s [%s] %s", sys_date,
		sys_time, NameAndIp(ent));

	if (connect)
		strcat(client, " connected\n");
	else
		strcat(client, " disconnected\n");


	file = fopen(file_name, "a");
	if (file)
	{
		fputs(client, file);
		fclose (file);
	}
	else
		gi.dprintf("Could not open connect log!\n");
}


void LogChat (char *text)
{
	FILE *file;
	char file_name[MAX_OSPATH];
	char entry[2048];

	if (text == NULL)
	{
		gi.dprintf("%s text argument was NULL\n", __func__);
		return;
	}

	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/logs/%schat.log",
			basedir->string, game_dir->string, cfgdir->string, sys_date);

	Com_sprintf(entry, sizeof entry, "%s [%s] %s", sys_date, sys_time, text);
	white_text(entry, entry);

	file = fopen(file_name, "a");
	if (file)
	{
		fputs(entry, file);
		fclose (file);
	}
	else
	{
		gi.dprintf("Could not open chat log! using: %s\n", file_name);
		gi.dprintf("Please make sure you have a logs folder in your cfg directory.\n");
	}
}

