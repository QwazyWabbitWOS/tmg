#ifndef S_MAP_H
#define S_MAP_H


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
