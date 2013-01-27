// g_local.h -- local definitions for game module

#include "q_shared.h"
#include <sys/timeb.h>

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

//RAV
#include "g_chase.h"
//
// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"TMG_MOD"
#define MOD_VERSION "0.2.09"
#define MOD "TMG_MOD"
// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
//RAV
#define svc_disconnect      7
//
#define svc_stufftext		11

//==================================================================

// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000

// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_POWER_ARMOR			0x00001000	// power armor (if any) is active

#define FL_SHOWPATH				0x00002000	// used to show current path (debugging)
#define	FL_NO_VWEAPON			0x00004000	//	STUPID hacks so we don't keep checking for
#define	FL_SUPPORTS_VWEAPON		0x00008000	//	VWeapon support, and eating up CPU time
#define FL_SHOW_FLAGPATHS		0x00010000

#define FL_RESPAWN				0x80000000	// used for item respawning
//RAV
#define FL_SPECIAL				0x00020000	// 
//
#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define MELEE_DISTANCE	80

#define BODY_QUEUE_SIZE		8

static vec3_t VEC_ORIGIN = {0,0,0};

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum 
{
	WEAPON_READY, 
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_GRENADES,
	AMMO_CELLS,
	AMMO_SLUGS
} ammo_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//range
#define RANGE_MELEE				0
#define RANGE_NEAR				1
#define RANGE_MID				2
#define RANGE_FAR				3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1

//monster ai flags
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_RESURRECTING			0x00004000

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

// armor types
#define ARMOR_NONE				0
#define ARMOR_JACKET			1
#define ARMOR_COMBAT			2
#define ARMOR_BODY				3
#define ARMOR_SHARD				4

// power armor types
#define POWER_ARMOR_NONE		0
#define POWER_ARMOR_SCREEN		1
#define POWER_ARMOR_SHIELD		2

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2


// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff


// noise types for PlayerNoise
#define PNOISE_SELF				0
#define PNOISE_WEAPON			1
#define PNOISE_IMPACT			2


//3ZB CTF state
#define GETTER		0
#define	DEFENDER	1
#define	SUPPORTER	2
#define	CARRIER		3
//


// edict->movetype values
typedef enum
{
MOVETYPE_NONE,			// never moves
MOVETYPE_NOCLIP,		// origin and angles change with no interaction
MOVETYPE_PUSH,			// no clip to world, push on box contact
MOVETYPE_STOP,			// no clip to world, stops on box contact
MOVETYPE_WALK,			// gravity
MOVETYPE_STEP,			// gravity, special edge handling
MOVETYPE_FLY,
MOVETYPE_TOSS,			// gravity
MOVETYPE_FLYMISSILE,	// extra size to monsters
MOVETYPE_BOUNCE
} movetype_t;



typedef struct
{
	int		base_count;
	int		max_count;
	float	normal_protection;
	float	energy_protection;
	int		armor;
} gitem_armor_t;


// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_ARMOR		4
#define IT_STAY_COOP	8
#define IT_KEY			16
#define IT_POWERUP		32
//ZOID
#define IT_TECH			64
//ZOID

//RAV
// gitem_t->weapmodel for weapons indicates model index
#define WEAP_BLASTER      1
#define WEAP_SHOTGUN      2
#define WEAP_SUPERSHOTGUN    3
#define WEAP_MACHINEGUN      4
#define WEAP_CHAINGUN      5
#define WEAP_GRENADES      6
#define WEAP_GRENADELAUNCHER  7
#define WEAP_ROCKETLAUNCHER    8
#define WEAP_HYPERBLASTER    9
#define WEAP_RAILGUN      10
#define WEAP_BFG        11
//ZOID
#define WEAP_GRAPPLE			12
//ZOID
//


#define MPI_QUAD				13
#define	MPI_PENTA				14
#define MPI_QUADF				15
#define MPI_INDEX				15	//MPI count

typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags
  
//	int      weapmodel;    // weapon model index (for weapons)

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use
} gitem_t;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;	// flash F1 icon if non 0, play sound
								// and increment only if 1, 2, or 3

	gclient_t	*clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	qboolean	autosaved;
//RAV
	char				  *apszTextFilterStrings[1000];
	int					nTextFilterCount;

	char				  *apszTextNonFilterStrings[1000];
	int					nTextNonFilterCount;
//
//
} game_locals_t;


//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

	// intermission state
	float		intermissiontime;		// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	edict_t		*sight_client;	// changed once each frame for coop games

	edict_t		*sight_entity;
	int			sight_entity_framenum;
	edict_t		*sound_entity;
	int			sound_entity_framenum;
	edict_t		*sound2_entity;
	int			sound2_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	edict_t		*current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies

	int			power_cubes;		// ugly necessity for coop

//RAV
	qboolean	 warmup;
	float		newframenum;
	float		allowpickup;
//
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct
{
	// world vars
	char		*sky;
	float		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
} spawn_temp_t;


typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		(*endfunc)(edict_t *);
} moveinfo_t;


typedef struct
{
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;

	vec3_t		saved_goal;
	float		search_time;
	float		trail_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

	int			power_armor_type;
	int			power_armor_power;
} monsterinfo_t;



extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern	int	sm_meat_index;
extern	int	snd_fry;

extern	int	jacket_armor_index;
extern	int	combat_armor_index;
extern	int	body_armor_index;

// means of death
#define MOD_UNKNOWN			0
#define MOD_BLASTER			1
#define MOD_SHOTGUN			2
#define MOD_SSHOTGUN		3
#define MOD_MACHINEGUN		4
#define MOD_CHAINGUN		5
#define MOD_GRENADE			6
#define MOD_G_SPLASH		7
#define MOD_ROCKET			8
#define MOD_R_SPLASH		9
#define MOD_HYPERBLASTER	10
#define MOD_RAILGUN			11
#define MOD_BFG_LASER		12
#define MOD_BFG_BLAST		13
#define MOD_BFG_EFFECT		14
#define MOD_HANDGRENADE		15
#define MOD_HG_SPLASH		16
#define MOD_WATER			17
#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22
#define MOD_SUICIDE			23
#define MOD_HELD_GRENADE	24
#define MOD_EXPLOSIVE		25
#define MOD_BARREL			26
#define MOD_BOMB			27
#define MOD_EXIT			28
#define MOD_SPLASH			29
#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_HIT				32
#define MOD_TARGET_BLASTER	33
#define MOD_GRAPPLE			34
#define MOD_FRIENDLY_FIRE	0x8000000

extern	int	meansOfDeath;

