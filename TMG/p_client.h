//
//  p_client.h
//

#ifndef P_CLIENT_H
#define P_CLIENT_H

void ClientThink (edict_t *ent, usercmd_t *ucmd);
void SP_info_player_start(edict_t *self);
void SP_info_player_deathmatch(edict_t *self);
void SP_info_player_coop(edict_t *self);
void SP_info_player_intermission (edict_t *ent);
qboolean IsFemale (edict_t *ent);
qboolean IsFemale (edict_t *ent);
qboolean IsNeutral (edict_t *ent);
void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
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
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientDisconnect (edict_t *ent);
qboolean Check_tag (edict_t *ent, char *namecheck);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void QuadTimeout (edict_t *ent);
void TossClientWeapon (edict_t *self);
void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);
float	PlayersRangeFromSpot (edict_t *spot);
float	PlayersRangeFromSpotRAV (edict_t *spot, edict_t *ent);
edict_t *SelectRandomDeathmatchSpawnPoint (void);
edict_t *SelectFarthestDeathmatchSpawnPoint (void);
edict_t *SpawnNearFlag (edict_t *ent);
edict_t *SelectDeathmatchSpawnPoint (void);
edict_t *SelectCoopSpawnPoint (edict_t *ent);
qboolean SelectSpawnPointRAV (edict_t *ent, vec3_t origin, vec3_t angles);
void Body_droptofloor(edict_t *ent);
void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void CopyToBodyQue (edict_t *ent);

void Connect (edict_t *ent);
void ClientBeginDeathmatch (edict_t *ent);
void ClientBegin (edict_t *ent);
void maxrate_think(edict_t *self);
void nodelta_think(edict_t *self);
unsigned CheckBlock (void *b, int c);
void PrintPmove (pmove_t *pm);
void MV (edict_t * ent);
void Get_Position ( edict_t *ent, vec3_t position );
void ChainPodThink (edict_t *ent);

/**
 RaVeNs remove edict/on forced reconnection
 */
void RavenDisconnect (edict_t *ent);

char *GetPort(edict_t *ent,char *ip);
qboolean TraceX (edict_t *ent,vec3_t p2);
qboolean GET_AVG_PING(edict_t *ent);
void SendStatusBar(edict_t *ent, char *bar);
void Spec_Check (edict_t *ent);


#endif /* P_CLIENT_H */
