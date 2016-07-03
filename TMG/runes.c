
#include "g_local.h"
#include "g_items.h"
#include "g_cmds.h"
#include "runes.h"
#include "timer.h"
#include "performance.h"

#define DEBUG_RUNES 0

char *rune_namefornum[] =
 {
	"Strength Rune",
	"Resist Rune",
	"Haste Rune",
	"Regen Rune",
    "Jump Rune",
	"Liquid Rune",
	"Stealth Rune",
	"Vampire Rune",
	"Speed Rune"
 };

static char *tnames[] =
{
	"item_tech1",
	"item_tech2",
	"item_tech3",
	"item_tech4",
	NULL
};

static edict_t *rune_spawn_point = NULL;
extern gitem_t *item_tech1, *item_tech2, *item_tech3, *item_tech4;
void CTFHasTech(edict_t *who);
int runespawn = 1;

// true if a person has a specific rune
qboolean rune_has_rune(edict_t *ent, int type)
{
	if (!ent->client)
		return (false);
	return
		(ent->client->pers.inventory[ITEM_INDEX(FindItem(rune_namefornum[type]))]);
}

// true if the person has a rune
qboolean rune_has_a_rune(edict_t *ent)
{
	int	i;
	if (!ent->client) return (false);	// only people can have runes
	for (i=RUNE_FIRST; i<=RUNE_LAST; i++)
	{
		if (rune_has_rune(ent, i))
		{
			return (true);
		}
	}
	return(false);
}

// a live client has touched a rune
qboolean rune_pickup(edict_t *self, edict_t *other)
{
	int i = 0;
	gitem_t *tech;

	if(match_state < STATE_PLAYING)
		return false;

	if(rune_has_a_rune(other))
	{
		CTFHasTech(other);
		return false;
	}

	while (tnames[i])
	{
		if ((tech = FindItemByClassname(tnames[i])) != NULL && other->client->pers.inventory[ITEM_INDEX(tech)])
		{
			CTFHasTech(other);
			return false; // has this one
		}
		i++;
	}
	
	// give them this rune
	other->client->pers.inventory[ITEM_INDEX(self->item)] = 1;
	return(true);
}

// moves the rune to a teleporter pad
void rune_move (edict_t *self)
{
	rune_select_spawn_point(self->s.origin);
	if (VectorCompare(self->s.origin, vec3_origin)) {
		G_FreeEdict(self);
		return;
	}
	self->touch = Touch_Item;
	self->nextthink = level.time + 120;
	self->think = rune_move;
	gi.linkentity(self);
}


// makes the rune touchable again after being droppped
void rune_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	ent->nextthink = level.time + 120;
	ent->think = rune_move;
}

// call rune_drop from here?
void rune_use (edict_t *ent, gitem_t *item) {
	// do nothing
}

// drops a rune
void rune_drop (edict_t *ent, gitem_t *item)
{
	edict_t	*rune;
	rune = Drop_Item(ent, item);
	rune->nextthink = level.time + 1;
	rune->think = rune_make_touchable;

	//	rune->s.renderfx |= rune_renderfx[item - FindItem(rune_namefornum[RUNE_FIRST])];//RF_SHELL_RED | RF_SHELL_GREEN;
	if (strcmp(rune->classname, "item_rune_strength") == 0){
		rune->s.renderfx |= RF_SHELL_RED;
	}

	if (strcmp(rune->classname, "item_rune_resist")==0){
		rune->s.renderfx |= RF_SHELL_BLUE;
	}

	if (strcmp(rune->classname, "item_rune_haste")==0){
		rune->s.renderfx |= RF_SHELL_DOUBLE;
	}

	if (strcmp(rune->classname, "item_rune_regen")==0){
		rune->s.renderfx |= RF_SHELL_GREEN;
	}
	//
	if (strcmp(rune->classname, "item_rune_jump") == 0){
		rune->s.renderfx |= RF_SHELL_BLUE | RF_SHELL_DOUBLE;
	}

	if (strcmp(rune->classname, "item_rune_liquid")==0){
		rune->s.renderfx |= RF_SHELL_GREEN | RF_SHELL_BLUE;
	}

	if (strcmp(rune->classname, "item_rune_invis")==0){
		rune->s.renderfx |= RF_SHELL_RED | RF_SHELL_HALF_DAM;
	}

	if (strcmp(rune->classname, "item_rune_vamp")==0){
		rune->s.renderfx |= RF_SHELL_RED | RF_SHELL_BLUE;
	}
	if (strcmp(rune->classname, "item_rune_speed")==0){
		rune->s.renderfx |= RF_SHELL_RED | RF_SHELL_GREEN | RF_SHELL_BLUE | RF_SHELL_HALF_DAM;
	}

	rune->noblock = true;

	ent->client->pers.inventory[ITEM_INDEX(item)] = 0;

	ValidateSelectedItem(ent);
}

