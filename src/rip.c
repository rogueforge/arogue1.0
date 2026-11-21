/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	3.13 (Berkeley) 6/16/81
 */

#include "curses.h"
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <fcntl.h>
#include <pwd.h>
#include "mach_dep.h"
#include "rogue.h"

static char *rip[] = {
"                       __________",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |    killed by     |",
"                  |                  |",
"                  |       1980       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

char	*killname();

/*
 * death:
 *	Do something really fun when he dies
 */

death(monst)
register short monst;
{
    register char **dp = rip, *killer;
    register struct tm *lt;
    time_t date;
    char buf[80];
    struct tm *localtime();

    time(&date);
    lt = localtime(&date);
    clear();
    move(8, 0);
    while (*dp)
	printw("%s\n", *dp++);
    mvaddstr(14, 28-((strlen(whoami)+1)/2), whoami);
    sprintf(buf, "%ld Points", pstats.s_exp );
    mvaddstr(15, 28-((strlen(buf)+1)/2), buf);
    killer = killname(monst);
    mvaddstr(17, 28-((strlen(killer)+1)/2), killer);
    mvaddstr(18, 26, (sprintf(prbuf, "%4d", 1900 + lt->tm_year), prbuf));
    move(LINES-1, 0);
    refresh();
    score(pstats.s_exp, KILLED, monst);
    endwin();
    exit(0);
}

/*
 * score -- figure score and post it.
 */

/* VARARGS2 */
score(amount, flags, monst)
unsigned long amount;
short monst;
{
    static struct sc_ent {
	unsigned long sc_score;
	char sc_name[80];
	int sc_flags;
	int sc_level;
	int sc_uid;
	short sc_ctype;
	short sc_monster;
    } top_ten[10];
    register struct sc_ent *scp;
    register int i;
    register struct sc_ent *sc2;
    register FILE *outf;
    register char *killer;
    register int prflags = 0;
    struct rlimit lim;
    static char *reason[] = {
	"killed",
	"quit",
	"A total winner",
    };
    int	endit();
    char *packend;

    signal(SIGINT, byebye);
    file_name[0] = '\0';
    signal(SIGHUP, SIG_IGN);
    if (flags != WINNER && flags != SCOREIT) {
	if (flags == CHICKEN)
	    packend = "when you quit";
	else
	    packend = "at your untimely demise";
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	getnstr(prbuf, LINELEN - 1);
	showpack(packend);
    }
    purse = 0;	/* Steal all the gold */

    /* if (flags != SCOREIT) */		/* moved to after "type return" */
    /*	endwin();	     */

    /*
     * Open file and read list
     */

    if (strncmp(score_file, SCOREFILE, LINELEN - 1) != 0)
	fd = open(score_file, O_RDWR|O_CREAT, 0600);
    if (fd < 0) return;
    outf = fdopen(fd, "w");

    for (scp = top_ten; scp < &top_ten[10]; scp++)
    {
	scp->sc_score = 0L;
	for (i = 0; i < 80; i++)
	    scp->sc_name[i] = rnd(255);
	scp->sc_flags = RN;
	scp->sc_level = RN;
	scp->sc_monster = RN;
	scp->sc_ctype = 0;
	scp->sc_uid = RN;
    }

    signal(SIGINT, SIG_DFL);
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_DFL);
#endif
    if (flags != SCOREIT)
    {
	mvaddstr(LINES - 1, 0, retstr);
	refresh();
	fflush(stdout);
	getnstr(prbuf, LINELEN - 1);
	endwin();			/* moved from above */
    }
    if (wizard)
	if (strcmp(prbuf, "names") == 0)
	    prflags = 1;
	else if (strcmp(prbuf, "edit") == 0)
	    prflags = 2;
    if (!waswizard || prflags == 2)
	hold_sc();
    i = encread((char *) top_ten, sizeof top_ten, fd);
    for (scp = sc2 = top_ten; scp < &top_ten[10]; scp++)
	if ((char *) (scp + 1) - (char *) top_ten <= i && scp->sc_score != 0
	    && scp->sc_flags >= 0 && scp->sc_flags <= 2
	    && scp->sc_monster >= D_DROWN && scp->sc_monster <= NUMMONST)
		*sc2++ = *scp;
    while (sc2 < &top_ten[10])
	sc2++->sc_score = 0;
    /*
     * Insert player in list if need be
     */
    if (!waswizard)
    {
	for (scp = top_ten; scp < &top_ten[10]; scp++)
	    if (amount > scp->sc_score)
		break;
	if (scp < &top_ten[10])
	{
	    for (sc2 = &top_ten[9]; sc2 > scp; sc2--)
		*sc2 = *(sc2-1);
	    scp->sc_score = amount;
	    strcpy(scp->sc_name, whoami);
	    scp->sc_flags = flags;
	    if (flags == WINNER)
		scp->sc_level = max_level;
	    else
		scp->sc_level = level;
	    scp->sc_monster = monst;
	    scp->sc_ctype = player.t_ctype;
	    scp->sc_uid = getuid();
	}
    }
    if (flags != SCOREIT) {
	clear();
	refresh();
    }
    /*
     * Print the list
     */
    printf("\nTop Ten Adventurers:\nRank\tScore\tName\n");
    for (scp = top_ten; scp < &top_ten[10]; scp++) {
	char *class;

	if (scp->sc_score && scp->sc_flags >= 0 && scp->sc_flags <= 2
	    && scp->sc_monster >= D_DROWN && scp->sc_monster <= NUMMONST) {
	    switch (scp->sc_ctype) {
		when C_FIGHTER:
		    class = "fighter";
		when C_MAGICIAN:
		    class = "magician";
		when C_CLERIC:
		    class = "cleric";
		when C_THIEF:
		    class = "thief";
		otherwise:
		    class = "unknown";
	    }

	    printf("%d\t%lu\t%s (%s): %s on level %d", (int) (scp - top_ten) + 1,
		scp->sc_score, scp->sc_name, class, reason[scp->sc_flags],
		scp->sc_level);
	    if (scp->sc_flags == 0) {
		printf(" by");
		killer = killname(scp->sc_monster);
		printf(" %s", killer);
	    }
	    if (prflags == 1)
	    {
		struct passwd *pp, *getpwuid();

		if ((pp = getpwuid(scp->sc_uid)) == NULL)
		    printf(" (%d)", scp->sc_uid);
		else
		    printf(" (%s)", pp->pw_name);
		putchar('\n');
	    }
	    else if (prflags == 2)
	    {
		fflush(stdout);
		fgets(prbuf, LINELEN, stdin);
		if (prbuf[0] == 'd')
		{
		    for (sc2 = scp; sc2 < &top_ten[9]; sc2++)
			*sc2 = *(sc2 + 1);
		    top_ten[9].sc_score = 0;
		    for (i = 0; i < 80; i++)
			top_ten[9].sc_name[i] = rnd(255);
		    top_ten[9].sc_flags = RN;
		    top_ten[9].sc_level = RN;
		    top_ten[9].sc_monster = RN;
		    scp--;
		}
	    }
	    else
		printf(".\n");
	}
    }
    /*
     * Update the list file
     */
    if ((!waswizard || prflags == 2)
	&& !(getrlimit(RLIMIT_FSIZE, &lim) == 0 && lim.rlim_cur < sizeof top_ten))
    {
	fseek(outf, 0L, 0);
	encwrite((char *) top_ten, sizeof top_ten, outf);
    }
    fclose(outf);

    mvaddstr(LINES - 1, 0, retstr);	/* this section added for xwindows */
    refresh();
    fflush(stdout);
    getnstr(prbuf, LINELEN - 1);

}

