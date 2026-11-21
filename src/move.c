/*
 * Hero movement commands
 *
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/*
 * Used to hold the new hero position
 */

coord nh;

/*
 * do_run:
 *	Start the hero running
 */

do_run(ch)
char ch;
{
    running = TRUE;
    after = FALSE;
    runch = ch;
}

/*
 * corr_move:
 *	Check to see that a move is legal.  If so, return correct character.
 * If not, if player came from a legal place, then try to turn him.
 */

corr_move(dy, dx)
int dy, dx;
{
    register char ch;
    short legal=0;		/* Number of legal alternatives */
    int y, x;			/* Holds legal new position */
    register int *ny, *nx;	/* Point to which direction to change */

    /* New position */
    nh.y = hero.y + dy;
    nh.x = hero.x + dx;

    /* A bad diagonal move is won't change his move */
    if (!diag_ok(&hero, &nh, &player)) return;

    /* If it is a legal move, just return */
    if (nh.x >= 0 && nh.x < COLS && nh.y > 0 && nh.y < LINES - 2) {
        ch = winat(nh.y, nh.x);
	switch (ch) {
	    case ' ':
	    case '|':
	    case '-':
		break;
	    default:
		return;
	}
    }

    /* Check the legal alternatives */
    if (dy == 0) {
	ny = &dy;
	nx = &dx;
    }
    else {
	ny = &dx;
	nx = &dy;
    }

    for (*nx = 0, *ny = -1; *ny < 2; *ny += 2) {
    	/* New position */
    	nh.y = hero.y + dy;
    	nh.x = hero.x + dx;

	if (nh.x < 0 || nh.x > COLS - 1 || nh.y < 1 || nh.y > LINES - 3)
	    continue;
	ch = winat(nh.y, nh.x);
	switch (ch) {
	    case ' ':
	    case '|':
	    case '-':
		break;
	    default:
		legal++;
		y = dy;
		x = dx;
	}
    }

    /* If we have 2 legal moves, make no change */
    if (legal != 1) return;

    /* Make the change */
    if (y == 0)	{	/* Move horizontally */
	if (x == 1) runch = 'l';
	else runch = 'h';
    }
    else {	/* Move vertically */
	if (y == 1) runch = 'j';
	else runch = 'k';
    }
    return;
}
	    

/*
 * do_move:
 *	Check to see that a move is legal.  If it is handle the
 * consequences (fighting, picking up, etc.)
 */

