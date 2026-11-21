#include "curses.h"
#include "rogue.h"
#include <ctype.h>

/*
 * all sorts of miscellaneous routines
 *
 */

/*
 * tr_name:
 *	print the name of a trap
 */

char *
tr_name(ch)
char ch;
{
    register char *s;

    switch (ch)
    {
	when TRAPDOOR:
	    s = terse ? "A trapdoor." : "You found a trapdoor.";
	when BEARTRAP:
	    s = terse ? "A beartrap." : "You found a beartrap.";
	when SLEEPTRAP:
	    s = terse ? "A sleeping gas trap.":"You found a sleeping gas trap.";
	when ARROWTRAP:
	    s = terse ? "An arrow trap." : "You found an arrow trap.";
	when TELTRAP:
	    s = terse ? "A teleport trap." : "You found a teleport trap.";
	when DARTTRAP:
	    s = terse ? "A dart trap." : "You found a poison dart trap.";
	when POOL:
	    s = terse ? "A shimmering pool." : "You found a shimmering pool";
	when MAZETRAP:
	    s = terse ? "A maze entrance." : "You found a maze entrance";
    }
    return s;
}

/*
 * Look:
 *	A quick glance all around the player
 */

look(wakeup)
bool wakeup;
{
    register int x, y;
    register char ch, och;
    register int oldx, oldy;
    register bool inpass, horiz, vert, do_light = FALSE, do_blank = FALSE;
    register int passcount = 0;
    register struct room *rp;
    register int ey, ex;

    /* Are we moving vertically or horizontally? */
    if (runch == 'h' || runch == 'l') horiz = TRUE;
    else horiz = FALSE;
    if (runch == 'j' || runch == 'k') vert = TRUE;
    else vert = FALSE;

    getyx(cw, oldy, oldx);	/* Save current position */

    /* Blank out the floor around our last position and check for
     * moving out of a corridor in a maze.
     */
    if (oldrp != NULL && (oldrp->r_flags & ISDARK) &&
	!(oldrp->r_flags & HASFIRE) && off(player, ISBLIND))
	do_blank = TRUE;
    for (x = player.t_oldpos.x - 1; x <= player.t_oldpos.x + 1; x++)
	for (y = player.t_oldpos.y - 1; y <= player.t_oldpos.y + 1; y++) {
	    ch = show(y, x);
	    if (do_blank && (y != hero.y || x != hero.x) && ch == FLOOR)
		mvwaddch(cw, y, x, ' ');
		
	    /* Moving out of a corridor? */
	    if (levtype == MAZELEV &&
		(ch != '|' && ch != '-') &&      /* Not a wall */
		((vert && x != player.t_oldpos.x && y==player.t_oldpos.y) ||
		 (horiz && y != player.t_oldpos.y && x==player.t_oldpos.x)))
		do_light = TRUE;	/* Just came to a turn */
	}

    inpass = ((rp = roomin(&hero)) == NULL); /* Are we in a passage? */

    /* Are we coming out of a wall into a corridor in a maze? */
    och = show(player.t_oldpos.y, player.t_oldpos.x);
    ch = show(hero.y, hero.x);
    if (levtype == MAZELEV && (och == '|' || och == '-' || och == SECRETDOOR) &&
	(ch != '|' && ch != '-' && ch != SECRETDOOR))
	do_light = off(player, ISBLIND); /* Light it up if not blind */

    /* Look around the player */
    ey = hero.y + 1;
    ex = hero.x + 1;
    for (x = hero.x - 1; x <= ex; x++)
	if (x >= 0 && x < COLS) for (y = hero.y - 1; y <= ey; y++)
	{
	    if (y <= 0 || y >= LINES - 2)
		continue;
	    if (isalpha(mvwinch(mw, y, x)))
	    {
		register struct linked_list *it;
		register struct thing *tp;

		if (wakeup)
		    it = wake_monster(y, x);
		else
		    it = find_mons(y, x);
		tp = (struct thing *) ldata(it);
		tp->t_oldch = mvinch(y, x);
		if (isatrap(tp->t_oldch)) {
		    register struct trap *trp = trap_at(y, x);

		    tp->t_oldch = (trp->tr_flags & ISFOUND) ? tp->t_oldch
							    : trp->tr_show;
		}
		if (tp->t_oldch == FLOOR && (rp->r_flags & ISDARK)
		    && !(rp->r_flags & HASFIRE) && off(player, ISBLIND))
			tp->t_oldch = ' ';
	    }

	    /*
	     * Secret doors show as walls
	     */
	    if ((ch = show(y, x)) == SECRETDOOR)
		ch = secretdoor(y, x);
	    /*
	     * Don't show room walls if he is in a passage and
	     * check for maze turns
	     */
	    if (off(player, ISBLIND))
	    {
		if (y == hero.y && x == hero.x
		    || (inpass && (ch == '-' || ch == '|')))
			continue;

		/* Are we at a crossroads in a maze? */
		if (levtype == MAZELEV &&
		    (ch != '|' && ch != '-') &&      /* Not a wall */
		    ((vert && x != hero.x && y == hero.y) ||
		     (horiz && y != hero.y && x == hero.x)))
		    do_light = TRUE;	/* Just came to a turn */
	    }
	    else if (y != hero.y || x != hero.x)
		continue;

	    wmove(cw, y, x);
	    waddch(cw, ch);
	    if (door_stop && !firstmove && running)
	    {
		switch (runch)
		{
		    when 'h':
			if (x == ex)
			    continue;
		    when 'j':
			if (y == hero.y - 1)
			    continue;
		    when 'k':
			if (y == ey)
			    continue;
		    when 'l':
			if (x == hero.x - 1)
			    continue;
		    when 'y':
			if ((x + y) - (hero.x + hero.y) >= 1)
			    continue;
		    when 'u':
			if ((y - x) - (hero.y - hero.x) >= 1)
			    continue;
		    when 'n':
			if ((x + y) - (hero.x + hero.y) <= -1)
			    continue;
		    when 'b':
			if ((y - x) - (hero.y - hero.x) <= -1)
			    continue;
		}
		switch (ch)
		{
		    case DOOR:
			if (x == hero.x || y == hero.y)
			    running = FALSE;
			break;
		    case PASSAGE:
			if (x == hero.x || y == hero.y)
			    passcount++;
			break;
		    case FLOOR:
			/* Stop by new passages in a maze (floor next to us) */
			if ((levtype == MAZELEV) &&
			    ((horiz && x == hero.x && y != hero.y) ||
			     (vert && y == hero.y && x != hero.x)))
				running = FALSE;
		    case '|':
		    case '-':
		    case ' ':
			break;
		    default:
			running = FALSE;
			break;
		}
	    }
	}
    if (door_stop && !firstmove && passcount > 1)
	running = FALSE;

    /* Do we have to light up the area (just stepped into a new corridor)? */
    if (do_light && wakeup && 	/* wakeup will be true on a normal move */
	!(rp->r_flags & ISDARK) &&   /* We have some light */
	!ce(hero, player.t_oldpos))  /* Don't do anything if we didn't move */
	    light(&hero);

    mvwaddch(cw, hero.y, hero.x, PLAYER);
    wmove(cw, oldy, oldx);
    if (wakeup) {
	player.t_oldpos = hero; /* Don't change if we didn't move */
	oldrp = rp;
    }
}

