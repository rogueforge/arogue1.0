/*
 * Read and execute the user commands
 *
 */

#include "curses.h"
#include <ctype.h>
#include <signal.h>
#include "rogue.h"

/*
 * command:
 *	Process the user commands
 */

command()
{
    register char ch;
    register int ntimes = 1;			/* Number of player moves */
    static char countch, direction, newcount = FALSE;
    bool an_after = FALSE;

    if (on(player, ISHASTE)) ntimes++;
    if (on(player, ISSLOW)) {
	if (player.t_turn != TRUE) ntimes--;
	player.t_turn ^= TRUE;
    }

    /*
     * Let the daemons start up
     */
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    while (ntimes--)
    {
	/* If player is infested, take off a hit point */
	if (on(player, HASINFEST)) {
	    if ((pstats.s_hpt -= infest_dam) <= 0) death(D_INFESTATION);
	}

	look(after);
	if (!running)
	    door_stop = FALSE;
	status(FALSE);
	lastscore = purse;
	wmove(cw, hero.y, hero.x);
	if (!((running || count) && jump))
	    draw(cw);			/* Draw screen */
	take = 0;
	after = TRUE;
	/*
	 * Read command or continue run
	 */
	if (!no_command)
	{
	    if (running) {
		/* If in a corridor, if we are at a turn with only one
		 * way to go, turn that way.
		 */
		if ((winat(hero.y, hero.x) == PASSAGE) && off(player, ISHUH) &&
		    (off(player, ISBLIND)))
		    switch (runch) {
			when 'h' : corr_move(0, -1);
			when 'j' : corr_move(1, 0);
			when 'k' : corr_move(-1, 0);
			when 'l' : corr_move(0, 1);
		    }
	        ch = runch;
	    }
	    else if (count) ch = countch;
	    else
	    {
		ch = readchar();
		if (mpos != 0 && !running)	/* Erase message if its there */
		    msg("");
	    }
	}
	else ch = '.';
	if (no_command)
	{
	    if (--no_command == 0)
		msg("You can move again.");
	}
	else
	{
	    /*
	     * check for prefixes
	     */
	    if (isdigit(ch))
	    {
		count = 0;
		newcount = TRUE;
		while (isdigit(ch))
		{
		    count = count * 10 + (ch - '0');
		    ch = readchar();
		}
		countch = ch;
		/*
		 * turn off count for commands which don't make sense
		 * to repeat
		 */
		switch (ch) {
		    case 'h': case 'j': case 'k': case 'l':
		    case 'y': case 'u': case 'b': case 'n':
		    case 'H': case 'J': case 'K': case 'L':
		    case 'Y': case 'U': case 'B': case 'N':
		    case 'q': case 'r': case 's': case 'f':
		    case 't': case 'C': case 'I': case '.':
		    case 'z': case 'p':
			break;
		    default:
			count = 0;
		}
	    }

	    /* Save current direction */
	    if (!running) /* If running, it is already saved */
	    switch (ch) {
		case 'h': case 'j': case 'k': case 'l':
		case 'y': case 'u': case 'b': case 'n':
		case 'H': case 'J': case 'K': case 'L':
		case 'Y': case 'U': case 'B': case 'N':
		    runch = Tolower(ch);
	    }

	    /* Perform the action */
	    switch (ch) {
		when 'f':
		    if (!on(player, ISBLIND))
		    {
			door_stop = TRUE;
			firstmove = TRUE;
		    }
		    if (count && !newcount)
			ch = direction;
		    else
			ch = readchar();
		    switch (ch)
		    {
			case 'h': case 'j': case 'k': case 'l':
			case 'y': case 'u': case 'b': case 'n':
			    ch = toupper(ch);
		    }
		    direction = ch;
	    }
	    newcount = FALSE;
	    /*
	     * execute a command
	     */
	    if (count && !running)
		count--;
	    switch (ch)
	    {
		when '!' : shell();
		when 'h' : do_move(0, -1);
		when 'j' : do_move(1, 0);
		when 'k' : do_move(-1, 0);
		when 'l' : do_move(0, 1);
		when 'y' : do_move(-1, -1);
		when 'u' : do_move(-1, 1);
		when 'b' : do_move(1, -1);
		when 'n' : do_move(1, 1);
		when 'H' : do_run('h');
		when 'J' : do_run('j');
		when 'K' : do_run('k');
		when 'L' : do_run('l');
		when 'Y' : do_run('y');
		when 'U' : do_run('u');
		when 'B' : do_run('b');
		when 'N' : do_run('n');
		when 't':
		    if (!get_dir())
			after = FALSE;
		    else
			missile(delta.y, delta.x,
				get_item("throw", NULL), &player);
		when 'Q' : after = FALSE; quit();
		when 'i' : after = FALSE; inventory(pack, 0, -1);
		when 'I' : after = FALSE; picky_inven();
		when 'd' : drop(NULL);
		when 'q' : quaff(-1, FALSE);
		when 'r' : read_scroll(-1, FALSE);
		when 'e' : eat();
		when 'w' : wield();
		when 'W' : wear();
		when 'T' : take_off();
		when 'P' : ring_on();
		when 'R' : ring_off();
		when 'o' : option();
		when 'c' : call(FALSE);
		when 'm' : call(TRUE);
		when '>' : after = FALSE; d_level();
		when '<' : after = FALSE; u_level();
		when '?' : after = FALSE; help();
		when '/' : after = FALSE; identify();
		when CTRL(T) :
		    if (get_dir()) steal();
		    else after = FALSE;
		when 'D' : dip_it();
		when 'G' : gsense();
		when '^' : set_trap(&player, hero.y, hero.x);
		when 's' : search(FALSE);
		when 'z' :
		    if (get_dir())
			do_zap(TRUE, NULL, FALSE);
		    else
			after = FALSE;
		when 'p' : pray();
		when 'C' : cast();
		when 'a' :
		    if (get_dir())
			affect();
		    else after = FALSE;
		when 'v' : after = FALSE;
			   msg("Advanced Rogue Version %s.",
				release);
		when CTRL(L) : after = FALSE; clearok(curscr, TRUE);
				touchwin(cw); /* MMMMMMMMMM */
		when CTRL(R) : after = FALSE; msg(huh);
		when 'S' : 
		    after = FALSE;
		    if (save_game())
		    {
			wclear(cw);
			draw(cw);
			endwin();
			exit(0);
		    }
		when '.' : ;			/* Rest command */
		when ' ' : after = FALSE;	/* Do Nothing */
		when CTRL(P) :
		    after = FALSE;
		    if (wizard)
		    {
			wizard = FALSE;
			trader = 0;
			msg("Not wizard any more");
		    }
		    else
		    {
			if (waswizard || passwd())
			{
			    msg("Welcome, oh mighty wizard.");
			    wizard = waswizard = TRUE;
			}
			else
			    msg("Sorry");
		    }
		when ESCAPE :	/* Escape */
		    door_stop = FALSE;
		    count = 0;
		    after = FALSE;
		when '#':
		    if (levtype == POSTLEV)		/* buy something */
			buy_it();
		    after = FALSE;
		when '$':
		    if (levtype == POSTLEV)		/* price something */
			price_it();
		    after = FALSE;
		when '%':
		    if (levtype == POSTLEV)		/* sell something */
			sell_it();
		    after = FALSE;
		otherwise :
		    after = FALSE;
		    if (wizard) switch (ch)
		    {
			when 'M' : create_obj(TRUE, 0, 0);
			when CTRL(I) : inventory(lvl_obj, 0, getbless());
			when CTRL(W) : whatis(NULL);
			when CTRL(D) : level++; new_level(NORMLEV);
			when CTRL(U) : level--; new_level(NORMLEV);
			when CTRL(F) : overlay(stdscr,cw);
			when CTRL(X) : overlay(mw,cw);
			when CTRL(J) : teleport();
			when CTRL(E) : msg("food left: %d\tfood level: %d", 
						food_left, foodlev);
			when CTRL(A) : activity();
			when CTRL(G) : 
			{
			    int tlev;
			    prbuf[0] = NULL;
			    msg("Which level? ");
			    if(get_str(prbuf,cw) == NORM) {
				tlev = atoi(prbuf);
				if(tlev < 1) {
				    mpos = 0;
				    msg("Illegal level.");
				}
				else if (tlev > 199) {
					levtype = MAZELEV;
					level = tlev - 200 + 1;
				}
				else if (tlev > 99) {
					levtype = POSTLEV;
					level = tlev - 100 + 1;
				} 
				else {
					levtype = NORMLEV;
					level = tlev;
				}
				new_level(levtype);
			    }
			}
			when CTRL(N) :
			{
			    register struct linked_list *item;

			    if ((item = get_item("charge", STICK)) != NULL)
				((struct object *)ldata(item))->o_charges=10000;
			}
#ifdef orig
			when CTRL(H) :
			{
			    register int i;
			    register struct linked_list *item;
			    register struct object *obj;

			    for (i = 0; i < 9; i++)
				raise_level();
			    /*
			     * Give the rogue a sword 
			     */
			    item = spec_item(WEAPON, TWOSWORD, 5, 5);
			    add_pack(item, TRUE);
			    cur_weapon = (struct object *) ldata(item);
			    cur_weapon->o_flags |= ISKNOW;
			    /*
			     * And his suit of armor
			     */
			    item = spec_item(ARMOR, PLATE_ARMOR, 7, 0);
			    obj = (struct object *) ldata(item);
			    obj->o_flags |= ISKNOW;
			    obj->o_weight = armors[PLATE_ARMOR].a_wght;
			    cur_armor = obj;
			    add_pack(item, TRUE);
			    purse += 5000;
			}
#else
			when CTRL(H):
			    if (get_dir())
				hstat();
#endif
			when '@':
				msg("Command? ");
				ch = readchar();
				switch (ch) {
#ifndef orig
				when CTRL(H) :
				{
				    register int i;
				    register struct linked_list *item;
				    register struct object *obj;

				    for (i = 0; i < 9; i++)
					raise_level();
				    /*
				     * Give the rogue a sword 
				     */
				    item = spec_item(WEAPON, TWOSWORD, 5, 5);
				    add_pack(item, TRUE);
				    cur_weapon = (struct object *) ldata(item);
				    cur_weapon->o_flags |= ISKNOW;
				    /*
				     * And his suit of armor
				     */
				    item = spec_item(ARMOR, PLATE_ARMOR, 7, 0);
				    obj = (struct object *) ldata(item);
				    obj->o_flags |= ISKNOW;
				    obj->o_weight = armors[PLATE_ARMOR].a_wght;
				    cur_armor = obj;
				    add_pack(item, TRUE);
				    purse += 5000;
				}
#endif
				when 's':
					add_intelligence(FALSE);
					add_dexterity(FALSE);
					add_strength(FALSE);
					add_wisdom(FALSE);
					add_const(FALSE);
				when 'S':
					msg("You feel like a new player");
					chg_str(5);
					add_generic(&pstats.s_intel,
						    &max_stats.s_intel,
						    R_ADDINTEL);
					add_generic(&pstats.s_wisdom,
						    &max_stats.s_wisdom,
						    R_ADDWISDOM);
					add_generic(&pstats.s_dext,
						    &max_stats.s_dext,
						    R_ADDHIT);
					add_generic(&pstats.s_const,
						    &max_stats.s_const, -1);
				when 'r':
					msg("Tried any magic lately?");
					pray_time = spell_power = trap_tries = 0;
				when 'e': {
					struct linked_list *ll;
					struct object *lb;

					if ((ll = get_item("enchant", 0)) == NULL)
						break;
					lb = OBJPTR(ll);
					lb->o_flags &= ~ISCURSED;
					switch(lb->o_type) {
					when RING:
						if (lb->o_which == R_DIGEST)
							lb->o_ac = 1;
						else
							lb->o_ac += 3;
					when ARMOR:
						lb->o_ac -= 5;
					when STICK:
						lb->o_charges += 25;
					when WEAPON:
						lb->o_hplus += 5;
						lb->o_dplus += 5;
					when POTION:
					case SCROLL:
						lb->o_flags |= ISBLESSED;
					}
					}
				when 'm': {
				    char chr;

				    msg("Which monster do you want displayed? ");
				    chr = readchar();
				    if (chr == ESCAPE) {
					msg("");
					break;
				    }
				    if (!isalpha(chr)) {
					msg("What kind of monster is that?!!");
					break;
				    }
				    monst_displ(&monsters[id_monst(chr)]);
				}
				when 'h':
					msg("That'll show 'em");
					max_stats.s_hpt += 5;
					status(TRUE);
				when 'f':
					show_fuses();
				}
			otherwise :
			    msg("Illegal command '%s'.", unctrl(ch));
			    count = 0;
		    }
		    else
		    {
			msg("Illegal command '%s'.", unctrl(ch));
			count = 0;
			after = FALSE;
		    }
	    }
	    /*
	     * turn off flags if no longer needed
	     */
	    if (!running)
		door_stop = FALSE;
	}
	/*
	 * If he ran into something to take, let him pick it up.
	 * unless its a trading post
	 */
	if (take != 0 && levtype != POSTLEV)
	    pick_up(take);
	if (!running)
	    door_stop = FALSE;

	/* If after is true, mark an_after as true so that if
	 * we are hasted, the first "after" will be noted.
	 */
	if (after) an_after = TRUE;
    }

    /*
     * Kick off the rest if the daemons and fuses
     */
    if (an_after)
    {
	look(FALSE);
	do_daemons(AFTER);
	do_fuses(AFTER);

	/* Special abilities */
	if ((player.t_ctype == C_THIEF) &&
	    (rnd(100) < (2*pstats.s_dext + 5*pstats.s_lvl))) search(TRUE);
	if (ISWEARING(R_SEARCH)) search(FALSE);
	if (ISWEARING(R_TELEPORT) && rnd(100) < 2) teleport();
    }
}

