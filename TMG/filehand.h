
#ifndef FILEHAND_H
#define FILEHAND_H


extern qboolean lessGeneral(char line[], char comp[]);
extern int checkAllowed (char *userinfo);
extern char *NameAndIp (edict_t *ent);
extern int IPMatch (char *clientIP, char *maskIP);
extern qboolean entryInFile (char *filename, char ip[]);
extern int CheckOpFile (edict_t *ent, char ip[], qboolean returnindex);
extern qboolean CheckNameProtect (char name[], char namepass[]);
extern qboolean ModifyOpLevel (int entry, int newlevel);
extern int AddOperator (char entry[], int level, char pass[16]);
extern void addEntry (char *filename, char ip[]);
extern void sv_ban_ip (edict_t *ent);
extern void ShowFile(edict_t *ent, char *filename);
extern void LogConnect (edict_t *ent, qboolean connect);
extern void LogChat (char *text);


#endif /* FILEHAND_H */
