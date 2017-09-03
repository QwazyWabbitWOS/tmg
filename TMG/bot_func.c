
#include "g_local.h"
#include "g_items.h"
#include "m_player.h"
#include "p_client.h"
#include "g_chase.h"
#include "bot.h"
#include "e_hook.h"
#include "g_cmds.h"
#include "timer.h"
#include "hud.h"
#include "botstr.h"

qboolean Get_YenPos(char *Buff, int *curr)
{
	int i = *curr + 1;
	while(1)
	{
		if(Buff[i] == '\0' || Buff[i] == '\n' || Buff[i] == '\r')
		{
			*curr = i;
			return true;
		}
		if(Buff[i] == '\\')
		{
			*curr = i;
			return true;
		}
		if(Buff[i] == '\t')
			Buff[i] = '\0';
		i++;
	}
	return false;
}
//======================================
//RaVeN  12-18-99
//check to see if we need to add or remove bots
//based on bot_free client spaces avaliable
//========================================

int GetNumBots(void)
{
	int i;
	int botCount = 0;

	for (i = 1; i <= game.maxclients; i++)
	{
		edict_t* ent = g_edicts + i;
		if (ent->inuse && ent->bot_client)
			botCount++;
	}
	return botCount;
}

void Adjust_Bot_Number (void)
{
	edict_t  *ent;
	int	i;

	//  check if we need to add a bot
	// for a dm server that wants to be "full"
	// but allow clients # to dictate bot coming
	// and going 

	if(match_state != STATE_PLAYING)
		return;
	if(level.intermissiontime)
		return;
	if(bot_time != level.time)
		return;
	if(!bot_num->value && use_bots->value && !chedit->value)
	{
		//ADD A BOT !!
		if ((wait_time <= level.time) && 
			(CountConnectedClients() < (maxclients->value - bot_free_clients->value )))
		{
			gi.AddCommandString ("sv spb 1");
			wait_time = level.time + 3;
		}
		//REMOVE A BOT !!  lowest on frags first
		if ((GetNumBots() > 0) && 
			(CountConnectedClients() > (maxclients->value - bot_free_clients->value )) &&
			(kill_time <= level.time))
		{
			// drop a bot to free a client spot
			for (i=1; i<=maxclients->value; i++)
			{
				ent = &g_edicts[i];
				if ((ent && !ent->bot_client) || 
					(CountConnectedClients() <= (maxclients->value - bot_free_clients->value )))
					continue;
				//disconnect bot
				ClientDisconnect(ent);
				kill_time = level.time + 3;
				gi.dprintf("%i bots are now left in the game\n", GetNumBots());
			}
		}
	} 
	//set up for next check
	bot_time = level.time + 3;
}

//ctf
void BotAssignTeamCtf(gclient_t *who)
{
	edict_t *player;
	int i;
	int team1count = 0, team2count = 0;

	for (i = 1; i <= maxclients->value; i++)
	{
		player = &g_edicts[i];

		if (!player->inuse || player->client == who)
			continue;

		switch (player->client->resp.ctf_team)
		{
		case CTF_TEAM1: 
			team1count++;
			break;
		case CTF_TEAM2: 
			team2count++;
			break;
		}
	}
	if (team1count < team2count) 
		who->resp.ctf_team = CTF_TEAM1;
	else if (team2count < team1count) 
		who->resp.ctf_team = CTF_TEAM2;
	else if (rand() & 1) 
		who->resp.ctf_team = CTF_TEAM1;
	else 
		who->resp.ctf_team = CTF_TEAM2;
}

//=======================================================
//============ TAUNTING/CHATTING/INSULTING ==============
//=======================================================

//==============================================
edict_t *BestScorePlayer(void)
{
	edict_t *bestplayer = NULL;
	int i;
	int bestscore = -999;
	edict_t *ent;

	for(i = 0; i < game.maxclients; i++)
	{
		ent = g_edicts + i + 1;
		if (!ent || !ent->inuse)
			continue;
		if (ent->client->resp.score <= bestscore)
			continue;
		bestplayer = ent;
		bestscore = ent->client->resp.score;
	}
	return bestplayer;
}

//==============================================
void InsultVictim(edict_t *ent, edict_t *victim)
{

	if (!ent->bot_client)
		return;

	if (!victim || !victim->client)
		return;

	if (ent->client->insulttime > level.time)
		return;

	// if bot just killed self then...
	if (victim == ent)
	{
		if (myrandom < 0.40)
			switch (rand() % 7)
		{
			case 0: safe_bprintf(3,"%s: Man! I suck!\n", ent->client->pers.netname); break;
			case 1: safe_bprintf(3,"%s: I hate that!\n", ent->client->pers.netname); break;
			case 2: safe_bprintf(3,"%s: shit!\n", ent->client->pers.netname); break;
			case 3: safe_bprintf(3,"%s: not again!\n", ent->client->pers.netname); break;
			case 4: safe_bprintf(3,"%s: Ugghhhh!\n", ent->client->pers.netname); break;
			case 5: safe_bprintf(3,"%s: wtf!\n", ent->client->pers.netname); break;
			case 6: safe_bprintf(3,"%s: This sucks!\n", ent->client->pers.netname);
		}
	}
	// if bot on top of frag board then...
	else if (ent == BestScorePlayer())
	{
		if (myrandom < 0.30) // Very Rarely
			switch (rand() % 9)
		{
			case 0: safe_bprintf(3,"%s: I'm the best!\n", ent->client->pers.netname); break;
			case 1: safe_bprintf(3,"%s: Kicking ASS!!\n", ent->client->pers.netname); break;
			case 2: safe_bprintf(3,"%s: Beat the bots!!\n", ent->client->pers.netname); break;
			case 3: safe_bprintf(3,"%s: I freaking rule!\n", ent->client->pers.netname); break;
			case 4: safe_bprintf(3,"%s: I RULE!!\n", ent->client->pers.netname); break;
			case 5: safe_bprintf(3,"%s: Your sister!!\n", ent->client->pers.netname); break;
			case 6: safe_bprintf(3,"%s: Your daughter!!!!\n", ent->client->pers.netname); break;
			case 7: safe_bprintf(3,"%s: Your mama!\n", ent->client->pers.netname); break;
			case 8: safe_bprintf(3,"%s: Who's Your daddy!!!\n", ent->client->pers.netname);
		}
	}
	// Otherwise....
	else if (myrandom < 0.80)
		switch (rand() % 12)
	{
		case 0: safe_bprintf(3,"%s: You REALLY suck!\n", ent->client->pers.netname); break;
		case 1: safe_bprintf(3,"%s: I need some better players!!!\n", ent->client->pers.netname); break;
		case 2: safe_bprintf(3,"%s: U R LAME !!!!\n", ent->client->pers.netname); break;
		case 3: safe_bprintf(3,"%s: This sucks!\n", ent->client->pers.netname); break;
		case 4: safe_bprintf(3,"%s: Suck it!\n", ent->client->pers.netname); break;
		case 5: safe_bprintf(3,"%s: You ALL suck!\n", ent->client->pers.netname); break;
		case 6: safe_bprintf(3,"%s: Suck THAT\n", ent->client->pers.netname); break;
		case 7: safe_bprintf(3,"%s: Muhhhhaahhhaaa\n", ent->client->pers.netname); break;
		case 8: safe_bprintf(3,"%s: Muhaaaaaaaaa!!\n", ent->client->pers.netname); break;
		case 9: safe_bprintf(3,"%s: Huuuhhhaaaaaa!\n", ent->client->pers.netname); break;
		case 10:safe_bprintf(3,"%s: Muhhhhhhaaaaa!!!\n", ent->client->pers.netname); break;
		case 11:safe_bprintf(3,"%s: Whoooooaaaaa!\n", ent->client->pers.netname);
	}
	ent->client->insulttime = level.time + 60 + (10*(rand()%6));
}