extern	edict_t			*g_edicts;

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
//ZOID
extern	cvar_t	*capturelimit;
//ZOID
extern	cvar_t	*passwd;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

extern	cvar_t	*sv_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*maxclients;

//ZOID
extern	qboolean	is_quad;
//ZOID

extern	cvar_t	*view_weapons;
//RAV

extern	cvar_t	*use_hook;
extern	cvar_t	*hook_speed;         // sets how fast the hook shoots out
extern	cvar_t	*hook_pullspeed;     // sets how fast the hook pulls a player
extern	cvar_t	*hook_sky;           // enables hooking to the sky
extern	cvar_t	*hook_maxtime;       // sets max time you can stay hooked
extern	cvar_t	*hook_damage;        // sets damage hook does to other players
extern	cvar_t	*reset_hook;
extern  cvar_t  *hook_color;
extern  cvar_t  *hook_offhand;

extern  cvar_t	*game_dir;
extern  cvar_t  *sv_botdetection;
extern  cvar_t  *server_ip;
extern  cvar_t  *warmup_time;
extern  cvar_t  *filter;

extern  cvar_t  *flood_msgs;
extern  cvar_t  *flood_persecond;
extern  cvar_t  *flood_waitdelay;
extern	cvar_t	*flood_team;

extern  cvar_t  *maxfps;
extern	cvar_t  *maxrate;
extern	cvar_t  *voosh;


extern  cvar_t  *max_hud_dist;
extern  cvar_t  *light_comp;
extern  cvar_t  *id_x;
extern  cvar_t  *id_y;
extern  cvar_t  *mod;
extern  cvar_t  *hud_freq;
extern  cvar_t  *motd_last;

extern  cvar_t  *hostname;

extern	cvar_t  *speed_msec;
extern	cvar_t  *speed_set;
extern	cvar_t  *speed_bust;

extern	cvar_t  *railwait;
extern	cvar_t  *raildamage;
extern	cvar_t  *railkick;

extern	cvar_t  *flashlight;
extern	cvar_t  *lights;//do not document this one
extern	cvar_t  *lights_out;

extern	cvar_t  *highscores;

extern	cvar_t	*mapvote;
extern	cvar_t	*map_randomize;

extern  cvar_t *rcon;

extern  cvar_t  *banned_items;
extern  cvar_t  *weapflags;
extern  cvar_t  *ammoflags;
extern  cvar_t  *ban_blaster;
extern  cvar_t  *start_weapons;
extern  cvar_t  *start_items;
extern  cvar_t  *armor_spawn;
extern  cvar_t  *ammo_spawn;
extern  cvar_t  *mega_health_spawn;
extern  cvar_t  *health_spawn;
extern  cvar_t  *spawn_time;
extern	cvar_t	*runes;
extern  cvar_t  *runeflags;
extern  cvar_t  *max_Vhealth;
extern  cvar_t  *pickup_tech;
extern  cvar_t  *hide_spawn;
extern  cvar_t  *wavs;
extern  cvar_t  *songtoplay;
extern  cvar_t  *use_song_file;
extern	cvar_t  *camper_check;
extern	cvar_t  *camp_time;
extern	cvar_t  *camp_distance;
extern  cvar_t  *resp_protect;
extern  cvar_t  *motd_line;
//RAV new bot detection
extern  cvar_t	*prox;
extern  cvar_t  *check_models;
//extern  cvar_t	*connect_time;
extern  cvar_t  *speed_check;

extern  cvar_t	*lag_limit;
extern  cvar_t	*menutime;
extern	cvar_t  *clear_teams;
extern	cvar_t  *even_teams;
extern	cvar_t  *reserved_slots;
extern  cvar_t	*reserved_password;

extern  cvar_t	*max_specs;
extern  cvar_t	*op_specs;
//DM ADDITIONS

extern	cvar_t	*sa_shells;
extern	cvar_t	*sa_bullets;
extern	cvar_t	*sa_grenades;
extern	cvar_t	*sa_rockets;
extern	cvar_t	*sa_cells;
extern	cvar_t	*sa_slugs;

extern  cvar_t *rocket_speed;
extern  cvar_t *rl_radius_damage;
extern  cvar_t *rl_radius;
extern  cvar_t  *rocket_wait;
extern  cvar_t  *railgun_wait;
extern  cvar_t  *shotgun_wait;
extern  cvar_t  *sshotgun_wait;
extern  cvar_t  *bfg_wait;
extern  cvar_t  *sg_damage;
extern  cvar_t  *sg_kick;
extern  cvar_t  *ssg_damage;
extern  cvar_t  *ssg_kick;
extern  cvar_t  *rg_damage;
extern  cvar_t  *rg_kick;
extern  cvar_t  *bfg_damage;
extern  cvar_t  *g_damage;
extern  cvar_t  *gl_damage;
extern  cvar_t  *rl_damage;
extern  cvar_t  *b_damage;
extern  cvar_t  *hb_damage;
extern  cvar_t  *mg_damage;
extern  cvar_t  *mg_kick;
extern  cvar_t  *cg_damage;
extern  cvar_t  *cg_kick;
extern  cvar_t  *lag_ping;
//ctf scoring
extern  cvar_t  *cap_point;
extern  cvar_t  *cap_team;
extern  cvar_t  *recover_flag;
extern  cvar_t  *frag_carrier;
extern  cvar_t  *carrier_save;
extern  cvar_t  *carrier_protect;
extern  cvar_t  *flag_defend;
extern  cvar_t  *flag_assist;
extern  cvar_t  *frag_carrier_assist;

//
extern  cvar_t  *show_time;
extern  cvar_t  *show_carrier;
extern  cvar_t  *clan_name;
extern  cvar_t  *clan_pass;
extern  cvar_t  *cl_check;
//
extern	cvar_t	*ctf;
extern	cvar_t	*ctf_forcejoin;

//3ZB
extern	cvar_t	*chedit;
extern	cvar_t	*vwep;
//extern	cvar_t	*maplist1;
extern	cvar_t	*botlist;
extern	cvar_t	*autospawn;
extern	cvar_t	*zigmode;
extern	float	spawncycle;
//
extern	cvar_t  *use_bots;
extern	cvar_t	*bot_num;
extern	cvar_t	*bot_free_clients;
extern	cvar_t	*bot_insult;
extern	cvar_t	*bot_chat;
extern	cvar_t	*bot_camptime;
extern	cvar_t	*bot_walkspeed;//20
extern	cvar_t	*bot_runspeed;//32
extern	cvar_t	*bot_duckpeed;//10
extern	cvar_t	*bot_waterspeed;//16

