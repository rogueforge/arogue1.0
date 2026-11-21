/*
 * Functions for dealing with problems brought about by weapons
 *
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"



/*
 * missile:
 *	Fire a missile in a given direction
 */

missile(ydelta, xdelta, item, tp)
	int ydelta, xdelta ;
	register struct linked_list *item ;
	register struct thing *tp ;
{
	register struct object *obj ;
	register struct linked_list *nitem ;

	/*
	* Get which thing we are hurling
	*/
	if (item == NULL) {
		return ;
	}
	obj = (struct object *) ldata(item) ;
	if (!dropcheck(obj) || is_current(obj)) {
		return ;
	}
	/*
	* Get rid of the thing. If it is a non-multiple item object, or
	* if it is the last thing, just drop it. Otherwise, create a new
	* item with a count of one.
	*/
	if (obj->o_count < 2) {
		detach(tp->t_pack, item) ;
		if (tp->t_pack == pack) {
			inpack-- ;
			if (obj->o_type == AMULET)
				amulet = FALSE ;
		}
	} else {
		obj->o_count-- ;
		nitem = (struct linked_list *) new_item(sizeof *obj) ;
		obj = (struct object *) ldata(nitem) ;
		*obj = *((struct object *) ldata(item)) ;
		obj->o_count = 1 ;
		item = nitem ;
	}
	updpack (FALSE);
	do_motion(obj, ydelta, xdelta, tp) ;
	/*
	* AHA! Here it has hit something. If it is a wall or a door,
	* or if it misses (combat) the monster, put it on the floor
	*/
	if (!hit_monster(unc(obj->o_pos), obj, tp)) {
		fall(item, TRUE) ;
	}
	else
		discard(item) ;
	mvwaddch(cw, hero.y, hero.x, PLAYER) ;
}

/*
 * do the actual motion on the screen done by an object traveling
 * across the room
 */
do_motion(obj, ydelta, xdelta, tp)
	register struct object *obj ;
	register int ydelta, xdelta ;
	register struct thing *tp ;
{

	/*
	* Come fly with us ...
	*/
	obj->o_pos = tp->t_pos ;
	for ( ; ;) {
		register int ch ;
		/*
		* Erase the old one
		*/
		if (!ce(obj->o_pos, tp->t_pos) &&
		    cansee(unc(obj->o_pos)) &&
		    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
			mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, show(obj->o_pos.y, obj->o_pos.x)) ;
		}
		/*
		* Get the new position
		*/
		obj->o_pos.y += ydelta ;
		obj->o_pos.x += xdelta ;
		if (shoot_ok(ch = winat(obj->o_pos.y, obj->o_pos.x)) && ch != DOOR && !ce(obj->o_pos, hero)) {
			/*
			* It hasn't hit anything yet, so display it
			* If it alright.
			*/
			if (cansee(unc(obj->o_pos)) &&
			    mvwinch(cw, obj->o_pos.y, obj->o_pos.x) != ' ') {
				mvwaddch(cw, obj->o_pos.y, obj->o_pos.x, obj->o_type) ;
				draw(cw) ;
			}
			continue ;
		}
		break ;
	}
}

/*
 * fall:
 *	Drop an item someplace around here.
 */

fall(item, pr)
	register struct linked_list *item ;
	bool pr ;
{
	register struct object *obj ;
	register struct room *rp ;
	static coord fpos ;

	obj = (struct object *) ldata(item) ;
	if (fallpos(&obj->o_pos, &fpos, TRUE)) {
		mvaddch(fpos.y, fpos.x, obj->o_type) ;
		obj->o_pos = fpos ;
		if ((rp = roomin(&hero)) != NULL &&
		    (!(rp->r_flags & ISDARK) ||
		    (rp->r_flags & HASFIRE))) {
			light(&hero) ;
			mvwaddch(cw, hero.y, hero.x, PLAYER) ;
		}
		attach(lvl_obj, item) ;
		return ;
	}
	if (pr) {
		msg("The %s vanishes as it hits the ground.",
		    obj->o_type == WEAPON ? weaps[obj->o_which].w_name : "object") ;		
	}
	discard(item) ;
}