//==============================================
void TauntVictim(edict_t *ent, edict_t *victim)
{
	vec3_t vtmp;

	if (!ent->bot_client || ent == victim)
		return;

	if (!victim || !victim->client)
		return;

	if (ent->client->taunttime > level.time)
		return;

	// Taunt only if near victim (don't reset timer)
	VectorSubtract(ent->s.origin, victim->s.origin, vtmp);
	if (VectorLength(vtmp) > 250)
		return;

	switch (rand() % 3)
	{
	case 0:
		ent->s.frame = FRAME_flip01-1;
		ent->client->anim_end = FRAME_flip12;
		break;
	case 1:
		ent->s.frame = FRAME_salute01-1;
		ent->client->anim_end = FRAME_salute11;
		break;
	case 2:
		ent->s.frame = FRAME_taunt01-1;
		ent->client->anim_end = FRAME_taunt17;
	}

	ent->client->taunttime = level.time + 30 + (10*(rand()%6));
}
//==============================================
void RandomChat(edict_t *ent)
{
	if (ent->client->chattime > level.time)
		return;

	if (ent->client->camptime > level.time)
	{
		if (myrandom < 0.50) // Camp and Chat about 50% of the time
		{
			switch (rand() % 6)
			{
			case 0: safe_bprintf(3,"%s: Bring it on Bitch!!!\n", ent->client->pers.netname); break;
			case 1: safe_bprintf(3,"%s: Defending Base!!\n", ent->client->pers.netname); break;
			case 2: safe_bprintf(3,"%s: I need help at Base!!!\n", ent->client->pers.netname); break;
			case 3: safe_bprintf(3,"%s: Hurry up and Cap it !!\n", ent->client->pers.netname); break;
			case 4: safe_bprintf(3,"%s: TMG Freaking RULEZ\n", ent->client->pers.netname); break;
			case 5: safe_bprintf(3,"%s: There's a camper at Base!\n", ent->client->pers.netname); break;
			}
		}
		ent->client->chattime = level.time + 60 + (20 * (rand() % 6 ));
	}
}

//===============================================
void CheckCampSite(edict_t *other)
{
	//set up next check
	other->client->nextcamp = level.time + (int) bot_camptime->value;;

	if (other->client->quad_framenum > level.framenum)
		return;

	if (other->client->camptime >= level.time)
		return;

	if (! (other->groundentity) || other->client->ctf_grapple)
		return; // camp 30% of time

	other->client->camptime = level.time + 5 + rand() % 10; // 20..30
	other->client->chattime = level.time + 10 + rand() % 15; // 10..25
	other->client->taunttime = other->client->camptime + 10; // turn OFF!
	VectorCopy(other->s.origin, other->client->lastorigin);
	safe_bprintf(3,"%s: Defending Base!!\n", other->client->pers.netname);
}

//=============================================
qboolean InsideWall(edict_t *ent)
{
	vec3_t torigin;

	VectorCopy(ent->s.origin, torigin);
	torigin[0] += 10.0;
	torigin[1] += 10.0;
	if (gi.pointcontents(torigin) & CONTENTS_SOLID && ent->client)
	{
		DbgPrintf("1 %s %s\n", ent->client->pers.netname, __FUNCTION__);
		return true;
	}

	VectorCopy(ent->s.origin, torigin);
	torigin[0] += 10.0;
	torigin[1] -= 10.0;
	if (gi.pointcontents(torigin) & CONTENTS_SOLID && ent->client)
	{
		DbgPrintf("2 %s %s\n", ent->client->pers.netname, __FUNCTION__);
		return true;
	}

	VectorCopy(ent->s.origin, torigin);
	torigin[0] -= 10.0;
	torigin[1] += 10.0;
	if (gi.pointcontents(torigin) & CONTENTS_SOLID && ent->client)
	{
		DbgPrintf("3 %s %s\n", ent->client->pers.netname, __FUNCTION__);
		return true;
	}

	VectorCopy(ent->s.origin, torigin);
	torigin[0] -= 10.0;
	torigin[1] -= 10.0;
	if (gi.pointcontents(torigin) & CONTENTS_SOLID && ent->client)
	{
		DbgPrintf("4 %s %s\n", ent->client->pers.netname, __FUNCTION__);
		return true;
	}

	return false;
}
//======================================================
void AdjustAngle(edict_t *ent, vec3_t targaim, float aim)
{

	VectorSet(ent->s.angles, (Get_pitch(targaim)), (Get_yaw(targaim)),  0.0F);

	ent->s.angles[1] += aim*0.70*(myrandom-0.5);
	if (ent->s.angles[1] > 180)
		ent->s.angles[1] -= 360;
	else
		if (ent->s.angles[1] < -180)
			ent->s.angles[1] += 360;

	ent->s.angles[0] += aim*0.70*(myrandom-0.5);
	if (ent->s.angles[0] > 90)
		ent->s.angles[0] = 90;
	else
		if (ent->s.angles[0] < -90)
			ent->s.angles[0] = -90;
}
//RAVEN

