#
# Makefile for rogue
#


HDRS=	rogue.h mach_dep.h curses.h 
OBJS=			\
	vers.o		\
	armor.o		\
	chase.o		\
	command.o	\
	daemon.o	\
	daemons.o	\
	fight.o		\
	encumb.o	\
	init.o		\
	io.o		\
	list.o		\
	main.o		\
	maze.o		\
	misc.o		\
	monsters.o	\
	move.o		\
	new_level.o	\
	options.o	\
	pack.o		\
	passages.o	\
	player.o	\
	potions.o	\
	rings.o		\
	rip.o		\
	rooms.o		\
	save.o		\
	scrolls.o	\
	sticks.o	\
	things.o	\
	trader.o	\
	weapons.o	\
	wizard.o	\
	rogue.o		\
	Tolower.o

CFILES=	vers.c		\
	armor.c		\
	chase.c		\
	command.c	\
	daemon.c	\
	daemons.c	\
	fight.c		\
	encumb.c	\
	init.c		\
	io.c		\
	list.c		\
	main.c		\
	maze.c		\
	misc.c		\
	monsters.c	\
	move.c		\
	new_level.c	\
	options.c	\
	pack.c		\
	passages.c	\
	player.c	\
	potions.c	\
	rings.c		\
	rip.c		\
	rooms.c		\
	save.c		\
	scrolls.c	\
	sticks.c	\
	things.c	\
	trader.c	\
	weapons.c	\
	wizard.c	\
	rogue.c		\
	Tolower.c


#
#	The following define is used for Berkeley 4bsd compilations.
#
DBSD= -DBSD -DDUMP

CFLAGS= -O $(DBSD)
LDFLAGS= 	# For VAXes
TERMLIB= -ltermlib
CRLIB= -lcurses

a.out: $(HDRS) $(OBJS) monst_show.o
	$(CC) $(LDFLAGS) $(OBJS) monst_show.o $(CRLIB) $(TERMLIB)
	size a.out

rogue: a.out
	mv a.out rogue

armor.o:	curses.h
armor.o:	rogue.h
chase.o:	curses.h
chase.o:	rogue.h
command.o:	curses.h
command.o:	rogue.h
daemon.o:	curses.h
daemon.o:	rogue.h
daemons.o:	curses.h
daemons.o:	rogue.h
edit.o:		mach_dep.h
edit.o:		curses.h
edit.o:		rogue.h
encumb.o:	curses.h
encumb.o:	rogue.h
fight.o:	curses.h
fight.o:	rogue.h
init.o:		curses.h
init.o:		rogue.h
io.o:		curses.h
io.o:		rogue.h
list.o:		curses.h
list.o:		rogue.h
main.o:		curses.h
main.o:		mach_dep.h
main.o:		rogue.h
maze.o:		curses.h
maze.o:		rogue.h
misc.o:		curses.h
misc.o:		rogue.h
monsters.o:	curses.h
monsters.o:	rogue.h
move.o:		curses.h
move.o:		rogue.h
new_level.o:	curses.h
new_level.o:	rogue.h
options.o:	curses.h
options.o:	rogue.h
pack.o:		curses.h
pack.o:		rogue.h
passages.o:	curses.h
passages.o:	rogue.h
player.o:	curses.h
player.o:	rogue.h
potions.o:	curses.h
potions.o:	rogue.h
rings.o:	curses.h
rings.o:	rogue.h
rip.o:		curses.h
rip.o:		mach_dep.h
rip.o:		rogue.h
rogue.o:	curses.h
rogue.o:	rogue.h
rooms.o:	curses.h
rooms.o:	rogue.h
save.o:		curses.h
save.o:		rogue.h
scrolls.o:	curses.h
scrolls.o:	rogue.h
sticks.o:	curses.h
sticks.o:	rogue.h
things.o:	curses.h
things.o:	rogue.h
trader.o:	curses.h
trader.o:	rogue.h
weapons.o:	curses.h
weapons.o:	rogue.h
wizard.o:	curses.h
wizard.o:	rogue.h

install: rogue
	cp rogue /usr/local/newrogue
	chall cron daemon /usr/local/newrogue
	chmod 4711 /usr/local/newrogue

lint:
	lint -hxbc $(CFILES) -lcurses > linterrs

clean:
shrink:
	rm -f $(OBJS) core rogue make.out ? rogue.tar nohup.out

count:
	wc -l $(HDRS) $(CFILES)

realcount:
	cc -E $(CFILES) | ssp - | wc -l

edit:  edit.o  
	$(CC) $(LDFLAGS) edit.o -o edit
