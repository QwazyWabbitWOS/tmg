
#ifndef HIGHSCORE_H
#define HIGHSCORE_H

extern cvar_t	*highscores;
extern cvar_t	*show_highscores;

extern char hscores [];

extern qboolean hs_show;

void InitHighScores (void);
void SaveHighScores (void);
void LoadHighScores (void);

#endif /* HIGHSCORE_H */
