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
extern void SP_info_player_intermission (edict_t *ent);
extern qboolean IsFemale (edict_t *ent);
extern qboolean IsFemale (edict_t *ent);
extern qboolean IsNeutral (edict_t *ent);
extern void ClientObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
extern void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
extern void SaveClientData (void);
extern void FetchClientEntData (edict_t *ent);
extern void player_pain (edict_t *self, edict_t *other, float kick, int damage);
extern void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
extern void InitClientPersistant (gclient_t *client);
extern void PutClientInServer (edict_t *ent);
extern void respawn (edict_t *ent,qboolean spawn);
extern void InitClientResp (gclient_t *client);
extern void InitBodyQue (void);
extern void ClientBeginServerFrame (edict_t *ent);
extern qboolean ClientConnect (edict_t *ent, char *userinfo);
extern void ClientDisconnect (edict_t *ent);
extern qboolean Check_tag (edict_t *ent, char *namecheck);
extern void ClientUserinfoChanged (edict_t *ent, char *userinfo);
extern void QuadTimeout (edict_t *ent);
extern void TossClientWeapon (edict_t *self);
extern void LookAtKiller (edict_t *self, edict_t *inflictor, edict_t *attacker);
extern float	PlayersRangeFromSpot (edict_t *spot);
extern float	PlayersRangeFromSpotRAV (edict_t *spot, edict_t *ent);
extern edict_t *SelectRandomDeathmatchSpawnPoint (void);
extern edict_t *SelectFarthestDeathmatchSpawnPoint (void);
extern edict_t *SpawnNearFlag (edict_t *ent);
extern edict_t *SelectDeathmatchSpawnPoint (void);
extern edict_t *SelectCoopSpawnPoint (edict_t *ent);
qboolean SelectSpawnPointRAV (edict_t *ent, vec3_t origin, vec3_t angles);
extern void Body_droptofloor(edict_t *ent);
extern void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
extern void CopyToBodyQue (edict_t *ent);

extern void Connect (edict_t *ent);
extern void ClientBeginDeathmatch (edict_t *ent);
extern void ClientBegin (edict_t *ent);
extern void maxrate_think(edict_t *self);
extern void nodelta_think(edict_t *self);
extern unsigned CheckBlock (void *b, int c);
extern void PrintPmove (pmove_t *pm);
extern void MV (edict_t * ent);
extern void Get_Position ( edict_t *ent, vec3_t position );
extern void ChainPodThink (edict_t *ent);

/**
 RaVeNs remove edict/on forced reconnection
 */
extern void RavenDisconnect (edict_t *ent);

extern char *GetPort(edict_t *ent,char *ip);
extern qboolean TraceX (edict_t *ent,vec3_t p2);
extern qboolean GET_AVG_PING(edict_t *ent);
extern void SendStatusBar(edict_t *ent, char *bar);
extern void Spec_Check (edict_t *ent);


#endif /* p_client_h */
