#include "g_local.h"
#include "intro.h"

int			wav_mod_current_level = -1;
int			wav_mod_num_wavs = 0;
char		wav_mod_names[MAXSONGS][MAX_QPATH];

// niq hack:
qboolean	wav_used[MAXSONGS];
int			unused_wav = 0;

cvar_t *wav;
cvar_t *wavs;
cvar_t *wav_random;
cvar_t *songtoplay;
cvar_t *use_song_file;

void Wav_InitCvars(void)
{
	wavs = gi.cvar("wavs", "1 ", 0);	// 1|0 play wavs or not
	songtoplay = gi.cvar("song", "misc/mm.wav ", 0); // default 1st song
	use_song_file = gi.cvar("use_song_file", "1", CVAR_ARCHIVE); // use the song file (intro.txt)
	wav_random = gi.cvar("wav_random", "1", CVAR_ARCHIVE); // randomize
	wav = gi.cvar("wav", "mm.wav", 0); // just to instantiate the wav cvar
}

void Wav_Mod_Setup(void)
{
	FILE *file;
	char file_name[MAX_QPATH];
	int file_size = 0;
	char *p_buffer = NULL;
	char *p_name = NULL;
	long counter = 0;
	int n_chars = 0;
	size_t count = 0;
	size_t lines = 0;

	gi.dprintf ("\n==== Wav Mod v.02 Setup ====\n");

	Wav_InitCvars();

	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/intro.txt", 
		basedir->string, game_dir->string, cfgdir->string);

	file = fopen(file_name, "r");

	if (!file)
	{
		gi.dprintf ("==== Wav Mod - missing intro.txt file ====\n");
		gi.cvar_set ("use_song_file", "0");
		return;
	}
	else
	{
		while (!feof(file))
		{
			int c = fgetc(file);
			file_size++;	// count the true file size
			if (c == '\n')
				lines++;
		}

		fseek(file, 0L, SEEK_SET);
		p_buffer = gi.TagMalloc(file_size, TAG_GAME);
		memset(p_buffer, 0, file_size);
		gi.dprintf("Allocated %ld bytes for file\n", file_size);
		gi.dprintf("Lines in file: %d\n", lines);

		count = fread((void *)p_buffer, sizeof(char), file_size, file);
		if (count == 0 || ferror(file))
		{
			gi.dprintf ("Error reading %s\n", file_name);
			gi.dprintf ("Characters read: %d\n", count);
			gi.cvar_set ("use_song_file", "0");
		}

		gi.dprintf("Adding Wav's to cycle: ");

		p_name = p_buffer;
		do
		{
			// niq: skip rest of line after a '#'
			if(*p_name == '#')
			{
				while ((*p_name != '\n') && counter < file_size)
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
				memcpy(&wav_mod_names[wav_mod_num_wavs][0], 
					p_name - n_chars, n_chars);
				memset(&wav_mod_names[wav_mod_num_wavs][n_chars], 0, 1);

				if (wav_mod_num_wavs > 0)
					gi.dprintf(", ");
				gi.dprintf("%s", wav_mod_names[wav_mod_num_wavs]);

				wav_mod_num_wavs++;
				n_chars = 0;

				if (wav_mod_num_wavs >= MAXSONGS)
				{
					gi.dprintf("\nExceeded %d song limit.\n", MAXSONGS);
					gi.dprintf("Unable to add more Wavs.\n");
					break;
				}
			}

			// next songname
			counter++;
			p_name++;

		} while (counter < file_size);

		gi.TagFree(p_buffer);
		fclose(file);

		if (wav_mod_num_wavs)
		{
			gi.dprintf("\nWav Mod load succeeded. Levels: %i\n\n", wav_mod_num_wavs);
			gi.cvar_set ("use_song_file", "1");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

char* Wav_Mod_Next()
{
	int i;

	if (use_song_file->value)
	{
		if (wav_random->value)
		{
			// NIQ hack start
			if(wav_mod_num_wavs >= 2)
			{
				// niq: hack to mapmode code to make sure we try all maps
				// before starting over.
				int map;
				int skipped;

				if(!unused_wav)
				{
					// reset random wavs 
					for(map = 0; map<wav_mod_num_wavs; map++)
						wav_used[map] = false;

					if(wav_mod_current_level == -1 && wav->string)
					{
						// no current WavMod wav:
						// if there is a current wav make sure we don't
						// pick it again right away if it is in the list
						for (i = 0; (i < wav_mod_num_wavs && wav_mod_current_level == -1); i++)
							if (!Q_stricmp(wav->string, wav_mod_names[i]))
								wav_mod_current_level = i;

					}

					if(wav_mod_current_level != -1)
					{
						// zap the wav
						wav_used[wav_mod_current_level] = true;

						// one less unused wav to choose from
						unused_wav = wav_mod_num_wavs - 1; 
					}
					else
					{
						// can choose any wav in list
						unused_wav = wav_mod_num_wavs; 
					}
				}

				// pick number of unused wavs to skip (less clustering likely)
				i = (int) floorf(random() * ((float)(unused_wav)));

				// skip to first unused wav (has to find one)
				map	= 0;
				while(wav_used[map])
					map++;

				// skip over i unused wavs (has to find them)
				skipped	= 0;
				while(skipped < i)
				{
					if(!wav_used[map++])
						skipped++;
				}

				// skip to unused wav if necessary (e.g. if last skip skipped to used one)
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

				i = (int) floorf(random() * ((float)(wav_mod_num_wavs)));

				if (!Q_stricmp(wav->string, wav_mod_names[i]))
				{
					if (++i >= wav_mod_num_wavs)
						i=0;
				}
				wav_mod_current_level = i;
			}
		}
		else
		{
			wav_mod_current_level = -1;

			for (i=0; i < wav_mod_num_wavs; i++)
				if (!Q_stricmp(wav->string, wav_mod_names[i]))
					wav_mod_current_level = i+1;
		}

		if (wav_mod_current_level >= wav_mod_num_wavs)
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

/* 
//QW// 
This was in g_spawn.c and was called by SP_worldspawn.
Caching a long list blows up the server when it can't cache 
the rest of the weapon and player sounds.
Game limit is 254 sounds (index 1 thru 255) and the original intro
code allowed for up to 256 songs.
*/
//RAV precache songs
void PrecacheSongs(void)
{
	FILE *file;
	static char names[MAXSONGS][MAX_QPATH];
	char file_name[MAX_QPATH];
	char song[MAX_QPATH] = { 0 };
	int levels = 0;
	size_t	count;


	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/intro.txt",
		basedir->string, game_dir->string, cfgdir->string);

	file = fopen(file_name, "r");
	if (file == NULL)
	{
		gi.dprintf("==== Wav Mod v.01 - missing intro.txt file ====\n");
	}
	else
	{
		int file_size = 0;
		char *p_buffer;
		char *p_name;
		long counter = 0;
		int n_chars = 0;

		// estimate the file size
		while (!feof(file))
		{
			fgetc(file);
			file_size++;
		}

		fseek(file, 0L, SEEK_SET);
		p_buffer = gi.TagMalloc(file_size, TAG_LEVEL);
		memset(p_buffer, 0, file_size);
		
		count = fread((void *)p_buffer, sizeof(char), file_size, file);
		if (!count)
			gi.dprintf("%s read %d of %d bytes in %s\n",
			__func__, count, file_size, file);

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
					(*p_name == '_') ||	(*p_name == '-') ||
					(*p_name == '/') ||	(*p_name == '\\')) && 
					counter < file_size)
				{
					n_chars++;
					counter++;
					p_name++;
				}
			}
			if (n_chars)
			{
				memcpy(&names[levels][0], p_name - n_chars, n_chars);
				memset(&names[levels][n_chars], 0, 1);
				//precache here 
				Com_sprintf(song, sizeof song, "misc/%s.wav", names[levels]);
				if (strlen(song) > 0)
				{
					gi.soundindex(song);
				}
				levels++;
				n_chars = 0;
				if (levels >= MAXSONGS)
				{
					gi.dprintf("Wav Mod: MAXSONGS reached.\n");
					break;
				}
			}

			// next mapname
			counter++;
			p_name++;
			// eat up non-characters (niq: except #)
			while (!((*p_name == '#') ||
				((*p_name >= 'a') && (*p_name <= 'z')) ||
				((*p_name >= 'A') && (*p_name <= 'Z')) ||
				((*p_name >= '0') && (*p_name <= '9')) ||
				(*p_name == '_') ||	(*p_name == '-') ||
				(*p_name == '/') ||	(*p_name == '\\')) && 
				counter < file_size)
			{
				counter++;
				p_name++;
			}
		}
		while (counter < file_size);
		gi.dprintf("\n\n");
		gi.TagFree(p_buffer);
		fclose(file);
	}
}