extern	cvar_t	*lan;
extern  cvar_t  *spec_check;
extern  cvar_t  *damage_locate;
extern  cvar_t  *damage_display;

//JSW
extern	cvar_t	*punish_suicide;
extern	cvar_t	*console_chat;
extern	cvar_t	*no_hum;
extern  cvar_t	*developer;
extern	qboolean	serverlocked;
extern	cvar_t	*basedir;
extern	cvar_t	*hook_carrierspeed;
extern	int		dmflag;
extern	cvar_t	*mercylimit;
extern	cvar_t	*randomrcon;
extern	cvar_t	*defaultoplevel;
extern	cvar_t	*nokill;
float dmflagtimer;
extern	cvar_t	*minfps;
extern	cvar_t	*use_grapple;
extern  cvar_t  *log_connect;
extern  cvar_t  *log_chat;
extern	cvar_t	*grapple_speed;
extern	cvar_t	*grapple_pullspeed;
extern	cvar_t	*grapple_damage;
/*extern	cvar_t	*allow_lasermines;
extern	cvar_t	*allow_energy_lasers;
extern cvar_t	*energylaser_time;
extern cvar_t	*energylaser_damage;
extern cvar_t	*energylaser_cells;
extern cvar_t	*energylaser_mdamage;
extern cvar_t	*energylaser_mradius;
extern cvar_t	*lasermine_timeout;
extern cvar_t	*allownuke;
extern cvar_t	*nuke_slugs;
extern cvar_t	*nuke_cells;
extern cvar_t	*nuke_radius;
extern cvar_t	*nuke_radius2;
*/extern cvar_t	*runes4all;
extern	cvar_t	*quad_notify;
extern	cvar_t	*teamtechs;
extern	cvar_t	*auto_flag_return;
//extern	cvar_t	*uneven_dif;
extern	cvar_t	*tmgclock;
extern	cvar_t	*allow_flagdrop;
extern	cvar_t	*extrasounds;
extern cvar_t	*map_c;
extern cvar_t	*map_r;
extern cvar_t	*map_o;
extern cvar_t	*map_d;
extern	cvar_t	*allow_vote;
extern	cvar_t	*vote_percentage;
extern	cvar_t	*vote_timeout;
extern	cvar_t	*cfgdir;

//JSW - Punish Suicide Flags
#define	PS_RESETSCORE	1
#define	PS_RESPAWNBASE	2
#define	PS_FORCESPEC	4
#define	PS_RESPAWN_CLOSEST	8

//OP flags
#define	OP_SPEC				1
#define	OP_SAY				2
#define	OP_SWITCH			4
#define	OP_CHANGEMAP		8
#define	OP_LOCKTEAMS		16
#define	OP_RESTART			32
#define	OP_KICK				64
#define	OP_STATUS			128
#define	OP_SILENCE			256
#define	OP_LOCKSERVER		512
#define	OP_LISTEN			1024
#define	OP_SHOWBANNEDFILE	2048
#define	OP_BAN				4096
#define	OP_LIGHTS			8192
#define	OP_PROTECTED		16384
#define	OP_ADDOP			32768
#define OP_MODOP			65536
#define	OP_RCON				131072
#define OP_NAMEPASS			262144
#define OP_PLAYERCONTROL	(OP_SWITCH|OP_KICK|OP_SILENCE|OP_BAN|OP_ADDOP|OP_MODOP)

//quad_notify flags
#define QUAD_NOTIFY_EXPIRE	1
#define	QUAD_NOTIFY_TAKEN	2
#define	QUAD_NOTIFY_SPAWN	4
#define	QUAD_NOTIFY_DROP	8
//end

#define world	(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1

typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;


extern	field_t fields[];
extern	gitem_t	itemlist[];


//
// g_cmds.c
//
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
edict_t *Drop_Item (edict_t *ent, gitem_t *item);
void SetRespawn (edict_t *ent, float delay);
void ChangeWeapon (edict_t *ent);
void SpawnItem (edict_t *ent, gitem_t *item);
void Think_Weapon (edict_t *ent);
int ArmorIndex (edict_t *ent);
int PowerArmorType (edict_t *ent);
gitem_t	*GetItemByIndex (int index);
qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *Find_Player_Edict_t (char *s);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);

// Ridah
void stuffcmd(edict_t *ent, char *text);
float	entdist(edict_t *ent1, edict_t *ent2);
void AddModelSkin (char *modelfile, char *skinname);

void my_bprintf (int printlevel, char *fmt, ...);
// end
// ACE compatibility routines
void safe_bprintf (int printlevel, char *fmt, ...);
void safe_centerprintf (edict_t *ent, char *fmt, ...);
void safe_cprintf (edict_t *ent, int printlevel, char *fmt, ...);

//JSW
void convert_string(char *src, char start, char end, char add, char *dest);
qboolean CheckFlood(edict_t *who);
void LogConnect(edict_t *ent, qboolean connect);
void LogChat(char *text);
void RestartLevel();
void MapVoteThink (qboolean passed, qboolean now);
float yesvotes;
float novotes;
qboolean mapvoteactive;
int mapvotetime;
qboolean votemapnow;


typedef struct 
{ 
   int  nummaps;          // number of maps in list 
   char mapname[128][MAX_QPATH];
   char mapnick[128][MAX_QPATH];
   int  currentmap;       // index to current map 
   qboolean active;
   int	votes[128];
   int	currentmapvote;
   int	nextmap;
} maplist_t; 
extern	maplist_t	*maplist;
extern	maplist_t	*maplistBase;

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
qboolean CheckTeamDamage (edict_t *targ, edict_t *attacker);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_NO_ARMOR			0x00000002	// armour does not protect from this damage
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // armor, shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12
#define DEFAULT_SSHOTGUN_COUNT	20

//
// g_monster.c
//
void monster_fire_bullet (edict_t *self, vec3_t start, vec3_t dir, int damage, int kick, int hspread, int vspread, int flashtype, int mod);
void monster_fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int flashtype, int mod);
void monster_fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype, int effect);
void monster_fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int flashtype);
void monster_fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int flashtype);
void monster_fire_railgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int flashtype);
void monster_fire_bfg (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int kick, float damage_radius, int flashtype);
void M_droptofloor (edict_t *ent);
void monster_think (edict_t *self);
void walkmonster_start (edict_t *self);
void swimmonster_start (edict_t *self);
void flymonster_start (edict_t *self);
void AttackFinished (edict_t *self, float time);
void monster_death_use (edict_t *self);
void M_CatagorizePosition (edict_t *ent);
qboolean M_CheckAttack (edict_t *self);
void M_FlyCheck (edict_t *self);
void M_CheckGround (edict_t *ent);