do_move(dy, dx)
int dy, dx;
{
    register char ch;
    coord old_hero;
    char hch;

    firstmove = FALSE;
    curprice = -1;		/* if in trading post, we've moved off obj */
    if (player.t_no_move) {
	player.t_no_move--;
	msg("You are still stuck in the bear trap");
	return;
    }
    /*
     * Do a confused move (maybe)
     */
    if ((rnd(100) < 80 && on(player, ISHUH)) || 
	(ISWEARING(R_DELUSION) && rnd(100) < 25))
	nh = *rndmove(&player);
    else {
	nh.y = hero.y + dy;
	nh.x = hero.x + dx;
    }

    /*
     * Check if he tried to move off the screen or make an illegal
     * diagonal move, and stop him if he did.
     */
    if (nh.x < 0 || nh.x > COLS-1 || nh.y < 1 || nh.y >= LINES - 2
	|| !diag_ok(&hero, &nh, &player))
    {
	after = running = FALSE;
	return;
    }
    if (running && ce(hero, nh))
	after = running = FALSE;
    ch = winat(nh.y, nh.x);

    /* Take care of hero trying to move close to something frightening */
    if (on(player, ISFLEE)) {
	if (rnd(10) < 1) {
	    turn_off(player, ISFLEE);
	    msg("You regain your composure.");
	}
	else if (DISTANCE(nh.y, nh.x, player.t_dest->y, player.t_dest->x) <
		 DISTANCE(hero.y, hero.x, player.t_dest->y, player.t_dest->x))
			return;
    }

    /* Take care of hero being held */
    if (on(player, ISHELD) && !isalpha(ch))
    {
	msg("You are being held");
	running = FALSE;
	return;
    }

    /* assume he's not in a wall */
    if (!isalpha(ch)) turn_off(player, ISINWALL);

    switch(ch) {
	case ' ':
	case '|':
	case '-':
	case SECRETDOOR:
	    if (off(player, CANINWALL)) {
	        after = running = FALSE;
	        return;
	    }
	    else if (running) {
	         after = running = FALSE;
	         return;
	    }	
	    turn_on(player, ISINWALL);
	    break;
	case TRAPDOOR:
	case TELTRAP:
	case BEARTRAP:
	case SLEEPTRAP:
	case ARROWTRAP:
	case DARTTRAP:
	case POOL:
	case MAZETRAP:
	    ch = be_trapped(&player, &nh);
	    if (ch == TRAPDOOR || ch == TELTRAP || 
		pool_teleport  || ch == MAZETRAP) {
		pool_teleport = FALSE;
		return;
	    }
	    break;
	case GOLD:
	case POTION:
	case SCROLL:
	case FOOD:
	case WEAPON:
	case ARMOR:
	case RING:
	case AMULET:
	case STICK:
	    running = FALSE;
	    take = ch;
	    break;
	default:
	    break;
    }

    hch = winat(hero.y, hero.x); /* Where hero was */
    old_hero = hero;	/* Save hero's old position */
    hero = nh;	/* Move the hero */

    if ((ch==PASSAGE || ch==WALL) && (hch=='-' || hch=='|' || hch==DOOR)) {
	/* Leaving a room -- darken it */
	register struct room *rp = roomin(&old_hero);
	bool is_lit = FALSE;

	if (!(rp->r_flags & ISDARK)) is_lit = TRUE;
	rp->r_flags |= ISDARK;	/* Fake darkness */
	light(&old_hero);
	if (is_lit) rp->r_flags &= ~ISDARK; /* Restore light state */
    }
    else if (ch == DOOR || ch == '|' || ch == '-') { /* Entering a room */
	running = FALSE;
	if (hch == PASSAGE || hch == WALL)
	    light(&hero);	/* knows whether the hero can see things in */
    }
    else if (ch == STAIRS)
	running = FALSE;
    else if (ch == POST) {
	running = FALSE;
	new_level(POSTLEV);
	return;
    }
    else if (isalpha(ch)) {
	running = FALSE;
	hero = old_hero;    /* Restore hero -- we'll fight instead of move */
	fight(&nh, ch, cur_weapon, FALSE);
	return;
    }
    ch = winat(old_hero.y, old_hero.x);
    wmove(cw, unc(old_hero));
    waddch(cw, ch);
    wmove(cw, unc(hero));
    waddch(cw, PLAYER);
}

/*
 * Called to illuminate a room.
 * If it is dark, remove anything that might move.
 */

