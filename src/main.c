/*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 */

#include "curses.h"
#include <signal.h>
#include <pwd.h>
#include <sys/time.h>
#include "mach_dep.h"
#include "rogue.h"

#ifdef CHECKTIME
static int num_checks;		/* times we've gone over in checkout() */
#endif

main(argc, argv, envp)
char **argv;
char **envp;
{
    register char *env;
    register struct passwd *pw;
    register struct linked_list *item;
    register struct object *obj;
    struct passwd *getpwuid();
    char *getpass(), *crypt();
    int quit(), lowtime, wpt, i, j;
    bool alldone;
    long now;

    /*
     * get home and options from environment
     */
    /* monst_show(); */
    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else if ((pw = getpwuid(getuid())) != NULL)
	strcpy(home, pw->pw_dir);
    else
	home[0] = '\0';
    strcat(home, "/");

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, "rogue.save");

    /* Get default score file */
    strcpy(score_file, SCOREFILE);

    if ((env = getenv("ROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	if ((pw = getpwuid(getuid())) == NULL)
	{
	    printf("Say, who the hell are you?\n");
	    exit(1);
	}
	else
	    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
    if (env == NULL || fruit[0] == '\0')
	strcpy(fruit, "slime-mold");

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, SCOREIT, 0);
	exit(0);
    }
    /*
     * Check to see if he is a wizard
     */
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), "mT")) == 0)
	{
	    wizard = TRUE;
	    argv++;
	    argc--;
	}

#if MAXLOAD|MAXUSERS
    if (too_much() && !wizard && !author())
    {
	printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	printf("Try again later.  Meanwhile, why not enjoy a%s %s?\n",
	    vowelstr(fruit), fruit);
	exit(1);
    }
#endif
    if (argc == 2)
	if (!restore(argv[1], envp)) /* Note: restore will never return */
	    exit(1);
    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    seed = dnum;
    srand48(seed);

    init_things();			/* Set up probabilities of things */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    initscr();				/* Start up cursor package */
    init_names();			/* Set up names of scrolls */
    setup();
    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    init_player();			/* Roll up the rogue */
    waswizard = wizard;
    new_level(FALSE);			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    start_daemon(doctor, (long) &player, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    start_daemon(stomach, 0, AFTER);
    start_daemon(runners, 0, AFTER);
    /*
     * Give the rogue his weaponry.  
     */
    alldone = FALSE;
    do {
	int ch;
	i = rnd(11);	/* number of acceptable weapons */
	switch(i) {
	    case 0: ch = 25; wpt = MACE;
	    when 1: ch = 25; wpt = SWORD;
	    when 2: ch = 15; wpt = TWOSWORD;
	    when 3: ch = 10; wpt = SPEAR;
	    when 4: ch = 20; wpt = TRIDENT;
	    when 5: ch = 20; wpt = SPETUM;
	    when 6: ch = 20; wpt = BARDICHE;
	    when 7: ch = 15; wpt = PIKE;
	    when 8: ch = 15; wpt = BASWORD;
	    when 9: ch = 20; wpt = HALBERD;
	    when 10:ch = 20; wpt = BATTLEAXE;
	}
	if(rnd(100) < ch) {		/* create this weapon */
	    alldone = TRUE;
	}
    } while(!alldone);
    if (player.t_ctype == C_FIGHTER)
	wpt = TWOSWORD;
    item = spec_item(WEAPON, wpt, rnd(2), rnd(2)+1);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * And his suit of armor
     */
    i = rnd(100);
    j = 0;
    while (armors[j].a_prob < i)
	j++;
    item = spec_item(ARMOR, j, 0, 0);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    obj->o_weight = armors[j].a_wght;
    add_pack(item, TRUE);
    cur_armor = obj;
    /*
     * Give him some food too
     */
    item = spec_item(FOOD, 0, 0, 0);
    obj = OBJPTR(item);
    obj->o_weight = things[TYP_FOOD].mi_wght;
    add_pack(item, TRUE);
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */

endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    printf("\n");	/* So the curser doesn't stop at the end of the line */
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

rnd(range)
register int range;
{
    return(range == 0 ? 0 : (lrand48() % range) & 0x7fffffff);
}

/*
 * roll:
 *	roll a number of dice
 */

roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}
# ifdef SIGTSTP
/*
 * handle stop and start signals
 */