//
// g_misc.c
//
void ThrowHead (edict_t *self, char *gibname, int damage, int type);
void ThrowClientHead (edict_t *self, int damage);
void ThrowGib (edict_t *self, char *gibname, int damage, int type);
void BecomeExplosion1(edict_t *self);

//
// g_ai.c
//
void AI_SetSightClient (void);

void ai_stand (edict_t *self, float dist);
void ai_move (edict_t *self, float dist);
void ai_walk (edict_t *self, float dist);
void ai_turn (edict_t *self, float dist);
void ai_run (edict_t *self, float dist);
void ai_charge (edict_t *self, float dist);
int range (edict_t *self, edict_t *other);

void FoundTarget (edict_t *self);
qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);
qboolean FacingIdeal(edict_t *self);

qboolean visible_box (edict_t *self, edict_t *other);
qboolean visible_fullbox (edict_t *self, edict_t *other);

//
// g_weapon.c
//
void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin);
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick);
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod);
void fire_blaster (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, int effect, qboolean hyper);
void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius);
void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held);
void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius, int radius_damage);
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick);
void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius);

//
// g_ptrail.c
//
void PlayerTrail_Init (void);
void PlayerTrail_Add (vec3_t spot);
void PlayerTrail_New (vec3_t spot);
edict_t *PlayerTrail_PickFirst (edict_t *self);
edict_t *PlayerTrail_PickNext (edict_t *self);
edict_t	*PlayerTrail_LastSpot (void);


//
// g_client.c
//
void respawn (edict_t *ent,qboolean spawn);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);

//
// g_player.c
//
void player_pain (edict_t *self, edict_t *other, float kick, int damage);
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// g_svcmds.c
//
void	ServerCommand (void);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void ValidateSelectedItem (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);

//
// g_pweapon.c
//
void PlayerNoise(edict_t *who, vec3_t where, int type);
void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent));

//
// m_move.c
//
qboolean M_CheckBottom (edict_t *ent);
qboolean M_walkmove (edict_t *ent, float yaw, float dist);
void M_MoveToGoal (edict_t *ent, float dist);
void M_ChangeYaw (edict_t *ent);

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);

//
// g_main.c
//
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);


//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
// ### Hentai ### BEGIN
#define ANIM_REVERSE	6
// ### Hentai ### END
//3ZB
//Zigock client info
#define ALEAT_MAX	10

typedef struct zgcl_s
{
	int			zclass;			//class no.

	int			botindex;		//botlist's index NO.

// true client�p zoom �t���O	
	int			aiming;			//0-not 1-aiming  2-firing zoomingflag
	float		distance;		//zoom����FOV�l
	float		olddistance;	//��zooming FOV�l
	qboolean	autozoom;		//autozoom
	qboolean	lockon;			//lockon flag false-not true-locking

// bot�p	
	int			zcstate;		//status
	int			zccmbstt;		//combat status

	//duck
	float		n_duckedtime;	//non ducked time

	//targets
	edict_t		*first_target;	//enemy		uses LockOntarget(for client)
	float		targetlock;		//target locking time
	short		firstinterval;	//enemy search count
	edict_t		*second_target;	//kindof items
	short		secondinterval;	//item pickup call count

	//waiting
	vec3_t		movtarget_pt;	//moving target waiting point
	edict_t		*waitin_obj;	//for waiting sequence complete

	//basical moving
	float		moveyaw;		//true moving yaw

	//combat
	int			total_bomb;		//total put bomb
	float		gren_time;		//grenade time

	//contents
//	int			front_contents;
	int			ground_contents;
	float		ground_slope;

	//count (inc only)
	int			tmpcount;

	//moving hist
	float		nextcheck;		//checking time
	vec3_t		pold_origin;	//old origin
	vec3_t		pold_angles;	//old angles

	//target object shot
	qboolean	objshot;		


	edict_t		*sighten;		//sighting enemy to me info from entity sight
	edict_t		*locked;		//locking enemy to me info from lockon missile

	//waterstate
	int			waterstate;

	//route
	qboolean	route_trace;
	int			routeindex;		//routing index
	float		rt_locktime;
	float		rt_releasetime;
	qboolean	havetarget;		//target on/off
	int			targetindex;

	//battle
	edict_t		*last_target;	//old enemy
	vec3_t		last_pos;		//old origin
	int			battlemode;		//mode
	int			battlecount;	//temporary count
	int			battlesubcnt;	//subcount
	int			battleduckcnt;	//duck
	float		fbattlecount;	//float temoporary count
	vec3_t		vtemp;			//temporary vec
	int			foundedenemy;	//foundedenemy
	char		secwep_selected;//secondweapon selected

	vec3_t		aimedpos;		//shottenpoint
	qboolean	trapped;		//trapflag

	//team
	short		tmplstate;		//teamplay state
	short		ctfstate;		//ctf state
	edict_t		*followmate;	//follow
	float		matelock;		//team mate locking time
} zgcl_t;

enum {
	PMENU_ALIGN_LEFT,
	PMENU_ALIGN_CENTER,
	PMENU_ALIGN_RIGHT
};

typedef struct pmenuhnd_s {
	struct pmenu_s *entries;
	int cur;
	int num;
	qboolean UseNumberKeys; //If true, number keys will select a menu item
	float MenuTimeout;	//If set, menu will time out and be removed after a set amount of time
	qboolean ShowBackground;	//Set if background should be shown
} pmenuhnd_t;

typedef struct pmenu_s {
	char *text;
	int align;
//	void *arg;
	int arg;
	void (*SelectFunc)(edict_t *ent, struct pmenu_s *entry);
} pmenu_t;

void PMenu_Open(edict_t *ent, pmenu_t *entries, int cur, int num, qboolean usekeys, qboolean showbackground);
void PMenu_Close(edict_t *ent);
void PMenu_Update(edict_t *ent);
void PMenu_Next(edict_t *ent);
void PMenu_Prev(edict_t *ent);
void PMenu_Select(edict_t *ent);
int WFMenuFromNumberKey(edict_t *ent, int slot);


//

// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	qboolean	powerArmorActive;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;

	gitem_t		*weapon;
	gitem_t		*lastweapon;

	int			power_cubes;	// used for tracking the cubes in coop games
	int			score;			// for calculating total unit score in coop games

//RAV
	int           pl_state; //playing 1, spec 0 , 2 warmup ,need spawned 3,cheat bot 5 , 
	int		   isop; //operators
