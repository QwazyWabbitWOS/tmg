#ifndef S_MAP_H
#define S_MAP_H

typedef struct
{
    char   aFile[MAX_QPATH];
    char   aName[MAX_QPATH];
    int    min;
    int    max;
    int    fVisited;
} MAP_ENTRY;

extern MAP_ENTRY   *mdsoft_map;

extern edict_t *mdsoft_NextMap( void );


#endif //S_MAP_H