light(cp)
coord *cp;
{
    register struct room *rp;
    register int j, k, x, y;
    register char ch, rch;
    register struct linked_list *item;
    int jlow, jhigh, klow, khigh;	/* Boundaries of lit area */

    if ((rp = roomin(cp)) != NULL && !on(player, ISBLIND)) {
	/*
	 * is he wearing ring of illumination? 
	 */
	if (ISWEARING(R_LIGHT))
	    rp->r_flags &= ~ISDARK;
	
	/* If we are in a maze, don't look at the whole room (level) */
	if (levtype == MAZELEV) {
	    jlow = max(0, hero.y - 2 - rp->r_pos.y);
	    jhigh = min(rp->r_max.y, hero.y + 2 - rp->r_pos.y + 1);
	    klow = max(0, hero.x - 2 - rp->r_pos.x);
	    khigh = min(rp->r_max.x, hero.x + 2 - rp->r_pos.x + 1);
	}
	else {
	    jlow = klow = 0;
	    jhigh = rp->r_max.y;
	    khigh = rp->r_max.x;
	}
	for (j = 0; j < rp->r_max.y; j++)
	{
	    for (k = 0; k < rp->r_max.x; k++)
	    {
		/* Is this in the give area -- needed for maze */
		if ((j < jlow || j >= jhigh) && (k < klow || k >= khigh))
		    continue;

		y = rp->r_pos.y + j;
		x = rp->r_pos.x + k;

		ch = show(y, x);
		wmove(cw, y, x);
		/*
		 * Figure out how to display a secret door
		 */
		if (ch == SECRETDOOR)
		{
		    if (j == 0 || j == rp->r_max.y - 1)
			ch = '-';
		    else
			ch = '|';
		}
		/* For monsters, if they were previously not seen and
		 * now can be seen, or vice-versa, make sure that will
		 * happen.
		 */
		if (isalpha(ch))
		{
		    struct thing *tp;	/* The monster */

		    item = wake_monster(y, x);
		    tp = THINGPTR(item);

		    /* Previously not seen -- now can see it */
		    if (tp->t_oldch == ' ' && cansee(tp->t_pos.y, tp->t_pos.x)) 
			tp->t_oldch = mvinch(y, x);

		    /* Previously seen -- now can't see it */
		    else if (off(player, ISBLIND) && tp->t_oldch != ' ' &&
			     !cansee(tp->t_pos.y, tp->t_pos.x))
			tp->t_oldch = ' ';
		}

		/*
		 * If the room is a dark room, we might want to remove
		 * monsters and the like from it (since they might
		 * move).
		 * A dark room or not in line-of-sight in a maze.
		 */
		if (((rp->r_flags & ISDARK) && !(rp->r_flags & HASFIRE)) ||
		    (levtype == MAZELEV &&
		     !maze_view(y, x))) {
		    rch = mvwinch(cw, y, x);
		    switch (rch)
		    {
			when DOOR:
			case STAIRS:
			case TRAPDOOR:
			case TELTRAP:
			case BEARTRAP:
			case SLEEPTRAP:
			case ARROWTRAP:
			case DARTTRAP:
			case POOL:
			case MAZETRAP:
			case '|':
			case '-':
			case ' ':
			    ch = rch;
			when FLOOR:
			    ch = (on(player, ISBLIND) ? FLOOR : ' ');
			otherwise:
			    ch = ' ';
		    }
		}
		mvwaddch(cw, y, x, ch);
	    }
	}
    }
}

/*
 * blue_light:
 *	magically light up a room (or level or make it dark)
 */

bool blue_light(blessed, cursed)
bool blessed, cursed;
{
    register struct room *rp;
    bool ret_val=FALSE;	/* Whether or not affect is known */

    rp = roomin(&hero);	/* What room is hero in? */

    /* Darken the room if the magic is cursed */
    if (cursed) {
	if ((rp == NULL) || (rp->r_flags & ISDARK)) msg("Nothing happens.");
	else {
	    if (!(rp->r_flags & HASFIRE)) msg("The room suddenly goes dark.");
	    else msg("Nothing happens.");
	    rp->r_flags |= ISDARK;
	    ret_val = TRUE;
	}
    }
    else {
	ret_val = TRUE;
	if (rp && (rp->r_flags & ISDARK) && !(rp->r_flags & HASFIRE)) {
	    addmsg("The room is lit");
	    if (!terse)
		addmsg(" by a %s blue light.",
		    blessed ? "bright" : "shimmering");
	    endmsg();
	}
	else if (winat(hero.y, hero.x) == PASSAGE)
	    msg("The corridor glows %sand then fades",
		    blessed ? "brightly " : "");
	else {
	    ret_val = FALSE;
	    msg("Nothing happens.");
	}
	if (blessed) {
	    short i;	/* Index through rooms */

	    for (i=0; i<MAXROOMS; i++)
		rooms[i].r_flags &= ~ISDARK;
	}
	else if (rp) rp->r_flags &= ~ISDARK;
    }

    /*
     * Light the room and put the player back up
     */
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    return(ret_val);
}

/*
 * show:
 *	returns what a certain thing will display as to the un-initiated
 */

show(y, x)
register int y, x;
{
    register char ch = winat(y, x);
    register struct linked_list *it;
    register struct thing *tp;

    if (isatrap(ch)) {
	register struct trap *trp = trap_at(y, x);

	return (trp->tr_flags & ISFOUND) ? ch : trp->tr_show;
    }
    else if (isalpha(ch)) {
	if ((it = find_mons(y, x)) == NULL)
	    msg("Can't find monster in show");
	tp = (struct thing *) ldata(it);

	if (on(*tp, ISDISGUISE)) ch = tp->t_disguise; /* As a mimic */
	else if (on(*tp, ISINVIS) || (on(*tp, ISSHADOW) && rnd(100) < 90) ||
		 on(*tp, CANSURPRISE)) {
	    if (off(player, CANSEE) || on(*tp, CANSURPRISE))
		ch = mvwinch(stdscr, y, x); /* Invisible */
	}
	else if (on(*tp, CANINWALL)) {
	    if (isrock(mvwinch(stdscr, y, x))) ch = winch(stdscr); /* As Xorn */
	}
    }
    return ch;
}

