#include <stdio.h>
#include "g_local.h"
#include "hud.h"

#define	MAXSONGS	256

int			wav_mod = 0;
int			wav_mod_current_level = -1;
int			wav_mod_n_levels = 0;
char		wav_mod_names[MAXSONGS][64];

// niq hack:
qboolean	wav_used[MAXSONGS];
int			unused_wav = 0;

cvar_t *use_song_file;
cvar_t *wav_random;
cvar_t *wav;
/////////////////////////////////////////////////////////////////////////////

void wav_mod_set_up(void)
{
	FILE *file;
	char file_name[256];

	use_song_file = gi.cvar ("use_song_file", "0", CVAR_ARCHIVE);
	wav_random = gi.cvar ("wav_random", "1", CVAR_ARCHIVE);
	wav = gi.cvar ("wav", "mm.wav",0);

	sprintf(file_name, "%s/%s/%s/intro.txt", 
		basedir->string, game_dir->string, cfgdir->string);

	file = fopen(file_name, "r");
	wav_mod = 0;
	wav_mod_current_level = -1;
	wav_mod_n_levels = 0;

	if (file != NULL)
	{
		int file_size;
		char *p_buffer;
		char *p_name;
		long counter = 0;
		int n_chars = 0;
		size_t count = 0;

		file_size = 0;
		while (!feof(file))
		{
			fgetc(file);
			file_size++;
		}
		rewind(file);

		p_buffer = gi.TagMalloc(file_size, TAG_GAME);
		memset(p_buffer, 0, file_size);

		count = fread((void *)p_buffer, sizeof(char), file_size, file);
		if (count == 0 || ferror(file))
		{
			gi.dprintf ("Error reading %s\n", file_name);
			gi.dprintf ("Characters read: %d\n", count);
		}

		gi.dprintf ("\n==== Wav Mod v.01 set up ====\n");
		gi.dprintf("Adding Wav's to cycle: ");

		p_name = p_buffer;
		do
		{
			// niq: skip rest of line after a '#' (works with Unix?)
			if(*p_name == '#')
			{
				while ((*p_name != '\n') && 
					(*p_name != '\r') && 
					counter < file_size)
				{
					p_name++;
					counter++;
				}
			}
			else
			{
				while ((((*p_name >= 'a') && (*p_name <= 'z')) ||
					((*p_name >= 'A') && (*p_name <= 'Z')) ||
					((*p_name >= '0') && (*p_name <= '9')) ||
					(*p_name == '_') || (*p_name == '-') ||
					(*p_name == '/') || (*p_name == '\\')) &&
					counter < file_size)
				{
					n_chars++;
					counter++;
					p_name++;
				}
			}

			if (n_chars)
			{
				memcpy(&wav_mod_names[wav_mod_n_levels][0], 
					p_name - n_chars, n_chars);
				memset(&wav_mod_names[wav_mod_n_levels][n_chars], 0, 1);

				if (wav_mod_n_levels > 0)
					gi.dprintf(", ");
				gi.dprintf("%s", wav_mod_names[wav_mod_n_levels]);

				wav_mod_n_levels++;
				n_chars = 0;

				if (wav_mod_n_levels >= MAXSONGS)
				{
					gi.dprintf("\nMAXSONGS exceeded\nUnable to add more Wav's.\n");
					break;
				}
			}

			// next songname
			counter++;
			p_name++;

			// eat up non-characters (niq: except #)
			while (!((*p_name == '#') || 
				((*p_name >= 'a') && (*p_name <= 'z')) ||
				((*p_name >= 'A') && (*p_name <= 'Z')) ||
				((*p_name >= '0') && (*p_name <= '9')) ||
				(*p_name == '_') || (*p_name == '-') ||
				(*p_name == '/') || (*p_name == '\\')) &&
				counter < file_size)
			{
				counter++;
				p_name++;
			}

		} while (counter < file_size);

		gi.dprintf("\n\n");

		gi.TagFree(p_buffer);
		fclose(file);

		if (wav_mod_n_levels)
		{
			wav_mod = true;
		}
	}
	else
	{
		gi.dprintf ("==== Wav Mod v.01 - missing intro.txt file ====\n");
		gi.cvar_set ("use_song_file", "0");
	}

	unused_wav = 0;
}

/////////////////////////////////////////////////////////////////////////////

char* wav_mod_next_map()
{
	int i;

	if (wav_mod)
	{
		if (wav_random->value)
		{
			// NIQ hack start
			if(wav_mod_n_levels >= 2)
			{
				// niq: hack to mapmode code to make sure we try all maps
				// before starting over.
				int map;
				int skipped;

				if(!unused_wav)
				{
					// reset random maps 
					for(map = 0; map<wav_mod_n_levels; map++)
						wav_used[map] = false;

					if(wav_mod_current_level == -1 && wav->string)
					{
						// no current MapMod map:
						// if there is a current map make sure we don't
						// pick it again right away if it is in the list
						for (i = 0; (i < wav_mod_n_levels && wav_mod_current_level == -1); i++)
							if (!Q_stricmp(wav->string, wav_mod_names[i]))
								wav_mod_current_level = i;

					}

					if(wav_mod_current_level != -1)
					{
						// zap the map
						wav_used[wav_mod_current_level] = true;

						// one less unused map to choose from
						unused_wav = wav_mod_n_levels - 1; 
					}
					else
					{
						// can choose any map in list
						unused_wav = wav_mod_n_levels; 
					}
				}

				// pick number of unused maps to skip (less clustering likely)
				i = (int) floor(random() * ((float)(unused_wav)));

				// skip to first unused map (has to find one)
				map	= 0;
				while(wav_used[map])
					map++;

				// skip over i unused maps (has to find them)
				skipped	= 0;
				while(skipped < i)
				{
					if(!wav_used[map++])
						skipped++;
				}

				// skip to unused map if necessary (e.g. if last skip skipped to used one)
				while(wav_used[map])
					map++;

				wav_mod_current_level = map;
				wav_used[map] = true;
				unused_wav--;
			}
			// NIQ hack end
			else
			{
				wav_mod_current_level = -1;

				i = (int) floor(random() * ((float)(wav_mod_n_levels)));

				if (!Q_stricmp(wav->string, wav_mod_names[i]))
				{
					if (++i >= wav_mod_n_levels)
						i=0;
				}
				wav_mod_current_level = i;
			}
		}
		else
		{
			wav_mod_current_level = -1;

			for (i=0; i < wav_mod_n_levels; i++)
				if (!Q_stricmp(wav->string, wav_mod_names[i]))
					wav_mod_current_level = i+1;
		}

		if (wav_mod_current_level >= wav_mod_n_levels)
		{
			wav_mod_current_level = 0;
		}

		if (wav_mod_current_level > -1)
		{
			return wav_mod_names[wav_mod_current_level];
		}

	}

	return NULL;
}
