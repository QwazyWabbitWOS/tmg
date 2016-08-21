/*vote.c
Contains all the vote handling functions
4/21/01 RaVeN
*/
#include "g_local.h"
#include "vote.h"
#include "s_map.h"
#include "performance.h"

//============================================================================

//Clear the map votes
void ClearMapVotes(void)
{
	int i;
	for (i=0; i < maplist->nummaps; ++i)
		maplist->votes[i] = 0;
}

//Find highest voted map
/*
 Returns
 -1  No votes
	0-31 Index to selected map
 */
int MapMaxVotes(void)
{
	int i;
	int numvotes;
	int index;

	numvotes = 0;
	index = NO_MAPVOTES;
	i = 0;

	while (i < maplist->nummaps)
	{
		if (maplist->votes[i] > numvotes)
		{
			numvotes = maplist->votes[i];
			index = i;
		}
		++i;
	}
	return(index);
}

void VoteForMap(int i)
{
	if (i >= 0 && i < maplist->nummaps)
		++maplist->votes[i];
}

void DumpMapVotes(void)
{
	int i;
	for (i = 0; i < maplist->nummaps; ++i)
		if (maplist->votes[i] != 0)
			DbgPrintf("%d. %s (%d votes)\n",
				  i, maplist->mapname[i], maplist->votes[i]);
}


//
// ClearMapList
//
// Clears/invalidates maplist. Might add more features in the future,
// but resetting .nummaps to 0 will suffice for now.
//
// Args:
//   ent      - entity (client) to print diagnostic messages to (future development).
//
// Return: (none)
//
void ClearMapList(void)
{
	maplist->nummaps = 0;
	ClearMapVotes();
}

// Choose the next map in the list, or use voting system
int MaplistNextMap(edict_t *ent)
{
	int votemap;
	int i = 0;
	//	int j;
	size_t end = 0;

	DumpMapVotes();

	//See if map voting is on
	if (mapvote->value)
	{
		votemap = MapMaxVotes();
		if (votemap != NO_MAPVOTES)	//Yes, there was one picked
		{
			i = votemap;
			end = strlen(maplist->mapname[i]);
		}
		ClearMapVotes();
	}

	strcpy(defaultmap,	maplist->mapname[0]);
	//check for blank mapname or bad map num
	if(end < 2 || i >= maplist->nummaps )
	{
		mapscrewed = true;
		maplist->currentmap = 0;
	}
	else
	{
		maplist->currentmap = i;
		ent->map = maplist->mapname[i];
	}
	return votemap;
}

