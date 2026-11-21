/*
 * Special wizard commands (some of which are also non-wizard commands
 * under strange circumstances)
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/*
 * whatis:
 *	What a certin object is
 */

whatis(what)
struct linked_list *what;
{
    register struct object *obj;
    register struct linked_list *item;

    if (what == NULL) {		/* do we need to ask which one? */
	if ((item = get_item("identify", 0)) == NULL)
	    return;
    }
    else
	item = what;
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    switch (obj->o_type)
    {
        when SCROLL:
	    s_know[obj->o_which] = TRUE;
	    if (s_guess[obj->o_which])
	    {
		free(s_guess[obj->o_which]);
		s_guess[obj->o_which] = NULL;
	    }
        when POTION:
	    p_know[obj->o_which] = TRUE;
	    if (p_guess[obj->o_which])
	    {
		free(p_guess[obj->o_which]);
		p_guess[obj->o_which] = NULL;
	    }
	when STICK:
	    ws_know[obj->o_which] = TRUE;
	    if (ws_guess[obj->o_which])
	    {
		free(ws_guess[obj->o_which]);
		ws_guess[obj->o_which] = NULL;
	    }
        when WEAPON:
        case ARMOR:
        when RING:
	    r_know[obj->o_which] = TRUE;
	    if (r_guess[obj->o_which])
	    {
		free(r_guess[obj->o_which]);
		r_guess[obj->o_which] = NULL;
	    }
    }
    if (what == NULL)
	msg("%s", inv_name(obj, FALSE, -1));
}

/*
 * create_obj:
 *	Create any object for wizard, scroll, magician, or cleric
 */
