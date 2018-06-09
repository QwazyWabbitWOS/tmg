
#include "g_local.h"
#include "g_items.h"
#include "anticheat.h"
#include "filehand.h"
#include "p_client.h"
#include "s_map.h"
#include "timer.h"
#include "filtering.h"
#include "hud.h"
#include "bot.h"
#include "g_ctf.h"
#include "highscore.h"
#include "log_manager.h"
#include "intro.h"
#include "statslog.h"

//
field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"accel", FOFS(accel), F_FLOAT},
	{"decel", FOFS(decel), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"pathtarget", FOFS(pathtarget), F_LSTRING},
	{"deathtarget", FOFS(deathtarget), F_LSTRING},
	{"killtarget", FOFS(killtarget), F_LSTRING},
	{"combattarget", FOFS(combattarget), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"delay", FOFS(delay), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"move_origin", FOFS(move_origin), F_VECTOR},
	{"move_angles", FOFS(move_angles), F_VECTOR},
	{"style", FOFS(style), F_INT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"sounds", FOFS(sounds), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(dmg), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"mass", FOFS(mass), F_INT},
	{"volume", FOFS(volume), F_FLOAT},
	{"attenuation", FOFS(attenuation), F_FLOAT},
	{"map", FOFS(map), F_LSTRING},

	// temp spawn vars -- only valid when the spawn function is called
	{"lip", STOFS(lip), F_INT, FFL_SPAWNTEMP},
	{"distance", STOFS(distance), F_INT, FFL_SPAWNTEMP},
	{"height", STOFS(height), F_INT, FFL_SPAWNTEMP},
	{"noise", STOFS(noise), F_LSTRING, FFL_SPAWNTEMP},
	{"pausetime", STOFS(pausetime), F_FLOAT, FFL_SPAWNTEMP},
	{"item", STOFS(item), F_LSTRING, FFL_SPAWNTEMP},
	{"gravity", STOFS(gravity), F_LSTRING, FFL_SPAWNTEMP},
	{"sky", STOFS(sky), F_LSTRING, FFL_SPAWNTEMP},
	{"skyrotate", STOFS(skyrotate), F_FLOAT, FFL_SPAWNTEMP},
	{"skyaxis", STOFS(skyaxis), F_VECTOR, FFL_SPAWNTEMP},
	{"minyaw", STOFS(minyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"maxyaw", STOFS(maxyaw), F_FLOAT, FFL_SPAWNTEMP},
	{"minpitch", STOFS(minpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"maxpitch", STOFS(maxpitch), F_FLOAT, FFL_SPAWNTEMP},
	{"nextmap", STOFS(nextmap), F_LSTRING, FFL_SPAWNTEMP}
};

// -------- just for savegames ----------
// all pointer fields should be listed here, or savegames
// won't work properly (they will crash and burn).
// this wasn't just tacked on to the fields array, because
// these don't need names, we wouldn't want map fields using
// some of these, and if one were accidentally present twice
// it would double swizzle (fuck) the pointer.

field_t		savefields[] =
{
	{"", FOFS(classname), F_LSTRING},
	{"", FOFS(target), F_LSTRING},
	{"", FOFS(targetname), F_LSTRING},
	{"", FOFS(killtarget), F_LSTRING},
	{"", FOFS(team), F_LSTRING},
	{"", FOFS(pathtarget), F_LSTRING},
	{"", FOFS(deathtarget), F_LSTRING},
	{"", FOFS(combattarget), F_LSTRING},
	{"", FOFS(model), F_LSTRING},
	{"", FOFS(map), F_LSTRING},
	{"", FOFS(message), F_LSTRING},

	{"", FOFS(client), F_CLIENT},
	{"", FOFS(item), F_ITEM},

	{"", FOFS(goalentity), F_EDICT},
	{"", FOFS(movetarget), F_EDICT},
	{"", FOFS(enemy), F_EDICT},
	{"", FOFS(oldenemy), F_EDICT},
	{"", FOFS(activator), F_EDICT},
	{"", FOFS(groundentity), F_EDICT},
	{"", FOFS(teamchain), F_EDICT},
	{"", FOFS(teammaster), F_EDICT},
	{"", FOFS(owner), F_EDICT},
	{"", FOFS(mynoise), F_EDICT},
	{"", FOFS(mynoise2), F_EDICT},
	{"", FOFS(target_ent), F_EDICT},
	{"", FOFS(chain), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		levelfields[] =
{
	{"", LLOFS(changemap), F_LSTRING},

	{"", LLOFS(sight_client), F_EDICT},
	{"", LLOFS(sight_entity), F_EDICT},
	{"", LLOFS(sound_entity), F_EDICT},
	{"", LLOFS(sound2_entity), F_EDICT},

	{NULL, 0, F_INT}
};

field_t		clientfields[] =
{
	{"", CLOFS(pers.weapon), F_ITEM},
	{"", CLOFS(pers.lastweapon), F_ITEM},
	{"", CLOFS(newweapon), F_ITEM},

	{NULL, 0, F_INT}
};

//
/*
============
InitGame

This will be called when the dll is first loaded, which
only happens when a new game is begun
============
*/
void InitGame (void)
{

#ifdef	_WIN32
	_CrtMemCheckpoint(&startup1);
#endif

	gi.dprintf ("==== InitGame (TMG "MOD_VERSION" "BUILD" version built "__DATE__") ====\n");
	gi.dprintf ("\n===========================================\n");
	gi.dprintf (" This game library contains the bot\n");
	gi.dprintf ("    detection system developed\n");
	gi.dprintf ("        by Doug 'RaVeN' Buckley.\n");
	gi.dprintf ("TMGBot 'Base' Bot code developed By Ponpoko\n");
	gi.dprintf ("  Corrections, improvements and bug fixes\n");
	gi.dprintf ("              by QwazyWabbit\n");
	gi.dprintf ("===========================================\n\n");
	bot_team_flag1 = NULL;
	bot_team_flag2 = NULL;

	gun_x = gi.cvar ("gun_x", "0", 0);
	gun_y = gi.cvar ("gun_y", "0", 0);
	gun_z = gi.cvar ("gun_z", "0", 0);

	//FIXME: sv_ prefix is wrong for these
	sv_rollspeed = gi.cvar ("sv_rollspeed", "200", 0);
	sv_rollangle = gi.cvar ("sv_rollangle", "2", 0);
	sv_maxvelocity = gi.cvar ("sv_maxvelocity", "2000", 0);
	sv_gravity = gi.cvar ("sv_gravity", "800", 0);

	// noset vars
	dedicated = gi.cvar ("dedicated", "1", CVAR_NOSET);

	ctf = gi.cvar("ctf", "1", CVAR_SERVERINFO | CVAR_LATCH);
	game_dir = gi.cvar ("game", "ctf",  CVAR_NOSET);

	// latched vars
	sv_cheats = gi.cvar ("cheats", "0", CVAR_LATCH);
	if (ctf->value)
		gi.cvar ("gamename", "TMG_CTF", CVAR_SERVERINFO | CVAR_LATCH);
	else
		gi.cvar ("gamename", "TMG_DM", CVAR_SERVERINFO | CVAR_LATCH);
	gi.cvar ("gamedate", __DATE__, CVAR_SERVERINFO | CVAR_LATCH);
	
	motdfile = gi.cvar ("motdfile", "motd.txt", 0);

	maxclients = gi.cvar ("maxclients", "4", CVAR_SERVERINFO | CVAR_LATCH);
	deathmatch = gi.cvar ("deathmatch", "0", CVAR_LATCH);
	coop = gi.cvar ("coop", "0", CVAR_LATCH);
	skill = gi.cvar ("skill", "1", CVAR_LATCH);
	maxentities = gi.cvar ("maxentities", "1024", CVAR_LATCH);

	//ZOID
	//This game.dll only supports deathmatch
	if (!deathmatch->value) {
		gi.dprintf("Forcing deathmatch.");
		gi.cvar_set("deathmatch", "1");
	}
	//force coop off
	gi.cvar_set("coop", "0");
	//ZOID

	// change anytime vars
	dmflags = gi.cvar ("dmflags", "0", CVAR_SERVERINFO|CVAR_ARCHIVE);
	fraglimit = gi.cvar ("fraglimit", "0", CVAR_SERVERINFO);
	timelimit = gi.cvar ("timelimit", "0", CVAR_SERVERINFO);
	//ZOID
	capturelimit = gi.cvar ("capturelimit", "0", CVAR_SERVERINFO);
	//ZOID
	passwd = gi.cvar ("password", "", CVAR_USERINFO);

	g_select_empty = gi.cvar ("g_select_empty", "0", CVAR_ARCHIVE);

	run_pitch = gi.cvar ("run_pitch", "0.002", 0);
	run_roll = gi.cvar ("run_roll", "0.005", 0);
	bob_up  = gi.cvar ("bob_up", "0.005", 0);
	bob_pitch = gi.cvar ("bob_pitch", "0.002", 0);
	bob_roll = gi.cvar ("bob_roll", "0.002", 0);

	//chain edit flag
	chedit = gi.cvar ("chain", "0", CVAR_LATCH);

	view_weapons = gi.cvar ("view_weapons", "1", CVAR_LATCH);

	// all hook related vars
	use_hook = gi.cvar("use_hook", "1", CVAR_SERVERINFO);
	hook_speed = gi.cvar("hook_speed", "2200", CVAR_SERVERINFO);
	hook_pullspeed = gi.cvar("hook_pullspeed", "900", CVAR_SERVERINFO);
	hook_sky = gi.cvar("hook_sky", "1", 0);
	hook_maxtime = gi.cvar("hook_maxtime", "5", 0);
	hook_damage = gi.cvar("hook_damage", "5", 0);
	hook_reset = gi.cvar("hook_reset", "0", 0);
	hook_color = gi.cvar("hook_color", "7", 0);
	hook_offhand = gi.cvar("hook_offhand", "1", 0);
	hook_carrierspeed = gi.cvar("hook_carrierspeed", hook_pullspeed->string, 0);
	if (hook_carrierspeed->value == 0)
		gi.cvar_set("hook_carrierspeed", hook_pullspeed->string);

	sv_botdetection = gi.cvar("sv_botdetection", "29", 0);
	
	server_ip =  gi.cvar("server_ip", "", CVAR_LATCH);
	lan = gi.cvar ("lan", "0", CVAR_LATCH);

	warmup_time =  gi.cvar("warmup_time", "30", CVAR_LATCH);
	//QW Some configurations used 1 to set zero warmup.
	if (warmup_time->value > 1 && warmup_time->value < 30)
	{
		gi.cvar_set("warmup_time", "30");
		gi.dprintf("TMG: Forcing warmup_time 30." 
			"Set warmup_time 0 to disable warmup mode.\n");
	}

	g_filter =  gi.cvar("filter", "1", CVAR_LATCH);

	// flood control
	flood_msgs = gi.cvar ("flood_msgs", "4", 0);
	flood_persecond = gi.cvar ("flood_persecond", "4", 0);
	flood_waitdelay = gi.cvar ("flood_waitdelay", "10", 0);
	flood_team = gi.cvar ("flood_team", "1", 0);

	maxfps = gi.cvar ("maxfps", "120", CVAR_LATCH);
	maxrate = gi.cvar ("maxrate", "10000", CVAR_LATCH);

	voosh = gi.cvar ("railserver", "1", CVAR_SERVERINFO | CVAR_LATCH);

	//hud stuff
	hud_freq = gi.cvar ("hud_freq", "2", 0); //QW// FIXME: not used, delete?
	max_hud_dist = gi.cvar ("id_dist", "150", 0);
	light_comp = gi.cvar ("light_comp", "100", 0);

	motd_line = gi.cvar ("motd_line", "", 0);	// MOTD
	motd_time = gi.cvar ("motd_time", "5", 0);

	id_x = gi.cvar ("id_x", "64", 0);
	id_y = gi.cvar ("id_y", "-58", 0);

	hostname = gi.cvar ("hostname", "TMG Server", 0);
	if (strlen(hostname->string) >= 40)
		gi.dprintf("TMG Warning: Hostname is too long for HUD banner.\n"
		"Shorten it to less than 40 characters.\n");

	speed_msec = gi.cvar ("speed_msec", "75", 0);
	speed_set = gi.cvar ("speed_set", "75", 0);
	speed_bust = gi.cvar ("speed_bust", "200", 0);

	railwait = gi.cvar ("railwait", "12", CVAR_LATCH);
	raildamage = gi.cvar ("raildamage", "300", CVAR_LATCH);
	railkick = gi.cvar ("railkick", "200", CVAR_LATCH);

	// Enable/disable the flashlight command
	flashlight = gi.cvar ("flashlight", "1", 0);
	
	//QW// Not used. FIXME: mark for deletion
	lights = gi.cvar ("lights", "1", CVAR_LATCH);
	
	// allow ops to control lighting
	lights_out = gi.cvar ("lights_out", "1", CVAR_LATCH);

	InitHighScores();
	Log_Init_vars();
	
	mapvote = gi.cvar("mapvote", "1", 0);

	rcon = gi.cvar ("rcon_password", "", 0);
	start_weapons = gi.cvar ("startweapons", "0", CVAR_LATCH);
	start_items = gi.cvar ("start_items", "0", CVAR_LATCH);
	ammoflags = gi.cvar ("ban_ammo", "0", CVAR_LATCH);
	runeflags = gi.cvar ("ban_rune", "64", CVAR_LATCH);
	banned_items = gi.cvar ("banned_items", "0", CVAR_LATCH);
	weapflags = gi.cvar ("wbanflags", "0", CVAR_LATCH);
	ban_blaster = gi.cvar ("ban_blaster", "0", CVAR_LATCH);
	runes = gi.cvar ("runes", "0",  CVAR_LATCH);

	spawn_time = gi.cvar("respawn_time", "30", CVAR_LATCH);
	mega_health_spawn = gi.cvar("mega_health_spawn", "30", CVAR_LATCH);
	ammo_spawn = gi.cvar("ammo_spawn", "20", CVAR_LATCH);
	armor_spawn = gi.cvar("armor_spawn", "30", CVAR_LATCH);
	health_spawn = gi.cvar("health_spawn", "20", CVAR_LATCH);
	max_Vhealth = gi.cvar ("max_health", "300", CVAR_LATCH);
	pickup_tech = gi.cvar("pickup_tech", "1 ", 0);
	hide_spawn = gi.cvar("hide_spawn", "1 ", 0);

	//anti camping
	camper_check = gi.cvar ("camper_check", "0", 0);
	camp_time = gi.cvar ("camp_time", "60", CVAR_LATCH);
	camp_distance = gi.cvar ("camp_distance", "256", CVAR_LATCH);

	resp_protect = gi.cvar ("resp_protect", "3", 0);

	check_models = gi.cvar ("check_models", "1", 0);

	//RAV new bot detection
	prox = gi.cvar ("prox", "0", 0);
	//check for speed hack
	speed_check = gi.cvar ("speed_check", "0", 0);
	//
	lag_limit = gi.cvar ("lag_limit", "500", 0);
	//voting menu
	menutime = gi.cvar ("menutime", "30", 0);

	//clear out ctf teams
	clear_teams = gi.cvar ("clear_teams", "0", 0);
	//even teams
	even_teams = gi.cvar ("even_teams", "100", 0);
	//reserved slots
	reserved_slots = gi.cvar ("reserved_slots", "0", CVAR_SERVERINFO);
	reserved_password = gi.cvar ("reserved_password", "", 0);

	//spec handling
	max_specs = gi.cvar ("max_specs", "0", 0);
	op_specs = gi.cvar ("op_specs", "0", 0);
	spec_check = gi.cvar ("spec_check", "0", 0);

	// start ammo
	// ATTENTION!
	// The game sets these cvars ///dont add to cfg file!!!
	sa_shells = gi.cvar ("sa_shells","0", CVAR_LATCH | CVAR_NOSET);
	sa_bullets = gi.cvar ("sa_bullets","0", CVAR_LATCH | CVAR_NOSET);
	sa_grenades = gi.cvar ("sa_grenades","0", CVAR_LATCH | CVAR_NOSET);
	sa_rockets = gi.cvar ("sa_rockets","0", CVAR_LATCH | CVAR_NOSET);
	sa_cells = gi.cvar ("sa_cells","0", CVAR_LATCH | CVAR_NOSET);
	sa_slugs = gi.cvar ("sa_slugs","0", CVAR_LATCH | CVAR_NOSET);
	// END ATTENTION!

	////////////weapon dammage and kick cvars//////////raven
	sg_damage = gi.cvar ("sg_damage", "4", CVAR_LATCH);
	sg_kick = gi.cvar ("sg_kick", "8", CVAR_LATCH);
	ssg_damage = gi.cvar ("ssg_damage", "6", CVAR_LATCH);
	ssg_kick = gi.cvar ("ssg_kick", "12", CVAR_LATCH);
	rg_damage = gi.cvar ("rg_damage", "100", CVAR_LATCH);
	rg_kick = gi.cvar ("rg_kick", "200", CVAR_LATCH);
	bfg_damage = gi.cvar ("bfg_damage", "200", CVAR_LATCH);
	g_damage = gi.cvar ("g_damage", "150", CVAR_LATCH);
	gl_damage = gi.cvar ("gl_damage", "120", CVAR_LATCH);
	rl_damage = gi.cvar ("rl_damage", "100", CVAR_LATCH);
	b_damage = gi.cvar ("b_damage", "15", CVAR_LATCH);
	hb_damage = gi.cvar ("hb_damage", "15", CVAR_LATCH);
	mg_damage = gi.cvar ("mg_damage", "8", CVAR_LATCH);
	mg_kick = gi.cvar ("mg_kick", "2", CVAR_LATCH);
	cg_damage = gi.cvar ("cg_damage", "6", CVAR_LATCH);
	cg_kick = gi.cvar ("cg_kick", "2", CVAR_LATCH);
	rocket_speed = gi.cvar ("rocket_speed", "650", CVAR_LATCH);
	rl_radius = gi.cvar ("rl_radius", "120", CVAR_LATCH);
	rl_radius_damage = gi.cvar ("rl_radius_damage", "120", CVAR_LATCH);

	//weapon fire speed adjustments
	rocket_wait = gi.cvar ("rocket_wait", "12", CVAR_LATCH);
	railgun_wait = gi.cvar ("railgun_wait", "18", CVAR_LATCH);
	shotgun_wait = gi.cvar ("shotgun_wait", "18", CVAR_LATCH);
	sshotgun_wait = gi.cvar ("sshotgun_wait", "17", CVAR_LATCH);
	bfg_wait = gi.cvar ("bfg_wait", "36", CVAR_LATCH);
	lag_ping = gi.cvar ("lag_ping", "500", 0);

	//ctf scoring
	cap_point = gi.cvar ("cap_point", "15", CVAR_LATCH);
	cap_team = gi.cvar ("cap_team", "10", CVAR_LATCH);
	recover_flag = gi.cvar ("recover_flag", "1", CVAR_LATCH);
	flag_bonus	= gi.cvar ("flag_bonus", "0", CVAR_LATCH);
	frag_carrier = gi.cvar ("frag_carrier", "2", CVAR_LATCH);
	carrier_save = gi.cvar ("carrier_save", "2", CVAR_LATCH);
	carrier_protect = gi.cvar ("carrier_protect", "1", CVAR_LATCH);
	flag_defend = gi.cvar ("flag_defend", "1", CVAR_LATCH);
	flag_assist = gi.cvar ("flag_assist", "1", CVAR_LATCH);
	frag_carrier_assist = gi.cvar ("frag_carrier_assist", "2", CVAR_LATCH);
	flag_return_time = gi.cvar ("flag_return_time", "30", CVAR_LATCH);

	// HUD control
	server_time = gi.cvar ("server_time", "0", 0);
	show_carrier = gi.cvar ("show_carrier", "1", 0);

	clan_name = gi.cvar ("clan_name", " ", CVAR_LATCH);
	clan_pass = gi.cvar ("clan_pass", " ", CVAR_LATCH);

	cl_check = gi.cvar ("cl_check", "1", CVAR_LATCH);

	//location damage
	damage_locate = gi.cvar("damage_locate", "0",CVAR_LATCH);
	damage_display = gi.cvar("damage_display", "1", 0);

	nokill = gi.cvar("nokill", "0", CVAR_ARCHIVE);
	punish_suicide = gi.cvar("punish_suicide", "0", CVAR_ARCHIVE);
	console_chat = gi.cvar("console_chat", "1", 0);
	no_hum = gi.cvar ("no_hum", "0", 0);
	developer = gi.cvar ("developer", "0", 0);
	gamedebug = gi.cvar ("gamedebug", "0", 0);
	basedir = gi.cvar("basedir", "", CVAR_NOSET);	//established by engine, read-only
	mercylimit = gi.cvar ("mercylimit", "0", 0);
	randomrcon = gi.cvar ("randomrcon", "0", CVAR_ARCHIVE);
	minfps = gi.cvar ("minfps", "25", CVAR_LATCH);
	use_grapple = gi.cvar ("use_grapple", "1", CVAR_SERVERINFO);
	grapple_speed = gi.cvar("grapple_speed", "650", 0);
	grapple_pullspeed = gi.cvar("grapple_pullspeed", "650", 0);
	grapple_damage = gi.cvar("grapple_damage", "10", 0);
	log_connect = gi.cvar ("log_connect", "1", 0);
	log_chat = gi.cvar ("log_chat", "0", 0);
	use_navfiles = gi.cvar("use_navfiles", "1", 0);

	// operator list management
	defaultoplevel = gi.cvar ("defaultoplevel", "0", 0);
	oplistBase = gi.TagMalloc (64 * sizeof(oplist_t), TAG_GAME);
	oplist = oplistBase + 1;

	//QW// Set these for debugging output
	debug_spawn = gi.cvar ("debug_spawn", "0", 0);
	debug_botspawn = gi.cvar ("debug_botspawn", "0", 0);
	debug_ops = gi.cvar ("debug_ops", "0", 0);

	//QW// Configure FFA and CTF config file names for OP menu
	ffa_cfg_file = gi.cvar ("ffa_cfgfile", "ffa.cfg", 0);
	ctf_cfg_file = gi.cvar ("ctf_cfgfile", "railwarz.cfg", 0);

	/*
	 allow_lasermines = gi.cvar ("allow_lasermines", "0", CVAR_LATCH);
	 allow_energy_lasers = gi.cvar ("allow_energy_lasers", "1", CVAR_SERVERINFO);
	 energylaser_cells = gi.cvar ("energylaser_cells", "20",	0);
	 energylaser_damage = gi.cvar ("energylaser_damage", "15", 0);
	 energylaser_time = gi.cvar ("energylaser_time", "25", 0);
	 energylaser_mdamage = gi.cvar ("energylaser_mdamage", "50", 0);
	 energylaser_mradius = gi.cvar ("energylaser_mradius", "64", 0);
	 allownuke = gi.cvar ("allownuke", "1", CVAR_SERVERINFO);
	 nuke_cells = gi.cvar ("nuke_cells", "100", 0);
	 nuke_slugs = gi.cvar ("nuke_slugs", "10", 0);
	 nuke_radius2 = gi.cvar ("nuke_radius2", "3000", 0);
	 nuke_radius = gi.cvar ("nuke_radius", "1000", 0);
	 lasermine_timeout = gi.cvar ("lasermine_timeout", "60", CVAR_LATCH);
	 */

	runes4all = gi.cvar ("runes4all", "0", CVAR_LATCH);
	quad_notify = gi.cvar ("quad_notify", "0", 0);
	teamtechs = gi.cvar ("teamtechs", "0", 0);
	auto_flag_return = gi.cvar ("auto_flag_return", "30", 0);
	tmgclock = gi.cvar ("tmgclock", "24", 0);

	allow_flagdrop = gi.cvar ("allow_flagdrop", "0", 0);
	dropflag_delay = gi.cvar ("dropflag_delay", "1", 0);
	if (dropflag_delay->value < 0 || dropflag_delay->value > 3)
		gi.cvar_set("dropflag_delay", "1");

	newscore = gi.cvar ("newscore", "1", 0); // new scoreboard shows captures

	extrasounds = gi.cvar ("extrasounds", "0", 0);
	ctf_forcejoin = gi.cvar("ctf_forcejoin", "", 0);
	ctf_deathscores = gi.cvar("ctf_deathscores", "0", 0);
	allow_vote = gi.cvar ( "allow_vote", "0", 0);
	vote_timeout = gi.cvar ("vote_timeout", "30", 0);
	vote_percentage = gi.cvar ("vote_percentage", "67", 0);

	cfgdir = gi.cvar ("cfgdir", "cfg", CVAR_NOSET);
	doors_stay_open = gi.cvar("doors_stay_open", "0", 0);

	modversion = gi.cvar ("modversion", MOD" "MOD_VERSION" ", CVAR_SERVERINFO | CVAR_NOSET);

	//QW//FIXME: this probably belongs in level_local_t
	votetime = 0;

	dmflag = (int)dmflags->value;	//JSW
	
	Wav_Mod_Setup();
	
	Bot_InitCvars();
	InitItems ();

	Com_sprintf (game.helpmessage1, sizeof(game.helpmessage1), "");
	Com_sprintf (game.helpmessage2, sizeof(game.helpmessage2), "");

	// initialize all entities for this game
	game.maxentities = maxentities->value;
	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;
	globals.max_edicts = game.maxentities;

	// initialize all clients for this game
	game.maxclients = maxclients->value;
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	globals.num_edicts = game.maxclients+1;

	InitAnticheat();
	mdsoft_InitMaps();

	// text filtering
	LoadTextFilterInfo();

	//RAV
	ServerInit (true);

	CTFInit();

	CheckOpFile(NULL, "*@*.*.*.*", false);

	// For the logs
	GetDate();
	GetTime();

	//QW// Initialize cvars pertaining to stats logs
	StatsInitVars();

	if(ctf->value)
		StatsLog("MODE: %s %s\n", "TMG_MOD CTF", MOD_VERSION );
	else
		StatsLog("MODE: %s %s\n", "TMG_MOD Deathmatch", MOD_VERSION );

	if(use_bots->value)
		Load_BotInfo();

	if(log_connect->value)
		LogConnect(NULL, false);
	if (log_chat->value)
		LogChat("Chat log Initializing\n");

}

//=========================================================

static void WriteField1 (FILE *f, field_t *field, byte *base)
{
	void		*p;
	size_t		len;
	long		index;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
	case F_GSTRING:
		if ( *(char **)p )
			len = strlen(*(char **)p) + 1;
		else
			len = 0;
		*(int *)p = (int) len;
		break;
	case F_EDICT:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(edict_t **)p - g_edicts;
		*(int *)p = (int) index;
		break;
	case F_CLIENT:
		if ( *(gclient_t **)p == NULL)
			index = -1;
		else
			index = *(gclient_t **)p - game.clients;
		*(int *)p = (int) index;
		break;
	case F_ITEM:
		if ( *(edict_t **)p == NULL)
			index = -1;
		else
			index = *(gitem_t **)p - itemlist;
		*(int *)p = (int) index;
		break;

	default:
		gi.error ("WriteEdict: unknown field type");
	}
}

static void WriteField2 (FILE *f, field_t *field, byte *base)
{
	size_t		len;
	void		*p;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
		case F_LSTRING:
		case F_GSTRING:
			if ( *(char **)p )
			{
				len = strlen(*(char **)p) + 1;
				fwrite (*(char **)p, len, 1, f);
			}
			break;
		default:
			break;
	}
}

static void ReadField (FILE *f, field_t *field, byte *base)
{
	void	*p;
	int		len;
	int		index;
	size_t	count;

	p = (void *)(base + field->ofs);
	switch (field->type)
	{
	case F_INT:
	case F_FLOAT:
	case F_ANGLEHACK:
	case F_VECTOR:
	case F_IGNORE:
		break;

	case F_LSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = gi.TagMalloc (len, TAG_LEVEL);
			count = fread (*(char **)p, len, 1, f);
		}
		break;
	case F_GSTRING:
		len = *(int *)p;
		if (!len)
			*(char **)p = NULL;
		else
		{
			*(char **)p = gi.TagMalloc (len, TAG_GAME);
			count = fread (*(char **)p, len, 1, f);
		}
		break;
	case F_EDICT:
		index = *(int *)p;
		if ( index == -1 )
			*(edict_t **)p = NULL;
		else
			*(edict_t **)p = &g_edicts[index];
		break;
	case F_CLIENT:
		index = *(int *)p;
		if ( index == -1 )
			*(gclient_t **)p = NULL;
		else
			*(gclient_t **)p = &game.clients[index];
		break;
	case F_ITEM:
		index = *(int *)p;
		if ( index == -1 )
			*(gitem_t **)p = NULL;
		else
			*(gitem_t **)p = &itemlist[index];
		break;

	default:
		gi.error ("ReadEdict: unknown field type");
	}
}

