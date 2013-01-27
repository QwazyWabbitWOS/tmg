
#include "g_local.h"
qboolean CheckModel(edict_t *ent, char *s)
{
	FILE    *banlist;
	char    line[IP_LENGTH];
	char	file_name[256];
	
	if(!check_models->value)
		return true;

#if defined(linux)
	sprintf(file_name, "%s/%s/%s/models.txt", basedir->string, game_dir->string, cfgdir->string);
#else
	sprintf(file_name, "%s\\%s\\%s\\models.txt", basedir->string, game_dir->string, cfgdir->string);
#endif
	
	if ( banlist = fopen(file_name, "r") )
	{
		while ( fgets(line, 80, banlist) ) 
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
