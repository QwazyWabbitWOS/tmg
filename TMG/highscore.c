///<highscore.c>>

#include "g_local.h"
#include "highscore.h"
#include "performance.h"

#define KEEP 10

cvar_t  *highscores;
cvar_t	*show_highscores;

// global 
char hscores [1000];


qboolean hs_show;

static int MP_Sort(const void *a, const void *b);

typedef struct highscore_s 
{
	char	netname[16];
	int		score;
	char  	date[12];
} HS_STRUCT;

// room to hold max # of players
HS_STRUCT g_TopScores[KEEP];

void InitHighScores (void)
{
	highscores = gi.cvar ("highscores", "1", CVAR_LATCH);
	show_highscores = gi.cvar ("show_highscores", "0", CVAR_LATCH);
}

void SaveHighScores (void)
{
	int		i = 0;
	edict_t	*ent;
	FILE	*HS_file;
	char	binfile[MAX_QPATH];
	char	txtfile[MAX_QPATH];
	char	string[128];
	size_t	count = 0;

	Com_sprintf(binfile, sizeof binfile, "%s/%s/%s/hs/%s_hs.bin", 
		basedir->string, game_dir->string, cfgdir->string, level.mapname);

	Com_sprintf(txtfile, sizeof txtfile, "%s/%s/%s/hs/%s_hs.txt", 
		basedir->string, game_dir->string, cfgdir->string, level.mapname);

	HS_file = fopen(binfile, "rb");
	
	if(HS_file)
	{
		count = fread(g_TopScores, sizeof(g_TopScores[0]) * KEEP, 1, HS_file);
		if (count == 0 || ferror(HS_file))
		{
			gi.dprintf ("Error in %s reading %s\n", __func__, binfile);
			gi.dprintf ("Characters read: %d\n", count);
		}

		fclose(HS_file);

		//JSW
		// HS_file loaded - see if any entity made the list
		for (i = 0 ; i < maxclients->value ; i++)
		{
			ent = g_edicts + 1 + i;
			if((ent->client->pers.pl_state == PL_PLAYING
				|| ent->client->pers.pl_state == PL_WARMUP)
				&& (ent->client->ps.stats[STAT_FRAGS] >
					g_TopScores[KEEP-1].score)
				&& (ent->client->ps.stats[STAT_FRAGS] > 0)
				&& (ent->bot_client == false))
			{
				// if it beat the lowest, keep score
				my_bprintf (PRINT_HIGH, "High scores changed\n");
				strcpy(g_TopScores[KEEP-1].netname, ent->client->pers.netname);
				g_TopScores[KEEP-1].score = ent->client->resp.score;
				strcpy(g_TopScores[KEEP-1].date, sys_date);
				// sort it
				qsort(g_TopScores, sizeof(g_TopScores)/sizeof(g_TopScores[0]),
					  sizeof(g_TopScores[0]), MP_Sort);
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
			ent = g_edicts + 1 + i;
			if (ent->inuse &&
				(ent->client->pers.pl_state == PL_PLAYING ||
				 ent->client->pers.pl_state == PL_WARMUP)
				&& (ent->bot_client == false))
			{
				strcpy(g_TopScores[count].netname, game.clients[i].pers.netname);
				g_TopScores[count].score = game.clients[i].resp.score;
				strcpy(g_TopScores[count].date, sys_date);
				count++;
				if (count >= KEEP)
					break;
			}
		}
		
		// sort it
		qsort(g_TopScores, sizeof(g_TopScores)/sizeof(g_TopScores[0]),
			  sizeof(g_TopScores[0]), MP_Sort);
	}

	// write the high score HS_file
	HS_file = fopen(binfile, "wb");
	if (HS_file)
	{
		fwrite(g_TopScores, sizeof(g_TopScores[0]), KEEP, HS_file);
		fclose(HS_file);
	}
	else
	{
		gi.dprintf("Can't write %s\n", binfile);
	}
	
	// print top scores to a man-readable file
	HS_file = fopen(txtfile, "wt");
	if (HS_file)
	{
		Com_sprintf(string, sizeof string, 
			"    Top %d Scores for %s\n\n", 
			KEEP, level.mapname);
		highlight_text(string, string);
		fprintf(HS_file, "%s", string);

		for (i = 0; i < KEEP; i++)
			fprintf(HS_file,
					"  %2d - %8s - %i - %-12.12s\n",
					i + 1,
					g_TopScores[i].date,
					g_TopScores[i].score,
					g_TopScores[i].netname);

		fprintf(HS_file,"\n     %s  %s\n", MOD, MOD_VERSION);
		fprintf(HS_file,"              www.railwarz.com");
		fclose(HS_file);
	}
	else
	{
		gi.dprintf("Can't write %s check directory exists\n", txtfile);
		return;
	}
}

void LoadHighScores (void)
{
	char	entry[1400] = "";
	char	string[1400] = "";
	size_t	stringlength;
	int		i;
	size_t	j;
	FILE    *hs_file;
	char    filename[MAX_QPATH];
	char    line[80];

	Com_sprintf(filename, sizeof filename, "%s/%s/%s/hs/%s_hs.txt", 
		basedir->string, game_dir->string, cfgdir->string, level.mapname);

	hs_file = fopen(filename, "r");
	if (!hs_file)
	{
		gi.dprintf("Can't open highscores using %s\n", filename);
		return;
	}
	string[0] = 0;
	stringlength = strlen(string);
	i = 0;
	while ( fgets(line, 80, hs_file) )
	{
		if (strstr (line, sys_date))
			highlight_text(line, NULL); // white -> green
		Com_sprintf (entry, sizeof entry,
					 "xv 2 yv %i string \"%s\" ", i*8 + 24, line);
		j = strlen(entry);
		if (stringlength + j > 1400)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
		i++;
	}

	fclose(hs_file);
	j = strlen(entry);
	if (stringlength + j < 1400)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	if (stringlength > sizeof string)
		DbgPrintf("%s Warning! Highscore stringlength too big!", __func__);
	Com_sprintf (hscores, sizeof hscores, string);
}

// used for the qsort algorithm
static int MP_Sort(const void *a, const void *b)
{
	return (((HS_STRUCT *)b)->score - ((HS_STRUCT *)a)->score);
}
