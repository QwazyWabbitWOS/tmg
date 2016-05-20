#define		RUNE_FIRST		0
#define		RUNE_STRENGTH	0
#define		RUNE_RESIST		1
#define		RUNE_HASTE		2
#define		RUNE_REGEN		3
#define		RUNE_JUMP       4
#define		RUNE_LIQUID		5
#define		RUNE_INVIS		6
#define		RUNE_VAMP		7
#define		RUNE_SPEED		8
#define		RUNE_LAST		8

#define		RUNE_REGEN_PER_SEC		3

extern char *rune_namefornum[];
extern int	rune_renderfx[];

int runespawn;

qboolean rune_has_rune(edict_t *ent, int type);
qboolean rune_has_a_rune(edict_t *ent);
qboolean rune_pickup(edict_t *self, edict_t *other);
void rune_move (edict_t *self);
void rune_make_touchable (edict_t *ent);
void rune_use (edict_t *ent, gitem_t *item);
void rune_drop (edict_t *ent, gitem_t *item);
void runes_drop (edict_t *ent);
void rune_select_spawn_point(vec3_t origin);
void Rune_Spawn(edict_t *rune, gitem_t *item);
void runes_spawn(edict_t *self);
void runes_spawn_start(void);