/*
 * be_trapped:
 *	The guy stepped on a trap.... Make him pay.
 */

be_trapped(th, tc)
register struct thing *th;
register coord *tc;
{
    register struct trap *tp;
    register char ch, *mname;
    register bool is_player = (th == &player),
	          can_see = cansee(tc->y, tc->x);
    register struct linked_list *mitem;


    tp = trap_at(tc->y, tc->x);
    tp->tr_flags |= ISFOUND;

    if (!is_player) {
	mitem = find_mons(th->t_pos.y, th->t_pos.x);
	mname = monsters[th->t_index].m_name;
    }
    else {
	count = running = FALSE;
	mvwaddch(cw, tp->tr_pos.y, tp->tr_pos.x, tp->tr_type);
    }
    switch (ch = tp->tr_type) {
	when TRAPDOOR:
	    if (is_player) {
		level++;
		new_level(NORMLEV);
		msg("You fell into a trap!");
	    }
	    else {
		if (can_see) msg("The %s fell into a trap!", mname);
		check_residue(th);
		remove_mon(&th->t_pos, mitem);
	    }
	when BEARTRAP:
	    if (is_stealth(th)) {
		if (is_player) msg("You pass a bear trap.");
		else if (can_see) msg("The %s passes a bear trap.", mname);
	    }
	    else {
		th->t_no_move += BEARTIME;
		if (is_player) msg("You are caught in a bear trap.");
		else if (can_see) msg("The %s is caught in a bear trap.",
					mname);
	    }
	when SLEEPTRAP:
	    if (is_player) {
		msg("A strange white mist envelops you.");
		if (!ISWEARING(R_ALERT)) {
		    msg("You fall asleep.");
		    no_command += SLEEPTIME;
		}
	    }
	    else {
		if (can_see) 
		    msg("A strange white mist envelops the %s.",mname);
		if (on(*th, ISUNDEAD)) {
		    if (can_see) 
			msg("The mist doesn't seem to affect the %s.",mname);
		}
		else {
		    th->t_no_move += SLEEPTIME;
		}
	    }
	when ARROWTRAP:
	    if (swing(th->t_ctype, th->t_stats.s_lvl-1, th->t_stats.s_arm, 1))
	    {
		if (is_player) {
		    msg("Oh no! An arrow shot you.");
		    if ((pstats.s_hpt -= roll(1, 6)) <= 0) {
			msg("The arrow killed you.");
			death(D_ARROW);
		    }
		}
		else {
		    if (can_see) msg("An arrow shot the %s.", mname);
		    if ((th->t_stats.s_hpt -= roll(1, 6)) <= 0) {
			if (can_see) msg("The arrow killed the %s.", mname);
			killed(mitem, FALSE, FALSE);
		    }
		}
	    }
	    else
	    {
		register struct linked_list *item;
		register struct object *arrow;

		if (is_player) msg("An arrow shoots past you.");
		else if (can_see) msg("An arrow shoots by the %s.", mname);
		item = new_item(sizeof *arrow);
		arrow = (struct object *) ldata(item);
		arrow->o_type = WEAPON;
		arrow->o_which = ARROW;
		arrow->o_hplus = rnd(3) - 1;
		arrow->o_dplus = rnd(3) - 1;
		init_weapon(arrow, ARROW);
		arrow->o_count = 1;
		arrow->o_pos = *tc;
		arrow->o_mark[0] = '\0';
		fall(item, FALSE);
	    }
	when TELTRAP:
	    if (is_player) teleport();
	    else {
		register int rm;

		/* Erase the monster from the old position */
		if (isalpha(mvwinch(cw, th->t_pos.y, th->t_pos.x)))
		    mvwaddch(cw, th->t_pos.y, th->t_pos.x, th->t_oldch);
		mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');

		/* Get a new position */
		do {
		    rm = rnd_room();
		    rnd_pos(&rooms[rm], &th->t_pos);
		} until(winat(th->t_pos.y, th->t_pos.x) == FLOOR);

		/* Put it there */
		mvwaddch(mw, th->t_pos.y, th->t_pos.x, th->t_type);
		th->t_oldch = mvwinch(cw, th->t_pos.y, th->t_pos.x);
		if (can_see) msg("The %s seems to have disappeared!", mname);
	    }
	when DARTTRAP:
	    if (swing(th->t_ctype, th->t_stats.s_lvl+1, th->t_stats.s_arm, 1)) {
		if (is_player) {
		    msg("A small dart just hit you in the shoulder.");
		    if ((pstats.s_hpt -= roll(1, 4)) <= 0) {
			msg("The dart killed you.");
			death(D_DART);
		    }

		    /* Now the poison */
		    if (!save(VS_POISON)) {
			/* 75% chance it will do point damage - else strength */
			if (rnd(100) < 75) {
			    pstats.s_hpt /= 2;
			    if (pstats.s_hpt == 0) death(D_POISON);
			}
			else if (!ISWEARING(R_SUSABILITY))
				chg_str(-1);
		    }
		}
		else {
		    if (can_see)
			msg("A small dart just hit the %s in the shoulder.",
				mname);
		    if ((th->t_stats.s_hpt -= roll(1,4)) <= 0) {
			if (can_see) msg("The dart killed the %s.", mname);
			killed(mitem, FALSE, FALSE);
		    }
		}
	    }
	    else {
		if (is_player)
		    msg("A small dart whizzes by your ear and vanishes.");
		else if (can_see)
		    msg("A small dart whizzes by the %s's ear and vanishes.",
			mname);
	    }
        when POOL: {
	    register int i;

	    i = rnd(100);
	    if (is_player) {
		if ((tp->tr_flags & ISGONE)) {
		    if (i < 30) {
			teleport();	   /* teleport away */
			pool_teleport = TRUE;
		    }
		    else if((i < 45) && level > 2) {
			level -= rnd(2) + 1;
			new_level(NORMLEV);
			pool_teleport = TRUE;
			msg("You here a faint groan from below.");
		    }
		    else if(i < 70) {
			level += rnd(4) + 1;
			new_level(NORMLEV);
			pool_teleport = TRUE;
			msg("You find yourself in strange surroundings.");
		    }
		    else if(i > 95) {
			msg("Oh no!!! You drown in the pool!!! --More--");
			wait_for(' ');
			death(D_DROWN);
		    }
		}
	    }
	    else {
		if (i < 30) {
		    if (can_see) msg("The %s drowned in the pool!", mname);
		    killed(mitem, FALSE, FALSE);
		}
	    }
	}
    when MAZETRAP:
	if (is_player) {
	    level++;
	    new_level(MAZELEV);
	    msg("You are surrounded by twisty passages!");
	}
	else {
	    if (can_see) msg("The %s fell into a trap!", mname);
	    check_residue(th);
	    remove_mon(&th->t_pos, mitem);
	}
    }
    flushout();		/* flush typeahead */
    return(ch);
}

