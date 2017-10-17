
#ifndef FILEHAND_H
#define FILEHAND_H


qboolean lessGeneral(char *line, char *comp);
int checkAllowed (char *userinfo);
char *NameAndIp (edict_t *ent);
int IPMatch (char *clientIP, char *maskIP);
qboolean entryInFile (char *filename, char ip[]);
int CheckOpFile (edict_t *ent, char ip[], qboolean returnindex);
qboolean CheckNameProtect (char name[], char namepass[]);
qboolean ModifyOpLevel (int entry, int newlevel);
int AddOperator (char entry[], int level, char pass[16]);
void AddEntry (char *filename, char *text);
void sv_ban_ip (edict_t *ent);
void ShowFile(edict_t *ent, char *filename);
void LogConnect (edict_t *ent, qboolean connect);
void LogChat (char *text);


#endif /* FILEHAND_H */