create_obj(prompt, which_item, which_type)
bool prompt;
int which_item, which_type;
{
    reg struct linked_list *item;
    reg struct object *obj;
    reg int wh;
    reg char ch, newitem, newtype, whc, msz, *pt;
    WINDOW *thiswin;

    thiswin = cw;
    if (prompt) {
	bool nogood;
	thiswin = hw;
	wclear(hw);
	wprintw(hw,"Item\tKey\n\n");
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_RING].mi_name,RING,
		things[TYP_STICK].mi_name,STICK);
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_POTION].mi_name,POTION,
		things[TYP_SCROLL].mi_name,SCROLL);
	wprintw(hw,"%s\t %c\n%s\t %c\n",things[TYP_ARMOR].mi_name,ARMOR,
		things[TYP_WEAPON].mi_name,WEAPON);
	wprintw(hw,"%s\t %c\n",things[TYP_FOOD].mi_name,FOOD);
	if (wizard) {
	    wprintw(hw,"%s\t %c\n",things[TYP_AMULET].mi_name,AMULET);
	    waddstr(hw,"monster\t m");
	}
	wprintw(hw,"\n\nWhat do you want to create? ");
	draw(hw);
	do {
	    ch = readchar();
	    if (ch == ESCAPE) {
		restscr(cw);
		return;
	    }
	    switch (ch) {
		case RING:
		case STICK:	
		case POTION:
		case SCROLL:	
		case ARMOR:	
		case WEAPON:
		case FOOD:
		    nogood = FALSE;
		    break;
		case 'm':
		    if (wizard) 
			nogood = FALSE;
		    break;
		case AMULET:
		    if (wizard)
			nogood = FALSE;
		    break;
		default:
		    nogood = TRUE;
	    }
	} while (nogood);
	newitem = ch;
    }
    else
	newitem = which_item;

    pt = "those";
    msz = 0;
    if(newitem == 'm') {
	makemons();		/* make monster and be done with it */
	return;
    }
    if(newitem == GOLD)
	pt = "gold";
    /* else if(isatrap(newitem))
	pt = "traps";
    */
    switch(newitem) {
	case POTION:	whc = TYP_POTION;	msz = MAXPOTIONS;
	when SCROLL:	whc = TYP_SCROLL;	msz = MAXSCROLLS;
	when FOOD:	whc = TYP_FOOD;		msz = MAXFOODS;
	when WEAPON:	whc = TYP_WEAPON;	msz = MAXWEAPONS;
	when ARMOR:	whc = TYP_ARMOR;	msz = MAXARMORS;
	when RING:	whc = TYP_RING;		msz = MAXRINGS;
	when STICK:	whc = TYP_STICK;	msz = MAXSTICKS;
	when AMULET:	
	    whc = TYP_AMULET;	
	    msz = MAXAMULET;
	    if (thiswin == hw)
		restscr(cw);
	    mpos = 0;
	otherwise:
	    if (thiswin == hw)
		restscr(cw);
	    mpos = 0;
	    msg("Even wizards can't create %s !!",pt);
	    return;
    }
    if(msz == 1) {		/* if only one type of item */
	ch = 'a';
    }
    else if (prompt) {
	struct magic_item *wmi;
	char wmn;
	int ii;

	mpos = 0;
	wmi = NULL;
	wmn = 0;
	switch(newitem) {
		case POTION:	wmi = &p_magic[0];
		when SCROLL:	wmi = &s_magic[0];
		when RING:	wmi = &r_magic[0];
		when STICK:	wmi = &ws_magic[0];
		when WEAPON:	wmn = 1;
		when ARMOR:	wmn = 2;
	}
	wclear(hw);
	thiswin = hw;
	if (wmi != NULL) {
	    for (ii = 0 ; ii < msz ; ii++) {
	        mvwaddch(hw,ii % 13,ii > 12 ? COLS/2 : 0, ii + 'a');
	        waddstr(hw,") ");
	        waddstr(hw,wmi->mi_name);
	        wmi++;
	    }
	}
	else if (wmn != 0) {
	    for(ii = 0 ; ii < msz ; ii++) {
	        mvwaddch(hw,ii % 13,ii > 12 ? COLS/2 : 0, ii + 'a');
	        waddstr(hw,") ");
	        if(wmn == 1)
		    waddstr(hw,weaps[ii].w_name);
	        else
		    waddstr(hw,armors[ii].a_name);
	    }
	}
	sprintf(prbuf,"Which %s? ",things[whc].mi_name);
	mvwaddstr(hw,LINES - 1, 0, prbuf);
	draw(hw);
	do {
	    ch = readchar();
	    if (ch == ESCAPE) {
	        restscr(cw);
	        msg("");
	        return;
	    }
	} until (isalpha(ch));
        if (thiswin == hw)			/* restore screen if need be */
	    restscr(cw);
        newtype = Tolower(ch) - 'a';
        if(newtype < 0 || newtype >= msz) {	/* if an illegal value */
	    mpos = 0;
	    msg("There is no such %s",things[whc].mi_name);
	    return;
        }
    }
    else 
	newtype = which_type;
    item = new_item(sizeof *obj);	/* get some memory */
    obj = OBJPTR(item);
    obj->o_type = newitem;		/* store the new items */
    obj->o_mark[0] = '\0';
    obj->o_which = newtype;
    obj->o_group = 0;
    obj->o_count = 1;
    obj->o_flags = 0;
    obj->o_dplus = obj->o_hplus = 0;
    obj->o_damage = obj->o_hurldmg = "0d0";
    obj->o_weight = 0;
    wh = obj->o_which;
    mpos = 0;
    if (!wizard)			/* users get 0 to +3 */
	whc = rnd(4);
    else			/* wizard gets to choose */
	whc = getbless();
    if (whc < 0)
	obj->o_flags |= ISCURSED;
    switch (obj->o_type) {
	case WEAPON:
	case ARMOR:
	    if (obj->o_type == WEAPON) {
		init_weapon(obj, wh);
		obj->o_hplus += whc;
		obj->o_dplus += whc;
	    }
	    else {				/* armor here */
		obj->o_weight = armors[wh].a_wght;
		obj->o_ac = armors[wh].a_class - whc;
	    }
	when RING:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    r_know[wh] = TRUE;
	    obj->o_ac = whc;
	    obj->o_weight = things[TYP_RING].mi_wght;
	when STICK:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    ws_know[wh] = TRUE;
	    fix_stick(obj);
	when AMULET:
	    obj->o_weight = things[TYP_AMULET].mi_wght;
	when SCROLL:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    obj->o_weight = things[TYP_SCROLL].mi_wght;
	    s_know[wh] = TRUE;
	when POTION:
	    if (whc > 0)
		obj->o_flags |= ISBLESSED;
	    obj->o_weight = things[TYP_POTION].mi_wght;
	    p_know[wh] = TRUE;
    }
    mpos = 0;
    obj->o_flags |= ISKNOW;
    if (add_pack(item, FALSE) == FALSE) {
	obj->o_pos = hero;
	fall(item, TRUE);
    }
}

