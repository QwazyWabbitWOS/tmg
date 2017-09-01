#include <stdio.h>
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

	wavs = gi.cvar("wavs", "1 ", 0);
	songtoplay = gi.cvar("song", "misc/mm.wav ", 0);
	use_song_file = gi.cvar ("use_song_file", "0", CVAR_ARCHIVE);
	wav_random = gi.cvar ("wav_random", "1", CVAR_ARCHIVE);
	wav = gi.cvar ("wav", "mm.wav",0);
	wav_mod_current_level = -1;
	wav_mod_num_wavs = 0;

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

		rewind(file);
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
				i = (int) floor(random() * ((float)(unused_wav)));

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

				i = (int) floor(random() * ((float)(wav_mod_num_wavs)));

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
I'm placing it here for now until I can refactor and figure out
if it's even needed. Caching a long list blows up the server
when it can't cache the rest of the weapon and player sounds.
Game limit is 256 sounds and the original code allowed for up to
256 songs. I've tried to reconcile the manifest constants with
the game limits in q_shared.h.
As long as we don't cache them in the index we're ok.

I think the proper fix for this is to index the next song into the
configstring at the start of a new map to make it ready for play at
the next intermission. Need to look into how this affects downloads.

NOTE: First item in the intro list doesn't get precached? Why not?
levels is 0 on entry, this looks like a bug to me.
*/
//RAV precache songs
void PrecacheSongs(void)
{
	FILE *file;
	char names[MAXSONGS][MAX_QPATH];
	char file_name[MAX_QPATH];
	char song[MAX_QPATH];
	int levels = 0;
	size_t	count;

	song[0] = '\0';

	sprintf(file_name, "%s/%s/%s/intro.txt",
		basedir->string, game_dir->string, cfgdir->string);

	file = fopen(file_name, "r");
	if (file != NULL)
	{
		int file_size = 0;
		char *p_buffer;
		char *p_name;
		long counter = 0;
		int n_chars = 0;

		while (!feof(file))
		{
			fgetc(file);
			file_size++;
		}
		rewind(file);
		p_buffer = gi.TagMalloc(file_size, TAG_LEVEL);
		memset(p_buffer, 0, file_size);
		count = fread((void *)p_buffer, sizeof(char), file_size, file);
		if (!count)
			gi.dprintf("%s read %d of %d bytes in %s\n",
			__FUNCTION__, count, file_size, file);

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
				if (levels > 0)
					//precache here 
					sprintf(song, "misc/%s.wav", names[levels]);
				if (strlen(song) > 0)
				{
					//int i;
					//DbgPrintf("song to be indexed is %s\n", song);
					gi.soundindex (song);
					//DbgPrintf("song index is %d\n", i);
				}
				levels++;
				n_chars = 0;
				if (levels >= MAXSONGS)
				{
					gi.dprintf("\nMAXSONGS exceeded\n"
						"Unable to add more Wav's.\n");
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
	else
	{
		gi.dprintf ("==== Wav Mod v.01 - missing intro.txt file ====\n");
	}
}