tstp()
{
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    signal(SIGTSTP, SIG_DFL);
    sigsetmask(0);
    kill(getpid(), SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    flushout();
}
# endif

setup()
{
#ifdef CHECKTIME
    int  checkout();
#endif

#ifndef DUMP
    signal(SIGHUP, auto_save);
    signal(SIGILL, quit);
    signal(SIGTRAP, quit);
    signal(SIGIOT, quit);
#ifdef SIGEMT
    signal(SIGEMT, quit);
#endif
    signal(SIGFPE, quit);
    signal(SIGBUS, quit);
    signal(SIGSEGV, quit);
    signal(SIGSYS, quit);
    signal(SIGPIPE, quit);
    signal(SIGTERM, auto_save);
#endif

    signal(SIGINT, quit);
#ifndef DUMP
    signal(SIGQUIT, endit);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, tstp);
#endif
#ifdef CHECKTIME
    if (!author())
    {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
	num_checks = 0;
    }
#endif
    sprintf(prbuf, "%d", LINES);
    setenv("LINES", prbuf, 1);
    sprintf(prbuf, "%d", COLS);
    setenv("COLUMNS", prbuf, 1);
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

playit()
{
    register char *opts;

    /*
     * set up defaults for slow terminals
     */

	if (baudrate() < 1200)
    {
	terse = TRUE;
	jump = TRUE;
    }

    /*
     * parse environment declaration of options
     */
    if ((opts = getenv("ROGUEOPTS")) != NULL)
	parse_opts(opts);


    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();			/* Command execution */
    endit();
}

#if MAXLOAD|MAXUSERS
/*
 * see if the system is being used too much for this game
 */
too_much()
{
#ifdef MAXLOAD
	double avec[3];
#endif

#ifdef MAXLOAD
	loadav(avec);
	return (avec[2] > (MAXLOAD / 10.0));
#else
	return (ucount() > MAXUSERS);
#endif
}
#endif

/*
 * author:
 *	See if a user is an author of the program
 */
author()
{
	switch (getuid()) {
#if AUTHOR
		case AUTHOR:
#endif
		case 0:
			return TRUE;
		default:
			return FALSE;
	}
}


#ifdef CHECKTIME
checkout()
{
	static char *msgs[] = {
	"The system is too loaded for games. Please leave in %d minutes",
	"Please save your game.  You have %d minutes",
	"This is your last chance. You had better leave in %d minutes",
	};
	int checktime;

	signal(SIGALRM, checkout);
	if (!holiday() && !author()) {
	    wclear(cw);
	    mvwaddstr(cw, LINES / 2, 0,
		"Game time is over. Your game is being saved.\n\n");
	    draw(cw);
	    auto_save();		/* NO RETURN */
	}
	if (too_much())	{
	    if (num_checks >= 3)
		fatal("You didn't listen, so now you are DEAD !!\n");
	    checktime = CHECKTIME / (num_checks + 1);
		chmsg(msgs[num_checks++], checktime);
		alarm(checktime * 60);
	}
	else {
	    if (num_checks) {
		chmsg("The load has dropped. You have a reprieve.");
		num_checks = 0;
	    }
	    alarm(CHECKTIME * 60);
	}
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char *fmt;
int arg;
{
	if (in_shell) {
		printf(fmt, arg);
		putchar('\n');
		fflush(stdout);
	}
	else
		msg(fmt, arg);
}
#endif

#ifdef LOADAV

#include <nlist.h>

struct nlist avenrun =
{
	"_avenrun"
};

loadav(avg)
reg double *avg;
{
	reg int kmem;

	if ((kmem = open("/dev/kmem", 0)) < 0)
		goto bad;
	nlist(NAMELIST, &avenrun);
	if (avenrun.n_type == 0) {
bad:
		avg[0] = avg[1] = avg[2] = 0.0;
		return;
	}
	lseek(kmem, (long) avenrun.n_value, 0);
	read(kmem, avg, 3 * sizeof (double));
}
#endif

#ifdef UCOUNT
/*
 * ucount:
 *	Count the number of people on the system
 */
#include <sys/types.h>
#include <utmp.h>
struct utmp buf;
ucount()
{
	reg struct utmp *up;
	reg FILE *utmp;
	reg int count;

	if ((utmp = fopen(UTMP, "r")) == NULL)
	    return 0;

	up = &buf;
	count = 0;
	while (fread(up, 1, sizeof (*up), utmp) > 0)
		if (buf.ut_type == USER_PROCESS)
			count++;
	fclose(utmp);
	return count;
}
#endif

/*
 * holiday:
 *	Returns TRUE when it is a good time to play rogue
 */
holiday()
{
	long now;
	struct tm *localtime();
	reg struct tm *ntime;

	time(&now);			/* get the current time */
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return TRUE;		/* OK on Sat & Sun */
	if(ntime->tm_hour < 8 || ntime->tm_hour >= 17)
		return TRUE;		/* OK before 8AM & after 5PM */
	if(ntime->tm_yday <= 7 || ntime->tm_yday >= 350)
		return TRUE;		/* OK during Christmas */
	if (access("/usr/tmp/.ryes",0) == 0)
	    return TRUE;		/* if author permission */

	return FALSE;			/* All other times are bad */
}
