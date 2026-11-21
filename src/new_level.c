#include "curses.h"
#include "rogue.h"

/*
 * new_level:
 *	Dig and draw a new level
 *
 */

new_level(ltype)
LEVTYPE	ltype;		/* designates type of level to create */
{
    register int rm, i, cnt;
    register char ch;
    register struct linked_list *item;
    register struct thing *tp;
    coord stairs;

    /* Start player off right */
    turn_off(player, ISHELD);
    turn_off(player, ISFLEE);
    extinguish(suffocate);
    hold_count = 0;
    trap_tries = 0;

    if (level > max_level)
	max_level = level;
    wclear(cw);
    wclear(mw);
    clear();
    /*
     * check to see if he missed a UNIQUE, If he did then put it back
     * in the monster table for next time
     */
    for (item = mlist; item != NULL; item = next(item)) {
	tp = THINGPTR(item);
	if (on(*tp, ISUNIQUE)) 
	    monsters[tp->t_index].m_normal = TRUE;
    }
    /*
     * Free up the monsters on the last level
     */
    for (item = mlist; item != NULL; item = next(item))
	free_list(((struct thing *) ldata(item))->t_pack);
    free_list(mlist);
    free_list(lvl_obj);			/* Free up previous objects (if any) */
    levtype = ltype;
    if (ltype == POSTLEV) {
	do_post();			/* do post stuff */
    }
    else if (ltype == MAZELEV) {
	do_maze();
	no_food++;
	put_things(ltype);		/* Place objects (if any) */
    }
    else {
	do_rooms();			/* Draw rooms */
	do_passages();			/* Draw passages */
	no_food++;
	put_things(ltype);		/* Place objects (if any) */
    }
    /*
     * Place the staircase down.
     */
    cnt = 0;
    do {
        rm = rnd_room();
	rnd_pos(&rooms[rm], &stairs);
    } until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);
    addch(STAIRS);
    /*
     * maybe add a trading post 
     */
    if (level > 5 && rnd(11) == 7 && ltype == NORMLEV) {
	cnt = 0;
	do {
	    rm = rnd_room();
	    if (rooms[rm].r_flags & ISTREAS)
		continue;
	    rnd_pos(&rooms[rm], &stairs);
	} until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);
	addch(POST);
    }

    /*
     * Place the traps (except for trading post)
     */
    ntraps = 0;	/* No traps yet */
    if (levtype == NORMLEV) {
	if (rnd(10) < level) {
	    ntraps = rnd(level/4)+1;
	    if (ntraps > MAXTRAPS)
		ntraps = MAXTRAPS;
	    i = ntraps;
	    while (i--)
	    {
		cnt = 0;
		do {
		    rm = rnd_room();
		    if (rooms[rm].r_flags & ISTREAS)
			continue;
		    rnd_pos(&rooms[rm], &stairs);
		} until (mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000);

		traps[i].tr_flags = 0;
		switch(rnd(8)) {
		    when 0: ch = TRAPDOOR;
		    when 1: ch = BEARTRAP;
		    when 2: ch = SLEEPTRAP;
		    when 3: ch = ARROWTRAP;
		    when 4: ch = TELTRAP;
		    when 5: ch = DARTTRAP;
		    when 6: ch = POOL;
			    traps[i].tr_flags = ISFOUND;
		    when 7: ch = MAZETRAP;
		}
		addch(ch);
		traps[i].tr_type = ch;
		traps[i].tr_show = FLOOR;
		traps[i].tr_pos = stairs;
	    }
	}
    }
    cnt = 0;
    do {
	rm = rnd_room();
	if (rooms[rm].r_flags & ISTREAS)
	    continue;
	rnd_pos(&rooms[rm], &hero);
    } until(	cnt++ > 5000 ||
    		(winat(hero.y, hero.x) == FLOOR &&
	         DISTANCE(hero.y, hero.x, stairs.y, stairs.x) > 16));
    oldrp = &rooms[rm];		/* Set the current room */
    player.t_oldpos = player.t_pos;	/* Set the current position */
    light(&hero);
    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
    status(TRUE);
}

/*
 * Pick a room that is really there
 */

rnd_room()
{
    register int rm;

    if (levtype != NORMLEV)
	rm = 0;
    else do
	{
	    rm = rnd(MAXROOMS);
	} while (rooms[rm].r_flags & ISGONE);
    return rm;
}

/*
 * put_things:
 *	put potions and scrolls on this level
 */