//JSW	int		   ismop; //match operators
	qboolean   motd;
	qboolean   db_hud;
	qboolean   db_id;
	qboolean   name_set;
	qboolean   skin_set;
	char name_change [16];
	char skin_change [24];
	int pitchspeed;
	int anglespeed;
	int glmonolightmap;
	qboolean   in_game;
	qboolean	HasVoted;
	char ip[24];
	//only allow one vote for a map per client per map !
	int vote_times;
	//JSW
	int	saytype;
	int	oplevel;
	char namepass[16];
	int	hookmsg;
	//end
//

} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
//ZOID
	int			ctf_team;			// CTF team
	int			ctf_state;
	float		ctf_lasthurtcarrier;
	float		ctf_lastreturnedflag;
	float		ctf_flagsince;
	float		ctf_lastfraggedcarrier;
	qboolean	id_state;
//ZOID
//3ZB
	int			context;
//
	vec3_t		cmd_angles;			// angles sent over in the last command
	int			game_helpchanged;
	int			helpchanged;
//RAV
	int			frags;
	int			suicide;
	int			deaths;
	int			speed_cheat;
	int			mayVote;
	int			spectator;
	int			menu_time;
	float		startframe;
	int			hook_wait;//stop the hook bug

	//JSW
	qboolean	bonus;
	qboolean	hasrcon;
	int	iddist;
	qboolean	no_hum;
	int	teamswitch;
	int spree;
	int bigspree;
	qboolean vote;
	//end

//

} client_respawn_t;

//========================================================================================
// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	qboolean	showscores;			// set layout stat
//ZOID
	qboolean	inmenu;				// in menu
	pmenuhnd_t	*menu;				// current menu
//ZOID
	qboolean	showinventory;		// set layout stat
	qboolean	showhelp;
	qboolean	showhelpicon;

	int			ammo_index;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	qboolean	weapon_thunk;

	gitem_t		*newweapon;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			damage_armor;		// damage absorbed by armor
	int			damage_parmor;		// damage absorbed by power armor
	int			damage_blood;		// damage taken out of health
	int			damage_knockback;	// impact damage
	vec3_t		damage_from;		// origin for vector calculation

	float		killer_yaw;			// when dead, look at killer

	weaponstate_t	weaponstate;
	vec3_t		kick_angles;	// weapon kicks
	vec3_t		kick_origin;
	float		v_dmg_roll, v_dmg_pitch, v_dmg_time;	// damage kicks
	float		fall_time, fall_value;		// for view drop on fall
	float		damage_alpha;
	float		bonus_alpha;
	vec3_t		damage_blend;
	vec3_t		v_angle;			// aiming direction
	float		bobtime;			// so off-ground doesn't change it
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	float		next_drown_time;
	int			old_waterlevel;
	int			breather_sound;

	int			machinegun_shots;	// for weapon raising

	// animation vars
	int			anim_end;
	int			anim_priority;
	qboolean	anim_duck;
	qboolean	anim_run;

	// powerup timers
	float		quad_framenum;
	float		invincible_framenum;
	float		breather_framenum;
	float		enviro_framenum;

	qboolean	grenade_blew_up;
	float		grenade_time;
	int			silencer_shots;
	int			weapon_sound;

	float		pickup_msg_time;

	float		respawn_time;		// can respawn when time > this
	
    int			chase_mode;
	int cam;
	

//ZOID
	edict_t		*ctf_grapple;		// entity of grapple
	int			ctf_grapplestate;		// true if pulling
	float		ctf_grapplereleasetime;	// time of grapple release
	float		ctf_grapplestart;
	float		ctf_regentime;		// regen tech
	float		ctf_techsndtime;
	float		ctf_lasttechmsg;
	edict_t		*chase_target;
	qboolean	update_chase;
//ZOID
//RAV
	edict_t		*hook;				// Pointer to grappling hook entity
	qboolean	hook_out;
	qboolean	hook_on;

   float    flood_locktill;    // locked from talking
   float    flood_when[10];    // when messages were said
   int      flood_whenhead;    // head pointer for when said
   float checkframe;
   float hudtime;
   float nametime;
   float skintime;
   float		respawn_framenum;
   int			damage_rune;
   float		rune_time;
   float		regen_acc;
   float   was_stealth;
//anti camping
	qboolean check_camping;
	vec3_t camp_pos;
	float camp_time;
	qboolean camp_warned;
	int camp_move_time;
//end
	qboolean needschecked;
	float checktime;


	int ping1;
	int ping2;
	int ping3;
	int ping4;
	float checkpingtime;
	qboolean overflowed;
//3ZB
	zgcl_t		zc;					//zigock client info
	float    chattime;         //time of next chat
	float    insulttime;       //time of next insult
	float    taunttime;        //time of next taunting;
	float    camptime;         //time spent camping
	vec3_t   lastorigin;       //origin of camping spot
	gitem_t  *campitem;        //item camping near
	float   nextcamp;
//
	//JSW
	qboolean	kill;	//was the last death by the kill command?
};


struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque
	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf
	
	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;
//3ZB
	edict_t		*union_ent;			//union item
	edict_t		*trainteam;			//train team
	int			arena;				//arena
//
	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	float		air_finished;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		yaw_speed;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		pain_debounce_time;
	float		damage_debounce_time;
	float		fly_sound_debounce_time;	//move to clientinfo
	float		last_move_time;

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;
	qboolean	show_hostile;

	float		powerarmor_time;

	char		*map;			// target_changelevel

	int			viewheight;		// height above origin where eyesight is determined
	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;
	int			sounds;			//make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;
	int			noise_index2;
	float		volume;
	float		attenuation;

	// timing variables
	float		wait;
	float		delay;			// before firing targets
	float		random;

	float		teleport_time;

	int			watertype;
	int			waterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	// move this to clientinfo?
	int			light_level;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;
	float misc_time[30];
	float spec_time;
	qboolean spec_warned;
	float spec_kick;
	qboolean speccheck;
	
	qboolean	bot_client;

//RAV
float   flastime;//flashlight timer (stops laggers)
	edict_t		*laser; 
	edict_t		*flashlight;
	float		stealth;
	qboolean	noblock;
	float		hooktime;//for bots on the laser hook
	int			tries;
	float		reset_time;
    float		enter_frame;
	float		bust_time;
	int			command;
	float		commandtimeout;

	qboolean is_muted;
	qboolean busted;
	int		display;
	float   pausetime;
//
	//JSW
	qboolean	hasflag;
	float		flagtimer;
};