/*
 * quit:
 *	Have player make certain, then exit.
 */

quit()
{
    /*
     * Reset the signal in case we got here via an interrupt
     */
    if (signal(SIGINT, quit) != quit)
	mpos = 0;
    msg("Really quit?");
    draw(cw);
    if (readchar() == 'y')
    {
	clear();
	move(LINES-1, 0);
	draw(stdscr);
	score((long) purse, CHICKEN, 0);
	byebye();
    }
    else
    {
	signal(SIGINT, quit);
	wmove(cw, 0, 0);
	wclrtoeol(cw);
	status(FALSE);
	draw(cw);
	mpos = 0;
	count = 0;
    }
}

/*
 * search:
 *	Player gropes about him to find hidden things.
 */

search(is_thief)
register bool is_thief;
{
    register int x, y;
    register char ch;

    /*
     * Look all around the hero, if there is something hidden there,
     * give him a chance to find it.  If its found, display it.
     */
    if (on(player, ISBLIND))
	return;
    for (x = hero.x - 1; x <= hero.x + 1; x++)
	for (y = hero.y - 1; y <= hero.y + 1; y++)
	{
	    ch = winat(y, x);
	    if (isatrap(ch)) {
		    register struct trap *tp;

		    if (isatrap(mvwinch(cw, y, x)))
			continue;
		    tp = trap_at(y, x);
		    if ((tp->tr_flags & ISTHIEFSET) ||
			(rnd(100) > 50 && !is_thief)) break;
		    tp->tr_flags |= ISFOUND;
		    mvwaddch(cw, y, x, ch);
		    count = 0;
		    running = FALSE;
		    msg(tr_name(tp->tr_type));
	    }
	    else if (ch == SECRETDOOR) {
		    if (rnd(100) < 20 && !is_thief) {
			mvaddch(y, x, DOOR);
			count = 0;
		    }
	    }
	}
}

