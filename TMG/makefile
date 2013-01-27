#
# Quake2 gamei386.so Makefile for Linux 2.0
#
# Jan '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#
# Probably requires GNU make
#

ARCH=i386
CC=gcc
OBJDIR=obj
BASE_CFLAGS=-Dstricmp=strcasecmp -D_strnicmp=strncasecm -Dstrnicmp=strncasecmp -Dstrcmpi=strcasecmp

#use these cflags to optimize it
CFLAGS=$(BASE_CFLAGS) -march=i486 -O6 -ffast-math -funroll-loops \
	-fomit-frame-pointer -fexpensive-optimizations -falign-loops=2 \
	-falign-jumps=2 -falign-functions=2
#use these when debugging 
#CFLAGS=$(BASE_CFLAGS) -g

LDFLAGS=-ldl -lm
SHLIBEXT=so
SHLIBCFLAGS=-fPIC
SHLIBLDFLAGS=-shared

DO_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

.c.o:
	$(DO_CC)

GAME_OBJS = \
	anticheat.o bot.o bot_fire.o bot_func.o bot_za.o camper.o e_hook.o eavy.o filehand.o \
	filtering.o g_chase.o g_cmds.o g_combat.o g_ctf.o g_func.o g_items.o g_main.o g_misc.o \
	g_monster.o g_phys.o g_save.o g_spawn.o g_svcmds.o g_target.o g_trigger.o g_utils.o \
	g_weapon.o gslog.o highscore.o hud.o intro.o locdamage.o m_move.o s_map.o models.o \
	p_client.o p_hud.o p_light.o p_menu.o p_trail.o p_view.o p_weapon.o q_shared.o \
	runes.o stdlog.o timer.o vote.o

game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(GAME_OBJS)


#############################################################################
# MISC
#############################################################################

clean:
	-rm -f *.o

depend:
	gcc -MM $(GAME_OBJS:.o=.c)

