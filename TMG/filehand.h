
#ifndef FILEHAND_H
#define FILEHAND_H

typedef struct oplist_s
{
	int	level;
    int	newlevel;
	char	entry[MAX_QPATH];
	char	namepass[16];
	char	name[MAX_QPATH];
	char	ip[MAX_QPATH];
	qboolean	flagged;
} oplist_t;

#define MAX_OPS	64

extern oplist_t	*oplist;

void ShowOps(void);
qboolean lessGeneral(char *line, char *comp);
int checkAllowed (char *userinfo);
char *NameAndIp (edict_t *ent);
int IPMatch (char *clientIP, char *maskIP);
qboolean entryInFile (char *filename, char* ip);
int LoadOpFile(void);
int CheckOpFile (edict_t *ent, char* ip, qboolean returnindex);
qboolean CheckNameProtect (const char* name, const char* namepass);
qboolean ModifyOpLevel (int entry, int newlevel);
int AddOperator (char entry[MAX_QPATH], int op_lev, char pass[16]);
void AddEntry (char *filename, char *text);
void sv_ban_ip (edict_t *ent);
void ShowFile(edict_t *ent, char *filename);
void LogConnect (edict_t *ent, qboolean connect);
void LogChat (char *text);

#endif /* FILEHAND_H */
