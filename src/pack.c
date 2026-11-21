#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/*
 * Routines to deal with the pack
 */

/*
 * add_pack:
 *	Pick up an object and add it to the pack.  If the argument is non-null
 * use it as the linked_list pointer instead of gettting it off the ground.
 */
bool
add_pack(item, silent)
register struct linked_list *item;
bool silent;
{
    register struct linked_list *ip, *lp;
    register struct object *obj, *op;
    register bool exact, from_floor;

    if (item == NULL)
    {
	from_floor = TRUE;
	if ((item = find_obj(hero.y, hero.x)) == NULL)
	    return(FALSE);
    }
    else
	from_floor = FALSE;
    obj = (struct object *) ldata(item);
    /*
     * If it is gold, just add its value to rogue's purse and get rid
     * of it.
     */
    if (obj->o_type == GOLD) {
	register struct linked_list *mitem;
	register struct thing *tp;

	if (!silent) {
	    if (!terse) addmsg("You found ");
	    msg("%d gold pieces.", obj->o_count);
	}

	/* First make sure no greedy monster is after this gold.
	 * If so, make the monster run after the rogue instead.
	 */
	 for (mitem = mlist; mitem != NULL; mitem = next(mitem)) {
	    tp = (struct thing *) ldata(mitem);
	    if (tp->t_dest == &obj->o_pos) tp->t_dest = &hero;
	}

	purse += obj->o_count;
	if (from_floor) {
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x,
		(roomin(&hero) == NULL ? PASSAGE : FLOOR));
	    discard(item);
	}
	return(TRUE);
    }

    /*
     * see if he can carry any more weight
     */
    if (itemweight(obj) + pstats.s_pack > pstats.s_carry) {
	msg("too much for you to carry");
	return FALSE;
    }
    /*
     * Link it into the pack.  Search the pack for a object of similar type
     * if there isn't one, stuff it at the beginning, if there is, look for one
     * that is exactly the same and just increment the count if there is.
     * it  that.  Food is always put at the beginning for ease of access, but
     * is not ordered so that you can't tell good food from bad.  First check
     * to see if there is something in thr same group and if there is then
     * increment the count.
     */
    if (obj->o_group)
    {
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_group == obj->o_group)
	    {
		/*
		 * Put it in the pack and notify the user
		 */
		op->o_count++;
		if (from_floor)
		{
		    detach(lvl_obj, item);
		    mvaddch(hero.y, hero.x,
			(roomin(&hero) == NULL ? PASSAGE : FLOOR));
		}
		discard(item);
		item = ip;
		goto picked_up;
	    }
	}
    }

    /*
     * Check for and deal with scare monster scrolls
     */
    if (obj->o_type == SCROLL && obj->o_which == S_SCARE)
	if (obj->o_flags & ISCURSED)
	{
	    msg("The scroll turns to dust as you pick it up.");
	    detach(lvl_obj, item);
	    mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
	    discard(item);
	    return(TRUE);
	}

    /*
     * Search for an object of the same type
     */
    exact = FALSE;
    for (ip = pack; ip != NULL; ip = next(ip))
    {
	op = (struct object *) ldata(ip);
	if (obj->o_type == op->o_type)
	    break;
    }
    if (ip == NULL)
    {
	/*
	 * Put it at the end of the pack since it is a new type
	 */
	for (ip = pack; ip != NULL; ip = next(ip))
	{
	    op = (struct object *) ldata(ip);
	    if (op->o_type != FOOD)
		break;
	    lp = ip;
	}
    }
    else
    {
	/*
	 * Search for an object which is exactly the same
	 */
	while (ip != NULL && op->o_type == obj->o_type)
	{
	    if (op->o_which == obj->o_which)
	    {
		exact = TRUE;
		break;
	    }
	    lp = ip;
	    if ((ip = next(ip)) == NULL)
		break;
	    op = (struct object *) ldata(ip);
	}
    }
    /*
     * Check if there is room
     */
    if (ip == NULL || !exact || !ISMULT(obj->o_type)) {
	if (inpack == MAXPACK-1) {
	    msg("You can't carry anything else.");
	    return(FALSE);
	}
    }
    inpack++;
    if (from_floor)
    {
	detach(lvl_obj, item);
	mvaddch(hero.y, hero.x, (roomin(&hero) == NULL ? PASSAGE : FLOOR));
    }
    if (ip == NULL)
    {
	/*
	 * Didn't find an exact match, just stick it here
	 */
	if (pack == NULL)
	    pack = item;
	else
	{
	    lp->l_next = item;
	    item->l_prev = lp;
	    item->l_next = NULL;
	}
    }
    else
    {
	/*
	 * If we found an exact match.  If it is food,
	 * increase the count, otherwise put it with its clones.
	 */
	if (exact && ISMULT(obj->o_type))
	{
	    op->o_count += obj->o_count;
	    inpack--;			/* adjust for previous addition */
	    discard(item);
	    item = ip;
	    goto picked_up;
	}
	if ((item->l_prev = prev(ip)) != NULL)
	    item->l_prev->l_next = item;
	else
	    pack = item;
	item->l_next = ip;
	ip->l_prev = item;
    }
