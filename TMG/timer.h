//
//  timer.h
//

#ifndef TIMER_H
#define TIMER_H

#define STATE_NEEDPLAYERS 0
#define STATE_ENDGAME     1
#define STATE_COUNTDOWN   2
#define STATE_WARMUP      3 
#define STATE_PLAYING     4

//QW added: Guard against bad state constants.
#if ( STATE_PLAYING < STATE_WARMUP )
	#error Bad STATE constants.
#endif

// global variables :
extern int match_state;
extern float match_state_end;

void ResetItems (void);
void RestartLevel (void);
void TimerThink (void);
void RandomRCON(void);

/**
 returns true if less than 60 seconds left in match
 */
int CountDownInFinalMinute (void);

/**
 Track the match time. Emit warnings
 at 5,4,3,2,1 minute thresholds and
 sounds at 30 and 10 seconds left.
 */
void CountDown (void);

void CheckState(void);

/**
 start the server in a 'waiting for players' state
 */
void ServerInit (int resetall);

#define STAT_COUNTDOWN          28

#endif  /* TIMER_H */
