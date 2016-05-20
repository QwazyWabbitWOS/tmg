//
//  p_client.h
//  tmg
//
//

#ifndef p_client_h
#define p_client_h

extern void ClientThink (edict_t *ent, usercmd_t *ucmd);
extern void SP_info_player_start(edict_t *self);
extern void SP_info_player_deathmatch(edict_t *self);
extern void SP_info_player_coop(edict_t *self);
void SP_info_player_intermission (edict_t *ent);
extern qboolean IsFemale (edict_t *ent);
extern qboolean IsFemale (edict_t *ent);
extern qboolean IsNeutral (edict_t *ent);
extern void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void InitClientPersistant (gclient_t *client);
void PutClientInServer (edict_t *ent);
void respawn (edict_t *ent,qboolean spawn);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);


#endif /* p_client_h */
