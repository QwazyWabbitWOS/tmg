
#ifndef P_HUD_H
#define P_HUD_H

void MoveClientToIntermission (edict_t *ent);
void BeginIntermission (edict_t *targ);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);
void DeathmatchScoreboard (edict_t *ent);
void Cmd_Score_f (edict_t *ent);
void HelpComputer (edict_t *ent);
void Cmd_Help_f (edict_t *ent);
void G_SetStats (edict_t *ent);

#endif /* P_HUD_H */
