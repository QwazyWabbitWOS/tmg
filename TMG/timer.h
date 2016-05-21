//
//  timer.h
//

#ifndef timer_h
#define timer_h

#define STATE_NEEDPLAYERS 0
#define STATE_ENDGAME 1
#define STATE_COUNTDOWN 2
#define STATE_WARMUP 3 
#define STATE_PLAYING 4


// global variables :
extern int match_state;
extern float match_state_end;

extern void ResetItems (void);
extern void RestartLevel (void);
extern void TimerThink (void);  //unused

//QW// declared but doesn't exist?
void MatchInit (int resetall);
void MatchThink (void);
//QW//

int CountDownInFinalMinute (void);
void CountDown (void);
void CheckState(void);


/**
 start the server in a 'waiting for players' state
 */
void ServerInit (int resetall);

#define for_each_playerbot(JOE_BLOGGS, INDEX) \
for(INDEX = 1;INDEX <= maxclients->value; INDEX++) \
	if ((JOE_BLOGGS = &g_edicts[INDEX]) && JOE_BLOGGS && JOE_BLOGGS->inuse)

#define for_each_player(JOE_BLOGGS, INDEX) \
for(INDEX = 1;INDEX <= maxclients->value; INDEX++) \
	if ((JOE_BLOGGS = &g_edicts[INDEX]) && JOE_BLOGGS && JOE_BLOGGS->inuse && !JOE_BLOGGS->bot_client)

#define STAT_COUNTDOWN          28

#endif  //timer_h