//=========================================================

/*
==============
WriteClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
static void WriteClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	gclient_t	temp;
	
	// all of the ints, floats, and vectors stay as they are
	temp = *client;

	// change the pointers to lengths or indexes
	for (field=clientfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=clientfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)client);
	}
}

/*
==============
ReadClient

All pointer variables (except function pointers) must be handled specially.
==============
*/
static void ReadClient (FILE *f, gclient_t *client)
{
	field_t		*field;
	size_t	count;

	count = fread(client, sizeof(*client), 1, f);

	for (field=clientfields ; field->name ; field++)
	{
		ReadField (f, field, (byte *)client);
	}
}

/*
============
WriteGame

This will be called whenever the game goes to a new level,
and when the user explicitly saves the game.

Game information include cross level data, like multi level
triggers, help computer info, and all client states.

A single player death will automatically restore from the
last save position.
============
*/
void WriteGame (char *filename, qboolean autosave)
{
	FILE	*f;
	int		i;
	char	str[16];

	if(dedicated->value)
		return;
	
	if (!autosave)
		SaveClientData ();

	f = fopen (filename, "wb");
	if (!f) 
	{
		gi.error ("Couldn't open %s", filename);
		return;
	}

	memset (str, 0, sizeof(str));
	strcpy (str, __DATE__);
	fwrite (str, sizeof(str), 1, f);

	game.autosaved = autosave;
	fwrite (&game, sizeof(game), 1, f);
	game.autosaved = false;

	for (i = 0; i < game.maxclients; i++)
		WriteClient (f, &game.clients[i]);

	fclose (f);
}