/*
 * hold_sc:
 *	Lock the score file.  The lock goes when the file is closed.
 *	Return TRUE if the file could be locked.
 */
hold_sc()
{
    register int cnt;
    static struct flock fl;

    fl.l_type = F_WRLCK;
    for (cnt = 0; fcntl(fd, F_SETLK, &fl) < 0; cnt++)
    {
	if (cnt == 5)
	    return FALSE;
	sleep(1);
    }
    return TRUE;
}

/*
 * open_score:
 *	Open the score file, and give up set-id privileges for good
 */
open_score()
{
    register int i;
    struct stat sbuf, sbuf2;

    while ((i = open("/dev/null", O_RDWR)) >= 0 && i < 3)
	continue;
    if (i >= 3)
	close(i);
    if (lstat(SCOREFILE, &sbuf) < 0)
	fd = open(SCOREFILE, O_RDWR|O_CREAT|O_EXCL, 0600);
    else if ((sbuf.st_mode & S_IFMT) != S_IFREG || sbuf.st_nlink != 1)
	fd = -1;
    else if ((fd = open(SCOREFILE, O_RDWR)) >= 0 && (fstat(fd, &sbuf2) < 0
	|| sbuf2.st_dev != sbuf.st_dev || sbuf2.st_ino != sbuf.st_ino))
    {
	close(fd);
	fd = -1;
    }
    if ((setregid(getgid(), getgid()) < 0 || setreuid(getuid(), getuid()) < 0)
	&& (getegid() != getgid() || geteuid() != getuid()))
    {
	perror("rogue");
	exit(1);
    }
}

