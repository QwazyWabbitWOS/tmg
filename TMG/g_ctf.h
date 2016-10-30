#ifndef G_CTF_H
#define G_CTF_H

#define CTF_VERSION			1.02
#define CTF_VSTRING2(x) #x
#define CTF_VSTRING(x) CTF_VSTRING2(x)
#define CTF_STRING_VERSION  CTF_VSTRING(CTF_VERSION)

/* STAT_* 0 through 16 are defined in q_shared.h */
#define STAT_CTF_TEAM1_PIC			17
#define STAT_CTF_TEAM1_CAPS			18
#define STAT_CTF_TEAM2_PIC			19
#define STAT_CTF_TEAM2_CAPS			20
#define STAT_CTF_FLAG_PIC			21
#define STAT_CTF_JOINED_TEAM1_PIC	22
#define STAT_CTF_JOINED_TEAM2_PIC	23
#define STAT_CTF_TEAM1_HEADER		24
#define STAT_CTF_TEAM2_HEADER		25
#define STAT_CTF_TECH				26

//#define STAT_CTF_ID_VIEW			27

#define STAT_CTF_RED_FLAG_CARRIER   29
#define STAT_CTF_BLUE_FLAG_CARRIER  30

#define MAX_MENU_MAPS	14	//number of lines in mapvote menu

typedef enum ctfteam_n {
	CTF_NOTEAM,
	CTF_TEAM1,
	CTF_TEAM2
} ctfteam_t;

typedef enum ctfstate_n {
	CTF_STATE_START,
	CTF_STATE_PLAYING
} ctfstate_t;

typedef enum ctfgrapplestate_n {
	CTF_GRAPPLE_STATE_FLY,
	CTF_GRAPPLE_STATE_PULL,
	CTF_GRAPPLE_STATE_HANG
} ctfgrapplestate_t;

extern cvar_t *ctf;
extern cvar_t *dropflag_delay;
extern cvar_t *newscore;
extern cvar_t *ctf_deathscores;

#define CTF_TEAM1_SKIN "ctf_r"
#define CTF_TEAM2_SKIN "ctf_b"

#define DF_CTF_FORCEJOIN	131072	
#define DF_ARMOR_PROTECT	262144
#define DF_CTF_NO_TECH      524288

#define CTF_CAPTURE_BONUS		cap_point->value // 15	// what you get for capture
#define CTF_TEAM_BONUS			cap_team->value //10	// what your team gets for capture
#define CTF_RECOVERY_BONUS		recover_flag->value //1	// what you get for recovery
#define CTF_FLAG_BONUS			flag_bonus->value	// what you get for picking up enemy flag
#define CTF_FRAG_CARRIER_BONUS	frag_carrier->value //2	// what you get for fragging enemy flag carrier
#define CTF_FLAG_RETURN_TIME	flag_return_time->value	// seconds until auto return

#define CTF_CARRIER_DANGER_PROTECT_BONUS	carrier_save->value //2	// bonus for fraggin someone who has recently hurt your flag carrier
#define CTF_CARRIER_PROTECT_BONUS			carrier_protect->value //1	// bonus for fraggin someone while either you or your target are near your flag carrier
#define CTF_FLAG_DEFENSE_BONUS				flag_defend->value //1	// bonus for fraggin someone while either you or your target are near your flag
#define CTF_RETURN_FLAG_ASSIST_BONUS		flag_assist->value //1	// awarded for returning a flag that causes a capture to happen almost immediately
#define CTF_FRAG_CARRIER_ASSIST_BONUS		frag_carrier_assist->value //2	// award for fragging a flag carrier if a capture happens almost immediately

#define CTF_TARGET_PROTECT_RADIUS			400	// the radius around an object being defended where a target will be worth extra frags
#define CTF_ATTACKER_PROTECT_RADIUS			400	// the radius around an object being defended where an attacker will get extra frags when making kills

#define CTF_CARRIER_DANGER_PROTECT_TIMEOUT	8
#define CTF_FRAG_CARRIER_ASSIST_TIMEOUT		10
#define CTF_RETURN_FLAG_ASSIST_TIMEOUT		10

//#define CTF_AUTO_FLAG_RETURN_TIMEOUT		auto_flag_return->value//30	// number of seconds before dropped flag auto-returns

#define CTF_TECH_TIMEOUT					60  // seconds before techs spawn again

#define CTF_GRAPPLE_SPEED					grapple_speed->value // speed of grapple in flight
#define CTF_GRAPPLE_PULL_SPEED				grapple_pullspeed->value	// speed player is pulled at

