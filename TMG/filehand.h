//
//  filehand.h
//  tmg
//

#ifndef filehand_h
#define filehand_h


qboolean lessGeneral(char line[IP_LENGTH], char comp[IP_LENGTH]);
int checkAllowed (char *userinfo);
char *NameAndIp (edict_t *ent);
int IPMatch (char *clientIP, char *maskIP);
qboolean entryInFile (char *filename, char ip[IP_LENGTH]);
int CheckOpFile (edict_t *ent, char ip[IP_LENGTH], qboolean returnindex);
qboolean CheckNameProtect (char name[IP_LENGTH], char namepass[IP_LENGTH]);
qboolean ModifyOpLevel (int entry, int newlevel);
int AddOperator (char entry[IP_LENGTH], int level, char pass[16]);
void addEntry (char *filename, char ip[IP_LENGTH]);
void sv_ban_ip (edict_t *ent);void ShowFile(edict_t *ent, char *filename);
void LogConnect (edict_t *ent, qboolean connect);
void LogChat (char *text);



#endif /* filehand_h */
