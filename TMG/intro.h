#ifndef INTRO_H

void Wav_Mod_Setup(void);	// Attempts to find and load intro.txt
char* Wav_Mod_Next(void);	// Retrieves name of next level in the list
void PrecacheSongs(void);	// Unused.

//QW// Set the max length of the song list.
#define	MAXSONGS	1000

extern  cvar_t  *wavs;
extern  cvar_t  *songtoplay;
extern  cvar_t  *use_song_file;

#endif //INTRO_H