void ReadGame (char *filename)
{
	FILE	*f;
	int		i;
	char	str[16];
	size_t	count;

	gi.FreeTags (TAG_GAME);

	if(dedicated->value)
		return;
	
	f = fopen (filename, "rb");
	if (!f)
	{
		gi.error ("Couldn't open %s", filename);
		return;
	}

	count = fread (str, sizeof(str), 1, f);
	if (strcmp (str, __DATE__))
	{
		fclose (f);
		gi.error ("Savegame from an older version.\n");
	}

	g_edicts =  gi.TagMalloc (game.maxentities * sizeof(g_edicts[0]), TAG_GAME);
	globals.edicts = g_edicts;

	count = fread (&game, sizeof(game), 1, f);
	game.clients = gi.TagMalloc (game.maxclients * sizeof(game.clients[0]), TAG_GAME);
	for (i=0 ; i<game.maxclients ; i++)
		ReadClient (f, &game.clients[i]);

	fclose (f);
}

//==========================================================


/*
==============
WriteEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
static void WriteEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	edict_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = *ent;

	// change the pointers to lengths or indexes
	for (field=savefields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=savefields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)ent);
	}

}

/*
==============
WriteLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
static void WriteLevelLocals (FILE *f)
{
	field_t		*field;
	level_locals_t		temp;

	// all of the ints, floats, and vectors stay as they are
	temp = level;

	// change the pointers to lengths or indexes
	for (field=levelfields ; field->name ; field++)
	{
		WriteField1 (f, field, (byte *)&temp);
	}

	// write the block
	fwrite (&temp, sizeof(temp), 1, f);

	// now write any allocated data following the edict
	for (field=levelfields ; field->name ; field++)
	{
		WriteField2 (f, field, (byte *)&level);
	}
}


/*
==============
ReadEdict

All pointer variables (except function pointers) must be handled specially.
==============
*/
static void ReadEdict (FILE *f, edict_t *ent)
{
	field_t		*field;
	size_t	count;

	count = fread (ent, sizeof(*ent), 1, f);

	for (field = savefields; field->name; field++)
	{
		ReadField (f, field, (byte *)ent);
	}
}

