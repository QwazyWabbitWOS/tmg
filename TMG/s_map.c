#include "g_local.h"
#include "s_map.h"
#include "performance.h"

MAP_ENTRY   *mdsoft_map = NULL;

maplist_t	*maplist;

static unsigned int mdsoft_map_size  = 0;
static unsigned int mdsoft_map_last  = 0;

static int parse_line(FILE	*fpFile, 
	char *pFile, char *pName, 
	int  *pMin, int  *pMax );

// instantiate cvars
cvar_t	*map_change;
cvar_t	*map_randomize;
cvar_t	*map_once;
cvar_t	*debug_smap;

// other cvars used:
// basedir, game_dir, maplist

/*
Call this at game initialization
*/
void mdsoft_InitMaps(void)
{
	maplist = gi.TagMalloc (sizeof(maplist_t), TAG_GAME);
	map_change = gi.cvar( "map_change", "1", 0 );
	map_randomize = gi.cvar( "map_randomize", "0", 0 );
	map_once = gi.cvar( "map_once", "0", 0 );
	debug_smap = gi.cvar( "debug_smap", "0", 0 );
	maplist->active = false;
	MaplistInit();
}

/*
//QW//
Select a map either sequentially or at random 
from the array of maps and return its entity 
for use at changelevel.

Return pointer to selected map else return NULL on error.

BUGS: 
1. Map search can fail while indexing through the list.
This failure mode should not be possible. 
Failure to select a map in the list should never 
occur unless the map file doesn't exist and the
function doesn't even test for that.
This makes it necessary to handle NULL return and
go through gyrations to fall back to current map
until human players break the cycle by voting
a map.
*/

edict_t *mdsoft_NextMap( void )
{
	edict_t     *ent    = NULL;
	int         count   = 0;
	int         fFound  = 0;
	int         nTimes  = 0;

	if(map_change->value == 0 )
		return NULL;

	if( mdsoft_map_size == 0 )
	{
		mdsoft_map_last = 0;
		return NULL;
	}

	if(map_once->value)
		ClearVisited();

	/* Work out mdsoft_map_last by using map name */
	do
	{
		/* Find random map to search from */
		if(map_randomize->value)
		{
			mdsoft_map_last = random() * (mdsoft_map_size-1);
			if( mdsoft_map_last <= 0 )
				mdsoft_map_last = 0 - mdsoft_map_last;

			if(debug_smap->value)
				DbgPrintf( "Random Map %d %s [fVisited = %d]\n",
				mdsoft_map_last,
				mdsoft_map[mdsoft_map_last].aFile, 
				mdsoft_map[mdsoft_map_last].fVisited);
		}

		/* Choose map */
		{
			int i;
			int point = (mdsoft_map_last) % mdsoft_map_size;
			int map_sought = point;

			count = 0;
			for (i = 0; i < maxclients->value; i++)
			{
				if (game.clients[i].pers.connected)
					count++;
			}

			if(debug_smap->value)
				DbgPrintf ("MAP CHANGE: Count = %d \n", count );

			do 
			{
				if( (0 != mdsoft_map[point].max) &&
					(0 == mdsoft_map[point].fVisited) )
				{
					if( (mdsoft_map[point].min <= count) &&
						(mdsoft_map[point].max >= count) )
					{
						mdsoft_map_last = point;
						point = -1;
						fFound = 1;

						if(debug_smap->value)
							DbgPrintf("Map Found = %d %s [fVisited = %d]\n",
							mdsoft_map_last,
							mdsoft_map[mdsoft_map_last].aFile,
							mdsoft_map[mdsoft_map_last].fVisited);
					}
					else
					{
						point = (point+1) % mdsoft_map_size;
					}
				}
				else
				{
					point = (point+1) % mdsoft_map_size;
				}
			}while( (point != -1) && (point != mdsoft_map_last) );

			/* Could not select an appropriate map */
			if(point == mdsoft_map_last)
			{
				if (debug_smap->value)
					DbgPrintf("Map could not be found, "
					"map_sought %d, point %d name %s\n", 
					map_sought, point, mdsoft_map[map_sought].aName);

				if(map_once->value)
					ClearVisited();

				/* Use next map in list */
				mdsoft_map_last = (mdsoft_map_last + 1) % mdsoft_map_size;
			}
		}
		nTimes++;
	} while( !fFound && (nTimes < 2) );

	if( fFound && !ent )
	{
		/* Set map as visited */
		if(map_once->value)
			mdsoft_map[mdsoft_map_last].fVisited = 1;

		/* Set next map */
		ent = G_Spawn ();
		if( ent )
		{
			ent->classname = "target_changelevel";
			ent->map = &mdsoft_map[mdsoft_map_last].aFile[0];

			if(debug_smap->value)
			{
				DbgPrintf ("Selected = %d %s [fVisited = %d]\n", 
					mdsoft_map_last, &mdsoft_map[mdsoft_map_last].aFile[0], 
					mdsoft_map[mdsoft_map_last].fVisited);
				gi.bprintf (PRINT_HIGH, "MAP CHANGE: %d ", mdsoft_map_last );
				gi.bprintf (PRINT_HIGH, &mdsoft_map[mdsoft_map_last].aFile[0] );
				gi.bprintf (PRINT_HIGH, " [min = %d, max = %d, players = %d]\n",
					mdsoft_map[mdsoft_map_last].min,
					mdsoft_map[mdsoft_map_last].max,
					count );
			}
		}
	}
	mdsoft_map_last++;
	return ent;
}

