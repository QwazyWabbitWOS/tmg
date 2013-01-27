#include "g_local.h"

typedef struct
{
    char   aFile[MAX_QPATH];
    char   aName[MAX_QPATH];
    int    min;
    int    max;
    int    fVisited;
} MAP_ENTRY;

static MAP_ENTRY   *mdsoft_map       = NULL;
static unsigned int mdsoft_map_size  = 0;
static unsigned int mdsoft_map_last  = 0;


static int mdsoft_read_map_entry(  FILE   *fpFile,
                                   char   *pFile,
                                   char   *pName,
                                   int    *pMin,
                                   int    *pMax );



edict_t *mdsoft_NextMap( void )
{
    edict_t     *ent    = NULL;
    int         count   = 0;
    int         fFound  = 0;
    int         nTimes  = 0;
	int i;

    if( (int)map_c->value == 0 )
        return NULL;

    /* Load Maps File */
    if( NULL == mdsoft_map )
    {
        FILE    *fpFile     = NULL;
//        cvar_t  *game       = gi.cvar( "gamedir", "mod-1", CVAR_SERVERINFO );
  //      cvar_t  *base       = gi.cvar( "basedir", ".", 0 );
//        cvar_t  *map_f      = gi.cvar( "map_file", "maps.lst", CVAR_SERVERINFO );

        /* Load maps.lst file */
        if( game_dir && basedir )
        {
            char mapfile[256] = {0};
            char *pFileName = &mapfile[0];

            strcat( mapfile, basedir->string );
            strcat( mapfile, "/" );
            strcat( mapfile, game_dir->string );

/*          if( NULL != map_f )
            {
                strcat( mapfile, "/" );
                strcat( mapfile, map_f->string );
            }
            else
            {
                strcat( mapfile, "/maps.lst" );
            }
*/

#if defined(linux)
			if(ctf->value)
				sprintf(mapfile, "%s/%s/%s/maps_ctf.txt", basedir->string, game_dir->string, cfgdir->string);
			else
				sprintf(mapfile, "%s/%s/%s/maps_dm.txt", basedir->string, game_dir->string, cfgdir->string);
#else
			if(ctf->value)
				sprintf(mapfile, "%s\\%s\\%s\\maps_ctf.txt", basedir->string, game_dir->string, cfgdir->string);
			else
				sprintf(mapfile, "%s\\%s\\%s\\maps_dm.txt", basedir->string, game_dir->string, cfgdir->string);
#endif
            
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

                    element = mdsoft_read_map_entry( fpFile,
                        &temp.aFile[0],
                        &temp.aName[0],
                        &temp.min,
                        &temp.max );

                    if( 2 <= element )
                    {
                        MAP_ENTRY *newone;

                        newone = realloc( mdsoft_map,
                                          (mdsoft_map_size+1) * sizeof(*newone) );

                        if( newone )
                        {
                            mdsoft_map = newone;
                            memcpy( &mdsoft_map[mdsoft_map_size], &temp, sizeof(temp) );
                            mdsoft_map_size++;
                        }
                    }
                }while( 2 <= element );

				maplist->active = true;
				
				if (developer->value)
				{
				for (i = 0; i < maplist->nummaps; i++)
				{
					gi.dprintf("Map loaded: %s \"%s\"\n", maplist->mapname[i], maplist->mapnick[i]);
				}
				gi.dprintf("%d maps loaded.\n", maplist->nummaps);
				}

                fclose( fpFile );
            }
            else
            {
                    gi.bprintf (PRINT_HIGH, "ERROR: Could not open maps list file [");
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
                if( mdsoft_map_last < 0 )
                    mdsoft_map_last = 0-mdsoft_map_last;

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
} /* end of mdsoft_NextMap() */




static int mdsoft_read_map_entry(  FILE   *fpFile,
                                   char   *pFile,
                                   char   *pName,
                                   int    *pMin,
                                   int    *pMax )
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
            (
             (((' ' == c) || ('\t' == c)) && !fInQuotes) ||
             (EOF == c) || ('\n' == c)
            )
          )
        {
            buffer[i] = '\0';

            switch( element )
            {
                case 0:
                {
                    strncpy( pFile, buffer, MAX_QPATH );
					strncpy(maplist->mapname[maplist->nummaps], buffer, MAX_QPATH);
                    break;
                }
                case 1:
                {
                    strncpy( pName, buffer, MAX_QPATH );
					strncpy(maplist->mapnick[maplist->nummaps], buffer, MAX_QPATH);
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
} /* end of mdsoft_read_map_entry() */

//Clear the map votes
void ClearMapVotes() 
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
int MapMaxVotes() 
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

void DumpMapVotes()
{
	int i;
	for (i = 0; i < maplist->nummaps; ++i)
		gi.dprintf("%d. %s (%d votes)\n",
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
void ClearMapList() 
{ 
   maplist->nummaps = 0; 
   ClearMapVotes();
} 
  

// MaplistNextMap
// Choose the next map in the list, or use voting system
void MaplistNextMap(edict_t *ent)
{ 
	int votemap;
	int i;
//	int j;
	int end = 0;
	
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