/*
 * secret_door:
 *	Figure out what a secret door looks like.
 */

secretdoor(y, x)
register int y, x;
{
    register int i;
    register struct room *rp;
    register coord *cpp;
    static coord cp;

    cp.y = y;
    cp.x = x;
    cpp = &cp;
    for (rp = rooms, i = 0; i < MAXROOMS; rp++, i++)
	if (inroom(rp, cpp))
	    if (y == rp->r_pos.y || y == rp->r_pos.y + rp->r_max.y - 1)
		return('-');
	    else
		return('|');

    return('p');
}

/*
 * find_obj:
 *	find the unclaimed object at y, x
 */

struct linked_list *
find_obj(y, x)
register int y;
int x;
{
    register struct linked_list *obj;
    register struct object *op;

    for (obj = lvl_obj; obj != NULL; obj = next(obj))
    {
	op = (struct object *) ldata(obj);
	if (op->o_pos.y == y && op->o_pos.x == x)
		return obj;
    }
    return NULL;
}

/*
 * eat:
 *	He wants to eat something, so let him try
 */

eat()
{
    register struct linked_list *item;
    register struct object *obj;
    unsigned long exp;
    int which;

    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    which = obj->o_which;
    del_pack(item);
    if (which == 1)
	msg("My, that was a yummy %s", fruit);
    else
	if (rnd(100) > 70)
	{
	    msg("Yuk, this food tastes awful");
	    exp = pstats.s_exp;
	    pstats.s_exp++;
	    if (exp > pstats.s_exp)
		pstats.s_exp - exp;
	    check_level();
	}
	else
	    msg("Yum, that tasted good");
    if ((food_left += HUNGERTIME + rnd(400) - 200) > STOMACHSIZE)
	food_left = STOMACHSIZE;
    hungry_state = F_OK;
    updpack(TRUE);
    if (obj == cur_weapon)
	cur_weapon = NULL;
}

