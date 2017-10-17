
#include "g_local.h"

qboolean CheckModel(edict_t *ent, char *s)
{
	FILE    *banlist;
	char    line[MAX_QPATH];
	char	file_name[256];

	if(!check_models->value)
		return true;

	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/models.txt", basedir->string, game_dir->string, cfgdir->string);

	banlist = fopen(file_name, "r");
	if (banlist)
	{
		while ( fgets(line, sizeof line, banlist) )
		{
			line[strlen(line)-1] = '\0';
			if ( strstr(s, line ) )
			{
				fclose(banlist);
				return true;
			}
		}
		fclose(banlist);
		return false;
	}
	else
	{
		gi.dprintf("Can't Open models.txt\n");
		return true;
	}
}
