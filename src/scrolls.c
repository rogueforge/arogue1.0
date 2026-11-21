
/*
 * Read a scroll and let it happen
 *
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

read_scroll(which, blessed)
register int which;
bool blessed;
{
    register struct object *obj, *nobj;
    register struct linked_list *item, *nitem;
    register int i,j;
    register char ch, nch;
    bool cursed, is_scroll;
    char buf[LINELEN];

    cursed = FALSE;
    is_scroll = FALSE;

    if (which < 0) {
	item = get_item("read", SCROLL);
	if (item == NULL)
	    return;

	obj = (struct object *) ldata(item);
	if (obj->o_type != SCROLL) {
	    msg ("You can't read that!");
	    return;
	}
	/* remove it from the pack */
	inpack--;
	detach(pack, item);

	msg("As you read the scroll, it vanishes.");
	/*
	 * Calculate the effect it has on the poor guy.
	 */
	cursed = obj->o_flags & ISCURSED;
	blessed = obj->o_flags & ISBLESSED;

	which = obj->o_which;
	is_scroll = TRUE;
    }


    switch(which)
    {
	when S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    msg("Your hands begin to glow red");
	    turn_on(player, CANHUH);
	when S_CURING:
	    /*
	     * A cure disease spell
	     */
	    if (on(player, HASINFEST) || on(player, HASDISEASE)) {
		if (on(player, HASDISEASE)) {
		    extinguish(cure_disease);
		    cure_disease();
		}
		if (on(player, HASINFEST)) {
		    msg(terse ? "You feel yourself improving."
			      : "You begin to feel yourself improving again.");
		    turn_off(player, HASINFEST);
		    infest_dam = 0;
		}
	    }
	    else msg("Nothing happens.");
	    if (is_scroll)
	        s_know[S_CURING] = TRUE;
	when S_LIGHT:
	    if (blue_light(blessed, cursed) && is_scroll)
		s_know[S_LIGHT] = TRUE;
	when S_HOLD:
	    if (cursed) {
		/*
		 * This scroll aggravates all the monsters on the current
		 * level and sets them running towards the hero
		 */
		aggravate();
		msg("You hear a high pitched humming noise.");
	    }
	    else if (blessed) { /* Hold all monsters on level */
		if (mlist == NULL) msg("Nothing happens.");
		else {
		    register struct linked_list *mon;
		    register struct thing *th;

		    for (mon = mlist; mon != NULL; mon = next(mon)) {
			th = (struct thing *) ldata(mon);
			turn_off(*th, ISRUN);
			turn_on(*th, ISHELD);
		    }
		    msg("A sudden peace comes over the dungeon.");
		}
	    }
	    else {
		/*
		 * Hold monster scroll.  Stop all monsters within two spaces
		 * from chasing after the hero.
		 */
		    register int x,y;
		    register struct linked_list *mon;
		    bool gotone=FALSE;

		    for (x = hero.x-2; x <= hero.x+2; x++) {
			for (y = hero.y-2; y <= hero.y+2; y++) {
			    if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x))) {
				if ((mon = find_mons(y, x)) != NULL) {
				    register struct thing *th;

				    gotone = TRUE;
				    th = (struct thing *) ldata(mon);
				    turn_off(*th, ISRUN);
				    turn_on(*th, ISHELD);
				}
			    }
			}
		    }
		    if (gotone) msg("A sudden peace surrounds you.");
		    else msg("Nothing happens.");
	    }
	when S_SLEEP:
	    /*
	     * if cursed, you fall asleep
	     */
	    if (is_scroll) s_know[S_SLEEP] = TRUE;
	    if (cursed) {
		if (ISWEARING(R_ALERT))
		    msg("You feel drowsy for a moment.");
		else {
		    msg("You fall asleep.");
		    no_command += 4 + rnd(SLEEPTIME);
		}
	    }
	    else {
		/*
		 * sleep monster scroll.  
		 * puts all monsters within 2 spaces asleep
		 */
		    register int x,y;
		    register struct linked_list *mon;
		    bool gotone=FALSE;

		    for (x = hero.x-2; x <= hero.x+2; x++) {
			for (y = hero.y-2; y <= hero.y+2; y++) {
			    if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x))) {
				if ((mon = find_mons(y, x)) != NULL) {
				    register struct thing *th;

				    th = (struct thing *) ldata(mon);
				    if (on(*th, ISUNDEAD))
					continue;
				    th->t_no_move += SLEEPTIME;
				    gotone = TRUE;
				}
			    }
			}
		    }
		    if (gotone) 
			msg("The monster(s) around you seem to have fallen asleep");
		    else 
			msg("Nothing happens.");
	    }
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    creat_mons(&player, (short) 0, TRUE);
	when S_IDENT:
	    /* 
	     * if its blessed then identify everything in the pack
	     */
	    if (blessed) {
		msg("You feel more Knowledgeable!");
		idenpack();
	    }
	    else {
		/*
		 * Identify, let the rogue figure something out
		 */
		if (is_scroll && s_know[S_IDENT] != TRUE) {
		    msg("This scroll is an identify scroll");
		}
		whatis(NULL);
	    }
	    if (is_scroll)
	        s_know[S_IDENT] = TRUE;
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    if (is_scroll && s_know[S_MAP] != TRUE) {
		msg("Oh, now this scroll has a map on it.");
		s_know[S_MAP] = TRUE;
	    }
	    overwrite(stdscr, hw);
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (i = 0; i < LINES; i++)
		for (j = 0; j < COLS; j++)
		{
		    switch (nch = ch = mvwinch(hw, i, j))
		    {
			case SECRETDOOR:
			    mvaddch(i, j, nch = DOOR);
			case '-':
			case '|':
			case DOOR:
			case PASSAGE:
			case ' ':
			case STAIRS:
			    if (mvwinch(mw, i, j) != ' ')
			    {
				register struct thing *it;

				it = (struct thing *) ldata(find_mons(i, j));
				if (it && it->t_oldch == ' ')
				    it->t_oldch = nch;
			    }
			    break;
			default:
			    nch = ' ';
		    }
		    if (nch != ch)
			waddch(hw, nch);
		}
	    /*
	     * Copy in what he has discovered
	     */
	    overlay(cw, hw);
	    /*
	     * And set up for display
	     */
	    overwrite(hw, cw);
	when S_GFIND:
	    /*
	     * Scroll of gold detection
	     */
	    if (lvl_obj != NULL) {
		struct linked_list *gitem;
		struct object *cur;
		int gtotal = 0;

		wclear(hw);
		for (gitem = lvl_obj; gitem != NULL; gitem = next(gitem)) {
		    cur = (struct object *) ldata(gitem);
		    if (cur->o_type == GOLD) {
			gtotal += cur->o_count;
			mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, GOLD);
		    }
		}
		if (gtotal) {
		    if (is_scroll) s_know[S_GFIND] = TRUE;
		    msg("You begin to feel greedy and you sense gold.");
		    overlay(hw,cw);
		    break;
		}
	    }
	    msg("You begin to feel a pull downward");
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him disappear and reappear
	     */
	    if (cursed) {
		level = level * 3 + 10 + rnd(10);
		new_level(NORMLEV);
		mpos = 0;
		msg("You are banished to the lower regions.");
	    }
	    else if (blessed) {
		int old_level;
		int much = rnd(5) - 4;
		old_level = level;
		if (much != 0) {
		    level += much;
		    if (level < 1)
			level = 1;
		    mpos = 0;
		    new_level(NORMLEV);		/* change levels */
		    if (level == old_level)
			status(TRUE);
		    msg("You are whisked away to another region.");
		}
	    }
	    else {
		int rm;
		struct room *cur_room;

		cur_room = roomin(&hero);
		rm = teleport();
	    }
	    if (is_scroll)
	        s_know[S_TELEP] = TRUE;
	when S_SCARE:
	    /*
	     * A monster will refuse to step on a scare monster scroll
	     * if it is dropped.  Thus reading it is a mistake and produces
	     * laughter at the poor rogue's boo boo.
	     */
	    msg("You hear maniacal laughter in the distance.");
	when S_REMOVE:
	    if (cursed) { /* curse all player's possessions */
		for (nitem = pack; nitem != NULL; nitem = next(nitem)) {
		    nobj = OBJPTR(nitem);
		    if (nobj->o_flags & ISBLESSED) 
			nobj->o_flags &= ~ISBLESSED;
		    else 
			nobj->o_flags |= ISCURSED;
		}
		msg("The smell of fire and brimstone fills the air.");
	    }
	    else if (blessed) {
		for (nitem = pack; nitem != NULL; nitem = next(nitem)) {
		    nobj = OBJPTR(nitem);
		    nobj->o_flags &= ~ISCURSED;
		}
		msg("Your pack glistens brightly");
	    }
	    else {
		if ((nitem = get_item("remove the curse on",0)) != NULL) {
		    nobj = OBJPTR(nitem);
		    nobj->o_flags &= ~ISCURSED;
		    msg("Removed the curse from %s",inv_name(nobj,TRUE,-1));
		}
	    }
	    s_know[S_REMOVE] = TRUE;
	when S_PETRIFY:
	    switch (mvinch(hero.y, hero.x)) {
		case TRAPDOOR:
		case DARTTRAP:
		case TELTRAP:
		case ARROWTRAP:
		case SLEEPTRAP:
		case BEARTRAP:
		    {
			register short i;

			/* Find the right trap */
			for (i=0; i<ntraps && !ce(traps[i].tr_pos, hero); i++);
			ntraps--;

			if (!ce(traps[i].tr_pos, hero))
			    msg("What a strange trap!");
			else {
			    while (i < ntraps) {
				traps[i] = traps[i + 1];
				i++;
			    }
			}
		    }
		    goto pet_message;
		case DOOR:
		case SECRETDOOR:
		    {
			register struct room *rp=roomin(&hero);
			register short i;

			/* Find the right door */
			for (i=0;
			     i<rp->r_nexits && !ce(rp->r_exit[i], hero); i++);
			rp->r_nexits--;

			if (!ce(rp->r_exit[i], hero))
			    msg("What a strange door!");
			else {
			    while (i < rp->r_nexits) {
				rp->r_exit[i] = rp->r_exit[i + 1];
				i++;
			    }
			}
		    }
			    
		case FLOOR:
		case PASSAGE:
pet_message:	    msg("The dungeon begins to rumble and shake!");
		    addch(WALL);

		    /* If the player is phased, unphase him */
		    if (on(player, CANINWALL)) {
			extinguish(unphase);
			turn_off(player, CANINWALL);
			msg("Your dizzy feeling leaves you.");
		    }

		    /* Mark the player as in a wall */
		    turn_on(player, ISINWALL);
		    break;
		default:
		    msg("Nothing happens.");
	    }
	when S_GENOCIDE:
	    msg("You have been granted the boon of genocide!--More--");
	    wait_for(' ');
	    msg("");
	    genocide();
	    if (is_scroll) s_know[S_GENOCIDE] = TRUE;
	when S_PROTECT: {
	    struct linked_list *ll;
	    struct object *lb;
	    msg("You are granted the power of protection.");
	    if ((ll = get_item("protect",0)) != NULL) {
		lb = OBJPTR(ll);
		lb->o_flags |= ISPROT;
		mpos = 0;
		msg("Protected %s.",inv_name(lb,TRUE,-1));
	    }
	    s_know[S_PROTECT] = TRUE;
	}
	when S_MAKEIT:
	    msg("You have been endowed with the power of creation.");
	    s_know[S_MAKEIT] = TRUE;
	    create_obj(TRUE, 0, 0);
	when S_ALLENCH: {
	    struct linked_list *ll;
	    struct object *lb;
	    int howmuch, flags;
	    if (is_scroll && s_know[S_ALLENCH] == FALSE) {
		msg("You are granted the power of enchantment.");
		msg("You may enchant anything(weapon,ring,armor,scroll,potion)");
	    }
	    if ((ll = get_item("enchant",0)) != NULL) {
		lb = OBJPTR(ll);
		lb->o_flags &= ~ISCURSED;
		if (is_scroll && (obj->o_flags & ISBLESSED)) {
		    howmuch = 2;
		    flags = ISBLESSED;
		}
		else if (is_scroll && (obj->o_flags & ISCURSED)) {
		    howmuch = -1;
		    flags = ISCURSED;
		}
		else {
		    howmuch = 1;
		    flags = ISBLESSED;
		}
		switch(lb->o_type) {
		    when RING:
			lb->o_ac += howmuch;
		    when ARMOR:
			lb->o_ac -= howmuch;
		    when STICK:
			lb->o_charges += howmuch + 10;
			if (lb->o_charges < 0)
			    lb->o_charges = 0;
		    when WEAPON:
			if (rnd(100) < 50)
			    lb->o_hplus += howmuch;
			else
			    lb->o_dplus += howmuch;
		    when POTION:
		    case SCROLL:
		    default:
			lb->o_flags |= flags;
		}
		mpos = 0;
		msg("Enchanted %s.",inv_name(lb,TRUE,-1));
	    }
	    s_know[S_ALLENCH] = TRUE;
	}
	otherwise:
	    msg("What a puzzling scroll!");
	    return;
    }
    look(TRUE);	/* put the result of the scroll on the screen */
    status(FALSE);
    if (is_scroll && s_know[which] && s_guess[which])
    {
	free(s_guess[which]);
	s_guess[which] = NULL;
    }
    else if (is_scroll && 
	     !s_know[which] && 
	     askme &&
	     (obj->o_flags & ISKNOW) == NULL &&
	     s_guess[which] == NULL) {
	msg(terse ? "Call it: " : "What do you want to call it? ");
	buf[0] = '\0';
	if (get_str(buf, cw) == NORM && buf[0] != '\0')
	{
	    s_guess[which] = new(strlen(buf)
		< sizeof (struct thing) ? sizeof (struct thing) : strlen(buf) + 1);
	    strcpy(s_guess[which], buf);
	}
    }
    if (is_scroll) discard(item);
    updpack(TRUE);
}