/*
 * dip_it:
 *	Dip an object into a magic pool
 */
dip_it()
{
	reg struct linked_list *what;
	reg struct object *ob;
	reg struct trap *tp;
	reg int wh, i;

	tp = trap_at(hero.y,hero.x);
	if (tp == NULL || tp->tr_type != POOL) {
	    msg("I see no shimmering pool here");
	    return;
	}
	if (tp->tr_flags & ISGONE) {
	    msg("This shimmering pool appears to have used once already");
	    return;
	}
	if ((what = get_item("dip",0)) == NULL) {
	    msg("");
	    after = FALSE;
	    return;
	}
	ob = OBJPTR(what);
	mpos = 0;
	if (ob == cur_armor) {
	    msg("You have to take off your armor before you can dip it.");
	    return;
	}
	else if(ob == cur_ring[LEFT_1] || ob == cur_ring[LEFT_2] ||
		ob == cur_ring[LEFT_3] || ob == cur_ring[LEFT_4] ||
		ob == cur_ring[RIGHT_1] || ob == cur_ring[RIGHT_2] ||
		ob == cur_ring[RIGHT_3] || ob == cur_ring[RIGHT_4]) {
	    msg("You have to take that ring off before you can dip it.");
	    return;
	}
	tp->tr_flags |= ISGONE;
	if (ob != NULL) {
	    wh = ob->o_which;
	    ob->o_flags |= ISKNOW;
	    i = rnd(100);
	    switch(ob->o_type) {
		case WEAPON:
		    if(i < 50) {		/* enchant weapon here */
			if ((ob->o_flags & ISCURSED) == NULL) {
				ob->o_hplus += 1;
				ob->o_dplus += 1;
			}
			else {		/* weapon was prev cursed here */
				ob->o_hplus = rnd(2);
				ob->o_dplus = rnd(2);
			}
			ob->o_flags &= ~ISCURSED;
		        msg("The %s glows blue for a moment.",weaps[wh].w_name);
		    }
		    else if(i < 70) {	/* curse weapon here */
			if ((ob->o_flags & ISCURSED) == NULL) {
				ob->o_hplus = -(rnd(2)+1);
				ob->o_dplus = -(rnd(2)+1);
			}
			else {			/* if already cursed */
				ob->o_hplus--;
				ob->o_dplus--;
			}
			ob->o_flags |= ISCURSED;
		        msg("The %s glows red for a moment.",weaps[wh].w_name);
		    }			
		    else
			msg("Nothing seems to happen");
		when ARMOR:
		    if (i < 50) {	/* enchant armor */
			if((ob->o_flags & ISCURSED) == NULL)
			    ob->o_ac -= rnd(2) + 1;
			else
			    ob->o_ac = -rnd(3)+ armors[wh].a_class;
			ob->o_flags &= ~ISCURSED;
		        msg("The %s glows blue for a moment",armors[wh].a_name);
		    }
		    else if(i < 75){	/* curse armor */
			if ((ob->o_flags & ISCURSED) == NULL)
			    ob->o_ac = rnd(3)+ armors[wh].a_class;
			else
			    ob->o_ac += rnd(2) + 1;
			ob->o_flags |= ISCURSED;
		        msg("The %s glows red for a moment.",armors[wh].a_name);
		    }
		    else
			msg("Nothing seems to happen");
		when STICK: {
		    int j;
		    j = rnd(8) + 1;
		    if(i < 50) {		/* add charges */
			ob->o_charges += j;
		        ws_know[wh] = TRUE;
			if (ob->o_flags & ISCURSED)
			    ob->o_flags &= ~ISCURSED;
		        msg("The %s %s glows blue for a moment.",
			    ws_made[wh],ws_type[wh]);
		    }
		    else if(i < 65) {	/* remove charges */
			if ((ob->o_charges -= i) < 0)
			    ob->o_charges = 0;
		        ws_know[wh] = TRUE;
			if (ob->o_flags & ISBLESSED)
			    ob->o_flags &= ~ISBLESSED;
			else
			    ob->o_flags |= ISCURSED;
		        msg("The %s %s glows red for a moment.",
			    ws_made[wh],ws_type[wh]);
		    }
		    else 
			msg("Nothing seems to happen");
		}
		when SCROLL:
		    s_know[wh] = TRUE;
		    msg("The '%s' scroll unfurls.",s_names[wh]);
		when POTION:
		    p_know[wh] = TRUE;
		    msg("The %s potion bubbles for a moment.",p_colors[wh]);
		when RING:
		    if(i < 50) {	 /* enchant ring */
			if ((ob->o_flags & ISCURSED) == NULL)
			    ob->o_ac += rnd(2) + 1;
			else
			    ob->o_ac = rnd(2) + 1;
			ob->o_flags &= ~ISCURSED;
		    }
		    else if(i < 80) { /* curse ring */
			if ((ob->o_flags & ISCURSED) == NULL)
			    ob->o_ac = -(rnd(2) + 1);
			else
			    ob->o_ac -= (rnd(2) + 1);
			ob->o_flags |= ISCURSED;
		    }
		    r_know[wh] = TRUE;
		    msg("The %s ring vibrates for a moment.",r_stones[wh]);
		otherwise:
		    msg("The pool bubbles for a moment.");
	    }
	}
	else
	    msg("nothing seems to happen");
}