/*
 * getbless:
 *	Get a blessing for a wizards object
 */
getbless()
{
	reg char bless;

	msg("Blessing? (+,-,n)");
	bless = readchar();
	if (bless == '+')
		return (rnd(3) + 1);
	else if (bless == '-')
		return (-rnd(3) - 1);
	else
		return (0);
}

/*
 * make a monster for the wizard
 */
makemons() 
{
    register int i;
    register short which_monst;
    register int num_monst = NUMMONST, pres_monst=1, num_lines=2*(LINES-3);
    char monst_name[LINELEN];

    /* Print out the monsters */
    while (num_monst > 0) {
	register left_limit;

	if (num_monst < num_lines) left_limit = (num_monst+1)/2;
	else left_limit = num_lines/2;

	wclear(hw);
	touchwin(hw);

	/* Print left column */
	wmove(hw, 2, 0);
	for (i=0; i<left_limit; i++) {
	    sprintf(monst_name, "[%d] %s\n",
				pres_monst, monsters[pres_monst].m_name);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	/* Print right column */
	for (i=0; i<left_limit && pres_monst<=NUMMONST; i++) {
	    sprintf(monst_name, "[%d] %s",
				pres_monst, monsters[pres_monst].m_name);
	    wmove(hw, i+2, COLS/2);
	    waddstr(hw, monst_name);
	    pres_monst++;
	}

	if ((num_monst -= num_lines) > 0) {
	    mvwaddstr(hw, LINES-1, 0, morestr);
	    draw(hw);
	    wait_for(' ');
	}

	else {
	    mvwaddstr(hw, 0, 0, "Which monster");
	    if (!terse) waddstr(hw, " do you wish to create");
	    waddstr(hw, "? ");
	    draw(hw);
	}
    }

get_monst:
    get_str(monst_name, hw);
    which_monst = atoi(monst_name);
    if ((which_monst < 1 || which_monst > NUMMONST)) {
	mvwaddstr(hw, 0, 0, "Please enter a number in the displayed range -- ");
	draw(hw);
	goto get_monst;
    }
    restscr(cw);
    creat_mons (&player, which_monst, TRUE);
    touchwin(cw);
}

/*
 * telport:
 *	Bamf the hero someplace else
 */

teleport()
{
    register struct room *new_rp, *old_rp = roomin(&hero);
    register int rm;
    coord c;
    bool is_lit = FALSE;	/* For saving room light state */

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    do
    {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == FLOOR);

    /* If hero gets moved, darken old room */
    new_rp = &rooms[rm];
    if (old_rp && old_rp != new_rp) {
	if (!(old_rp->r_flags & ISDARK)) is_lit = TRUE;
	old_rp->r_flags |= ISDARK;	/* Fake darkness */
	light(&c);
	if (is_lit) old_rp->r_flags &= ~ISDARK; /* Restore light state */
    }
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);

    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	register struct linked_list *ip, *nip;
	register struct thing *mp;

	turn_off(player, ISHELD);
	hold_count = 0;
	for (ip = mlist; ip; ip = nip) {
	    mp = (struct thing *) ldata(ip);
	    nip = next(ip);
	    if (on(*mp, DIDHOLD)) {
		turn_off(*mp, DIDHOLD);
		turn_on(*mp, CANHOLD);
	    }
	    turn_off(*mp, DIDSUFFOCATE); /* Suffocation -- see below */
	}
    }

    /* Make sure player does not suffocate */
    extinguish(suffocate);

    count = 0;
    running = FALSE;
    flushout();
    return rm;
}

/*
 * passwd:
 *	see if user knows password
 */

passwd()
{
    register char *sp, c;
    char buf[LINELEN], *crypt();

    msg("Wizard's Password:");
    mpos = 0;
    sp = buf;
    while ((c = readchar()) != '\n' && c != '\r' && c != '\033')
	if (c == killchar())
	    sp = buf;
	else if (c == erasechar() && sp > buf)
	    sp--;
	else if (sp < &buf[LINELEN - 1])
	    *sp++ = c;
    if (sp == buf)
	return FALSE;
    *sp = '\0';
    return ((sp = crypt(buf, "mT")) != NULL && strcmp(PASSWD, sp) == 0);
}