/*
 * Used to modify the playes strength
 * it keeps track of the highest it has been, just in case
 */

chg_str(amt)
register int amt;
{
    register int ring_str;		/* ring strengths */
    register struct stats *ptr;		/* for speed */

    ptr = &pstats;
    ring_str = ring_value(R_ADDSTR);
    ptr->s_str -= ring_str;
    ptr->s_str += amt;
    if (ptr->s_str < 3)
	ptr->s_str = 3;
    if (ptr->s_str > 25)
	ptr->s_str = 25;
    if (ptr->s_str > max_stats.s_str)
	max_stats.s_str = ptr->s_str;
    ptr->s_str += ring_str;
    if (ptr->s_str < 0)
	ptr->s_str = 0;
    updpack(TRUE);
}

/*
 * add_haste:
 *	add a haste to the player
 */

add_haste(blessed)
bool blessed;
{
    short hasttime;

    if (blessed) hasttime = 10;
    else hasttime = 6;

    if (on(player, ISSLOW)) { /* Is person slow? */
	extinguish(noslow);
	noslow();

	if (blessed) hasttime = 4;
	else return;
    }

    if (on(player, ISHASTE)) {
	msg("You faint from exhaustion.");
	no_command += rnd(hasttime);
	lengthen(nohaste, rnd(hasttime) + hasttime);
    }
    else {
	turn_on(player, ISHASTE);
	fuse(nohaste, 0, roll(hasttime, hasttime), AFTER);
    }
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */

aggravate()
{
    register struct linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
	runto(&((struct thing *) ldata(mi))->t_pos, &hero);
}

/*
 * for printfs: if string starts with a vowel, return "n" for an "an"
 */
char *
vowelstr(str)
register char *str;
{
    switch (*str)
    {
	case 'a':
	case 'e':
	case 'i':
	case 'o':
	case 'u':
	    return "n";
	default:
	    return "";
    }
}

/* 
 * see if the object is one of the currently used items
 */
is_current(obj)
register struct object *obj;
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon || 
	obj == cur_ring[LEFT_1] || obj == cur_ring[LEFT_2] ||
	obj == cur_ring[LEFT_3] || obj == cur_ring[LEFT_4] ||
	obj == cur_ring[RIGHT_1] || obj == cur_ring[RIGHT_2] ||
	obj == cur_ring[RIGHT_3] || obj == cur_ring[RIGHT_4]) {
	msg(terse ? "In use." : "That's already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
get_dir()
{
    register char *prompt;
    register bool gotit;

    prompt = terse ? "Direction?" :  "Which direction? ";
    msg(prompt);
    do
    {
	gotit = TRUE;
	switch (readchar())
	{
	    when 'h': case'H': delta.y =  0; delta.x = -1;
	    when 'j': case'J': delta.y =  1; delta.x =  0;
	    when 'k': case'K': delta.y = -1; delta.x =  0;
	    when 'l': case'L': delta.y =  0; delta.x =  1;
	    when 'y': case'Y': delta.y = -1; delta.x = -1;
	    when 'u': case'U': delta.y = -1; delta.x =  1;
	    when 'b': case'B': delta.y =  1; delta.x = -1;
	    when 'n': case'N': delta.y =  1; delta.x =  1;
	    when ESCAPE: return FALSE;
	    otherwise:
		mpos = 0;
		msg(prompt);
		gotit = FALSE;
	}
    } until (gotit);
    if (on(player, ISHUH) && rnd(100) > 80)
	do
	{
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}


/*
 * Maze_view:
 *	Returns true if the player can see the specified location within
 *	the confines of a maze (within one column or row)
 */

bool
maze_view(y, x)
int y, x;
{
    register int start, goal, delta, ycheck, xcheck, absy, absx;
    register bool row;

    /* Get the absolute value of y and x differences */
    absy = hero.y - y;
    absx = hero.x - x;
    if (absy < 0) absy = -absy;
    if (absx < 0) absx = -absx;

    /* Must be within one row or column */
    if (absy > 1 && absx > 1) return(FALSE);

    if (absy <= 1) {		/* Go along row */
	start = hero.x;
	goal = x;
	row = TRUE;
	ycheck = hero.y;
    }
    else {			/* Go along column */
	start = hero.y;
	goal = y;
	row = FALSE;
	xcheck = hero.x;
    }
    if (start <= goal) delta = 1;
    else delta = -1;

    while (start != goal) {
	if (row) xcheck = start;
	else ycheck = start;
	switch (winat(ycheck, xcheck)) {
	    case '|':
	    case '-':
	    case WALL:
	    case DOOR:
	    case SECRETDOOR:
		return(FALSE);
	}
	start += delta;
    }
    return(TRUE);
}
