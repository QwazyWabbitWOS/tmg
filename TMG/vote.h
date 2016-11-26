//
//  vote.h
//

#ifndef VOTE_H
#define VOTE_H

#define NO_MAPVOTES -1

void ClearMapVotes(void) ;
int MapMaxVotes(void);
void VoteForMap(int i);
void DumpMapVotes(void);
void ClearMapList(void);
int MaplistNextMap(edict_t *ent);



#endif /* VOTE_H */
