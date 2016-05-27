#include "g_local.h"
#include "s_map.h"

MAP_ENTRY   *mdsoft_map = NULL;

maplist_t	*maplist;

static unsigned int mdsoft_map_size  = 0;
static unsigned int mdsoft_map_last  = 0;

static int 
parse_line(FILE	*fpFile, char *pFile, char *pName, int  *pMin, int  *pMax );

// instantiate cvars
cvar_t	*map_c;	// map_change
cvar_t	*map_r;	// map_randomize
cvar_t	*map_o;	// map_once
cvar_t	*map_d;	// map_debug

// other cvars used:
// basedir, game_dir, maplist

/*
 Call this at game initialization
 */
void mdsoft_InitMaps(void)
{
	maplist = gi.TagMalloc (sizeof(maplist_t), TAG_GAME);
	map_c = gi.cvar( "map_change", "1", 0 );
	map_r = gi.cvar( "map_randomize", "0", 0 );
	map_o = gi.cvar( "map_once", "0", 0 );
	map_d = gi.cvar( "map_debug", "0", 0 );
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
	Select a map at random from the array of maps
	and return its entity for use at changelevel.

 Return pointer to selected map, return NULL on error.

 Note: uses realloc, mdsoft_map is global and must
	be freed in ShutdownGame.
 FIXME: find a way to use gi.TagMalloc instead.

 */

edict_t *mdsoft_NextMap( void )
{
	edict_t     *ent    = NULL;
	int         count   = 0;
	int         fFound  = 0;
	int         nTimes  = 0;
	int i;

	if( (int) map_c->value == 0 )
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
					temp.max      = 0;
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
						newone = gi.TagMalloc(size, TAG_GAME);

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
					DbgPrintf("Map loaded: %s \"%s\"\n",
						   maplist->mapname[i], maplist->mapnick[i]);
				}

				DbgPrintf("%d maps loaded.\n", maplist->nummaps);

				fclose( fpFile );
			}
			else
			{
				gi.bprintf (PRINT_HIGH,
							"ERROR: Could not open maps list file [");
				gi.bprintf (PRINT_HIGH, pFileName );
				gi.bprintf (PRINT_HIGH, "]\n" );
			}
		}
	}

	/* Work out mdsoft_map_last by using map name */

	if( mdsoft_map_size )
	{
		do
		{
			/* Find random map to search from */
			if( (NULL != map_r) &&
			   ((int)map_r->value > 0 ) )
			{
				mdsoft_map_last = random() * (mdsoft_map_size-1);
				if( mdsoft_map_last <= 0 )
					mdsoft_map_last = 0 - mdsoft_map_last;

				if( (NULL != map_d) &&
				   ((int)map_d->value > 0 ) )
					gi.bprintf( PRINT_HIGH,
							   "Random Map %d %s\n",
							   mdsoft_map_last,
							   mdsoft_map[mdsoft_map_last].aFile);
			}

			/* Choose map */
			{
				int i;
				int point = (mdsoft_map_last+1) % mdsoft_map_size;

				count = 0;
				for (i = 0; i < maxclients->value; i++)
				{
					if (game.clients[i].pers.connected)
						count++;
				}

				/*gi.bprintf (PRINT_HIGH, "MAP CHANGE: Count = %d \n", count );*/

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

							if( (NULL != map_d) &&
							   ((int)map_d->value > 0 ) )
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
					if( (NULL != map_d) &&
					   ((int)map_d->value > 0 ) )
						gi.bprintf(PRINT_HIGH, "Map could not be found\n" );

					/* Clear visited flags */
					if( (NULL != map_o) &&
					   ((int)map_o->value > 0 ) )
					{
						int i;

						if( (NULL != map_d) &&
						   ((int)map_d->value > 0 ) )
							gi.bprintf(PRINT_HIGH, "Clearing Visited flags\n" );

						for( i=0; i < mdsoft_map_size; i++ )
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
		if( (NULL != map_o) &&
		   ((int)map_o->value > 0 ) )
		{
			mdsoft_map[mdsoft_map_last].fVisited = 1;
		}

		/* Set next map */
		ent = G_Spawn ();
		if( ent )
		{
			ent->classname = "target_changelevel";
			ent->map = &mdsoft_map[mdsoft_map_last].aFile[0];

			if( (NULL != map_d) &&
			   ((int)map_d->value > 0 ) )
			{
				gi.bprintf (PRINT_HIGH, "MAP CHANGE: %d ", mdsoft_map_last );
				gi.bprintf (PRINT_HIGH, &mdsoft_map[mdsoft_map_last].aFile[0] );

				gi.bprintf (PRINT_HIGH,
							" [min = %d,max = %d, players = %d]\n",
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