picked_up:
    /*
     * Notify the user
     */
    obj = (struct object *) ldata(item);
    if (!silent)
    {
	if (!terse)
	    addmsg("You now have ");
	msg("%s (%c)", inv_name(obj, !terse, -1), pack_char(obj));
    }
    if (obj->o_type == AMULET)
	amulet = TRUE;
    updpack(FALSE);
    return(TRUE);
}

/*
 * inventory:
 *	list what is in the pack
 */
inventory(list, type, know)
register struct linked_list *list;
register int type;
int know;
{
    register struct object *obj;
    register char ch;
    register int n_objs;
    char inv_temp[LINELEN];
    int sav;

    n_objs = 0;
    for (ch = 'a'; list != NULL; ch++, list = next(list))
    {
	obj = (struct object *) ldata(list);
	if (type && type != obj->o_type &&
	    !(type == MARKABLE && obj->o_type != FOOD) &&
	    !(type == CALLABLE &&
	     (obj->o_type == SCROLL || obj->o_type == POTION ||
	      obj->o_type == RING || obj->o_type == STICK)))
		continue;
	sav = (obj->o_flags & ISKNOW);
	if (know >= 0)
	    obj->o_flags |= ISKNOW;
	switch (n_objs++)
	{
	    /*
	     * For the first thing in the inventory, just save the string
	     * in case there is only one.
	     */
	    case 0:
		sprintf(inv_temp, "%c) %s", ch, inv_name(obj, FALSE, know));
		break;
	    /*
	     * If there is more than one, clear the screen, print the
	     * saved message and fall through to ...
	     */
	    case 1:
		if (slow_invent)
		    msg(inv_temp);
		else
		{
		    wclear(hw);
		    waddstr(hw, inv_temp);
		    waddch(hw, '\n');
		}
	    /*
	     * Print the line for this object
	     */
	    default:
		if (slow_invent)
		    msg("%c) %s", ch, inv_name(obj, FALSE, know));
		else
		    wprintw(hw, "%c) %s\n", ch, inv_name(obj, FALSE, know));
	}
	if (know == 0 && !sav)
		 obj->o_flags &= ~ISKNOW;
    }
    if (n_objs == 0)
    {
	if (terse)
	    msg(type == 0 ? "Empty handed." :
			    "Nothing appropriate");
	else
	    msg(type == 0 ? "You are empty handed." :
			    "You don't have anything appropriate");
	return FALSE;
    }
    if (n_objs == 1)
    {
	msg(inv_temp);
	return TRUE;
    }
    if (!slow_invent)
    {
	mvwaddstr(hw, LINES-1, 0, spacemsg);
	draw(hw);
	wait_for(' ');
	clearok(cw, TRUE);
	touchwin(cw);
    }
    return TRUE;
}

/*
 * pick_up:
 *	Add something to characters pack.
 */
pick_up(ch)
char ch;
{
    switch(ch)
    {
	default:
	    debug("Where did you pick that up???");
	case GOLD:
	case ARMOR:
	case POTION:
	case FOOD:
	case WEAPON:
	case SCROLL:	
	case AMULET:
	case RING:
	case STICK:
	    add_pack(NULL, FALSE);
	    break;
    }
}

/*
 * picky_inven:
 *	Allow player to inventory a single item
 */
picky_inven()
{
    register struct linked_list *item;
    register char ch, mch;

    if (pack == NULL)
	msg("You aren't carrying anything");
    else if (next(pack) == NULL)
	msg("a) %s", inv_name((struct object *) ldata(pack), FALSE, -1));
    else
    {
	msg(terse ? "Item: " : "Which item do you wish to inventory: ");
	mpos = 0;
	if ((mch = readchar()) == ESCAPE)
	{
	    msg("");
	    return;
	}
	for (ch = 'a', item = pack; item != NULL; item = next(item), ch++)
	    if (ch == mch)
	    {
		msg("%c) %s",ch,inv_name((struct object *) ldata(item), FALSE, -1));
		return;
	    }
	if (!terse)
	    msg("'%s' not in pack", unctrl(mch));
	msg("Range is 'a' to '%c'", --ch);
    }
}


/*
 * get_item:
 *	pick something out of a pack for a purpose
 */