/**
 If ctf is enabled, initialize the ctf structs and vars
 else return.
 */
void CTFInit(void);

//JSW
void	ShowFile (edict_t *ent, char *filename);
void	ResetCaps(void);
void	CheckPlayers(void);
void	FillMapNames(void);
qboolean Check_for_SpecLimit(edict_t *who);
void RavCheckTeams(void);
void MapVote(edict_t *ent);

void OpenJoinMenu(edict_t *ent);
void Spectate(edict_t *ent, pmenu_t *menu);
void CTFChaseCam(edict_t *ent, pmenu_t *p);
void RAVspec(edict_t *ent, pmenu_t *p);
void CTFReturnToMain(edict_t *ent, pmenu_t *p);
void CTFCredits(edict_t *ent, pmenu_t *p);
void CTFShowScores(edict_t *ent, pmenu_t *p);

void JoinGame(edict_t *ent, pmenu_t *menu);
void OPMenu(edict_t *ent, pmenu_t *p);
void KickMe(edict_t *ent, pmenu_t *p);

void LightsOn(edict_t *ent, pmenu_t *menu);
void LightsOff(edict_t *ent, pmenu_t *menu);

void VoteYes (edict_t *ent, pmenu_t *menu);
void VoteNo (edict_t *ent, pmenu_t *menu);


/**
 Count spectators
 */
int CountSpecClients (void);

void SP_info_player_team1(edict_t *self);
void SP_info_player_team2(edict_t *self);

void CTFJoinTeam1(edict_t *ent, pmenu_t *p);
void CTFJoinTeam2(edict_t *ent, pmenu_t *p);
char *CTFTeamName(int team);
char *CTFOtherTeamName(int team);
int CTFOtherTeam(int team);
void CTFAssignSkin(edict_t *ent, char *s);
void CTFAssignTeam(gclient_t *who);
void CTFJobAssign (void);
void CTFSetupNavSpawn(void);
edict_t *SelectCTFSpawnPoint (edict_t *ent);
void CTFResetFlags(void);
qboolean CTFPickup_Flag(edict_t *ent, edict_t *other);
void CTFDropFlagTouch(edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);
void CTFDrop_Flag(edict_t *ent, gitem_t *item);//RAV
void CTFEffects(edict_t *player);
void CTFCalcScores(void);
void SetCTFStats(edict_t *ent);
void CTFDeadDropFlag(edict_t *self);
void CTFScoreboardMessage (edict_t *ent, edict_t *killer);
void CTFScoreboardMessageNew (edict_t *ent, edict_t *killer);
void CTFTeam_f (edict_t *ent, int desired_team);
void CTFID_f (edict_t *ent);
void CTFSay_Team_Location(edict_t *who, char *buf);
void CTFSay_Team(edict_t *who, char *msg);
void CTFJoinTeam(edict_t *ent, int desired_team);

void CTFFlagThink(edict_t *ent);
void CTFFlagSetup (edict_t *ent);
void CTFResetFlag(int ctf_team);

void CTFFragBonuses(edict_t *targ, edict_t *inflictor, edict_t *attacker);
void CTFCheckHurtCarrier(edict_t *targ, edict_t *attacker);

// GRAPPLE
void CTFWeapon_Grapple (edict_t *ent);
void CTFPlayerResetGrapple(edict_t *ent);
void CTFGrapplePull(edict_t *self);
void CTFResetGrapple(edict_t *self);
void CTFGrappleTouch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
void P_ProjectSource_Reverse(gclient_t *client,
							 vec3_t point,
							 vec3_t distance,
							 vec3_t forward,
							 vec3_t right,
							 vec3_t result);
void CTFFireGrapple (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect);
void CTFGrappleFire (edict_t *ent, vec3_t g_offset, int damage, int effect);
void CTFWeapon_Grapple_Fire (edict_t *ent);

// RaVeN hook for map chaining
void RaV_hook (edict_t *ent);
void RaV_unhook (edict_t *ent);


void CTFGrappleDrawCable(edict_t *self);

//TECH
void CTFHasTech(edict_t *who);
gitem_t *CTFWhat_Tech(edict_t *ent);
qboolean CTFPickup_Tech (edict_t *ent, edict_t *other);
void CTFDrop_Tech(edict_t *ent, gitem_t *item);
void CTFDeadDropTech(edict_t *ent);
void CTFSetupTechSpawn(void);
int CTFApplyResistance(edict_t *ent, int dmg);
int CTFApplyStrength(edict_t *ent, int dmg);
qboolean CTFApplyStrengthSound(edict_t *ent);
qboolean CTFApplyHaste(edict_t *ent);
void CTFApplyHasteSound(edict_t *ent);
void CTFApplyRegeneration(edict_t *ent);
qboolean CTFHasRegeneration(edict_t *ent);
void CTFRespawnTech(edict_t *ent);

