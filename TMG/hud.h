#ifndef HUD_H
#define HUD_H

//QW// Perhaps a countdown for a mapvote timer? Unused.
extern float vote_end;

extern int	map_mod_;		// 1 if maps.txt was found and loaded 0 if not

//void		map_mod_set_up(void);		// Attempts to find and load maps.txt

extern char		*map_mod_next_map(void);	// Retrieves name of next level in the list

extern char *tn_showHud (edict_t *ent);	//RAV

extern void vote_think (edict_t *ent, float timenow);
extern int vote_state, vote_pro, vote_con, newdmflags;
extern edict_t *votestarter, *votetarget;
extern int CountConnectedClients (void);
extern int rav_getFPM(gclient_t* cl);
extern int rav_getFPH(gclient_t* cl);
extern int rav_time(void);

extern char *rav_gettech(edict_t *ent);

extern int	wav_mod_;				// 1 if maps.txt was found and loaded 0 if not
extern void wav_mod_set_up(void);		// Attempts to find and load maps.txt
extern char *wav_mod_next_map(void);	// Retrieves name of next level in the list
extern int rav_getnumclients(void);

extern cvar_t *wav;
extern cvar_t *use_song_file;

#endif //HUD_H
