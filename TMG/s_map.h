#ifndef S_MAP_H
#define S_MAP_H

typedef struct
{
    char   aFile[MAX_QPATH];
    char   aName[MAX_QPATH];
    int    min;
    int    max;
    int    fVisited;
} MAP_ENTRY;

extern MAP_ENTRY   *mdsoft_map;

extern edict_t *mdsoft_NextMap( void );
extern void ClearMapVotes(void) ;
extern int MapMaxVotes(void);
extern int  LoadMapList(edict_t *ent, char *filename);
extern void Cmd_Maplist_f (edict_t *ent);
extern void Display_Maplist_Usage(edict_t *ent);
extern void VoteForMap(int i);
extern void DumpMapVotes(void);
extern void ClearMapList(void);
extern void MaplistNextMap(edict_t *ent);


#endif //S_MAP_H