/*
 * Creat_mons creates the specified monster -- any if 0 
 */

bool
creat_mons(person, monster, report)
struct thing *person;	/* Where to create next to */
short monster;
bool report;
{
    register int x, y;
    register char appear = 0;
    struct thing play_copy;	/* A copy with no phasing */
    coord mp;

    /* Make sure phasing is off in the copy */
    play_copy = *person;
    turn_off(play_copy, CANINWALL);

    /*
     * Search for an open place
     */
    for (y = person->t_pos.y-1; y <= person->t_pos.y+1; y++)
	for (x = person->t_pos.x-1; x <= person->t_pos.x+1; x++)
	{
	    /*
	     * Don't put a monster in top of the creator or player.
	     */
	    if (y == person->t_pos.y && x == person->t_pos.x)
		continue;
	    if (y == hero.y && x == hero.x) continue;

	    /*
	     * Or anything else nasty 
	     */
	    if (step_ok(y, x, NOMONST, &play_copy)) {
		if (rnd(++appear) == 0)
		{
		    mp.y = y;
		    mp.x = x;
		}
	    }
	}
    if (appear) {
	struct linked_list *nitem;

	nitem = new_item(sizeof (struct thing));
	new_monster(nitem, monster == 0 ? randmonster(FALSE, FALSE)
					: monster, &mp, TRUE);
	runto(&mp, &hero);
	light(&hero);
	return(TRUE);
    }
    if (report) msg("You hear a faint cry of anguish in the distance.");
    return(FALSE);
}