/*
 * help:
 *	Give single character help, or the whole mess if he wants it
 */

help()
{
    register struct h_list *strp = helpstr;
    register char helpch;
    register int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar();
    mpos = 0;
    /*
     * If its not a *, print the right help string
     * or an error if he typed a funny character.
     */
    if (helpch != '*')
    {
	wmove(cw, 0, 0);
	while (strp->h_ch)
	{
	    if (strp->h_ch == helpch)
	    {
		msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
		break;
	    }
	    strp++;
	}
	if (strp->h_ch != helpch)
	    msg("Unknown character '%s'", unctrl(helpch));
	return;
    }
    /*
     * Here we print help for everything.
     * Then wait before we return to command mode
     */
    wclear(hw);
    cnt = 0;
    while (strp->h_ch) {
	mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
	waddstr(hw, strp->h_desc);
	strp++;
	if (++cnt >= 46 && strp->h_ch) {
	    wmove(hw, LINES-1, 0);
	    wprintw(hw, morestr);
	    draw(hw);
	    wait_for(' ');
	    wclear(hw);
	    cnt = 0;
	}
    }
    wmove(hw, LINES-1, 0);
    wprintw(hw, spacemsg);
    draw(hw);
    wait_for(' ');
    wclear(hw);
    draw(hw);
    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status(FALSE);
    touchwin(cw);
}