//ZOID
#include "g_ctf.h"
//ZOID
//RAV
void FL_think (edict_t *self);
void FL_make(edict_t *self);
#include "anticheat.h"
#include "timer.h"
void LoadHighScores (void);
void highscore (void);
char hscores [1400];
qboolean hsran;//RAV highscores
qboolean show_hs;
qboolean  hs_show;
//RUNES
vec3_t v_forward;
vec3_t v_right;
vec3_t v_up;
//Track the flag
float      flagchecktime;
float redflagtime;
float blueflagtime;
vec3_t redflagnow;
vec3_t blueflagnow;
vec3_t redflag_origin;
vec3_t blueflag_origin;
qboolean redflaggone;
qboolean blueflaggone;
//System Time
void GetTime();
char sys_time[8];
void GetDate();
char sys_date[10];
//Camper check
#define CAMP_WARN_TIME camp_time->value - 10// sec to camp before warning
#define CAMP_TIME camp_time->value  // sec to be officially camping
#define CAMP_DISTANCE camp_distance->value // camping radius
#define CAMP_WARN_MSG "Hey, quit camping!!(Observer in 10 sec)"
#define CAMP_MSG "You've been removed for (camping)"
void CheckForCamping (edict_t *ent);
//
#define rndnum(y,z) ((random()*((z)-((y)+1)))+(y))
extern gitem_t *flag1_item;
extern gitem_t *flag2_item;
//map.c
int  LoadMapList(edict_t *ent, char *filename); 
void ClearMapList(); 
void Cmd_Maplist_f (edict_t *ent); 
void Display_Maplist_Usage(edict_t *ent);
void ClearMapVotes() ;
int MapMaxVotes();
void VoteForMap(int i);
void DumpMapVotes();
void MaplistNextMap();
void Cmd_ShowVotes_f(edict_t *ent);

//Voting /MAP
qboolean	voted;
float	votetime;
//MENUS
//void map_mod_set_up(void);
//void SpecialMenu(edict_t *ent);
void MapVote(edict_t *ent);

qboolean G_EntExists(edict_t *ent);
qboolean G_ClientInGame(edict_t *ent);
qboolean locked_teams; // teams status
qboolean mapscrewed;
char defaultmap[40];
//even teams stuff
qboolean notfairRED;
qboolean notfairBLUE;
float redtime;
float bluetime;
qboolean force_specs;
char time_left[5];
char time_left2[5];
qboolean techspawn;

//3ZB
//general func
void player_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//bot spawn & remove
qboolean	SpawnBot(int i);
void		Bot_LevelChange(void);
void		Load_BotInfo(void);
void		Bot_SpawnCall(void);
void		RemoveBot(void);
void		SpawnBotReserving(void);

//weapon
void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);
void CTFWeapon_Grapple (edict_t *ent);
int hstime;

// wideuse
qboolean Bot_trace (edict_t *ent,edict_t *other);
qboolean Bot_trace2 (edict_t *ent,vec3_t ttz);
float Get_yaw (vec3_t vec);		//
float Get_pitch (vec3_t vec);	//
float Get_vec_yaw (vec3_t vec,float yaw);
void ShowGun(edict_t *ent);
void SpawnItem3 (edict_t *ent, gitem_t *item);
int Bot_moveT ( edict_t *ent,float ryaw,vec3_t pos,float dist,float *bottom);
void Set_BotAnim(edict_t *ent,int anim,int frame,int end);
void plat_go_up (edict_t *ent);
int Get_KindWeapon(gitem_t	*it);
qboolean TargetJump(edict_t *ent,vec3_t tpos);
qboolean Bot_traceS (edict_t *ent,edict_t *other);
qboolean Bot_Fall(edict_t *ent,vec3_t pos,float dist);

void SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles);
void ClientUserinfoChanged (edict_t *ent, char *userinfo);
void CopyToBodyQue (edict_t *ent);

//route util
qboolean TraceX (edict_t *ent,vec3_t p2);
void Move_LastRouteIndex(void);
void Get_RouteOrigin(int index,vec3_t pos);

//Bot Func
void ZigockJoinMenu(edict_t *ent);
qboolean ZigockStartClient(edict_t *ent);
void Cmd_AirStrike(edict_t *ent);
void BotEndServerFrame (edict_t *ent);
void SpawnItem2 (edict_t *ent, gitem_t *item);
void Get_WaterState(edict_t *ent);
void Bot_Think (edict_t *self);
void PutBotInServer (edict_t *ent);
void SpawnBotReserving2(int *red,int *blue);

//Combat AI
void Combat_Level0(edict_t *ent,int foundedenemy,int enewep,float aim,float distance,int skill);
void Combat_LevelX(edict_t *ent,int foundedenemy,int enewep,float aim,float distance,int skill);
void UsePrimaryWeapon(edict_t *ent);

//Explotion Index
void UpdateExplIndex(edict_t* ent);

//flag
qboolean ZIGDrop_Flag(edict_t *ent, gitem_t *item);

//p_view.c
void BotEndServerFrame (edict_t *ent);

//Bot AI routine
void Bots_Move_NORM (edict_t *ent);		//normal AI

//spawn
void SetBotFlag1(edict_t *ent);	//�`�[��1�̊�
void SetBotFlag2(edict_t *ent);  //�`�[��2�̊�
void CTFSetupNavSpawn(void);	//�i�r�̐ݒu

//ctf
void CTFJobAssign (void);		//job assign

//----------------------------------------------------------------

//moving speed
#define	MOVE_SPD_WALK	(int)bot_walkspeed->value//20
#define MOVE_SPD_RUN	(int)bot_runspeed->value//32
#define MOVE_SPD_DUCK	(int)bot_duckpeed->value//10
#define MOVE_SPD_WATER	(int)bot_waterspeed->value//16
#define MOVE_SPD_JUMP	32
#define VEL_BOT_JUMP	340//341		//jump vel
//#define VEL_BOT_ROCJ	500		//roc jump
#define VEL_BOT_WJUMP	341//150		//waterjump vel
#define VEL_BOT_LADRUP	200			//ladderup vel
#define VEL_BOT_WLADRUP	200	//0			//water ladderup gain


//classes
#define CLS_NONE	0	//normal
#define CLS_ALPHA	1	//sniper
#define CLS_BETA	2
#define CLS_GAMMA	3
#define CLS_DELTA	4
#define CLS_EPSILON	5
#define CLS_ZETA	6
#define CLS_ETA		7

// function's state P
#define	PSTATE_TOP			0
#define	PSTATE_BOTTOM		1
#define PSTATE_UP			2
#define PSTATE_DOWN			3