// drops any rune a person might have
void runes_drop (edict_t *ent)
{
	int	i;

	if(rune_has_rune(ent, RUNE_INVIS))
	{
		ent->stealth = level.time +2;
		rune_drop (ent, FindItem(rune_namefornum[RUNE_INVIS]));
	}
	else
	{
		for (i=RUNE_FIRST; i<=RUNE_LAST; i++) {
			if (rune_has_rune(ent, i))
				rune_drop (ent, FindItem(rune_namefornum[i]));
	 }
	}
}


// finds a place to put a rune
// or vec3_origin if it can't find one
void rune_select_spawn_point(vec3_t origin)
{
	rune_spawn_point = G_Find(rune_spawn_point, FOFS(classname), "info_player_deathmatch");
	if (!rune_spawn_point)
		rune_spawn_point = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
	if (!rune_spawn_point) {
		gi.dprintf ("Couldn't find spawn point for rune\n");
		VectorClear(origin);
	} else {
		VectorCopy(rune_spawn_point->s.origin, origin);
	}
}



static edict_t *FindRuneSpawn(void)
{
	edict_t *spot = NULL;
	int i = rand() % 16;

	while (i--)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	if (!spot)
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
	return spot;
}


void SpawnTech(gitem_t *item, edict_t *spot);

// spawns a rune
void Rune_Spawn(edict_t *rune, gitem_t *item)
{
	edict_t *spot;
	
	rune->item = item;
	rune->classname = rune->item->classname;
	if ((spot = FindRuneSpawn()) != NULL)
	{
		SpawnTech(rune->item, spot);
		if (DEBUG_RUNES) 
			DbgPrintf("Tech 0x%x type %s spawned time: %.1f\n", rune, rune->classname, level.time);
		rune->noblock = true;
		return;
	}
	else
		SpawnItem(rune, rune->item);
	rune->noblock = true;
}

// spawns all the runes
void runes_spawn(edict_t *self)
{
	edict_t	*rune;
	int	i, j;

	for (i=0; i<(int)runes->value; i++)
	{	// runes number of each rune
		for (j=RUNE_FIRST; j<=RUNE_LAST; j++)
		{	// run thru all runes
			rune = G_Spawn();
			rune_select_spawn_point(rune->s.origin);
			if (VectorCompare(rune->s.origin, vec3_origin))
			{
				G_FreeEdict(rune);
			}
			else
			{
				Rune_Spawn(rune, FindItem(rune_namefornum[j]));
			}
		}
	}
	G_FreeEdict(self);
}

// so we can spawn the runes after the level starts
void runes_spawn_start(void)
{
	edict_t	*temp;

	if (runespawn && runes4all->value != 1)
		return;
	if (runes4all->value)
	{
		if (runespawn >= maxclients->value/8)
			return;
		runespawn++;
	}
	else
		runespawn = 1;
	temp = G_Spawn();
	temp->think = runes_spawn;
	temp->nextthink = level.time + 3 + warmup_time->value;
}
