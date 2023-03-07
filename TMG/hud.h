#ifndef HUD_H
#define HUD_H

// globals
extern int vote_state;
extern int vote_pro;
extern int vote_con;
extern int newdmflags;
extern edict_t* votestarter;
extern edict_t* votetarget;

char* ShowHud(edict_t* ent);	//RAV

extern edict_t* votestarter;
extern edict_t* votetarget;

int CountConnectedClients(void);
void CalcFPM(edict_t* ent);
void CalcFPH(edict_t* ent);
void TimeLeft(void);

char* rav_gettech(edict_t* ent);

#endif //HUD_H