struct linked_list *
get_item(purpose, type)
char *purpose;
int type;
{
    reg struct linked_list *obj, *pit, *savepit;
    struct object *pob;
    reg char ch, och, anr;
    int cnt;

    if (pack == NULL)
	msg("You aren't carrying anything.");
    else  {
	/* see if we have any of the type requested */
	if(type != 0 && type != CALLABLE && type != MARKABLE) {
	    pit = pack;
	    anr = 0;
	    for(ch = 'a' ; pit != NULL ; pit = next(pit), ch++) {
		pob = OBJPTR(pit);
		if(type == pob->o_type) {
		    ++anr;
		    savepit = pit;	/* save in case of only 1 */
		}
	    }
	    if (anr == 0 && type != WEAPON) {
		msg("Nothing to %s",purpose);
		after = FALSE;
		return NULL;
	    }
	    else if (anr == 1) {	/* only found one of 'em */
		do {
		    struct object *opb;
		    opb = OBJPTR(savepit);
		    msg("%s what (* for the item)?",purpose);
		    och = Tolower(readchar());
		    if (och == '*') {
			mpos = 0;
			msg("%c) %s",pack_char(opb),inv_name(opb,FALSE,-1));
			continue;
		    }
		    if (och == ESCAPE) {
			msg("");
			after = FALSE;
			return NULL;
		    }
		    if (type != WEAPON) {
			if(isalpha(och) && och != pack_char(opb)) {
			    mpos = 0;
			    msg("You can't %s that",purpose);
			    after = FALSE;
			    return NULL;
			}
		    }
		} while(!isalpha(och));
		mpos = 0;
		return savepit;		/* return this item */
	    }
	}
	for (;;) {
	    msg("%s what? (* for list): ",purpose);
	    ch = readchar();
	    mpos = 0;
	    if (ch == ESCAPE) {		/* abort if escape hit */
		after = FALSE;
		msg("");		/* clear display */
		return NULL;
	    }
	    if (ch == '*') {
		wclear(hw);
		pit = pack;		/* point to pack */
		cnt = 0;
		for(ch = 'a'; pit != NULL ; pit = next(pit), ch++) {
		    pob = OBJPTR(pit);
		    if(type==0          || type==CALLABLE || 
		       type == MARKABLE || type==pob->o_type) {
		      wprintw(hw,"%c) %s\n\r",ch,inv_name(pob,FALSE,-1));
		      if (++cnt >= LINES - 2 && next(pit) != NULL) {
			cnt = 0;
			dbotline(hw, spacemsg);
			wclear(hw);
		      }
		    }
		}
		wmove(hw, LINES - 1,0);
		wprintw(hw,"%s what? ",purpose);
		draw(hw);		/* write screen */
		anr = FALSE;
		do {
		    ch = Tolower(readchar());
		    if (isalpha(ch) || ch == ESCAPE)
			anr = TRUE; 
		} while(!anr);		/* do till we got it right */
		restscr(cw);		/* redraw orig screen */
		if(ch == ESCAPE) {
		    after = FALSE;
		    msg("");		/* clear top line */
		    return NULL;	/* all done if abort */
		}
		/* ch has item to get from pack */
	    }
	    for(obj = pack,och = 'a'; obj != NULL;obj = next(obj),och++)
		if (ch == och)
		    break;
	    if (obj == NULL) {
		msg("Please specify a letter between 'a' and '%c'", och-1);
		continue;
	    }
	    else {
	        return obj;
	    }
	}
    }
    return NULL;
}

pack_char(obj)
register struct object *obj;
{
    register struct linked_list *item;
    register char c;

    c = 'a';
    for (item = pack; item != NULL; item = next(item))
	if ((struct object *) ldata(item) == obj)
	    return c;
	else
	    c++;
    return 'z';
}

/* 
 * del_pack:
 *	Take something out of the hero's pack
 */
del_pack(what)
reg struct linked_list *what;
{
	reg struct object *op;

	op = OBJPTR(what);
	cur_null(op);		/* check for current stuff */
	if (op->o_count > 1) {
	    op->o_count--;
	}
	else {
	    detach(pack,what);
	    discard(what);
	    inpack--;
	}
	updpack(FALSE);
}

/*
 * cur_null:
 *	This updates cur_weapon etc for dropping things
 */
cur_null(op)
reg struct object *op;
{
	if (op == cur_weapon)
	    cur_weapon = NULL;
	else if (op == cur_armor)
	    cur_armor = NULL;
	else if (op == cur_ring[LEFT_1])
	    cur_ring[LEFT_1] = NULL;
	else if (op == cur_ring[LEFT_2])
	    cur_ring[LEFT_2] = NULL;
	else if (op == cur_ring[LEFT_3])
	    cur_ring[LEFT_3] = NULL;
	else if (op == cur_ring[LEFT_4])
	    cur_ring[LEFT_4] = NULL;
	else if (op == cur_ring[RIGHT_1])
	    cur_ring[RIGHT_1] = NULL;
	else if (op == cur_ring[RIGHT_2])
	    cur_ring[RIGHT_2] = NULL;
	else if (op == cur_ring[RIGHT_3])
	    cur_ring[RIGHT_3] = NULL;
	else if (op == cur_ring[RIGHT_4])
	    cur_ring[RIGHT_4] = NULL;
}

/*
 * idenpack:
 *	Identify all the items in the pack
 */
idenpack()
{
	reg struct linked_list *pc;

	for (pc = pack ; pc != NULL ; pc = next(pc))
		whatis(pc);
}
