#ifndef HUD_H
#define HUD_H

// globals
extern int vote_state;
extern int vote_pro;
extern int vote_con;
extern int newdmflags;
edict_t *votestarter;
edict_t *votetarget;

/**
 Retrieves name of next level in the list
 */
char	*map_mod_next_map(void);

char *ShowHud (edict_t *ent);	//RAV

void vote_think (edict_t *ent, float timenow);

extern edict_t *votestarter;
extern edict_t *votetarget;

int CountConnectedClients (void);
void CalcFPM(edict_t *ent);
void CalcFPH(edict_t* ent);
void TimeLeft(void);

char *rav_gettech(edict_t *ent);

extern int	wav_mod;			// 1 if maps.txt was found and loaded 0 if not
void wav_mod_set_up(void);		// Attempts to find and load maps.txt
char* wav_mod_next_map(void);	// Retrieves name of next level in the list
int rav_getnumclients(void);

extern cvar_t *wav;
extern cvar_t *use_song_file;

#endif //HUD_H