//END RAVEN ADDITIONS 

//----------------------------------------------------------------
//Load Bot Info
//
// Load bot's information from the config file
//
//----------------------------------------------------------------
void Load_BotInfo(void)
{
	char	MessageSection[50];
	char	Buff[1024];
	int		i,j,k,l;
	char	filename[MAX_QPATH];
	FILE	*fp;

	SpawnWaitingBots = 0;
	ListedBotCount = 0;

	//init message
	memset(ClientMessage, 0, sizeof(ClientMessage));

	//set message section
	if(!ctf->value && chedit->value) 
		strcpy(MessageSection, MESS_CHAIN_DM);
	else if(ctf->value && !chedit->value) 
		strcpy(MessageSection, MESS_CTF);
	else if(ctf->value && chedit->value) 
		strcpy(MessageSection, MESS_CHAIN_CTF);
	else 
		strcpy(MessageSection, MESS_DEATHMATCH);

	//init botlist
	ListedBots = 0;
	j = 1;
	for(i = 0; i < MAXBOTS; i++)
	{
		//netname
		sprintf(Buff,"TMGBot[%i]", i);
		strcpy(Bot[i].netname, Buff);
		//model
		strcpy(Bot[i].model, "female");
		//skin
		strcpy(Bot[i].model, "athena");

		//param
		Bot[i].param[BOP_WALK] = 0;
		Bot[i].param[BOP_AIM] = 5;
		Bot[i].param[BOP_PICKUP] = 5;
		Bot[i].param[BOP_COMBATSKILL] = 8;
		Bot[i].param[BOP_ROCJ] = 0;
		Bot[i].param[BOP_VRANGE] = 90;
		Bot[i].param[BOP_HRANGE] = 90;
		Bot[i].param[BOP_REACTION] = 0;

		//spawn flag
		Bot[i].spflg = 0;
		//team
		Bot[i].team = j;
		if(++j > 2) 
			j = 1;
	}

	//botlist value
	botlist = gi.cvar ("botlist", "default", CVAR_LATCH);

	//load info
	sprintf(Buff, "%s/%s/%s/TMGBot.cfg", 
		basedir->string, game_dir->string, cfgdir->string);

	fp = fopen(Buff, "rt");
	if(fp == NULL)
	{
		gi.dprintf("File not found %s\n", Buff);
		gi.dprintf("Using default bot configuration.\n");
		return;
	}
	else
	{
		strcpy(filename, Buff);
		fseek( fp, 0, SEEK_SET);
		while(1)
		{
			if(fgets( Buff, sizeof(Buff), fp ) == NULL) 
				goto MESS_NOTFOUND;
			if(!strnicmp(MessageSection, Buff, strlen(MessageSection))) 
				break;
		}

		while(1)
		{
			if(fgets( Buff, sizeof(Buff), fp ) == NULL) 
				goto MESS_NOTFOUND;
			if(Buff[0] == '.' || Buff[0] == '[' || Buff[0] == '#')
				break;
			k = strlen(Buff);
			if((strlen(Buff) + strlen(ClientMessage)) > MAX_STRING_CHARS - 1)
				break;
			strcat(ClientMessage, Buff);
		}

MESS_NOTFOUND:
		//if(botlist->string == NULL) 
		//	strcpy(MessageSection, BOTLIST_SECTION_DM);
		//else
		sprintf(MessageSection, "[%s]", botlist->string);
		fseek( fp, 0, SEEK_SET);
		while(1)
		{
			if(fgets( Buff, sizeof(Buff), fp ) == NULL)
			{
				MessageSection[0] = 0;
				break;
			}
			if(!strnicmp(MessageSection, Buff, strlen(MessageSection)))
				break;
		}
		//when not found
		if(MessageSection[0] == 0)
		{
			strcpy(MessageSection, BOTLIST_SECTION_DM);
			fseek( fp, 0, SEEK_SET);
			while(1)
			{
				if(fgets( Buff, sizeof(Buff), fp ) == NULL) 
					goto BOTLIST_NOTFOUND;
				if(!strnicmp(MessageSection, Buff, strlen(MessageSection))) 
					break;
			}
		}

		for(i = 0; i < MAXBOTS; i++)
		{
			if(fgets( Buff, sizeof(Buff), fp ) == NULL)
				break;
			if(Buff[0] == '[')
				break;
			
			if(Buff[0] == '\n' || Buff[0] == '\r' || Buff[0] == '#')
			{
				i--;
				continue;
			}
			j = 2, k = 1;
			if(!strncmp(Buff,"\\\\",2))
			{
				//netname
				if(Get_YenPos(Buff, &k))
				{
					Buff[k] = 0;
					if(strlen(&Buff[j]) < 21) 
						strcpy(Bot[i].netname, &Buff[j]);
					j = k + 1;
				}
				else 
					break;
				//model name
				if(Get_YenPos(Buff, &k))
				{
					Buff[k] = 0;
					if(strlen(&Buff[j]) < 21) 
						strcpy(Bot[i].model, &Buff[j]);
					j = k + 1;
					k++;
				}
				else 
					break;
				//skin name
				if(Get_YenPos(Buff, &k))
				{
					Buff[k] = 0;
					if(strlen(&Buff[j]) < 21) 
						strcpy(Bot[i].skin, &Buff[j]);
					j = k + 1;
					k++;
				}
				else 
					break;

				for(l = 0;l < MAXBOP;l++)
				{
					//param0-7
					if(Get_YenPos(Buff, &k))
					{
						Buff[k] = 0;
						Bot[i].param[l] = (unsigned char)atoi(&Buff[j]);
						j = k + 1;
						k++;
					}
					else 
						break;
				}
				if(l < MAXBOP) 
					break;
				//team
				if(Get_YenPos(Buff, &k))
				{
					Buff[k] = 0;
					if(Buff[j] == 'R') 
						Bot[i].team = 1;
					else if(Buff[j] == 'B') 
						Bot[i].team = 2;
					else 
						Bot[i].team = 1;
					j = k + 1;
					k++;
				}
				else 
					break;
				//auto spawn
				if(Get_YenPos(Buff,&k))
				{
					Buff[k] = 0;
					Bot[i].spflg = atoi(&Buff[j]);
					//gi.dprintf("%i %s\n",Bot[i].spflg,&Buff[j]);
					if(Bot[i].spflg == BOT_SPRESERVED && 
						autospawn->value && !chedit->value)
						SpawnWaitingBots++; 
					else 
						Bot[i].spflg = BOT_SPAWNNOT;
				}
				else 
					break;
				ListedBots++;
			}
			else
			{
				gi.dprintf("Error loading bot configuration!\n");
				gi.dprintf("Error line is: %s", Buff);
				gi.dprintf("Check %s\n", filename);
			}
		}
	}
BOTLIST_NOTFOUND:
	fclose(fp);

	gi.dprintf("%i bots parsed\n", ListedBots);
	spawncycle = level.time + FRAMETIME * 100;
}