/*
 * trap_at:
 *	find the trap at (y,x) on screen.
 */

struct trap *
trap_at(y, x)
register int y, x;
{
    register struct trap *tp, *ep;

    ep = &traps[ntraps];
    for (tp = traps; tp < ep; tp++)
	if (tp->tr_pos.y == y && tp->tr_pos.x == x)
	    break;
    if (tp == ep)
	debug((sprintf(prbuf, "Trap at %d,%d not in array", y, x), prbuf));
    return tp;
}

/*
 * set_trap:
 *	set a trap at (y, x) on screen.
 */

set_trap(tp, y, x)
register struct thing *tp;
register int y, x;
 {
    register bool is_player = (tp == &player);
    register int selection = rnd(7) + 1;
    register char ch, och;
    char thief_bonus = 0;

    switch (och = mvinch(y, x)) {
	case WALL:
	case FLOOR:
	case PASSAGE:
	    break;
	default:
	    msg("The trap failed!");
	    return;
    }

    if (is_player && player.t_ctype == C_THIEF) thief_bonus = 10;

    if (ntraps >= MAXTRAPS || ++trap_tries >= MAXTRPTRY || levtype == POSTLEV ||
	rnd(50) >= (tp->t_stats.s_dext + tp->t_stats.s_lvl/2 + thief_bonus)) {
	if (is_player) msg("The trap failed!");
	return;
    }

    /* Set up for redraw */
    clearok(cw, TRUE);
    touchwin(cw);

    if (is_player) {
	wclear(hw);
	touchwin(hw);
	mvwaddstr(hw, 2, 0,
	    "[1] Trap Door\n[2] Bear Trap\n[3] Sleep Trap\n[4] Arrow Trap\n");
	waddstr(hw, "[5] Teleport Trap\n[6] Dart Trap");
	if (wizard) {
		waddstr(hw, "\n[7] Shimmering Pool\n[8] Trading Post");
	}
	mvwaddstr(hw, 0, 0, "Which kind of trap do you wish to set? ");
	draw(hw);

	selection = (short) (readchar() - '0');
	while (selection < 1 || selection > 6) {
	    if (wizard && selection < 9)
		break;
	    if (selection == (short) ESCAPE - (short) '0') {
		after = FALSE;
		return;
	    }
	    mvwaddstr(hw, 0, 0, "Please enter a selection between 1 and 6:  ");
	    draw(hw);
	    selection = (short) (readchar() - '0');
	}
    }

    switch (selection) {
	when 1: ch = TRAPDOOR;
	when 2: ch = BEARTRAP;
	when 3: ch = SLEEPTRAP;
	when 4: ch = ARROWTRAP;
	when 5: ch = TELTRAP;
	when 6: ch = DARTTRAP;
	when 7: ch = POOL;
	when 8: ch = POST;
    }

    mvaddch(y, x, ch);
    traps[ntraps].tr_type = ch;
    traps[ntraps].tr_flags = ISTHIEFSET;
    traps[ntraps].tr_show = och;
    traps[ntraps].tr_pos.y = y;
    traps[ntraps++].tr_pos.x = x;
}