put_things(ltype)
LEVTYPE	ltype;		/* designates type of level to create */
{
    register int i, rm, cnt;
    register struct linked_list *item;
    register struct object *cur;
    bool got_unique = FALSE;
    int length, width;
    coord tp;

    /*
     * Once you have found the amulet, the only way to get new stuff is
     * go down into the dungeon.
     */
    if (amulet && level < max_level)
	return;

    /* 
     * There is a chance that there is a treasure room on this level 
     * Increasing chance after level 9 
     */
    if (ltype != MAZELEV && rnd(30) < level - 8) {	
	register  i, j;
	register struct room *rp;

	/* Count the number of free spaces */
	i = 0;	/* 0 tries */
	do {
	    rp = &rooms[rnd_room()];
	    width = rp->r_max.y - 2;
	    length = rp->r_max.x - 2;
	} until ((width*length <= MAXTREAS) || (i++ > MAXROOMS*4));

	/* Mark the room as a treasure room */
	rp->r_flags |= ISTREAS;

	/* Make all the doors secret doors */
	for (i=0; i<rp->r_nexits; i++) {
	    cmov(rp->r_exit[i]);
	    addch(SECRETDOOR);
	}

	/* Put in the monsters and treasures */
	for (j=1; j<rp->r_max.y-1; j++)
	    for (i=1; i<rp->r_max.x-1; i++) {
		coord trp;

		trp.y = rp->r_pos.y+j;
		trp.x = rp->r_pos.x+i;

		/* Monsters */
		if ((rnd(100) < (MAXTREAS*100)/(width*length)) &&
		    (mvwinch(mw, rp->r_pos.y+j, rp->r_pos.x+i) == ' ')) {
		    register struct thing *tp;

		    /* Make a monster */
		    item = new_item(sizeof *tp);
		    tp = THINGPTR(item);

		    /* 
		     * Put it there and aggravate it (unless it can escape) 
		     * only put one UNIQUE per treasure room at most
		     */
		    if (got_unique)
			new_monster(item,randmonster(FALSE, TRUE),&trp,FALSE);
		    else {
			new_monster(item,randmonster(FALSE, FALSE),&trp,FALSE);
			if (on(*tp, ISUNIQUE))
			    got_unique = TRUE;
		    }
		    turn_on(*tp, ISMEAN);
		    if (off(*tp, CANINWALL)) {
			tp->t_dest = &hero;
			turn_on(*tp, ISRUN);
		    }

		    /* If it is a mimic, undisguise it */
		    if (on(*tp, ISDISGUISE)) turn_off(*tp, ISDISGUISE);
		}

		/* Treasures */
		if ((rnd(100) < (MAXTREAS*100)/(width*length)) &&
		    (mvinch(rp->r_pos.y+j, rp->r_pos.x+i) == FLOOR)) {
		    item = new_thing();
		    attach(lvl_obj, item);
		    cur = (struct object *) ldata(item);
	
		    mvaddch(trp.y, trp.x, cur->o_type);
		    cur->o_pos = trp;
		}
	    }
    }

    /*
     * Do MAXOBJ attempts to put things on a level
     */
    for (i = 0; i < MAXOBJ; i++)
	if (rnd(100) < 40)
	{
	    /*
	     * Pick a new object and link it in the list
	     */
	    item = new_thing();
	    attach(lvl_obj, item);
	    cur = (struct object *) ldata(item);
	    /*
	     * Put it somewhere
	     */
	    cnt = 0;
	    do {
		rm = rnd_room();
		rnd_pos(&rooms[rm], &tp);
	    } until (winat(tp.y, tp.x) == FLOOR || cnt++ > 500);
	    mvaddch(tp.y, tp.x, cur->o_type);
	    cur->o_pos = tp;
	}
    /*
     * If he is really deep in the dungeon and he hasn't found the
     * amulet yet, put it somewhere on the ground
     */
    if (level > AMLEVEL && !amulet && rnd(100) < 70)
    {
	item = new_item(sizeof *cur);
	attach(lvl_obj, item);
	cur = (struct object *) ldata(item);
	cur->o_hplus = cur->o_dplus = 0;
	cur->o_damage = cur->o_hurldmg = "0d0";
	cur->o_ac = 11;
	cur->o_mark[0] = '\0';
	cur->o_type = AMULET;
	cur->o_weight = things[TYP_AMULET].mi_wght;
	cur->o_flags = 0;
	cur->o_group = 0;
	cur->o_count = 1;

	/*
	 * Put it somewhere
	 */
	cnt = 0;
	do {
	    rm = rnd_room();
	    rnd_pos(&rooms[rm], &tp);
	} until (winat(tp.y, tp.x) == FLOOR || cnt++ > 500);
	mvaddch(tp.y, tp.x, cur->o_type);
	cur->o_pos = tp;
    }
}
