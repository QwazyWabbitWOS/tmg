//
//  filehand.h
//  tmg
//

#ifndef FILEHAND_H
#define FILEHAND_H


extern qboolean lessGeneral(char line[IP_LENGTH], char comp[IP_LENGTH]);
extern int checkAllowed (char *userinfo);
extern char *NameAndIp (edict_t *ent);
extern int IPMatch (char *clientIP, char *maskIP);
extern qboolean entryInFile (char *filename, char ip[IP_LENGTH]);
extern int CheckOpFile (edict_t *ent, char ip[IP_LENGTH], qboolean returnindex);
extern qboolean CheckNameProtect (char name[IP_LENGTH], char namepass[IP_LENGTH]);
extern qboolean ModifyOpLevel (int entry, int newlevel);
extern int AddOperator (char entry[IP_LENGTH], int level, char pass[16]);
extern void addEntry (char *filename, char ip[IP_LENGTH]);
extern void sv_ban_ip (edict_t *ent);
extern void ShowFile(edict_t *ent, char *filename);
extern void LogConnect (edict_t *ent, qboolean connect);
extern void LogChat (char *text);


#endif /* FILEHAND_H */
