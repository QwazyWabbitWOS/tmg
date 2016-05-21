//
// eavy.h
//

#ifndef EAVY_VERSION
#define EAVY_VERSION 1.02+

#define EAVY_RESTRICTED_RADIUS 512

extern char *ReadTextFile(char *filename);
extern char *EAVYLoadEntities(char *mapname, char *entities);
extern void EAVYCTF_Init(void);
extern edict_t *EAVYFindFarthestFlagPosition(edict_t *flag);
extern void EAVYSpawnFlags(void);
extern void EAVYSpawnTeamNearFlagCheck(void);
extern void EAVYSpawnTeamNearFlag(edict_t *flag);
extern void EAVYSetupFlagSpots(void);
extern void EAVYCallSpawnCompatibilityCheck(edict_t *ent);

#endif /* EAVY_VERSION */
