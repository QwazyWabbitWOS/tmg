#ifndef S_MAP_H
#define S_MAP_H

#define MAX_MAPS 300

typedef struct
{
    char   aFile[MAX_QPATH];
    char   aName[MAX_QPATH];
    int    min;
    int    max;
    int    fVisited;
} MAP_ENTRY;

typedef struct
{
	int  nummaps;          // number of maps in list
	char mapname[MAX_MAPS][MAX_QPATH];
	char mapnick[MAX_MAPS][MAX_QPATH];
	int  currentmap;       // index to current map
	qboolean active;
	int	votes[MAX_MAPS];
	int	currentmapvote;
	int	nextmap;
} maplist_t;

extern	maplist_t	*maplist;
extern MAP_ENTRY   *mdsoft_map;

edict_t *mdsoft_NextMap( void );

/**
 Call this at game intialization
 */
void mdsoft_InitMaps(void);

// cvars used by this module
extern cvar_t	*map_change;
extern cvar_t	*map_randomize;
extern cvar_t	*map_once;
extern cvar_t	*debug_smap;


#endif /* S_MAP_H */