//----------------------------------------------------------------
//Get Number of Client
//
// Total Client
//
//----------------------------------------------------------------

int Get_NumOfPlayer (void) //Bots plus players
{
	int i;
	edict_t *ent;

	int count = 0;
	for (i = 0; i < maxclients->value; i++)
	{
		ent = g_edicts + 1 + i;
		if (ent->inuse)
			count++;
	}
	return count;
}

//----------------------------------------------------------------
//Get New Client
//
// Get new client edict
//
//----------------------------------------------------------------

edict_t *Get_NewClient (void)
{
	int			i;
	edict_t		*e;
	gclient_t	*client;

	e = &g_edicts[(int)maxclients->value];
	for ( i = maxclients->value ; i >= 1  ; i--, e--)
	{
		client = &game.clients[i - 1];
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!e->inuse && !client->pers.connected && 
			( e->freetime < 2 || level.time - e->freetime > 0.5 ) )
		{
			G_InitEdict (e);
			return e;
		}
	}

	gi.dprintf ("ED_Alloc: no free edicts for new bot!!");
	return NULL;
}

//----------------------------------------------------------------
//Bot Think
//
// Bot's think code
//
//----------------------------------------------------------------
void Bot_Think (edict_t *self)
{
	edict_t	*other;
	int		i;

	if(!self ||!self->client)
		return;
	//Dont move if the level is over !!!!
	if(match_state != STATE_PLAYING)
		return;	

	// Check if bot stuck in wall..
	if (!((int)level.time % 10))
		if (InsideWall(self))
		{
			DbgPrintf("7777 %s spawned inside wall: %f %f %f\n", 
				self->client->pers.netname,
				self->s.origin[0], self->s.origin[1], self->s.origin[2]); 
			Cmd_Kill_f(self); // suicide
			self->nextthink = level.time + FRAMETIME;
			return;
		}

		if (self->linkcount != self->monsterinfo.linkcount)
		{
			M_CheckGround (self);
		}

		if(self->deadflag)
		{
			if(self->client->ctf_grapple)
				CTFPlayerResetGrapple(self);


			if(self->s.modelindex == skullindex || self->s.modelindex == headindex) self->s.frame = 0;
			else if(self->s.frame < FRAME_crdeath1 && self->s.frame != 0) self->s.frame = FRAME_death308;
			self->s.modelindex2 = 0;	// remove linked weapon model
			//ZOID
			self->s.modelindex3 = 0;	// remove linked ctf flag
			//ZOID

			self->client->zc.route_trace = false;
			if(self->client->respawn_time <= level.time)
			{
				if(self->svflags & SVF_MONSTER)
				{
					self->client->respawn_time = level.time;
					CopyToBodyQue (self);
					PutBotInServer(self);
				}
			}
		}
		else
		{
			Bots_Move_NORM (self);
			if(!self->inuse) //QW// This never traps. Is it needed?
				return;			//removed botself

			ClientBeginServerFrame (self);
		}
		if (self->linkcount != self->monsterinfo.linkcount)
		{
			M_CheckGround (self);
		}

		for (i = 1; i <= maxclients->value; i++)
		{
			other = g_edicts + i;
			if (other->inuse && other->client->chase_target == self)
				UpdateChaseCam(other);
		}

		if(self->client->resp.shots != 0)
		{
			self->client->resp.eff = 100 * self->client->resp.frags / self->client->resp.shots;
		}

		M_CategorizePosition (self);
		BotEndServerFrame (self);
		self->nextthink = level.time + FRAMETIME;
		return;
}

//----------------------------------------------------------------
//Initialize Bot
//
// Initialize bot edict
//
//----------------------------------------------------------------

void InitializeBot (edict_t *ent, int botindex)
{
	gclient_t	*client;
	char		pinfo[200];
	int			index;

	index = ent - g_edicts - 1;
	ent->client = &game.clients[index];

	client = ent->client;

	memset (&client->zc, 0, sizeof(zgcl_t));
	memset (&client->pers, 0, sizeof(client->pers));
	memset (&client->resp, 0, sizeof(client->resp));

	client->zc.botindex = botindex;
	client->resp.enterframe = level.framenum;

	sprintf(pinfo,"\\rate\\25000\\msg\\1\\fov\\90\\skin\\%s/%s\\name\\%s\\hand\\0",
		Bot[botindex].model, Bot[botindex].skin, Bot[botindex].netname);

	if(ctf->value)
		BotAssignTeamCtf(ent->client);

	ClientUserinfoChanged (ent, pinfo);

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	if(!voosh->value){
		client->pers.max_bullets	= 200;
		client->pers.max_shells		= 100;
		client->pers.max_rockets	= 50;
		client->pers.max_grenades	= 50;
		client->pers.max_cells		= 200;
		client->pers.max_slugs		= 50;
	}

	ent->client->pers.connected = false;
	gi.dprintf ("%s connected\n", ent->client->pers.netname);

	if(ctf->value)
		safe_bprintf(PRINT_HIGH, "%s joined the %s team.\n",
			client->pers.netname, CTFTeamName(ent->client->resp.ctf_team));
	else 	
		safe_bprintf (PRINT_HIGH, "%s entered the game\n", 
			client->pers.netname);
}