total_winner()
{
    register struct linked_list *item;
    register struct object *obj;
    register int worth;
    register char c;
    register int oldpurse;

    file_name[0] = '\0';
    clear();
    standout();
    addstr("                                                               \n");
    addstr("  @   @               @   @           @          @@@  @     @  \n");
    addstr("  @   @               @@ @@           @           @   @     @  \n");
    addstr("  @   @  @@@  @   @   @ @ @  @@@   @@@@  @@@      @  @@@    @  \n");
    addstr("   @@@@ @   @ @   @   @   @     @ @   @ @   @     @   @     @  \n");
    addstr("      @ @   @ @   @   @   @  @@@@ @   @ @@@@@     @   @     @  \n");
    addstr("  @   @ @   @ @  @@   @   @ @   @ @   @ @         @   @  @     \n");
    addstr("   @@@   @@@   @@ @   @   @  @@@@  @@@@  @@@     @@@   @@   @  \n");
    addstr("                                                               \n");
    addstr("     Congratulations, you have made it to the light of day!    \n");
    standend();
    addstr("\nYou have joined the elite ranks of those who have escaped the\n");
    addstr("Dungeons of Doom alive.  You journey home and sell all your loot at\n");
    addstr("a great profit and are admitted to the fighters guild.\n");
    mvaddstr(LINES - 1, 0, spacemsg);
    refresh();
    wait_for(' ');
    clear();
    mvaddstr(0, 0, "   Worth  Item");
    oldpurse = purse;
    for (c = 'a', item = pack; item != NULL; c++, item = next(item))
    {
	obj = (struct object *) ldata(item);
	worth = get_worth(obj);
	whatis(item);
	mvprintw(c - 'a' + 1, 0, "%c) %5d  %s", c, worth, inv_name(obj, FALSE, -1));
	purse += worth;
    }
    mvprintw(c - 'a' + 1, 0,"   %5d  Gold Pieces          ", oldpurse);
    refresh();
    if (pstats.s_exp + (unsigned)purse < pstats.s_exp)
	score(pstats.s_exp, 2);		/* too good: no bonus! */
    else
    	score(pstats.s_exp + (unsigned) purse, 2);
    exit(0);
}

char *
killname(monst)
register short monst;
{
    static char mons_name[80];

    if (monst >= 0) {
	switch (monsters[monst].m_name[0]) {
	    case 'a':
	    case 'e':
	    case 'i':
	    case 'o':
	    case 'u':
		sprintf(mons_name, "an %s", monsters[monst].m_name);
		break;
	    default:
		sprintf(mons_name, "a %s", monsters[monst].m_name);
	}
	return(mons_name);
    }
    else
	switch (monst)
	{
	    case D_ARROW:
		return "an arrow";
	    case D_DART:
		return "a dart";
	    case D_BOLT:
		return "a bolt";
	    case D_POISON:
		return "poison";	/* Cursed healing potion */
	    case D_POTION:
		return "a cursed potion";
	    case D_PETRIFY:
		return "petrification";
	    case D_SUFFOCATION:
		return "suffocation";
	    case D_INFESTATION:
		return "a parasite";
	    case D_DROWN:
		return "drowning";
	}
}

/*
 * showpack:
 *	Display the contents of the hero's pack
 */
showpack(howso)
char *howso;
{
	reg char *iname;
	reg int cnt, worth, ch;
	reg struct linked_list *item;
	reg struct object *obj;

	idenpack();
	cnt = 1;
	clear();
	mvprintw(0, 0, "Contents of your pack %s:\n",howso);
	ch = 'a';
	for (item = pack; item != NULL; item = next(item)) {
		obj = OBJPTR(item);
		iname = inv_name(obj, FALSE, -1);
		mvprintw(cnt, 0, "%c) %s\n",ch,iname);
		ch += 1;
		if (++cnt >= LINES - 2 && next(item) != NULL) {
			cnt = 1;
			mvaddstr(LINES - 1, 0, morestr);
			refresh();
			wait_for(' ');
			clear();
		}
	}
	mvprintw(cnt + 1,0,"--- %d  Gold Pieces ---",purse);
	refresh();
}

void
byebye()
{
    if (!isendwin()) {
        clear();
	endwin();
    }
    printf("\n");
    exit(0);
}