/*
 * identify:
 *	Tell the player what a certain thing is.
 */

identify()
{
    register char ch, *str;

    msg("What do you want identified? ");
    ch = readchar();
    mpos = 0;
    if (ch == ESCAPE)
    {
	msg("");
	return;
    }
    if (isalpha(ch))
	str = monsters[id_monst(ch)].m_name;
    else switch(ch)
    {
	case '|':
	case '-':
	    str = "wall of a room";
	when GOLD:	str = "gold";
	when STAIRS :	str = "passage leading down";
	when DOOR:	str = "door";
	when FLOOR:	str = "room floor";
	when VPLAYER:	str = "The hero of the game ---> you";
	when IPLAYER:	str = "you (but invisible)";
	when PASSAGE:	str = "passage";
	when POST:	str = "trading post";
	when POOL:	str = "a shimmering pool";
	when TRAPDOOR:	str = "trapdoor";
	when ARROWTRAP:	str = "arrow trap";
	when SLEEPTRAP:	str = "sleeping gas trap";
	when BEARTRAP:	str = "bear trap";
	when TELTRAP:	str = "teleport trap";
	when DARTTRAP:	str = "dart trap";
	when MAZETRAP:	str = "entrance to a maze";
	when POTION:	str = "potion";
	when SCROLL:	str = "scroll";
	when FOOD:	str = "food";
	when WEAPON:	str = "weapon";
	when ' ' :	str = "solid rock";
	when ARMOR:	str = "armor";
	when AMULET:	str = "The Amulet of Yendor";
	when RING:	str = "ring";
	when STICK:	str = "wand or staff";
	otherwise:	str = "unknown character";
    }
    msg("'%s' : %s", unctrl(ch), str);
}

