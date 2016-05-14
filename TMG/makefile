#
# Quake2 gamei386.so Makefile for Linux 2.0
#
# Jan '98 by Zoid <zoid@idsoftware.com>
#
# ELF only
#
# Edited May 5, 2016 by QwazyWabbit
#
# Requires GNU make
#

ARCH=i386

# on x64 machines do this preparation:
# sudo apt-get install ia32-libs
# sudo apt-get install libc6-dev-i386
# this will let you build 32-bits on ia64 systems
#

#use these cflags to optimize this build
CFLAGS=-O3 -m32 -march=$(ARCH) -DARCH="$(ARCH)"
#use these when debugging 
#CFLAGS=-g -m32 -DARCH="$(ARCH)" -Wall

# flavors of Linux
ifeq ($(shell uname),Linux)
#SVNDEV := -D'SVN_REV="$(shell svnversion -n .)"'
#CFLAGS += $(SVNDEV)
CFLAGS += -DLINUX
LIBTOOL = ldd
endif

# OS X wants to be Linux and FreeBSD too.
ifeq ($(shell uname),Darwin)
#SVNDEV := -D'SVN_REV="$(shell svnversion -n .)"'
#CFLAGS += $(SVNDEV)
CFLAGS += -DLINUX
LIBTOOL = otool
endif

SHLIBEXT=so
#set position independent code
SHLIBCFLAGS=-fPIC

ORIGDIR=Source

DO_SHLIB_CC=$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

.c.o:
	$(DO_SHLIB_CC)

#############################################################################
# SETUP AND BUILD
# GAME
#############################################################################

GAME_OBJS = \
	anticheat.o bot.o bot_fire.o bot_func.o bot_za.o camper.o e_hook.o eavy.o filehand.o \
	filtering.o g_chase.o g_cmds.o g_combat.o g_ctf.o g_func.o g_items.o g_main.o g_misc.o \
	g_monster.o g_phys.o g_save.o g_spawn.o g_svcmds.o g_target.o g_trigger.o g_utils.o \
	g_weapon.o gslog.o highscore.o hud.o intro.o locdamage.o m_move.o s_map.o models.o \
	p_client.o p_hud.o p_light.o p_menu.o p_trail.o p_view.o p_weapon.o q_shared.o \
	runes.o stdlog.o timer.o vote.o

game$(ARCH).$(SHLIBEXT) : $(GAME_OBJS)
	$(CC) $(CFLAGS) -shared -o $@ $(GAME_OBJS) -ldl -lm
	$(LIBTOOL) -r $@


#############################################################################
# MISC
#############################################################################

clean:
	/bin/rm -f $(GAME_OBJS) game$(ARCH).$(SHLIBEXT)

depend:
	gcc -MM $(GAME_OBJS:.o=.c)

depends:
	$(CC) $(CFLAGS) -MM *.c > dependencies

all:
	make clean
	make depends
	make

-include dependencies

