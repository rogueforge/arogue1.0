/*
 * Contains functions for dealing with things that happen in the
 * future.
 *
 * @(#)daemon.c	8.0	(Bell Labs)	12/16/82
 */

#include "curses.h"
#include "rogue.h"

#define EMPTY 0
#define DAEMON -1
#define MAXDAEMONS 30

#define _X_ { EMPTY }

struct delayed_action {
	int d_type;
	int (*d_func)();
	int d_arg;
	int d_time;
} d_list[MAXDAEMONS] = {
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
	_X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
};
static int demoncnt = 0;	/* number of active daemons */


/*
 * d_slot:
 *	Find an empty slot in the daemon/fuse list
 */
struct delayed_action *
d_slot()
{
	reg int i;
	reg struct delayed_action *dev;

	for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
		if (dev->d_type == EMPTY)
			return dev;
	return NULL;
}


/*
 * find_slot:
 *	Find a particular slot in the table
 */
struct delayed_action *
find_slot(func)
reg int (*func)();
{
	reg int i;
	reg struct delayed_action *dev;

	for (i = 0, dev = d_list; i < MAXDAEMONS; i++, dev++)
		if (dev->d_type != EMPTY && func == dev->d_func)
			return dev;
	return NULL;
}


/*
 * daemon:
 *	Start a daemon, takes a function.
 */
daemon(func, arg, type)
reg int arg, type, (*func)();
{
	reg struct delayed_action *dev;

	dev = d_slot();
	if (dev != NULL) {
		dev->d_type = type;
		dev->d_func = func;
		dev->d_arg = arg;
		dev->d_time = DAEMON;
		demoncnt += 1;			/* update count */
	}
}


/*
 * kill_daemon:
 *	Remove a daemon from the list
 */
kill_daemon(func)
reg int (*func)();
{
	reg struct delayed_action *dev;

	if ((dev = find_slot(func)) == NULL)
		return;
	/*
	 * Take it out of the list
	 */
	dev->d_type = EMPTY;
	demoncnt -= 1;			/* update count */
}


/*
 * do_daemons:
 *	Run all the daemons that are active with the current flag,
 *	passing the argument to the function.
 */
do_daemons(flag)
reg int flag;
{
	reg struct delayed_action *dev;

	/*
	 * Loop through the devil list
	 */
	for (dev = d_list; dev < &d_list[MAXDAEMONS]; dev++)
	/*
	 * Executing each one, giving it the proper arguments
	 */
		if (dev->d_type == flag && dev->d_time == DAEMON)
			(*dev->d_func)(dev->d_arg);
}


/*
 * fuse:
 *	Start a fuse to go off in a certain number of turns
 */
fuse(func, arg, time, type)
reg int (*func)(), arg, time, type;
{
	reg struct delayed_action *wire;

	wire = d_slot();
	if (wire != NULL) {
		wire->d_type = type;
		wire->d_func = func;
		wire->d_arg = arg;
		wire->d_time = time;
		demoncnt += 1;			/* update count */
	}
}


/*
 * lengthen:
 *	Increase the time until a fuse goes off
 */
lengthen(func, xtime)
reg int (*func)(), xtime;
{
	reg struct delayed_action *wire;

	if ((wire = find_slot(func)) == NULL)
		return;
	wire->d_time += xtime;
}


/*
 * extinguish:
 *	Put out a fuse
 */
extinguish(func)
reg int (*func)();
{
	reg struct delayed_action *wire;

	if ((wire = find_slot(func)) == NULL)
		return;
	wire->d_type = EMPTY;
	demoncnt -= 1;
}


/*
 * do_fuses:
 *	Decrement counters and start needed fuses
 */
do_fuses(flag)
reg int flag;
{
	reg struct delayed_action *wire;

	/*
	 * Step though the list
	 */
	for (wire = d_list; wire < &d_list[MAXDAEMONS]; wire++) {
	/*
	 * Decrementing counters and starting things we want.  We also need
	 * to remove the fuse from the list once it has gone off.
	 */
	    if(flag == wire->d_type && wire->d_time > 0	&&
	      --wire->d_time == 0) {
		wire->d_type = EMPTY;
		(*wire->d_func)(wire->d_arg);
		demoncnt -= 1;
	    }
	}
}