/*
 * init_weapon:
 *	Set up the initial goodies for a weapon
 */

init_weapon(weap, type)
	register struct object *weap ;
	char type ;
{
	register struct init_weps *iwp ;

	iwp = &weaps[type] ;
	weap->o_damage = iwp->w_dam ;
	weap->o_hurldmg = iwp->w_hrl ;
	weap->o_launch = iwp->w_launch ;
	weap->o_flags = iwp->w_flags ;
	weap->o_weight = iwp->w_wght;
	if (weap->o_flags & ISMANY) {
		weap->o_count = rnd(8) + 8 ;
		weap->o_group = newgrp() ;
	} else {
		weap->o_count = 1 ;
	}
}

/*
 * Does the missile hit the monster
 */

hit_monster(y, x, obj, tp)
	register int y, x ;
	struct object *obj ;
	register struct thing *tp ;
{
	static coord mp ;

	mp.y = y ;
	mp.x = x ;
	if (tp == &player) {
		/* Make sure there is a monster where it landed */
		if (!isalpha(mvwinch(mw, y, x))) {
			return(FALSE) ;
		}
		return(fight(&mp, winat(y, x), obj, TRUE)) ;
	} else {
		if (!ce(mp, hero)) {
			return(FALSE) ;
		}
		return(attack(tp, obj, TRUE)) ;
	}
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char *num(n1, n2)
	register int n1, n2 ;
{
	static char numbuf[LINELEN] ;

	if (n1 == 0 && n2 == 0) {
		return "+0" ;
	}
	if (n2 == 0) {
		sprintf(numbuf, "%s%d", n1 < 0 ? "" : "+", n1) ;
	} else {
		sprintf(numbuf, "%s%d, %s%d", n1 < 0 ? "" : "+", n1, n2 < 0 ? "" : "+", n2) ;
	}
	return(numbuf) ;
}

/*
 * wield:
 *	Pull out a certain weapon
 */

wield()
{
	register struct linked_list *item ;
	register struct object *obj, *oweapon ;

	oweapon = cur_weapon ;
	if (!dropcheck(cur_weapon)) {
		cur_weapon = oweapon ;
		return ;
	}
	cur_weapon = oweapon ;
	if ((item = get_item("wield", WEAPON)) == NULL) {
		after = FALSE ;
		return ;
	}
	obj = (struct object *) ldata(item) ;
	if (obj->o_type != WEAPON) {
	    msg ("You can't wield that!");
	    return;
	}
	if (is_current(obj)) {
		after = FALSE ;
		return ;
	}
	if (terse) {
		addmsg("W") ;
	} else {
		addmsg("You are now w") ;
	}
	msg("ielding %s", inv_name(obj, TRUE, -1)) ;
	cur_weapon = obj ;
}

/*
 * pick a random position around the give (y, x) coordinates
 */
fallpos(pos, newpos, passages)
	register coord *pos, *newpos ;
	register bool passages ;
{
	register int y, x, cnt, ch ;

	cnt = 0 ;
	for (y = pos->y - 1 ; y <= pos->y + 1 ; y++) {
		for (x = pos->x - 1 ; x <= pos->x + 1 ; x++) {
		/*
			check to make certain the spot is empty, if it is,
			put the object there, set it in the level list
			and re-draw the room if he can see it
		*/
			if (y == hero.y && x == hero.x) {
				continue ;
			}
			if (((ch = winat(y, x)) == FLOOR ||
			    (passages && ch == PASSAGE)) &&
			    rnd(++cnt) == 0) {
				newpos->y = y ;
				newpos->x = x ;
			}
		}
	}
	return (cnt != 0) ;
}
