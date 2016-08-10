///<highscore.c>>

#include "g_local.h"
#include "highscore.h"
#include "performance.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define SCORESTOKEEP 10

cvar_t  *highscores;
cvar_t	*show_highscores;

// global 
char hscores [1000];


qboolean hs_show;

static int MP_Sort(const void *a, const void *b);

typedef struct {
	char	netname[16];
	int		score;
	char  	date[12];
} HS_STRUCT;

// room to hold max # of players
HS_STRUCT g_TopScores[SCORESTOKEEP];

void InitHighScores (void)
{
	highscores = gi.cvar ("highscores", "1", CVAR_LATCH);
	show_highscores = gi.cvar ("show_highscores", "0", CVAR_LATCH);
}

void SaveHighScores (void)
{
	int		i;
	edict_t	*cl_ent;
	FILE	*HS_file;
	char	binfile[MAX_QPATH];
	char	txtfile[MAX_QPATH];
	char	string[128];
	int		count = 0;
	size_t	cnt = 0;

	if (DEBUG_HSCORES) 
		DbgPrintf("%s entered\n", __func__);
	i =  sprintf(binfile, "./");
	i += sprintf(binfile + i, "%s/%s", game_dir->string, cfgdir->string);
	i += sprintf(binfile + i, "/hs/%s_hs.bin", level.mapname);
	
	i =  sprintf(txtfile, "./");
	i += sprintf(txtfile + i, "%s/%s", game_dir->string, cfgdir->string);
	i += sprintf(txtfile + i, "/hs/%s_hs.txt", level.mapname);
	
	if (DEBUG_HSCORES) 
		DbgPrintf("Opened for reading %s\n", binfile);
	HS_file = fopen(binfile, "rb");
	
	if(HS_file)
	{
		cnt = fread(g_TopScores, sizeof(g_TopScores[0]) * SCORESTOKEEP, 1, HS_file);
		fclose(HS_file);

		//JSW
		// HS_file loaded - see if any entity made the list
		for (i = 0 ; i < maxclients->value ; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if((game.clients[i].pers.pl_state == PL_PLAYING 
				|| cl_ent->client->pers.pl_state == PL_WARMUP)
				&& (game.clients[i].ps.stats[STAT_FRAGS] > g_TopScores[SCORESTOKEEP-1].score)
				&& (game.clients[i].ps.stats[STAT_FRAGS] > 0))
			{ // if it beat the lowest, keep score
				//my_bprintf (PRINT_HIGH, "High scores changed\n");
				strcpy(g_TopScores[SCORESTOKEEP-1].netname, game.clients[i].pers.netname);
				g_TopScores[SCORESTOKEEP-1].score = game.clients[i].resp.score;
				if (DEBUG_HSCORES) 
					DbgPrintf("Keeping %s - %d\n", 
					g_TopScores[SCORESTOKEEP-1].netname, 
					g_TopScores[SCORESTOKEEP-1].score);
				strcpy(g_TopScores[SCORESTOKEEP-1].date, sys_date);
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
		count = 0;
		for (i = 0 ; i < maxclients->value; i++)
		{
			cl_ent = g_edicts + 1 + i;
			if (cl_ent->inuse && (cl_ent->client->pers.pl_state == PL_PLAYING || cl_ent->client->pers.pl_state == PL_WARMUP))
			{
				strcpy(g_TopScores[count].netname, game.clients[i].pers.netname);
				g_TopScores[count].score = game.clients[i].resp.score;
				strcpy(g_TopScores[count].date, sys_date);
				count++;
				if (count >= SCORESTOKEEP)
					break;
			}
		}
		
		// sort it
		qsort(g_TopScores, sizeof(g_TopScores)/sizeof(g_TopScores[0]), sizeof(g_TopScores[0]), MP_Sort);
	}

	// write the high score HS_file
	HS_file = fopen(binfile, "wb");
	if (HS_file)
	{
		fwrite(g_TopScores, sizeof(g_TopScores[0]), SCORESTOKEEP, HS_file);
		fclose(HS_file);
		if (DEBUG_HSCORES) 
			DbgPrintf("File written %s\n", binfile);
	}
	else
	{
		if (DEBUG_HSCORES) 
			DbgPrintf("Can't write %s\n", binfile);
	}
	
	// print top scores to a man-readable file
	if (DEBUG_HSCORES) 
		DbgPrintf("Opened for writing %s\n", txtfile);
	HS_file = fopen(txtfile, "wt");
	if (HS_file)
	{
		sprintf(string, "    Top %d Scores for %s\n\n", SCORESTOKEEP, level.mapname);
		highlight_text(string, string);
		//fprintf(HS_file,"    Top %d Scores for %s\n\n", SCORESTOKEEP, level.mapname);
		fprintf(HS_file, "%s", string);
		for (i = 0; i < SCORESTOKEEP; i++)
			fprintf(HS_file, "  %2d - %8s - %i - %-12.12s\n", i + 1, 
					g_TopScores[i].date, g_TopScores[i].score, g_TopScores[i].netname);
		fprintf(HS_file,"\n     %s  %s\n", MOD, MOD_VERSION);
		fprintf(HS_file,"              www.railwarz.com");
		fclose(HS_file);
		if (DEBUG_HSCORES) 
			DbgPrintf("File written %s\n", txtfile);
	}
	else
	{
		gi.dprintf("Can't write %s check directory exists\n", txtfile);
		return;
	}
}

void LoadHighScores (void)
{
	char	entry[1400];
	char	string[1400];
	size_t	stringlength;
	int		i;
	size_t	j;
	FILE    *motd_file;
	char    filename[MAX_QPATH];
	char    line[80];

	i =  sprintf(filename, "./");
	i += sprintf(filename + i, "%s/%s", game_dir->string, cfgdir->string);
	i += sprintf(filename + i, "/hs/%s_hs.txt", level.mapname);
	
	if (!(motd_file = fopen(filename, "r")))
	{
		if (DEBUG_HSCORES) 
			DbgPrintf("Can't open highscores using %s\n", filename);
		return;
	}
	string[0] = 0;
	stringlength = strlen(string);
	i = 0;
	while ( fgets(line, 80, motd_file) )
	{
		if (strstr (line, sys_date))
			highlight_text(line, NULL); // white -> green
		Com_sprintf (entry, sizeof(entry), "xv 2 yv %i string \"%s\" ", i*8 + 24, line);
		j = strlen(entry);
		if (stringlength + j > 1400)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
		i++;
	}

	fclose(motd_file);
	j = strlen(entry);
	if (stringlength + j < 1400)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	Com_sprintf (hscores, sizeof(hscores), string);
}

// used for the qsort algorithm
static int MP_Sort(const void *a, const void *b)
{
	return (((HS_STRUCT *)b)->score - ((HS_STRUCT *)a)->score);
}