void CTFOpenJoinMenu(edict_t *ent);
qboolean CTFStartClient(edict_t *ent);

qboolean CTFCheckRules(void);

// CTF Voting
void VoteMapNames3(void);
void VoteChangeMap(edict_t *ent, pmenu_t *p);
void VoteMap3(edict_t *ent, pmenu_t *p);
void VoteMapNames2(void);
void VoteMap2(edict_t *ent, pmenu_t *p);
void VoteMapNames(void);
void VoteMap(edict_t *ent, pmenu_t *p);

// OP level commands
char *GetIpOp(edict_t *ent);
void List_Op(edict_t *ent);
void OpPlayer(edict_t *ent, pmenu_t *p);
void OpMe(edict_t *ent, pmenu_t *p);
void OpLockServer(edict_t *ent, pmenu_t *p);
void OpLock(edict_t *ent, pmenu_t *p);
void List_Switch(edict_t *ent);
void OpRestart(edict_t *ent, pmenu_t *p);
void OpChangeMap(edict_t *ent, pmenu_t *p);
void OpMap(edict_t *ent, pmenu_t *p);
void OpMapNames3(void);
void OpMap3(edict_t *ent, pmenu_t *p);
void OpMapNames2(void);
void OpMap2(edict_t *ent, pmenu_t *p);
void OpMapNames(void);

void OpBotToggle(edict_t *ent, pmenu_t *menu);
void OpBotChatToggle (edict_t *ent, pmenu_t *menu);
void OpBotInsultToggle (edict_t *ent, pmenu_t *menu);

void BotsMenu(edict_t *ent, pmenu_t *p);

void SpecMe(edict_t *ent, pmenu_t *p);
void List_Spec(edict_t *ent);
void SpecPlayer(edict_t *ent, pmenu_t *p);
void List_Kick(edict_t *ent);



////////////////////////////////
void SP_misc_ctf_banner (edict_t *ent);
void SP_misc_ctf_small_banner (edict_t *ent);

extern char *ctf_statusbar;
extern char *rail_statusbar;//rav


void SP_trigger_teleport (edict_t *ent);
void SP_info_teleport_destination (edict_t *ent);

void PickMap(edict_t *ent, pmenu_t *p);
void List_Players(edict_t *ent);
void ListPlayers(edict_t *ent, pmenu_t *p);
void LightsMenu(edict_t *ent, pmenu_t *p);
void PlayerMenu(edict_t *ent, pmenu_t *p);
void List_KickBan(edict_t *ent);
void KicknBanPlayer(edict_t *ent, pmenu_t *p);
void KickPlayer(edict_t *ent, pmenu_t *p);
void KickMe(edict_t *ent, pmenu_t *p);
void BanMe(edict_t *ent, pmenu_t *p);
void List_Ban(edict_t *ent);
void BanPlayer(edict_t *ent, pmenu_t *p);
void KicknBanMe(edict_t *ent, pmenu_t *p);
void List_UnSilence(edict_t *ent);
void UnSilencePlayer(edict_t *ent, pmenu_t *p);
void UnSilenceMe(edict_t *ent, pmenu_t *p);
void List_Silence(edict_t *ent);
void SilencePlayer(edict_t *ent, pmenu_t *p);
void SilenceMe(edict_t *ent, pmenu_t *p);
void SwitchMe(edict_t *ent, pmenu_t *p);
void SwitchPlayer(edict_t *ent, pmenu_t *p);
//JSW void MOpPlayer(edict_t *ent, pmenu_t *p);
//JSW void MOpMe(edict_t *ent, pmenu_t *p);
//JSW
void UpdateOpMenu(edict_t *ent);
void UpdateBotMenu(edict_t *ent);
void UpdatePlayerMenu(edict_t *ent);
char *GetIp(edict_t *ent);
void SpawnExtra(vec3_t position, char *classname);
void ChangeNow (edict_t *ent, pmenu_t *menu);
void ChangeLater (edict_t *ent, pmenu_t *menu);
void Locked(edict_t *ent, pmenu_t *p);
int CTFUpdateJoinMenu(edict_t *ent);

#endif	/* G_CTF_H */
