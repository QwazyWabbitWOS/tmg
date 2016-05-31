
#ifndef HIGHSCORE_H
#define HIGHSCORE_H

extern cvar_t	*highscores;
extern cvar_t	*show_highscores;

extern char hscores [];

extern qboolean hs_show;

extern void InitHighScores (void);
extern void SaveHighScores (void);
extern void LoadHighScores (void);

#endif /* HIGHSCORE_H */