/*
 * activity:
 *	Show wizard number of demaons and memory blocks used
 */
activity()
{
	msg("Daemons = %d : Memory Items = %d : Memory Used = %d",
	    demoncnt,total,sbrk(0));
}

struct fusenames {
	int (*faddr)();
	char *name;
	int len;
} fuselist[] = {
	rollwand,	"rollwand",	0,
	doctor,		"doctor",	0,
	stomach,	"stomach",	0,
	runners,	"runners",	0,
	swander,	"swander",	WANDERTIME,
	wghtchk,	"wghtchk",	0,
	unstink,	"unstink",	STINKTIME,
	res_strength,	"res_strength",	8,
	un_itch,	"un_itch",	24,
	cure_disease,	"cure_disease",	SICKTIME*16,
	suffocate,	"suffocate",	8,
	nohaste,	"nohaste",	80,
	unconfuse,	"unconfuse",	20+HUHDURATION,
	sight,		"sight",	SEEDURATION,
	unclrhead,	"unclrhead",	CLRDURATION,
	unsee,		"unsee",	SEEDURATION,
	unphase,	"unphase",	PHASEDURATION,
	appear,		"appear",	GONETIME,
	noslow,		"noslow",	8,
	(int(*)())0,	(char*)0,
};

show_fuses()
{
	struct delayed_action *dp;
	struct fusenames *fp;
	char *n;
	char buf[LINELEN];
	int i = 0;
	int w;
	int ch;

	wclear(hw);
	for (dp = d_list; dp < &d_list[MAXDAEMONS]; dp++) {
		++i;
		if (dp->d_func == (int (*)())0)
			continue;
		for (fp = fuselist; fp->faddr; fp++)
			if (dp->d_func == fp->faddr)
				break;
		if (fp->faddr == (int (*)())0) {
			static char buf[80];

			sprintf(buf, "unknown daemon at addr 0x%x", dp->d_func);
			n = buf;
		} else
			n = fp->name;
		if (dp->d_time == DAEMON)
			wprintw(hw, "%2d) daemon %-15s type: %s\n\r", i, n,
				dp->d_type == AFTER ? "AFTER" : "BEFORE");
		else
			wprintw(hw, "%2d) fuse   %-15s type: %s time: %d\n\r",
				i, n, dp->d_type == AFTER ? "AFTER " : "BEFORE",
				dp->d_time);
	}
	wprintw(hw, "\n\nCommand (l-lengthen, d-delete, e-execute): ");
	draw(hw);
	ch = readchar();
	if (ch == ESCAPE) {
		restscr(cw);
		return;
	}
	wprintw(hw, "\nWhich: ");
	draw(hw);
	get_str(buf, hw);
	w = atoi(buf);
	if (w < 1 || w > i) {
		restscr(cw);
		return;
	}
	dp = &d_list[w - 1];
	if (dp->d_func == (int (*)())0 || dp->d_type == EMPTY) {
		restscr(cw);
		return;
	}
	for (fp = fuselist; fp->faddr; fp++)
		if (dp->d_func == fp->faddr)
			break;
	switch (ch) {
	case 'l':
		wprintw(hw, "\nLengthed: %s", fp->name);
		draw(hw);
		if (dp->d_time == DAEMON || fp->len == 0)
			break;
		lengthen(fp->faddr, fp->len);
		break;
	case 'd':
		wprintw(hw, "\nDeleted: %s", fp->name);
		draw(hw);
		if (dp->d_time == DAEMON)
			kill_daemon(fp->faddr);
		else
			extinguish(fp->faddr);
		break;
	case 'e':
		wprintw(hw, "\nExecuted: %s", fp->name);
		draw(hw);
		(*dp->d_func)(dp->d_arg);
		if (dp->d_time > 0)
			extinguish(fp->faddr);
		break;
	}
	sleep(1);
	restscr(cw);
}