/*
==============
ReadLevelLocals

All pointer variables (except function pointers) must be handled specially.
==============
*/
static void ReadLevelLocals (FILE *f)
{
	field_t		*field;
	size_t	count;

	count = fread (&level, sizeof(level), 1, f);

	for (field = levelfields; field->name; field++)
	{
		ReadField (f, field, (byte *)&level);
	}
}

/*
=================
WriteLevel

=================
*/
void WriteLevel (char *filename)
{
	int		i;
	edict_t	*ent;
	FILE	*f;
	void	(*base)(void);

	if(dedicated->value)
		return;

	f = fopen (filename, "wb");
	if (!f)
	{
		gi.error ("Couldn't open %s", filename);
		return;
	}

	// write out edict size for checking
	i = sizeof(edict_t);
	fwrite (&i, sizeof(i), 1, f);

	// write out a function pointer for checking
	base = InitGame;
	fwrite (&base, sizeof(base), 1, f);

	// write out level_locals_t
	WriteLevelLocals (f);

	// write out all the entities
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];
		if (!ent->inuse)
			continue;
		fwrite (&i, sizeof(i), 1, f);
		WriteEdict (f, ent);
	}
	i = -1;
	fwrite (&i, sizeof(i), 1, f);

	fclose (f);
}


/*
=================
ReadLevel

SpawnEntities will already have been called on the
level the same way it was when the level was saved.

That is necessary to get the baselines
set up identically.

The server will have cleared all of the world links before
calling ReadLevel.

No clients are connected yet.
=================
*/
void ReadLevel (char *filename)
{
	int		entnum;
	FILE	*f;
	int		i;
	void	(*base)(void);
	edict_t	*ent;
	size_t	count;

	f = fopen (filename, "rb");
	if (!f)
	{
		gi.error ("Couldn't open %s", filename);
		return;
	}

	// free any dynamic memory allocated by loading the level
	// base state
	gi.FreeTags (TAG_LEVEL);

	// wipe all the entities
	memset (g_edicts, 0, game.maxentities*sizeof(g_edicts[0]));
	globals.num_edicts = maxclients->value+1;

	// check edict size
	count = fread (&i, sizeof(i), 1, f);
	if (i != sizeof(edict_t))
	{
		fclose (f);
		gi.error ("ReadLevel: mismatched edict size");
	}

	// check function pointer base address
	count = fread (&base, sizeof(base), 1, f);
	if (base != InitGame)
	{
		fclose (f);
		gi.error ("ReadLevel: function pointers have moved");
	}

	// load the level locals
	ReadLevelLocals (f);

	// load all the entities
	while (1)
	{
		count = fread (&entnum, sizeof(entnum), 1, f);
		if (count != 1)
		{
			fclose (f);
			gi.error ("ReadLevel: failed to read entnum");
		}
		if (entnum == -1)
			break;
		if (entnum >= globals.num_edicts)
			globals.num_edicts = entnum+1;

		ent = &g_edicts[entnum];
		ReadEdict (f, ent);

		// let the server rebuild world links for this ent
		memset (&ent->area, 0, sizeof(ent->area));
		gi.linkentity (ent);
	}

	fclose (f);

	// mark all clients as unconnected
	for (i=0 ; i<maxclients->value ; i++)
	{
		ent = &g_edicts[i+1];
		ent->client = game.clients + i;
		ent->client->pers.connected = false;
	}

	// do any load time things at this point
	for (i=0 ; i<globals.num_edicts ; i++)
	{
		ent = &g_edicts[i];

		if (!ent->inuse)
			continue;

		// fire any cross-level triggers
		if (ent->classname)
			if (strcmp(ent->classname, "target_crosslevel_target") == 0)
				ent->nextthink = level.time + ent->delay;
	}
}


