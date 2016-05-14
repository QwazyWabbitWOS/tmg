#include "g_local.h"
#include "eavy.h"
#include <stdio.h>
#include <ctype.h>

char *ReadTextFile(char *filename)
{
    FILE        *fp;
    char        *filestring = NULL;
    long int	i;
    int         ch;

    while (true) {
        fp = fopen(filename, "r");
        if (!fp) break;

        for (i=0; (ch = fgetc(fp)) != EOF; i++);

        filestring = gi.TagMalloc(i+1, TAG_LEVEL);
        if (!filestring) break;

        fseek(fp, 0, 0);
        for (i=0; (ch = fgetc(fp)) != EOF; i++)
            filestring[i] = ch;
        filestring[i] = '\0';

        break;
    }

    if (fp) fclose(fp);

    return(filestring);
}

char *EAVYLoadEntities(char *mapname, char *entities)
{
    char   entfilename[256] = "";
    char   *newentities;
	int    i;
	size_t islefn;
  
	sprintf(entfilename, "./%s/%s/ent/", game_dir->string, cfgdir->string);
	  
	islefn = strlen(entfilename);
    for (i=0; mapname[i]; i++)
        entfilename[i + islefn] = tolower(mapname[i]);
    entfilename[i + islefn] = '\0';
    strcat(entfilename, ".ent");

    newentities = ReadTextFile(entfilename);

    if (newentities)
        return(newentities);
    else
        return(entities);
}

void EAVYCTF_Init(void)
{
    if (!ctf->value)
        return;

    EAVYSpawnFlags();
    EAVYSetupFlagSpots();
	ResetCaps();
}

edict_t *EAVYFindFarthestFlagPosition(edict_t *flag)
{
    edict_t *bestspot, *spot = NULL;
    float   bestdistance, bestflagdistance = 0;
    vec3_t  v;

    bestspot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

    while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        bestdistance = VectorLength(v);

        if (bestdistance > bestflagdistance)
        {
            bestflagdistance = bestdistance;
            bestspot = spot;
        }
    }
    return bestspot;
}

void EAVYSpawnFlags(void)
{
    edict_t *redflag, *blueflag;

    redflag = G_Find (NULL, FOFS(classname), "item_flag_team1");
    blueflag = G_Find (NULL, FOFS(classname), "item_flag_team2");

    if (ctf->value && !redflag)
    {
        redflag = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
        redflag = EAVYFindFarthestFlagPosition(redflag);
        if (!redflag)
            redflag = G_Find (NULL, FOFS(classname), "info_player_start");
        if (redflag)
        {
            redflag->classname = "item_flag_team1";
            ED_CallSpawn (redflag);
        }
    }

    if (redflag && !blueflag)
    {
        blueflag = EAVYFindFarthestFlagPosition(redflag);
        if (blueflag)
        {
            blueflag->classname = "item_flag_team2";
            ED_CallSpawn (blueflag);
        }
    }

    if (ctf->value && !blueflag)
        gi.dprintf ("EAVY.EAVYSpawnFlags = FAILED!\n");

}

void EAVYSpawnTeamNearFlagCheck(void)
{
    edict_t *flag, *spot = NULL;
    float   dist;
    vec3_t  v;

    flag = G_Find (NULL, FOFS(classname), "item_flag_team1");
    while((spot = G_Find (spot, FOFS(classname), "info_player_team2")))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        dist = VectorLength (v);
        if (EAVY_RESTRICTED_RADIUS > dist)
        {
            spot->classname = "info_player_deathmatch";
            spot->svflags &= ~SVF_NOCLIENT;
            spot->s.effects &= ~EF_COLOR_SHELL;
            spot->s.renderfx &= ~RF_SHELL_BLUE;
            ED_CallSpawn (spot);

        }
    }
    flag = G_Find (NULL, FOFS(classname), "item_flag_team2");
    while((spot = G_Find (spot, FOFS(classname), "info_player_team1")))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        dist = VectorLength (v);
        if (EAVY_RESTRICTED_RADIUS > dist)
        {
            spot->classname = "info_player_deathmatch";
            spot->svflags &= ~SVF_NOCLIENT;
            spot->s.effects &= ~EF_COLOR_SHELL;
            spot->s.renderfx &= ~RF_SHELL_RED;
            ED_CallSpawn (spot);

        }
    }
}

void EAVYSpawnTeamNearFlag(edict_t *flag)
{
    edict_t *spot = NULL;
    float   dist;
    vec3_t  v;

    while((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")))
    {
        VectorSubtract (spot->s.origin, flag->s.origin, v);
        dist = VectorLength (v);
        if (EAVY_RESTRICTED_RADIUS > dist)
        {
            if (!strcmp(flag->classname, "item_flag_team1"))
                spot->classname = "info_player_team1";
            if (!strcmp(flag->classname, "item_flag_team2"))
                spot->classname = "info_player_team2";
            spot->svflags |= SVF_NOCLIENT;
            spot->solid = SOLID_NOT;
            ED_CallSpawn (spot);
        }
    }
}

void EAVYSetupFlagSpots(void)
{
    edict_t *ent, *spot;

    spot = G_Find (NULL, FOFS(classname), "misc_ctf_small_banner");
    if (!spot && ctf->value)
    {
        ent = G_Find (NULL, FOFS(classname), "info_player_team1");
        spot = G_Find (NULL, FOFS(classname), "info_player_team2");
        if (!ent && !spot)
        {
            ent = G_Find (NULL, FOFS(classname), "item_flag_team1");
            spot = G_Spawn ();
            spot->classname = "misc_ctf_small_banner";
            spot->spawnflags = 0;
            VectorCopy (ent->s.origin, spot->s.origin);
            ED_CallSpawn (spot);
            EAVYSpawnTeamNearFlag (ent);

            ent = G_Find (NULL, FOFS(classname), "item_flag_team2");
            spot = G_Spawn ();
            spot->classname = "misc_ctf_small_banner";
            spot->spawnflags = 1;
            VectorCopy (ent->s.origin, spot->s.origin);
            ED_CallSpawn (spot);
            EAVYSpawnTeamNearFlag (ent);
            EAVYSpawnTeamNearFlagCheck();
        }
    }
}

void SP_misc_teleporter_dest (edict_t *ent);

void EAVYCallSpawnCompatibilityCheck(edict_t *ent)
{
    cvar_t *SpawnTeam = gi.cvar("EAVYspawnteam", "", 0);

    if (SpawnTeam->value && !strcmp(ent->classname, "info_player_team1"))
    {
        ent->svflags &= ~SVF_NOCLIENT;
        ent->s.effects |= EF_COLOR_SHELL;
        ent->s.renderfx |= RF_SHELL_RED;
        SP_misc_teleporter_dest (ent);
        return;
    }
    if (SpawnTeam->value && !strcmp(ent->classname, "info_player_team2"))
    {
        ent->svflags &= ~SVF_NOCLIENT;
        ent->s.effects |= EF_COLOR_SHELL;
        ent->s.renderfx |= RF_SHELL_BLUE;
        SP_misc_teleporter_dest (ent);
        return;
    }

    if (!strcmp(ent->classname, "info_flag_red") || !strcmp(ent->classname, "info_flag_team1"))
    {
        ent->classname = "item_flag_team1";

        return;
    }
    if (!strcmp(ent->classname, "info_flag_blue") || !strcmp(ent->classname, "info_flag_team2"))
    {
        ent->classname = "item_flag_team2";

        return;
    }
}
/* (C) EAVY */
// Freitag, 20. März 1998, 00:57

