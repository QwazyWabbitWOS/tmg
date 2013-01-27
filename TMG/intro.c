#include "g_local.h"
#include "hud.h"
#include <stdio.h>

#include <stdio.h>

#define	MAXSONGS	256

int			wav_mod_ = 0;
int			wav_mod_current_level_ = -1;
int			wav_mod_n_levels_ = 0;
char		wav_mod_names_[MAXSONGS][64];

// niq hack:
qboolean	wav_used[MAXSONGS];
int			unused_wav=0;

cvar_t *use_song_file;
cvar_t *wav_random;
cvar_t *wav;
/////////////////////////////////////////////////////////////////////////////

void wav_mod_set_up()
{
	FILE *file;
	char file_name[256];
	
	use_song_file = gi.cvar ("use_song_file", "0", CVAR_ARCHIVE);
	wav_random = gi.cvar ("wav_random", "1", CVAR_ARCHIVE);
	wav = gi.cvar ("wav", "mm.wav",0);

#if defined(linux)
	sprintf(file_name, "%s/%s/%s/intro.txt", basedir->string, game_dir->string, cfgdir->string);
#else
	sprintf(file_name, "%s\\%s\\%s\\intro.txt", basedir->string, game_dir->string, cfgdir->string);
#endif

	file = fopen(file_name, "r");
	wav_mod_ = 0;
	wav_mod_current_level_ = -1;
	wav_mod_n_levels_ = 0;
	if (file != NULL)
	{
		long file_size;
		char *p_buffer;
		char *p_name;
		long counter = 0;
		int n_chars = 0;

		file_size = 0;
		while (!feof(file))
		{
		  fgetc(file);
		  file_size++;
		}
		rewind(file);

		p_buffer = gi.TagMalloc(file_size, TAG_GAME);
		memset(p_buffer,0,file_size);

		fread((void *)p_buffer, sizeof(char), file_size, file);

		gi.dprintf ("\n==== Wav Mod v.01 set up ====\n");
		gi.dprintf("Adding Wav's to cycle: ");

		p_name = p_buffer;
		do
		{
			// niq: skip rest of line after a '#' (works with Unix?)
			if(*p_name == '#')
			{
				while ((*p_name != '\n') && (*p_name != '\r') && counter < file_size)
					{
					p_name++;
					counter++;
					}
			}
			else
			{
				while ((((*p_name >= 'a') && (*p_name <= 'z')) || ((*p_name >= 'A') && (*p_name <= 'Z')) || ((*p_name >= '0') && (*p_name <= '9')) || (*p_name == '_') || (*p_name == '-') || (*p_name == '/') || (*p_name == '\\')) && counter < file_size)
				{
					n_chars++;
					counter++;
					p_name++;
				}
			}

			if (n_chars)
			{
				memcpy(&wav_mod_names_[wav_mod_n_levels_][0], p_name - n_chars, n_chars);
				memset(&wav_mod_names_[wav_mod_n_levels_][n_chars], 0, 1);

				if (wav_mod_n_levels_ > 0)
					gi.dprintf(", ");
				gi.dprintf("%s", wav_mod_names_[wav_mod_n_levels_]);

				wav_mod_n_levels_++;
				n_chars = 0;

				if (wav_mod_n_levels_ >= MAXSONGS)
				{
					gi.dprintf("\nMAXSONGS exceeded\nUnable to add more Wav's.\n");
					break;
				}
			}

			// next songname
			counter++;
			p_name++;

			// eat up non-characters (niq: except #)
			while (!((*p_name == '#') || ((*p_name >= 'a') && (*p_name <= 'z')) || ((*p_name >= 'A') && (*p_name <= 'Z')) || ((*p_name >= '0') && (*p_name <= '9')) || (*p_name == '_') || (*p_name == '-') || (*p_name == '/') || (*p_name == '\\')) && counter < file_size)
			{
				counter++;
				p_name++;
			}

		} while (counter < file_size);

		gi.dprintf("\n\n");

		gi.TagFree(p_buffer);
		fclose(file);

		if (wav_mod_n_levels_)
		{
			wav_mod_ = true;
		}
	}
	else
	{
		gi.dprintf ("==== Wav Mod v.01 - missing intro.txt file ====\n");
		gi.cvar_set ("use_song_file", "0");
	}

	unused_wav=0;
}

/////////////////////////////////////////////////////////////////////////////

char* wav_mod_next_map()
{
	int i;

	if (wav_mod_)
	{
		if (wav_random->value)
		{
			// NIQ hack start
			if(wav_mod_n_levels_ >= 2)
			{
				// niq: hack to mapmode code to make sure we try all maps
				// before starting over.
				int map;
				int skipped;

				if(!unused_wav)
					{
					// reset random maps 
					for(map=0; map<wav_mod_n_levels_; map++)
						wav_used[map] = false;

					if(wav_mod_current_level_ == -1 && wav->string)
						{
						// no current MapMod map:
						// if there is a current map make sure we don't
						// pick it again right away if it is in the list
						for (i=0; (i<wav_mod_n_levels_ && wav_mod_current_level_== -1); i++)
							if (!Q_stricmp(wav->string, wav_mod_names_[i]))
								wav_mod_current_level_ = i;

						}

					if(wav_mod_current_level_ != -1)
						{
						// zap the map
						wav_used[wav_mod_current_level_] = true;

						// one less unused map to choose from
						unused_wav = wav_mod_n_levels_ - 1; 
						}
					else
						{
						// can choose any map in list
						unused_wav = wav_mod_n_levels_; 
						}
					}

				// pick number of unsued maps to skip (less clustering likely)
				i = (int) floor(random() * ((float)(unused_wav)));

				// skip to first unused map (has to find one)
				map	= 0;
				while(wav_used[map])
					map++;

				// skip over i unused maps (has to find them)
				skipped	= 0;
				while(skipped<i)
					{
					if(!wav_used[map++])
						skipped++;
					}

				// skip to unused map if necessary (e.g. if last skip skipped to used one)
				while(wav_used[map])
					map++;

				wav_mod_current_level_ = map;
				wav_used[map] = true;
				unused_wav--;
			}
			// NIQ hack end
		else
			{
				wav_mod_current_level_ = -1;

				i = (int) floor(random() * ((float)(wav_mod_n_levels_)));

				if (!Q_stricmp(wav->string, wav_mod_names_[i]))
				{
					if (++i >= wav_mod_n_levels_)
						i=0;
				}
				wav_mod_current_level_ = i;
			}
		}
		else
		{
			wav_mod_current_level_ = -1;
	
			for (i=0; i < wav_mod_n_levels_; i++)
				if (!Q_stricmp(wav->string, wav_mod_names_[i]))
					wav_mod_current_level_ = i+1;
		}

		if (wav_mod_current_level_ >= wav_mod_n_levels_)
		{
			wav_mod_current_level_ = 0;
		}

		if (wav_mod_current_level_ > -1)
		{
			return wav_mod_names_[wav_mod_current_level_];
		}

	}

	return NULL;
}
