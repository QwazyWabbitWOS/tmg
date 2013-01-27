//<<highscore.c>>

#include "g_local.h"

#define SCORESTOKEEP 10
    
typedef struct {
	char	netname[16];
	int		score;
	char  	date[10];
} HS_STRUCT;

// room to hold max # of players
HS_STRUCT g_TopScores[SCORESTOKEEP];

// used for the
int MP_Sort(const void *a, const void *b)
{
	return (((HS_STRUCT *)b)->score - ((HS_STRUCT *)a)->score);
}

void highscore (void)
{
	int i,   count=0;
	edict_t  *cl_ent;
	FILE     *HS_file;
	char     filename[256], filename2[256];
	char	string[128];
	
	i =  sprintf(filename, "./");
	i += sprintf(filename + i, "%s/%s", game_dir->string, cfgdir->string);
	i += sprintf(filename + i, "/hs/%s_hs.bin", level.mapname);
	i =  sprintf(filename2, "./");
	i += sprintf(filename2 + i, "%s/%s", game_dir->string, cfgdir->string);
	i += sprintf(filename2 + i, "/hs/%s_hs.txt", level.mapname);
	
	// is the high score file for this map already loaded?
	// no - load it
	if(HS_file = fopen(filename, "rb"))
	{
		fread(g_TopScores, sizeof(g_TopScores[0]) * SCORESTOKEEP, 1, HS_file);
		fclose(HS_file);

		//JSW
		// HS_file loaded - see if any entity made the list
		for (i = 0 ; i < maxclients->value ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if((game.clients[i].pers.pl_state == 1 || cl_ent->client->pers.pl_state == 2)
				&& (game.clients[i].ps.stats[STAT_FRAGS] > g_TopScores[SCORESTOKEEP-1].score)
				&& (game.clients[i].ps.stats[STAT_FRAGS] > 0))
			{ // if it beat the lowest, keep score
				//my_bprintf (PRINT_HIGH, "High scores changed\n");
				strcpy(g_TopScores[SCORESTOKEEP-1].netname, game.clients[i].pers.netname);
				g_TopScores[SCORESTOKEEP-1].score = game.clients[i].ps.stats[STAT_FRAGS];
				strcpy(g_TopScores[SCORESTOKEEP-1].date , sys_date);
				// sort it
				qsort(g_TopScores, sizeof(g_TopScores)/sizeof(g_TopScores[0]), sizeof(g_TopScores[0]), MP_Sort);
			}
		}
		//end
	}
	else
	{
		// if it doesnt exist, create it with the top current players in it
		memset(g_TopScores, 0, sizeof(g_TopScores));
		count=0;
		for (i = 0 ; i < maxclients->value; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (cl_ent->inuse && (cl_ent->client->pers.pl_state == 1 || cl_ent->client->pers.pl_state == 2))
			{
				strcpy(g_TopScores[count].netname, game.clients[i].pers.netname);
				g_TopScores[count].score = game.clients[i].ps.stats[STAT_FRAGS];
				strcpy(g_TopScores[count].date , sys_date);
				count++;
				if (count >= SCORESTOKEEP)
					break;
			}
		}
		
		// sort it
		qsort(g_TopScores, sizeof(g_TopScores)/sizeof(g_TopScores[0]), sizeof(g_TopScores[0]), MP_Sort);
	}

	// write the high score HS_file
	HS_file = fopen(filename, "wb");
	if (HS_file != NULL)
	{
		fwrite(g_TopScores, sizeof(g_TopScores[0]), SCORESTOKEEP, HS_file);
		fclose(HS_file);
	}

	// print top scores to a readable file
	HS_file = fopen(filename2, "wt");
	if (HS_file != NULL)
	{
		sprintf(string, "    Top %d Scores for %s\n\n", SCORESTOKEEP, level.mapname);
		convert_string(string, 0, 127, 128, string);
		//fprintf(HS_file,"    Top %d Scores for %s\n\n", SCORESTOKEEP, level.mapname);
		fprintf(HS_file, string);
		for (i = 0; i < SCORESTOKEEP; i++)
			fprintf(HS_file, "  %2d - %8s - %i - %-12.12s\n", i+1, g_TopScores[i].date , g_TopScores[i].score, g_TopScores[i].netname);
		fprintf(HS_file,"\n     %s  %s\n", MOD, MOD_VERSION);
		fprintf(HS_file,"              www.railwarz.com");
		fclose(HS_file);
	}
}

void LoadHighScores (void)
{
	char	entry[1400];
	char	string[1400];
	int		stringlength;
	int   i, j;
	FILE    *motd_file;
	char    filename[256];
	char    line[80];
	
	i =  sprintf(filename, "./");
	i += sprintf(filename + i, "%s/%s", game_dir->string, cfgdir->string);
	i += sprintf(filename + i, "/hs/%s_hs.txt", level.mapname);
	if (!(motd_file = fopen(filename, "r")))
		return;
	string[0] = 0;
	stringlength = strlen(string);
	i = 0;
	while ( fgets(line, 80, motd_file) )
	{
		Com_sprintf (entry, sizeof(entry), "xv 2 yv %i string \"%s\" ", i*8 + 24, line);
		j = strlen(entry);
		if (stringlength + j > 1400)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
		i++;
	}
	// be good now ! ... close the file
	fclose(motd_file);
	j = strlen(entry);
	if (stringlength + j < 1400)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	Com_sprintf (hscores, sizeof(hscores), string);
}

