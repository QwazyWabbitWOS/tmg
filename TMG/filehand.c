
#include "g_local.h"
#include "filehand.h"
#include "anticheat.h"

oplist_t* oplist;

/**
Display the operators and their access levels
currently held in the server.
*/
void ShowOps(void)
{
	int numentries = LoadOpFile();

	gi.dprintf("OP  %-24s      %-5s        %-9s\n", "Entry", "Level", "Password");
	gi.dprintf("--- ----------------------  ------------------ --------\n");
	for (int i = 0; i < numentries; i++) {
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
qboolean lessGeneral(char* line, char* comp)
{
	if (strcspn(line, "*") > strcspn(comp, "*"))
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
int checkAllowed(char* userinfo)
{
	FILE* ipfile;
	char line[MAX_INFO_STRING], ip[MAX_QPATH], aline[MAX_INFO_STRING];
	int status = ALLOWED; // assume the best of everyone.
	char* loopback = "127.0.0.1";

	// let loopback match.
	if (Q_strnicmp(Info_ValueForKey(userinfo, "ip"), loopback, strlen(loopback)) == 0)
		return ALLOWED;

	Com_sprintf(ip, sizeof ip, "%s@%s",
		Info_ValueForKey(userinfo, "name"),
		Info_ValueForKey(userinfo, "ip"));

	ipfile = tn_open("ip_allowed.txt", "r", false);
	if (ipfile)
	{
		while ((fgets(aline, MAX_INFO_STRING, ipfile)) && (status == 1))
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
		while (fgets(line, MAX_INFO_STRING, ipfile) && status == 0)
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
		ipfile = tn_open("ip_allowed.txt", "r", false);
		if (ipfile)
		{
			while ((fgets(aline, MAX_INFO_STRING, ipfile)) && (status == 1))
			{
				if (!strchr("#", *aline))
				{
					if (IPMatch(ip, aline) == 1 && lessGeneral(aline, line))
					{
						gi.dprintf("*** IPControl - Letting %s in:\n"
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
char* NameAndIp(edict_t* ent)
{
	static char ip[MAX_QPATH];
	char* c;

	Com_sprintf(ip, sizeof ip, "%s@%s",
		ent->client->pers.netname,
		Info_ValueForKey(ent->client->pers.userinfo, "ip"));

	c = strchr(ip, ':');
	if (c)
		*c = '\0';
	return ip;
}

// IP checking function
// Extended to also include name (raven@194.165.241.59, case sensitive)
//
int IPMatch(char* clientIP, char* maskIP)
{
	int match = 1;
	qboolean OK = true;
	qboolean psd = false;
	char* cp = clientIP;
	char* mp = maskIP;
	char* chop;

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
				if ((*cp != *mp) && (!strchr("?", *mp)))
				{
					match = 0;
					/*safe_bprintf(PRINT_HIGH, "(%c <> %c)\n", *cp, *mp);*/
				}
				cp++;
				mp++;

				if ((strchr("\0", *mp)) && (!strchr("\0", *cp)) && (!strchr(":", *cp))) { match = 0; }
				if ((strchr("\0", *cp)) && ((!strchr("\0", *mp)) || (!strchr("\n", *mp)))) match = 0;
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
			if ((*cp != *mp) && (!strchr("?", *mp)))
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

qboolean entryInFile(char* filename, char* ip)
{
	FILE* thefile;
	char line[MAX_INFO_STRING];
	qboolean inFile;

	inFile = false;

	thefile = tn_open(filename, "r", true);
	if (thefile)
	{
		while (fgets(line, sizeof line, thefile))
		{
			if (IPMatch(ip, line))
			{
				inFile = true;
			}
		}
		fclose(thefile);
	}
	return inFile;
}

// Load the user_o.txt file, populate the oplist array
// return the number of entries loaded.
int LoadOpFile(void)
{
	FILE* opfile;
	char line[MAX_INFO_STRING] = { 0 };
	char temp[MAX_INFO_STRING] = { 0 };
	int i = 0;
	int a = 0;
	char* result = NULL;

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
				while (fgets(line, sizeof line, opfile) != NULL)
				{
					oplist[i].flagged = 0;
					a = 1;
					for (result = strtok(line, " \t\n"); result != NULL; result = strtok(NULL, " \t\n"))
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
						a++;
					}
					// Update the rest of the struct. NOTE: We can't do this inside the 'for' loop.
					strcpy(temp, oplist[i].entry); // temp storage for extractions
					strcpy(oplist[i].name, strtok(temp, "@"));	// extract the name
					strcpy(temp, oplist[i].entry);
					strcpy(oplist[i].ip, strrchr(temp, '@') + 1); // extract the IP address
					i++;
				}
			}
			else
				result = NULL;
		} while (result != NULL);

		fclose(opfile);
	}
	return i;
}

int CheckOpFile(edict_t* ent, char* ip, qboolean returnindex)
{
	int i = 0;
	int flagged = -1;

	int numentries = LoadOpFile();

	for (i = 0; i < numentries; i++) {
		if (IPMatch(ip, oplist[i].entry)) {
			if (ent != NULL) {
				if (debug_ops->value) {
					gi.dprintf("Checking %s oplist[%i].namepass %s vs client->pers.namepass: %s\n",
						ent->client->pers.netname, i, oplist[i].namepass, ent->client->pers.namepass);
				}
				if (strcmp(oplist[i].namepass, ent->client->pers.namepass) == 0) {
					oplist[i].flagged = true;
					flagged = i;
					ent->client->pers.oplevel = oplist[flagged].level;
					if (ent && debug_ops->value && flagged != -1) {
						gi.dprintf("Player %s matches entry %s, level = %d/%d\n",
							ent->client->pers.netname, oplist[flagged].entry, oplist[flagged].level, ent->client->pers.oplevel);
					}
				}
				else {
					ent->client->pers.oplevel = 0;
					oplist[i].flagged = true;
					flagged = i;
					return flagged;
				}
			}
			else // We're making a modification to an operator in the list and have a match.
			{
				oplist[i].flagged = true;
				flagged = i;
			}
		}
	}
	if (flagged < 0) {
		ent->client->pers.oplevel = 0;
	}

	if (debug_ops->value)
		gi.dprintf("%s: returning %i\n", __func__, flagged);

	return flagged;
}

/**
	The oplist has two functions:
	Set operator access for those registered as ops.
	Protect registered player names.
*/
qboolean CheckNameProtect(const char* name, const char* namepass)
{
	qboolean inFile = false;
	qboolean namepassMatches = false;

	int numentries = LoadOpFile();

	for (int i = 0; i < numentries; i++)
	{
		if (strcmp(oplist[i].name, name) == 0 && (oplist[i].level & OP_NAMEPASS))
		{
			inFile = true;
			if (debug_ops->value)
				gi.dprintf("name .%s. matched entry .%s. and is in file.\n"
					"namepass in oplist is .%s., client pass is .%s.\n", name,
					oplist[i].name, Q_stricmp(oplist[i].namepass, namepass), oplist[i].namepass, namepass);

			if (strcmp(oplist[i].namepass, namepass) == 0)
			{
				namepassMatches = true;
			}
		}
	}

	if (inFile && namepassMatches)
		return false;
	else
		return true;
}


/**
 Change the operator level of designated player entry
 and write the data to the operator file.
 Returns true on success, otherwise false.
 */
qboolean ModifyOpLevel(int entry, int newlevel)
{
	FILE* opfile;
	int i = 0;
	char line[MAX_INFO_STRING];

	if (entry < 0)
		return false;

	int numentries = LoadOpFile();
	oplist[entry].level = newlevel; // Set the new level in the oplist.
	opfile = tn_open("user_o.txt", "w", true);
	if (opfile)
	{
		for (i = 0; i < numentries; i++)
		{
			Com_sprintf(line, sizeof line, "%s %d %s\n", oplist[i].entry, oplist[i].level, oplist[i].namepass);
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
int AddOperator(char entry[MAX_QPATH], int op_lev, char pass[16])
{
	FILE* opfile;
	char line[MAX_QPATH];

	opfile = tn_open("user_o.txt", "a+", true);
	if (opfile)
	{
		Com_sprintf(line, sizeof line, "%s %d %s\n", entry, op_lev, pass);
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
void AddEntry(char* filename, char* text)
{
	FILE* ipfile;

	// First, check to see that client isn't already in the file
	if (entryInFile(filename, text))
		return;

	ipfile = tn_open(filename, "a+", true);
	if (ipfile)
	{
		fputs(text, ipfile);
		fputs("\n", ipfile);
		fclose(ipfile);
	}
}

// Ban the specified player: ban user@ip
void sv_ban_ip(edict_t* ent)
{
	char banned[MAX_QPATH];

	if ((ent == NULL) || (ent->client->pers.oplevel & OP_BAN))
	{
		if (ent == NULL)
			strcpy(banned, gi.argv(2));
		else
			strcpy(banned, gi.argv(1));

		if (IPMatch(banned, "*@*.*.*.*") == 1)
		{
			safe_cprintf(ent, PRINT_HIGH, "%s is banned.\n", banned);
			gi.dprintf("%s is banned.\n", banned);
			AddEntry("ip_banned.txt", banned);
		}
		else
		{
			safe_cprintf(ent, PRINT_HIGH, "Usage: ban user@ip %s\n", banned);
			gi.dprintf("Usage: ban user@ip %s\n", banned);
		}
	}
	else
		safe_cprintf(ent, PRINT_HIGH, "You must be an operator to ban users.\n");
}

//JSW
/**
 Show the content of the operators list file
 to the client who requested it.
 */
void ShowFile(edict_t* ent, char* filename)
{
	FILE* file;
	int c;	// variable to hold temporary file data

	file = tn_open(filename, "r", true);
	if (file == NULL)
	{	// file did not open
		if (ent == NULL)
			gi.dprintf("Could not load file %s.\n", filename);
		else
			gi.cprintf(ent, PRINT_HIGH,
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

void LogConnect(edict_t* ent, qboolean connect)
{
	FILE* file;
	char file_name[MAX_QPATH];
	char client[256];

	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/logs/connect.log",
		basedir->string, game_dir->string, cfgdir->string);

	if (!ent)
	{
		file = fopen(file_name, "a");
		if (file)
			fclose(file);
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
		fclose(file);
	}
	else
		gi.dprintf("Could not open connect log!\n");
}


void LogChat(char* text)
{
	FILE* file;
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
		fclose(file);
	}
	else
	{
		gi.dprintf("Could not open chat log! using: %s\n", file_name);
		gi.dprintf("Please make sure you have a logs folder in your cfg directory.\n");
	}
}

