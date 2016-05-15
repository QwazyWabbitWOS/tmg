//
//  g_cmds.h
//
//  Created by Geoff Joy on 5/11/16.
//

#ifndef g_cmds_h
#define g_cmds_h

//
// g_cmds.c
//
char *ClientTeam1 (edict_t *ent);
int ClientTeam (edict_t *ent);
int ClientPermTeam (edict_t *ent);
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);

//private to g_cmds.c, defined here to catch errors
void SelectNextItem (edict_t *ent, int itflags);
void SelectPrevItem (edict_t *ent, int itflags);
void Cmd_Give_f (edict_t *ent);
void Cmd_God_f (edict_t *ent);
void Cmd_Notarget_f (edict_t *ent);
void Cmd_Noclip_f (edict_t *ent);
void Cmd_Use_f (edict_t *ent);
void Cmd_Drop_f (edict_t *ent);
void Cmd_Inven_f (edict_t *ent);
void Cmd_InvUse_f (edict_t *ent);
void Cmd_LastWeap_f (edict_t *ent);
void Cmd_WeapPrev_f (edict_t *ent);
void Cmd_WeapNext_f (edict_t *ent);
void Cmd_WeapLast_f (edict_t *ent);
void Cmd_InvDrop_f (edict_t *ent);

/** Player suicide */
void Cmd_Kill_f (edict_t *ent);

void Cmd_PutAway_f (edict_t *ent);
int PlayerSort (void const *a, void const *b);
void Cmd_Players_f (edict_t *ent);
void Cmd_Operators_f (edict_t *ent);
void MapVoteThink(qboolean passed, qboolean now);
void Cmd_MapVote (edict_t *ent);
void Cmd_Wave_f (edict_t *ent);
void Say_Op(edict_t *who, char *msg);
void Cmd_ShowVotes_f(edict_t *ent);
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0);
void Cmd_ZoomIn(edict_t *ent);
void Cmd_ZoomOut(edict_t *ent);
void Cmd_AutoZoom(edict_t *ent);
void UndoChain(edict_t *ent ,int step);

//DLL exports this
void ClientCommand (edict_t *ent);

void ValidateSelectedItem (edict_t *ent);



#endif /* g_cmds_h */