#define PDOOR_TOGGLE		32

// height
#define TOP_LIMIT			52
#define TOP_LIMIT_WATER		100
#define BOTTOM_LIMIT		-52
#define BOTTOM_LIMIT_WATER	-8190
#define BOTTOM_LIMITM		-300

//waterstate
#define	WAS_NONE			0
#define	WAS_FLOAT			1
#define	WAS_IN				2

//route
//chaining pod state
#define GRS_NORMAL		0
#define GRS_ONROTATE	1
#define GRS_TELEPORT	2
#define GRS_ITEMS		3
#define GRS_ONPLAT		4
#define	GRS_ONTRAIN		5
#define GRS_ONDOOR		6
#define GRS_PUSHBUTTON	7

#define GRS_GRAPSHOT	20
#define GRS_GRAPHOOK	21
#define GRS_GRAPRELEASE	22

#define GRS_REDFLAG		-10
#define GRS_BLUEFLAG	-11

#define POD_LOCKFRAME	15	//20
#define POD_RELEFRAME	20	//25

#define MAX_SEARCH		12	//max search count/FRAMETIME
#define MAX_DOORSEARCH	10

//trace param
#define	TRP_NOT			0	//don't trace
#define TRP_NORMAL		1	//trace normal
#define	TRP_ANGLEKEEP	2	//trace and keep angle
#define TRP_MOVEKEEP	3	//angle and move vec keep but move
#define TRP_ALLKEEP		4	//don't move

// bot spawning status
#define BOT_SPAWNNOT	0
#define BOT_SPRESERVED	1
#define BOT_SPAWNED		2
#define BOT_NEXTLEVEL	3

//combat
#define AIMING_POSGAP		5
#define AIMING_ANGLEGAP_S	0.75	//shot gun
#define AIMING_ANGLEGAP_M	0.35 //machine gun

//team play state
#define TMS_NONE		0
#define TMS_LEADER		1
#define TMS_FOLLOWER	2

//ctf state
#define CTFS_NONE		0
#define CTFS_CARRIER	1
#define CTFS_ROAMER		2
#define CTFS_OFFENCER	3
#define CTFS_DEFENDER	4
#define CTFS_SUPPORTER	5

#define FOR_FLAG1		1
#define FOR_FLAG2		2

//fire----------------------------------------------------------
#define FIRE_SLIDEMODE		0x00000001	//slide with route
#define	FIRE_PRESTAYFIRE	0x00000002	//X	pre don't move fire
#define FIRE_STAYFIRE		0x00000004	//X	don't move
#define FIRE_CHIKEN			0x00000008	//X chiken fire
#define FIRE_RUSH			0x00000010	//X	rush
#define FIRE_JUMPNRUSH		0x00000020	//
#define	FIRE_ESTIMATE		0x00000040	//X estimate �\��
#define FIRE_SCATTER		0x00000080	//scatter �o���T��
#define	FIRE_RUNNIN			0x00000100	//run & shot(normal)
#define FIRE_JUMPROC		0x00000200	//X �W�����v�ӂ�����

#define FIRE_REFUGE			0x00001000	//X	���
#define FIRE_EXPAVOID		0x00002000	//X	�����悯

#define FIRE_QUADUSE		0x00004000	//X	Quad���̘A�˕���I��
#define FIRE_AVOIDINV		0x00008000	//X ���肪�y���^�̎�������

#define FIRE_BFG			0x00010000	//X ���ʂ�BFG������

#define FIRE_SHIFT_R		0x00020000	//X �E�X���C�h
#define FIRE_SHIFT_L		0x00040000	//X ���X���C�h

#define FIRE_SHIFT			(FIRE_SHIFT_R | FIRE_SHIFT_L)//X �E�X���C�h

#define FIRE_REFLECT		0x00080000	// �ǂɔ��˂�����

#define FIRE_IGNORE			0x10000000	//�������ē�����

// means of death

#define MOD_SNIPERAIL			50		//SNIPE RAIL
#define MOD_LOCMISSILE			51		//LOCKON MISSILE

#define MOD_BFG100K				52		//

#define MOD_AIRSTRIKE			70		//AIRSTRIKE
//----------------------------------------------------------------
//general status list
#define STS_IDLE		0x00000000	//normal running
#define STS_THINK		0x00000001	//stand and analise
#define STS_LADDERUP	0x00000002	//crimb the ladder
#define STS_ROCJ		0x00000004	//rocket jumping
#define STS_TURBOJ		0x00000008	//turbo jump
#define STS_WATERJ		0x00000010	//turbo jump

#define STS_SJMASK		(STS_ROCJ | STS_TURBOJ | STS_WATERJ)	//special jump mask
#define STS_SJMASKEXW	(STS_ROCJ | STS_TURBOJ )				//special jump mask ex. water


#define STS_TALKING		0x00000200	//talking
#define STS_ESC_WXPL	0x00000400	//escape from explode

#define STS_MOVE_WPOINT	0x00000800	//moving waiting point
//#define STS_W_EXPL		0x00001000	//wait for end of explode

//wait
#define STS_W_DONT		0x00001000	//don't wait door or plat
#define STS_W_DOOROPEN	0x00002000	//wait for door open or down to bottom
#define STS_W_ONPLAT	0x00004000	//wait for plat or door reach to da top
#define STS_W_ONDOORUP	0x00008000	//wait for door reach to da top
#define STS_W_ONDOORDWN	0x00010000	//wait for door reach to da bottom
#define STS_W_ONTRAIN	0x00020000	//wait for plat or door reach to da top
#define STS_W_COMETRAIN	0x00040000  //wait for train come
#define STS_W_COMEPLAT	0x00080000  //wait for plat come

#define STS_WAITS		(STS_W_DONT | STS_W_DOOROPEN | STS_W_COMEPLAT | STS_W_ONPLAT | STS_W_ONDOORUP | STS_W_ONDOORDWN | STS_W_ONTRAIN)
#define STS_WAITSMASK	(STS_W_DOOROPEN | STS_W_ONPLAT | STS_W_ONDOORUP | STS_W_COMEPLAT | STS_W_ONDOORDWN | STS_W_ONTRAIN)
#define STS_WAITSMASK2	(STS_W_ONDOORDWN |STS_W_ONDOORUP | STS_W_ONPLAT | STS_W_ONTRAIN)
#define STS_WAITSMASKCOM (STS_W_DOOROPEN | STS_W_ONPLAT | STS_W_ONDOORUP | STS_W_ONDOORDWN | STS_W_ONTRAIN)
//----------------------------------------------------------------
//general status list
#define CTS_ENEM_NSEE	0x00000001	//have enemy but can't see
#define CTS_AGRBATTLE	0x00000002	//aglessive battle
#define	CTS_ESCBATTLE	0x00000004	//escaping battle(item want)
#define CTS_HIPBATTLE	0x00000008	//high position battle(camp)