/*
 * d_level:
 *	He wants to go down a level
 */

d_level()
{
    bool no_phase=FALSE;

    if (winat(hero.y, hero.x) != STAIRS) {
	if (off(player, CANINWALL)) {	/* Must use stairs if can't phase */
	    msg("I see no way down.");
	    return;
	}
	extinguish(unphase);	/* Using phase to go down gets rid of it */
	no_phase = TRUE;
    }
    level++;
    new_level(NORMLEV);
    if (no_phase) unphase();
}

/*
 * u_level:
 *	He wants to go up a level
 */

u_level()
{
    if (winat(hero.y, hero.x) == STAIRS)
    {
	if (amulet)
	{
	    if (--level == 0)
		total_winner();
	    new_level(NORMLEV);
	    msg("You feel a wrenching sensation in your gut.");
	    return;
	}
    }
    msg("I see no way up.");
}

/*
 * Let him escape for a while
 */

shell()
{
    register int pid;
    register char *sh;
    int ret_status;

    /*
     * Set the terminal back to original mode
     */
    sh = getenv("SHELL");
    wclear(hw);
    wmove(hw, LINES-1, 0);
    draw(hw);
    endwin();
    in_shell = TRUE;
    fflush(stdout);
    /*
     * Fork and do a shell
     */
    while((pid = fork()) < 0)
	sleep(1);
    if (pid == 0)
    {
	/*
	 * Set back to original user, just in case
	 */
	setuid(getuid());
	setgid(getgid());
	execl(sh == NULL ? "/bin/sh" : sh, "shell", "-i", 0);
	perror("No shelly");
	exit(-1);
    }
    else
    {
	int endit();

	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	while (wait(&ret_status) != pid)
	    continue;
	signal(SIGINT, endit);
	signal(SIGQUIT, endit);
	printf(retstr);
	noecho();
	crmode();
	in_shell = FALSE;
	wait_for('\n');
	clearok(cw, TRUE);
	touchwin(cw);
    }
}

