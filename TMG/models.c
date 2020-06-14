
#include "g_local.h"

/**
 Checks the models.txt file for permitted models
 and returns true if permitted or if models.txt 
 file doesn't exist.
 Return 0 if model doesn't match any in list.
 If we're not checking models, permit any.
*/
qboolean CheckModel(edict_t* ent, char* s)
{
	FILE* permlist;
	char	line[MAX_QPATH];
	char	file_name[MAX_QPATH];

	if (!check_models->value)
		return true;

	Com_sprintf(file_name, sizeof file_name, "%s/%s/%s/models.txt",
		basedir->string, game_dir->string, cfgdir->string);

	permlist = fopen(file_name, "r");
	if (permlist)
	{
		while (fgets(line, sizeof line, permlist))
		{
			line[strlen(line) - 1] = '\0'; // chomp off '\n'
			if (strstr(s, line) && strlen(line) != 0)
			{
				fclose(permlist);
				return true;
			}
		}
		fclose(permlist);
		return false;	//no matches, return false.
	}
	else
	{
		gi.dprintf("TMG WARNING: Can't Open %s\n", file_name);
		return true; // no list, permit any skins.
	}
}
