#ifndef HUD_H
#define HUD_H

extern int	map_mod_;							// 1 if maps.txt was found and loaded 0 if not
//void		map_mod_set_up(void);					// Attempts to find and load maps.txt
char		*map_mod_next_map(void);				// Retrieves name of next level in the list

char *tn_showHud (edict_t *ent);//RAV

void vote_think (edict_t *ent, float timenow);
extern int vote_state, vote_pro, vote_con, newdmflags;
extern edict_t *votestarter, *votetarget;
extern float vote_end;

extern int	wav_mod_;				// 1 if maps.txt was found and loaded 0 if not
void wav_mod_set_up(void);		// Attempts to find and load maps.txt
char *wav_mod_next_map(void);	// Retrieves name of next level in the list
int rav_getnumclients(void);

extern cvar_t *wav;
extern cvar_t *use_song_file;

#endif //HUD_H