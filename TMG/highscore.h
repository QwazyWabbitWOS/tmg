
#ifndef HIGHSCORE_H
#define HIGHSCORE_H

extern cvar_t  *highscores;

extern char hscores [];

extern qboolean show_hs;
extern qboolean hs_show;

extern void InitHighScores (void);
extern void SaveHighScores (void);
extern void LoadHighScores (void);

#endif /* HIGHSCORE_H */