/* 
Test visited flags for all maps
and clear them all only if they have
all been visited.
*/
void ClearVisited(void)
{
	int i;

	for(i = 0; i < mdsoft_map_size; i++ )
	{
		if (!mdsoft_map[i].fVisited)
			return; // if any one of them is not visited
	}

	if(debug_smap->value)
		DbgPrintf("Clearing Visited flags\n" );

	for(i = 0; i < mdsoft_map_size; i++ )
		mdsoft_map[i].fVisited = 0;
}

/**
Parse a line from the previously opened fpFile.
Elements of the line are mapname, mapnick and optional
min and max players for each map.
Data goes into two different structs.
Name and nickname go into the maplist array for use at changelevel.
All four elements are passed back to caller for populating the 
table of map entries for use in the randomization and voting
system. The optional min/max are for player-sensitive map selection.
*/
static int
	parse_line(FILE *fpFile, char *pFile, char *pName, int *pMin, int *pMax)
{
	char buffer[MAX_QPATH]  = {0};
	int  c;
	int  i                  = 0;
	int  fInQuotes          = 0;
	int  element            = 0;

	do
	{
		c = fgetc( fpFile );

		/* Use buffer */
		if( (i > 0) &&
			((((' ' == c) || ('\t' == c)) && !fInQuotes) ||
			(EOF == c) || ('\n' == c) || ('\r' == c)))
		{
			buffer[i] = '\0';

			switch( element )
			{
			case 0:
				{
					strncpy( pFile, buffer, MAX_QPATH );
					strncpy(maplist->mapname[maplist->nummaps],
						buffer, MAX_QPATH);
					break;
				}
			case 1:
				{
					strncpy( pName, buffer, MAX_QPATH );
					strncpy(maplist->mapnick[maplist->nummaps],
						buffer, MAX_QPATH);
					maplist->nummaps++;
					break;
				}
			case 2:
				{
					*pMin = atoi( buffer );
					break;
				}
			case 3:
				{
					*pMax = atoi( buffer );
					break;
				}
			}

			i = 0;
			element++;
		}
		else
		{
			switch( c )
			{
			case '\"':
				{
					fInQuotes = 1 - fInQuotes;
					break;
				}

			case '\t':
			case ' ':
				{
					if( !fInQuotes )
						break;
				} /* fallthrough */
			default:
				{
					if( i < (MAX_QPATH-1) )
					{
						buffer[i] = c;
						i++;
					}
					break;
				}
			}
		}
	} while( (c != EOF) && (c != '\n') );

	return element;
}

//QW//
/* 
This function initializes the maplist array.
Note: uses realloc, mdsoft_map must
be explicitly freed in ShutdownGame.
FIXME: find a way to use gi.TagMalloc instead.
*/
void MaplistInit( void )
{
	int i;
	FILE    *fpFile     = NULL;
	char mapfile[MAX_QPATH] = {0};
	char *pFileName = &mapfile[0];

	if(map_change->value == 0)
		return;

	/* Form and load maps list file */
	strcat( mapfile, basedir->string );
	strcat( mapfile, "/" );
	strcat( mapfile, game_dir->string );

	if(ctf->value)
		sprintf(mapfile, "%s/%s/%s/maps_ctf.txt",
		basedir->string, game_dir->string, cfgdir->string);
	else
		sprintf(mapfile, "%s/%s/%s/maps_dm.txt",
		basedir->string, game_dir->string, cfgdir->string);

	fpFile = fopen( pFileName, "r" );
	if( fpFile )
	{
		MAP_ENTRY   temp;
		int         element;

		do
		{
			temp.min      = 0;
			temp.max      = (int) maxclients->value;
			temp.fVisited = 0;

			element = parse_line( fpFile,
				&temp.aFile[0],
				&temp.aName[0],
				&temp.min,
				&temp.max );

			if( 2 <= element )
			{
				MAP_ENTRY *newone;

				int size = (mdsoft_map_size + 1) * sizeof(*newone);
				//FIXME: gi.TagMalloc here.
				newone = realloc(mdsoft_map, size);

				if( newone )
				{
					mdsoft_map = newone;
					memcpy( &mdsoft_map[mdsoft_map_size],
						&temp, sizeof(temp) );
					mdsoft_map_size++;
				}
			}
		} while ( 2 <= element );

		maplist->active = true;

		for (i = 0; i < maplist->nummaps; i++)
		{
			if (debug_smap->value)
				DbgPrintf("Map loaded: %s \"%s\" %d %d\n",
				mdsoft_map[i].aFile, mdsoft_map[i].aName,
				mdsoft_map[i].min, mdsoft_map[i].max);
		}

		if (debug_smap->value)
			DbgPrintf("%d maps loaded.\n", maplist->nummaps);

		fclose( fpFile );
	}
	else
	{
		maplist->active = false;
		gi.bprintf (PRINT_HIGH,
			"WARNING: Could not open maps list file"
			" [%s]\n", pFileName);
	}
}
