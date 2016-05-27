/*vote.c
Contains all the vote handling functions
4/21/01 RaVeN
*/
#include "g_local.h"
#include "vote.h"
#include "s_map.h"

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
	index = -1;
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


// MaplistNextMap
// Choose the next map in the list, or use voting system
void MaplistNextMap(edict_t *ent)
{
	int votemap;
	int i = 0;
	//	int j;
	size_t end = 0;

	if(developer->value)
		DumpMapVotes();

	//	j = maplist->currentmap;

	/*	switch ((int)map_randomize->value)        // choose next map in list
	 {
	 case 0:        // sequential rotation
		if (maplist->nummaps > 1)
		{
	 do
	 {
	 i = (j + 1) % maplist->nummaps;
	 j++;
	 if(j > maplist->nummaps+1)
	 {
	 //let the blank mapname check reset us to start of file
	 i= maplist->currentmap+1;
	 break;
	 }
	 } while ((int)mapvote->value);
		}
		else
	 i = maplist->currentmap+1;
		break;
	 case 1:     // random rotation
		if (maplist->nummaps > 1)
		{
	 do
	 {
	 i = (int) (random() * maplist->nummaps);
	 } while (i == j);
		}
		else
	 i = 0;
		break;
	 default:       // should never happen, but set to first map if it does
		i=0;
	 } // end switch
	 */

	//See if map voting is on
	if (mapvote->value)
	{
		votemap = MapMaxVotes();
		if (votemap >= 0)	//Yes there was one picked
		{
			i = votemap;
			end = strlen(maplist->mapname[i]);
		}
		ClearMapVotes();
	}

	strcpy(defaultmap,	maplist->mapname[0]);
	//check for blank mapname or bad map num
	if(end < 2 || i >=maplist->nummaps )
	{
		mapscrewed = true;
		maplist->currentmap = 0;
	}
	else
	{
		maplist->currentmap = i;
		ent->map = maplist->mapname[i];
	}
}

