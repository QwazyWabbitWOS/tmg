//
//  vote.h
//

#ifndef VOTE_H
#define VOTE_H

#define NO_MAPVOTES -1

extern void ClearMapVotes(void) ;
extern int MapMaxVotes(void);
extern void VoteForMap(int i);
extern void DumpMapVotes(void);
extern void ClearMapList(void);
extern int MaplistNextMap(edict_t *ent);



#endif /* VOTE_H */