/*
 * allow a user to call a potion, scroll, or ring something
 */
call(mark)
bool mark;
{
    register struct object *obj;
    register struct linked_list *item;
    register char **guess, *elsewise;
    register bool *know;

    if (mark) item = get_item("mark", MARKABLE);
    else item = get_item("call", CALLABLE);
    /*
     * Make certain that it is somethings that we want to wear
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    switch (obj->o_type)
    {
	when RING:
	    guess = r_guess;
	    know = r_know;
	    elsewise = (r_guess[obj->o_which] != NULL ?
			r_guess[obj->o_which] : r_stones[obj->o_which]);
	when POTION:
	    guess = p_guess;
	    know = p_know;
	    elsewise = (p_guess[obj->o_which] != NULL ?
			p_guess[obj->o_which] : p_colors[obj->o_which]);
	when SCROLL:
	    guess = s_guess;
	    know = s_know;
	    elsewise = (s_guess[obj->o_which] != NULL ?
			s_guess[obj->o_which] : s_names[obj->o_which]);
	when STICK:
	    guess = ws_guess;
	    know = ws_know;
	    elsewise = (ws_guess[obj->o_which] != NULL ?
			ws_guess[obj->o_which] : ws_made[obj->o_which]);
	otherwise:
	    if (!mark) {
		msg("You can't call that anything.");
		return;
	    }
	    else know = (bool *) 0;
    }
    if (know && know[obj->o_which] && !mark) {
	msg("That has already been identified.");
	return;
    }
    if (mark) {
	if (obj->o_mark[0]) {
	    addmsg(terse ? "M" : "Was m");
	    msg("arked \"%s\"", obj->o_mark);
	}
	msg(terse ? "Mark it: " : "What do you want to mark it? ");
	prbuf[0] = '\0';
    }
    else {
	addmsg(terse ? "C" : "Was c");
	msg("alled \"%s\"", elsewise);
	msg(terse ? "Call it: " : "What do you want to call it? ");
	if (guess[obj->o_which] != NULL)
	    cfree(guess[obj->o_which]);
	strcpy(prbuf, elsewise);
    }
    if (get_str(prbuf, cw) == NORM) {
	if (mark) {
	    strncpy(obj->o_mark, prbuf, MARKLEN-1);
	    obj->o_mark[MARKLEN-1] = '\0';
	}
	else {
	    guess[obj->o_which] = new((unsigned int) strlen(prbuf) + 1);
	    strcpy(guess[obj->o_which], prbuf);
	}
    }
}

add_generic(cur, maximum, ring)
short *cur, *maximum;
int ring;
{
	int ring_str;

	if (ring == -1)
		ring_str = 0;
	else
		ring_str = ring_value(ring);
	*cur -= ring_str;
	*cur += 5;
	if (*cur > 25) *cur = 25;
	if (*maximum < *cur)
		*maximum = *cur;
	*cur += ring_str;
}