void PutBotInServer (edict_t *ent)
{
	edict_t		*touch[MAX_EDICTS];
	int			i,j,entcount;
	gitem_t		*item;
	gclient_t	*client;
	vec3_t	spawn_origin;
	vec3_t	spawn_angles;
	trace_t		rs_trace;
	gitem_t	    *ammo;
	zgcl_t		*zc;

	zc = &ent->client->zc;

	//current weapon
	client = ent->client;
	if(debug_botspawn->value)
		DbgPrintf("%s %s\n", __FUNCTION__, client->pers.netname);

	//RAV
	//  start weapons & respawn protection
	//	if ((resp_protect->value > 0) && (match_state > STATE_COUNTDOWN)
	//		&& (ent->client->pers.pl_state == PL_PLAYING))
	//	{
	//	client->respawn_framenum = level.framenum + 20;
	//	}
	if ((int)(start_weapons->value) & 1) 
	{
		item = FindItem("Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 2) 
	{
		item = FindItem("Super Shotgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 4) 
	{
		item = FindItem("Machinegun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 8) 
	{
		item = FindItem("Chaingun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 16) 
	{
		item = FindItem("Grenade Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 32) 
	{
		item = FindItem("Rocket Launcher");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 64) 
	{
		item = FindItem("Hyperblaster");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 128) 
	{
		item = FindItem("Railgun");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	if ((int)(start_weapons->value) & 256) 
	{
		item = FindItem("BFG10K");
		client->pers.inventory[ITEM_INDEX(item)] = 1;
		ammo = FindItem (item->ammo);
		//JSW		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		if ( dmflag & DF_INFINITE_AMMO )
			Add_Ammo (ent, ammo, 1000);
		else
			Add_Ammo (ent, ammo, ammo->quantity);
		client->pers.weapon = item;
	}
	// Start Items
	if ((int)(start_items->value) & 1)
	{
		edict_t *it_ent;

		item = FindItem("Body Armor");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 2)
	{
		edict_t *it_ent;

		item = FindItem("Combat Armor");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 4)
	{
		edict_t *it_ent;

		item = FindItem("Jacket Armor");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 8)
	{
		edict_t *it_ent;

		item = FindItem("Power Screen");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 16)
	{
		edict_t *it_ent;

		item = FindItem("Power Shield");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 32)
	{
		edict_t *it_ent;

		item = FindItem("Quad Damage");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 64)
	{
		edict_t *it_ent;

		item = FindItem("Invulnerability");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 128)
	{
		edict_t *it_ent;

		item = FindItem("Silencer");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 256)
	{
		edict_t *it_ent;

		item = FindItem("Rebreather");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 512)
	{
		edict_t *it_ent;

		item = FindItem("Environment Suit");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 256)
	{
		edict_t *it_ent;

		item = FindItem("Rebreather");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 512)
	{
		edict_t *it_ent;

		item = FindItem("Environment Suit");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 1024)
	{
		edict_t *it_ent;

		item = FindItem("Bandolier");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 2048)
	{
		edict_t *it_ent;

		item = FindItem("Ammo Pack");
		it_ent = G_Spawn();
		it_ent->classname = item->classname;
		SpawnItem2 (it_ent, item);
		Touch_Item (it_ent, ent, NULL, NULL);
		if (it_ent->inuse)
			G_FreeEdict(it_ent);
	}
	if ((int)(start_items->value) & 4096)
	{
		edict_t *it_ent;

		item = FindItem("Health");
		it_ent = G_Spawn();
		SP_item_health_mega (it_ent);
		it_ent->classname = item->classname;
		Touch_Item (it_ent, ent, NULL, NULL);
	}
	//RAILWARZ
	if (voosh->value)
	{
		item = FindItem("Slugs");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1000;

		item = FindItem("Railgun");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
		client->pers.weapon = item;
	}
	else{
		item = FindItem("Blaster");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = 1;
		client->pers.weapon = item;
	}
	/********************************************
	start  ammo
	********************************************/

	if (sa_shells->value)
	{
		item = FindItem("Shells");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = sa_shells->value;

	}


	if (sa_bullets->value)
	{
		item = FindItem("Bullets");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = sa_bullets->value;

	}

	if (sa_grenades->value)
	{
		item = FindItem("Grenades");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = sa_grenades->value;

	}

	if (sa_rockets->value)
	{
		item = FindItem("Rockets");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = sa_rockets->value;

	}

	if (sa_cells->value)
	{
		item = FindItem("Cells");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = sa_cells->value;
	}

	if (sa_slugs->value)
	{
		item = FindItem("Slugs");
		client->pers.selected_item = ITEM_INDEX(item);
		client->pers.inventory[client->pers.selected_item] = sa_slugs->value;

	}

	// End

	client->silencer_shots = 0;
	client->weaponstate = WEAPON_READY;
	client->newweapon = NULL;

	//clear powerups
	client->quad_framenum = 0;
	client->invincible_framenum = 0;
	client->breather_framenum = 0;
	client->enviro_framenum = 0;
	client->grenade_blew_up = false;
	client->grenade_time = 0;

	j = zc->botindex;
	i = zc->routeindex;
	memset (&client->zc,0,sizeof(zgcl_t));
	zc->botindex = j;
	zc->routeindex = i;

	//ZOID
	client->ctf_grapple = NULL;

	item = FindItem("Grapple");
	if(ctf->value || use_hook->value)
		client->pers.inventory[ITEM_INDEX(item)] = 1; //ponpoko
	//ZOID

	// clear entity values
	ent->classname = "player";
	ent->movetype = MOVETYPE_STEP;
	ent->solid = SOLID_BBOX; 
	ent->model = "players/male/tris.md2";
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 32);

	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->gib_health = -40;

	ent->gravity = 1.0;
	ent->mass = 200;
	ent->target_ent = NULL;
	ent->s.frame = 0;

	// clear entity state values
	ent->s.modelindex = 255;		// will use the skin specified model
	ent->s.skinnum = ent - g_edicts - 1;
	ShowGun(ent);					// ### Hentai ### special gun model

	ent->s.sound = 0;

	ent->monsterinfo.scale = MODEL_SCALE;

	ent->pain = player_pain;
	ent->die = player_die;
	ent->touch = NULL;

	ent->moveinfo.decel = level.time;
	ent->pain_debounce_time = level.time;
	ent->targetname = NULL;

	ent->moveinfo.speed = 1.0;
	ent->moveinfo.state = GETTER;

	ent->prethink = NULL;
	ent->think = Bot_Think;
	ent->nextthink = level.time + FRAMETIME * 5;	//QW// hold for 1/2 second
	ent->svflags /*|*/= SVF_MONSTER ;
	ent->s.renderfx = 0;
	ent->s.effects = 0;

	ent->s.event = EV_OTHER_TELEPORT;	//prevent lerping
	SelectSpawnPoint (ent, spawn_origin, spawn_angles);
	VectorCopy (spawn_origin, ent->s.origin);
	VectorCopy (spawn_angles, ent->s.angles);
	spawn_origin[2] += 1;
	rs_trace = gi.trace(ent->s.origin,ent->mins,ent->maxs,spawn_origin,ent,MASK_SOLID);
	if(!rs_trace.allsolid) 
		VectorCopy (rs_trace.endpos, ent->s.origin);
	VectorClear(ent->velocity);
	ent->moveinfo.speed = 0;
	ent->groundentity = rs_trace.ent;
	ent->client->ps.pmove.pm_flags &= ~PMF_DUCKED;

	Set_BotAnim(ent,ANIM_BASIC,FRAME_run1,FRAME_run6);
	client->anim_run = true;

	ent->client->ctf_grapple = NULL;
	ent->client->quad_framenum = level.framenum;
	ent->client->invincible_framenum = level.framenum;
	ent->client->enviro_framenum = level.framenum;
	ent->client->breather_framenum = level.framenum;
	ent->client->weaponstate = WEAPON_READY;
	ent->takedamage = DAMAGE_YES;;
	ent->air_finished = level.time + 12;
	ent->clipmask = MASK_PLAYERSOLID;//MASK_MONSTERSOLID;
	ent->flags &= ~FL_NO_KNOCKBACK;

	ent->client->anim_priority = ANIM_BASIC;
	//	ent->client->anim_run = true;
	ent->s.frame = FRAME_run1-1;
	ent->client->anim_end = FRAME_run6;
	ent->deadflag = DEAD_NO;
	ent->svflags &= ~SVF_DEADMONSTER;


	ent->client->pers.in_game = true;

	zc->waitin_obj = NULL;
	zc->first_target = NULL;
	zc->first_target = NULL;
	zc->zcstate = STS_IDLE;

	/*	if(ent->client->resp.enterframe == level.framenum && !chedit->value)
	{
	/	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_LOGIN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	}
	else if(!chedit->value)
	{
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RESPAWN);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	}

	*/
	ent->client->pers.pl_state = PL_PLAYING;
	ent->bot_client = true;
	gi.linkentity (ent);
	VectorAdd (spawn_origin, ent->mins, ent->absmin);
	VectorAdd (spawn_origin, ent->maxs, ent->absmax);
	entcount = gi.BoxEdicts (ent->absmin, ent->absmax, 
		touch, MAX_EDICTS, AREA_SOLID);
	while (entcount-- > 0)
	{
		if(Q_stricmp (touch[entcount]->classname, "player") == 0)
			if(touch[entcount] != ent)
				T_Damage (touch[entcount], ent, ent, 
				vec3_origin, touch[entcount]->s.origin, 
				vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
	}

	if(ctf->value)
	{
		if(ent->client->hook)
			abandon_hook_reset(ent->client->hook);
		CTFPlayerResetGrapple(ent);
		client->zc.ctfstate = CTFS_OFFENCER;
	}

	//set up next camp check
	ent->client->nextcamp = level.time + (int)bot_camptime->value;

	gi.linkentity (ent);
	G_TouchTriggers (ent);
	BotEndServerFrame(ent);
}

//----------------------------------------------------------------
//Spawn Bot
//
// spawn bots
//
//	int i	index of bot list
//
//----------------------------------------------------------------

qboolean SpawnBot(int i)
{
	edict_t		*bot,*ent;
	int			k,j;

	if(	Get_NumOfPlayer () >= game.maxclients )
	{
		gi.cprintf (NULL,PRINT_HIGH,"Can't add bots\n");
		return false;
	}

	bot = Get_NewClient();
	if(bot == NULL) return false;

	InitializeBot (bot, i);
	PutBotInServer ( bot );

	j = targetindex;
	if(chedit->value)
	{
		for(k = CurrentIndex - 1;k > 0 ;k--)
		{
			if(Route[k].index == 0) break;

			if(Route[k].state == GRS_NORMAL)
			{
				if(--j <= 0) break;
			}
		}

		bot->client->zc.rt_locktime = level.time + FRAMETIME * 20;
		bot->client->zc.route_trace = true;
		bot->client->zc.routeindex = k;
		VectorCopy(Route[k].Pt,bot->s.origin);
		VectorAdd (bot->s.origin, bot->mins, bot->absmin);
		VectorAdd (bot->s.origin, bot->maxs, bot->absmax);
		bot->client->ps.pmove.pm_flags |= PMF_DUCKED;
		gi.linkentity (bot);
		//		bot->s.modelindex = 0;
		bot->bot_client = true;
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (bot-g_edicts);
		gi.WriteByte (MZ_LOGIN);
		gi.multicast (bot->s.origin, MULTICAST_PVS);

		ent = &g_edicts[1];
		if(ent->inuse && ent->client && !(ent->bot_client))
		{
			ent->takedamage = DAMAGE_NO;
			ent->movetype = MOVETYPE_NOCLIP;
			ent->target_ent = bot;
			ent->solid = SOLID_NOT;
			ent->client->ps.pmove.pm_type = PM_FREEZE;
			ent->client->ps.pmove.pm_flags |= PMF_NO_PREDICTION ;
			VectorCopy(ent->s.origin,ent->moveinfo.start_origin);
		}
	}

	return true;
}

//----------------------------------------------------------------
// Count the number of active bots and spawn
// new ones up to the number of waiting bots.
//----------------------------------------------------------------
void Bot_SpawnCall(void)
{
	int i = GetNumBots();

	if(SpawnBot(i))
		Bot[i].spflg = BOT_SPAWNED;
	else
	{
		Bot[i].spflg = BOT_SPAWNNOT;
		targetindex = 0;
	}
	SpawnWaitingBots--;
}

//----------------------------------------------------------------
//Spawn Bot Reserving
//
// spawn bots reserving
//
//----------------------------------------------------------------
void SpawnBotReserving(void)
{
	int	i;

	for(i = 0;i < MAXBOTS; i++)
	{
		if(Bot[i].spflg == BOT_SPAWNNOT)
		{
			Bot[i].spflg = BOT_SPRESERVED;
			SpawnWaitingBots++;
			return;
		}
	}
	gi.cprintf (NULL, PRINT_HIGH, 
		"Maximum bots (%i) spawned: Unable to spawn bot\n", MAXBOTS);
}
//----------------------------------------------------------------
//Spawn Bot Reserving 2
//
// randomized spawn bots reserving
//
//----------------------------------------------------------------
void SpawnBotReserving2(int *red,int *blue)
{
	int	i,j;

	j = (int)(random() * ListedBots);

	for(i = 0;i < ListedBots; i++,j++)
	{
		if(j >= ListedBots) j = 0;
		if(Bot[j].spflg == BOT_SPAWNNOT)
		{
			Bot[j].spflg = BOT_SPRESERVED;
			SpawnWaitingBots++;
			if(*red > *blue) Bot[j].team = 2;
			else Bot[j].team = 1;

			if(Bot[j].team == 1) *red = *red + 1;
			else if(Bot[j].team == 2) *blue = *blue + 1;
			//gi.cprintf(NULL,PRINT_HIGH,"team %i\n",Bot[j].team);
			return;
		}
	}
	SpawnBotReserving();
}

//----------------------------------------------------------------
//Remove Bot
//
// remove bots
//
//	int i	index of bot list
//
//----------------------------------------------------------------
void RemoveBot(void)
{
	int			i;
	int			botindex;
	edict_t		*e,*ent;
	gclient_t	*client;

	for(i = MAXBOTS - 1;i >= 0;i--)
	{
		if(Bot[i].spflg == BOT_SPAWNED || Bot[i].spflg == BOT_NEXTLEVEL)
		{
			break;
		}
	}

	if(i < 0)
	{
		gi.cprintf (NULL, PRINT_HIGH, "No bots found");
		return;
	}

	botindex = i;

	e = &g_edicts[(int)maxclients->value];
	for ( i = maxclients->value ; i >= 1  ; i--, e--)
	{
		if(!e->inuse) 
			continue;
		client = /*e->client;*/&game.clients[i - 1];
		if(client == NULL) 
			continue;
		// the first couple seconds of server time can involve a lot of
		// freeing and allocating, so relax the replacement policy
		if (!client->pers.connected && (e->svflags & SVF_MONSTER))
		{
			if(client->zc.botindex == botindex)
			{
				if(Bot[botindex].spflg != BOT_NEXTLEVEL) 
					Bot[botindex].spflg = BOT_SPAWNNOT;
				else 
					Bot[botindex].spflg = BOT_SPRESERVED;

				if (!level.intermissiontime)
					gi.bprintf (PRINT_HIGH, "%s disconnected\n", 
								e->client->pers.netname);

				// send effect
				gi.WriteByte (svc_muzzleflash);
				gi.WriteShort (e-g_edicts);
				gi.WriteByte (MZ_LOGOUT);
				gi.multicast (e->s.origin, MULTICAST_PVS);

				e->s.modelindex = 0;
				e->solid = SOLID_NOT;

				if (ctf->value) 
				{
					CTFPlayerResetGrapple(e);
					//CTFDeadDropFlag(e);
					//CTFDeadDropTech(e);
				}

				gi.linkentity (e);

				e->inuse = false;
				G_FreeEdict (e);

				if(targetindex)
				{
					ent = &g_edicts[1];

					if(ent->inuse)
					{
						ent->health = 100;
						ent->movetype = MOVETYPE_STEP;
						ent->takedamage = DAMAGE_AIM;
						ent->target_ent = NULL;
						ent->solid = SOLID_BBOX;
						ent->client->ps.pmove.pm_type = PM_NORMAL;
						ent->client->ps.pmove.pm_flags = PMF_DUCKED;
						VectorCopy(ent->moveinfo.start_origin,ent->s.origin);
						VectorCopy(ent->moveinfo.start_origin,ent->s.old_origin);
					}
					targetindex = 0;
				}
				return;
			}
		}
	}
	gi.error ("Can't remove bot.");
}

//----------------------------------------------------------------
//Level Change Removing
//
//
//
//----------------------------------------------------------------
void Bot_LevelChange(void)
{
	int i,j,k;

	j = 0;
	k = 0;
	for(i = 0;i < MAXBOTS;i++)
	{
		if(Bot[i].spflg)
		{
			if(Bot[i].spflg == BOT_SPAWNED)
			{
				k++;
				Bot[i].spflg = BOT_NEXTLEVEL;
			}
			j++;
		}
	}
	for(i = 0; i < k; i++)
	{
		RemoveBot();
	}
	SpawnWaitingBots = k;
}


//===============================

//	AirStrike

//===============================
static void AirSight_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	//	if (ent->owner->client && !(ent->owner->svflags & SVF_DEADMONSTER))
	//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	gi.sound (ent, CHAN_AUTO, gi.soundindex("3zb/airexp.wav"), 1, ATTN_NONE, 0);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	mod = MOD_AIRSTRIKE;

	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void AirSight_Think(edict_t *ent)
{
	//	gi.sound (ent, CHAN_AUTO, gi.soundindex("medic/medatck1.wav"), 1, ATTN_NORM, 0);
	gi.sound (ent, CHAN_BODY, gi.soundindex("3zb/airloc.wav"), 1, ATTN_NONE, 0);

	ent->dmg = 120 + random() * 60;
	ent->dmg_radius = 200;

	ent->s.modelindex = gi.modelindex ("sprites/airsight.sp2");
	VectorCopy(ent->target_ent->s.origin,ent->s.origin);

	if( ent->owner->client->resp.ctf_team == CTF_TEAM2 && ctf->value)
	{
		ent->s.frame = 1;
	}
	else ent->s.frame = 0;

	ent->think = AirSight_Explode;
	ent->nextthink = level.time + FRAMETIME * 6;
	gi.linkentity (ent);
}

void AirStrike_Think(edict_t *ent)
{
	int	i,j;
	edict_t	*target,*sight;
	trace_t	rs_trace;

	vec3_t	point;

	ent->nextthink = level.time + ent->moveinfo.speed * 0.5 /300;
	ent->think = G_FreeEdict;
	//	ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");

	VectorCopy(ent->s.origin,point);
	point[2] = ent->moveinfo.start_angles[2];

	j = 1;
	for ( i = 1 ; i <= maxclients->value; i++)
	{
		target =  &g_edicts[i];
		if(!target->inuse || target->deadflag || target == ent->owner) 
			continue;

		if( target->classname[0] == 'p')
		{
			if(!ctf->value || (ctf->value && ent->owner->client->resp.ctf_team != target->client->resp.ctf_team))
			{
				rs_trace = gi.trace (point, NULL, NULL,
					target->s.origin, ent, 
					CONTENTS_SOLID | CONTENTS_WINDOW |
					CONTENTS_LAVA | CONTENTS_SLIME);

				if(rs_trace.fraction == 1.0)
				{
					sight = G_Spawn();

					sight->classname = "airstrike";
					sight->think = AirSight_Think;
					sight->nextthink = level.time + FRAMETIME * 2 * (float)j;
					sight->movetype = MOVETYPE_NOCLIP;
					sight->solid = SOLID_NOT;
					sight->owner = ent->owner;
					sight->target_ent = target;
					gi.linkentity (sight);
					j++;
				}
			}
		}
	}

}

void Cmd_AirStrike(edict_t *ent)
{
	edict_t	*viper;
	trace_t	rs_trace;

	vec3_t	strpoint,tts,tte,tmp;

	vec_t	f;

	VectorCopy(ent->s.origin,strpoint);
	strpoint[2] += 8190;

	rs_trace = gi.trace (ent->s.origin,NULL,NULL,strpoint,ent, MASK_SHOT);

	if(!(rs_trace.surface && (rs_trace.surface->flags & SURF_SKY)))
	{
		gi.cprintf(ent,PRINT_HIGH,"can't call Viper.\n");
		return;
	}
	/*	if((rs_trace.endpos[2] - ent->s.origin[2]) < 300)
	{
	gi.cprintf(ent,PRINT_HIGH,"can't call Viper.\n");
	}*/

	VectorCopy(rs_trace.endpos,strpoint);
	strpoint[2] -= 16;	//I shifted down a little bit


	f = ent->s.angles[YAW]*M_PI*2 / 360;
	tts[0] = cos(f) * (-8190) ;
	tts[1] = sin(f) * (-8190) ;
	tts[2] = 0;

	tte[0] = cos(f) *8190 ;
	tte[1] = sin(f) *8190 ;
	tte[2] = 0;

	viper = G_Spawn();
	VectorClear (viper->mins);
	VectorClear (viper->maxs);
	viper->movetype = /*MOVETYPE_FLYMISSILE;//MOVETYPE_STEP;*/MOVETYPE_NOCLIP;
	viper->solid = SOLID_NOT;
	viper->owner = ent;
	viper->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");

	VectorCopy(ent->s.angles,viper->s.angles);
	viper->s.angles[2] = 0;
	rs_trace = gi.trace (strpoint,NULL,NULL,tts,ent,  MASK_SHOT);
	tts[0] = cos(f) * (-600) ;
	tts[1] = sin(f) * (-600) ;
	VectorAdd(rs_trace.endpos,tts,tmp);
	VectorCopy(tmp,viper->s.origin);


	viper->velocity[0] = cos(f) * 300;
	viper->velocity[1] = sin(f) * 300;
	viper->velocity[2] = 0;

	rs_trace = gi.trace (strpoint,NULL,NULL,tte,ent,  MASK_SHOT);
	VectorSubtract(viper->s.origin,rs_trace.endpos,tts);
	f = VectorLength(tts);

	gi.sound (viper, CHAN_AUTO, gi.soundindex("world/flyby1.wav"), 1, ATTN_NONE, 0);

	gi.sound (ent, CHAN_AUTO, gi.soundindex("world/incoming.wav"), 1, ATTN_NONE, 0);

	viper->nextthink = level.time + f *0.75 /300;
	viper->think = AirStrike_Think;
	viper->moveinfo.speed = f;

	//	viper->s.sound = gi.soundindex ("weapons/rockfly.wav");

	//	viper->s.effects |= EF_ROTATE | EF_COLOR_SHELL;
	//	viper->s.renderfx |= RF_SHELL_BLUE | RF_SHELL_GREEN;
	VectorCopy(strpoint,viper->moveinfo.start_angles);	//strikepoint

	//	viper->think = Pod_think;
	//	viper->nextthink = level.time + FRAMETIME;
	viper->classname = "viper";
	viper->s.origin[2] += 16;
	gi.linkentity (viper);
}

