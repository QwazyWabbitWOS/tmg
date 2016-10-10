#include "g_local.h"
#include "s_map.h"
#include "performance.h"

MAP_ENTRY   *mdsoft_map = NULL;

maplist_t	*maplist;

static unsigned int mdsoft_map_size  = 0;
static unsigned int mdsoft_map_last  = 0;

static int 
parse_line(FILE	*fpFile, char *pFile, char *pName, int  *pMin, int  *pMax );

// instantiate cvars
cvar_t	*map_change;
cvar_t	*map_randomize;
cvar_t	*map_once;
cvar_t	*map_debug;

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
	map_debug = gi.cvar( "map_debug", "0", 0 );
	maplist->active = false;
	mdsoft_NextMap();
}

/*
 //QW//
 This function has two modes:
 
 1. mdsoft_map == NULL, the array doesn't exist.
	Load the maplist from a file named according
	to the game mode, DM vs CTF and populate the
	mdsoft_map array for the voting system.

	maplist->active is latched true by the function
	once the file is loaded and the mdsoft_map is
	populated.

 2. mdsoft_map != NULL, the array exists.
	Select a map either sequentially or at random 
	from the array of maps and return its entity 
	for use at changelevel.

 Return pointer to selected map, return NULL on error.

 Note: uses realloc, mdsoft_map is global and must
	be freed in ShutdownGame.
 
 FIXME: find a way to use gi.TagMalloc instead.

 BUGS: 
 Map search can fail while indexing through the list.
 This failure mode should not be possible. 
 Failure to select a map in the list should never 
 occur unless the map file doesn't exist and the
 function doesn't even test for that.
 This makes it necessary to handle NULL return and
 go through gyrations to fall back to current map
 until human players break the cycle by voting
 a map.

 Editorial comment: 
 This function is entirely too tricky for its 
 own good. It needs to be broken up into at least
 five parts.
 0. Initialize the list.
 1. Seek sequential.
 2. Seek random.
 3. Verify selected map file exists, report 
    and reselect if it doesn't.
 4. Set next map.

 */

edict_t *mdsoft_NextMap( void )
{
	edict_t     *ent    = NULL;
	int         count   = 0;
	int         fFound  = 0;
	int         nTimes  = 0;
	int i;

	if(map_change->value == 0 )
		return NULL;

	/* Load Maps File */
	if( NULL == mdsoft_map )
	{
		FILE    *fpFile     = NULL;

		/* Load maps.lst file */
		if( game_dir && basedir )
		{
			char mapfile[MAX_QPATH] = {0};
			char *pFileName = &mapfile[0];

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
						newone = realloc(mdsoft_map, size); //FIXME: gi.TagMalloc here.

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
					if (map_debug->value)
						DbgPrintf("Map loaded: %s \"%s\" %d %d\n",
								mdsoft_map[i].aFile, mdsoft_map[i].aName,
								mdsoft_map[i].min, mdsoft_map[i].max);
				}

				if (map_debug->value)
					DbgPrintf("%d maps loaded.\n", maplist->nummaps);

				fclose( fpFile );
			}
			else
			{
				gi.bprintf (PRINT_HIGH,
							"WARNING: Could not open maps list file"
							" [%s]\n", pFileName);
			}
		}
	}

	/* Work out mdsoft_map_last by using map name */

	if( mdsoft_map_size )
	{
		do
		{
			/* Find random map to search from */
			if(map_randomize->value)
			{
				mdsoft_map_last = random() * (mdsoft_map_size-1);
				if( mdsoft_map_last <= 0 )
					mdsoft_map_last = 0 - mdsoft_map_last;

				if(map_debug->value)
					gi.bprintf( PRINT_HIGH,
							   "Random Map %d %s\n",
							   mdsoft_map_last,
							   mdsoft_map[mdsoft_map_last].aFile);
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

				if (map_debug->value)
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

							if(map_debug->value)
								gi.bprintf( PRINT_HIGH,
										   "Map Found %s [fVisited = %d]\n",
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
					if (map_debug->value)
						DbgPrintf("Map could not be found, "
						"map_sought %d, point %d name %s\n", 
						map_sought, point, mdsoft_map[map_sought].aName);

					/* Clear visited flags */
					if(map_once->value)
					{
						int i;

						if(map_debug->value)
							gi.bprintf(PRINT_HIGH, "Clearing Visited flags\n" );

						for(i = 0; i < mdsoft_map_size; i++ )
							mdsoft_map[i].fVisited = 0;
					}

					/* Use next map in list */
					mdsoft_map_last = (mdsoft_map_last+1) % mdsoft_map_size;
				}
			}

			nTimes++;
		} while( !fFound && (nTimes < 2) );
	}
	else
	{
		mdsoft_map_last = 0;
	}

	if( fFound && !ent )
	{
		/* Set map as visited */
		if(map_once->value)
		{
			mdsoft_map[mdsoft_map_last].fVisited = 1;
		}

		/* Set next map */
		ent = G_Spawn ();
		if( ent )
		{
			ent->classname = "target_changelevel";
			ent->map = &mdsoft_map[mdsoft_map_last].aFile[0];

			if(map_debug->value)
			{
				DbgPrintf ("MAP CHANGE: Selected = %d %s\n", 
					mdsoft_map_last, &mdsoft_map[mdsoft_map_last].aFile[0]);
				gi.bprintf (PRINT_HIGH, "MAP CHANGE: %d ", mdsoft_map_last );
				gi.bprintf (PRINT_HIGH, &mdsoft_map[mdsoft_map_last].aFile[0] );
				gi.bprintf (PRINT_HIGH, " [min = %d, max = %d, players = %d]\n",
							mdsoft_map[mdsoft_map_last].min,
							mdsoft_map[mdsoft_map_last].max,
							count );
			}
		}
	}

	return ent;
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
