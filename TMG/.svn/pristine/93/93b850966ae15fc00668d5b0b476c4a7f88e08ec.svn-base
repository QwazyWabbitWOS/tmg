#ifndef G_CHASE_H
#define G_CHASE_H

#define CHASE_FREECAM	0
#define CHASE_CHASECAM	1
#define CHASE_FLOATCAM	2
#define CHASE_EYECAM	3

#define CHASE_FIRSTMODE	CHASE_FREECAM
#define CHASE_LASTMODE	CHASE_EYECAM

void SwitchModeChaseCam(edict_t *ent);
//void ToggleChaseCam(edict_t *ent, pmenu_t *p);
void UpdateChaseCam(edict_t *ent);
void ChaseNext(edict_t *ent);
void ChasePrev(edict_t *ent);
void ChaseRemoveTarget(edict_t *target);
void GetChaseTarget(edict_t *ent);
void ChaseHelp(edict_t *ent);

#endif //G_CHASE_H