/*
 * rndmove:
 *	move in a random direction if the monster/person is confused
 */

coord *
rndmove(who)
struct thing *who;
{
    register int x, y;
    register int ex, ey, nopen = 0;
    register struct linked_list *item;
    register struct object *obj;
    static coord ret;  /* what we will be returning */
    static coord dest;

    ret = who->t_pos;
    /*
     * Now go through the spaces surrounding the player and
     * set that place in the array to true if the space can be
     * moved into
     */
    ey = ret.y + 1;
    ex = ret.x + 1;
    for (y = who->t_pos.y - 1; y <= ey; y++)
	if (y > 0 && y < LINES - 2)
	    for (x = who->t_pos.x - 1; x <= ex; x++)
	    {
		if (x < 0 || x >= COLS)
		    continue;
		if (step_ok(y, x, NOMONST, who))
		{
		    dest.y = y;
		    dest.x = x;
		    if (!diag_ok(&who->t_pos, &dest, who))
			continue;
		    if (rnd(++nopen) == 0)
			ret = dest;
		}
	    }
    return &ret;
}

/*
 * isatrap:
 *	Returns TRUE if this character is some kind of trap
 */
isatrap(ch)
reg char ch;
{
	switch(ch) {
		case DARTTRAP:
		case TELTRAP:
		case TRAPDOOR:
		case ARROWTRAP:
		case SLEEPTRAP:
		case POOL:
		case MAZETRAP:
		case BEARTRAP:	return(TRUE);
		default:	return(FALSE);
	}
}