//shoot
#define CTS_PREAIMING	0x00000010	//prepare for snipe or lockon
#define CTS_AIMING		0x00000020	//aimning for snipe or lockon
#define CTS_GRENADE		0x00000040  //hand grenade mode
#define CTS_JUMPSHOT	0x00000080	//jump shot

#define CTS_COMBS (CTS_AGRBATTLE | CTS_ESCBATTLE | CTS_HIPBATTLE | CTS_ENEM_NSEE)

//----------------------------------------------------------------
//route struct
#define MAXNODES			10000	//5000 added 5000 pods
#define MAXLINKPOD			6		//don't modify this
#define CTF_FLAG1_FLAG		0x0000
#define CTF_FLAG2_FLAG		0x8000

typedef struct
{
	vec3_t	Pt;		//target point
	union
	{
		vec3_t			Tcourner;				//target courner(train and grap-shot only)
		unsigned short	linkpod[MAXLINKPOD];	//(GRS_NORMAL,GRS_ITEMS only 0 = do not select pod)
	};
	edict_t	*ent;	//target ent
	short	index;	//index num
	short	state;	//targetstate
} route_t;

//----------------------------------------------------------------
//bot info struct
#define MAXBOTS		64
#define MAXBOP		16

// bot params
#define BOP_WALK		0	//flags
#define BOP_AIM			1	//aiming
#define BOP_PICKUP		2	//frq PICKUP
#define BOP_OFFENCE		3	//chiken fire etc.
#define BOP_COMBATSKILL	4	//combat skill
#define BOP_ROCJ		5	//rocket jump
#define BOP_REACTION	6	//reaction skill exp. frq SEARCH ENEMY
#define BOP_VRANGE		7	//V-View of RANGE	�c
#define BOP_HRANGE		8	//H-View of Range	��
#define BOP_PRIWEP		9	//primary weapon
#define BOP_SECWEP		10	//secondary weapon
#define BOP_DODGE		11	//dodge
#define BOP_ESTIMATE	12	//estimate
#define BOP_NOISECHK	13	//noisecheck
#define BOP_NOSTHRWATER	14	//can't see through water
#define BOP_TEAMWORK	15	//teamwork

typedef	struct
{
	char	netname[21];		//netname
	char	model[21];			//model
	char	skin[21];			//skin
	int		spflg;				//spawned flag 0-not 1-waiting 2-spawned
	int		team;				//team NO. 0-noteam 1-RED 2-BLUE
	int		arena;				//if arena is on
	unsigned char	param[MAXBOP];		//Params
}	botinfo_t;

//----------------------------------------------------------------
//message section name
#define MESS_DEATHMATCH		"[MessDeathMatch]"
#define MESS_CHAIN_DM		"[MessChainDM]"
#define	MESS_CTF			"[MessCTF]"
#define MESS_CHAIN_CTF		"[MessChainCTF]"

//----------------------------------------------------------------
//bot list section name
#define BOTLIST_SECTION_DM	"[BotList]"
#define BOTLIST_SECTION_TM	"[BotListTM]"

//----------------------------------------------------------------
#define MAX_BOTSKILL		10

#define FALLCHK_LOOPMAX	30

//laser Index
#define MAX_LASERINDEX		30
extern edict_t*		LaserIndex[MAX_LASERINDEX];

//Explotion Index
#define MAX_EXPLINDEX		12
extern edict_t*		ExplIndex[MAX_EXPLINDEX];
//


extern	int			cumsindex;
extern	int			targetindex;		//debugtarget

extern	int			ListedBotCount;		//bot count of list
float bot_time;
float wait_time;
float kill_time;
extern	int			SpawnWaitingBots;
extern	char		ClientMessage[MAX_STRING_CHARS];
extern	botinfo_t	Bot[MAXBOTS];
extern	route_t		Route[MAXNODES];
extern	int			CurrentIndex;
extern	float		JumpMax;
extern	int			botskill;
extern	int			trace_priority;
extern	int			FFlg[MAX_BOTSKILL];
extern int NumBotsInGame; //Raven
extern	int			ListedBots;

//for avoid abnormal frame error
extern	int			skullindex;
extern	int			headindex;


//item index
extern	int			mpindex[MPI_INDEX];

//PON-CTF
extern	edict_t		*bot_team_flag1;
extern	edict_t		*bot_team_flag2;
//PON-CTF

//pre searched items
extern	gitem_t	*Fdi_GRAPPLE;
extern	gitem_t	*Fdi_BLASTER;
extern	gitem_t *Fdi_SHOTGUN;
extern	gitem_t *Fdi_SUPERSHOTGUN;
extern	gitem_t *Fdi_MACHINEGUN;
extern	gitem_t *Fdi_CHAINGUN;
extern	gitem_t *Fdi_GRENADES;
extern	gitem_t *Fdi_GRENADELAUNCHER;
extern	gitem_t *Fdi_ROCKETLAUNCHER;
extern	gitem_t *Fdi_HYPERBLASTER;
extern	gitem_t *Fdi_RAILGUN;
extern	gitem_t *Fdi_BFG;

extern	gitem_t *Fdi_SHELLS;
extern	gitem_t *Fdi_BULLETS;
extern	gitem_t *Fdi_CELLS;
extern	gitem_t *Fdi_ROCKETS;
extern	gitem_t *Fdi_SLUGS;
extern	float	ctfjob_update;
//
qboolean mapvotefilled;

//#include "g_ctf.h"
//JSW
void	ShowFile (edict_t *ent, char *filename);
void	ResetCaps(void);
void	CheckPlayers(void);

typedef struct 
{
	int	level;
    int	newlevel;
	char	entry[IP_LENGTH];
	char	namepass[16];
	char	name[IP_LENGTH];
	char	ip[IP_LENGTH];
	qboolean	flagged;
} oplist_t;

extern oplist_t	*oplist;
extern oplist_t	*oplistBase;
int entriesinopfile;

typedef struct ctfgame_s
{
	int team1, team2;
	int total1, total2; // these are only set when going into intermission!
	int players1, players2, players3, specs;
	float last_flag_capture;
	int last_capture_team;
} ctfgame_t;
ctfgame_t ctfgame;
