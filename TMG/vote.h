//
//  vote.h
//

#ifndef VOTE_H
#define VOTE_H

#define NO_MAPVOTES -1

//QW// in vote.c
extern float yesvotes;
extern float novotes;
extern qboolean mapvoteactive;
extern int mapvotetime;
extern qboolean votemapnow;
extern qboolean mapvotefilled;

void ClearMapVotes(void) ;
int MapMaxVotes(void);
void VoteForMap(int i);
void DumpMapVotes(void);
void ClearMapList(void);
int MaplistNextMap(edict_t *ent);



#endif /* VOTE_H */
