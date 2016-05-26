//
//  vote.h
//  tmg
//

#ifndef vote_h
#define vote_h

extern void ClearMapVotes(void) ;
extern int MapMaxVotes(void);
extern void VoteForMap(int i);
extern void DumpMapVotes(void);
extern void ClearMapList(void);
extern void MaplistNextMap(edict_t *ent);


#endif /* vote_h */
