/*
 * All the fighting gets done here
 *
 */

#include "curses.h"
#include <ctype.h>
#include "rogue.h"

/* 
 * This are the beginning experience levels for all players
 * all further experience levels are computed by muliplying by 2
 */
static long e_levels[4] = {
	113L,	/* Fighter */
	135L,	/* Magician */
	87L,	/* Cleric */
	72L,	/* Thief */
};

struct matrix att_mat[5] = {
/* Base		Max_lvl,	Factor,		Offset,		Range */
{  10,		17,		2,		1,		2 },
{  9,		21,		2,		1,		5 },
{  10,		19,		2,		1,		3 },
{  10,		21,		2,		1,		4 },
{   7,		25,		1,		0,		2 }
};

/*
 * fight:
 *	The player attacks the monster.
 */

fight(mp, mn, weap, thrown)
register coord *mp;
char mn;
struct object *weap;
bool thrown;
{
    register struct thing *tp;
    register struct linked_list *item;
    register bool did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL)
	debug("Fight what @ %d,%d", mp->y, mp->x);
    tp = (struct thing *) ldata(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    player.t_quiet = 0;
    tp->t_quiet = 0;
    runto(mp, &hero);
    /*
     * Let him know it was really a mimic (if it was one).
     */
    if (on(*tp, ISDISGUISE) && (tp->t_type != tp->t_disguise) &&
	off(player, ISBLIND))
    {
	msg("Wait! That's a %s!", monsters[tp->t_index].m_name);
	turn_off(*tp, ISDISGUISE);
	did_hit = thrown;
    }
    if (on(*tp, CANSURPRISE) && off(player, ISBLIND)) {
	msg("Wait! There's a %s!", monsters[tp->t_index].m_name);
	turn_off(*tp, CANSURPRISE);
	did_hit = thrown;
    }

    if (did_hit)
    {
	register char *mname;

	did_hit = FALSE;
	mname = (on(player, ISBLIND)) ? "it" : monsters[tp->t_index].m_name;
	if (!can_blink(tp) &&
	    (off(*tp, MAGICHIT) || weap->o_hplus > 0 || weap->o_dplus > 0) &&
	    (off(*tp, BMAGICHIT) || weap->o_hplus > 1 || weap->o_dplus > 1) &&
	    roll_em(&player, tp, weap, thrown, cur_weapon))
	{
	    did_hit = TRUE;
	    tp->t_wasshot = TRUE;

	    if (thrown)
		thunk(weap, mname);
	    else
		hit(NULL, mname);

	    /* If the player hit a rust monster, he better have a + weapon */
	    if (on(*tp, CANRUST) && !thrown &&
		(cur_weapon->o_flags & ISMETAL) &&
		!(cur_weapon->o_flags & ISPROT) &&
		(cur_weapon->o_hplus < 1) && (cur_weapon->o_dplus < 1)) {
		if (rnd(100) < 50) cur_weapon->o_hplus--;
		else cur_weapon->o_dplus--;
		msg(terse ? "Your %s weakens!"
			  : "Your %s appears to be weaker now!",
		    weaps[cur_weapon->o_which].w_name);
	    }
		
	    /* If the player hit something that shrieks, wake the dungeon */
	    if (on(*tp, CANSHRIEK)) {
		turn_off(*tp, CANSHRIEK);
		msg("The %s emits a piercing shriek.", mname);
		aggravate();
	    }

	    /* If the player hit something that can surprise, it can't now */
	    if (on(*tp, CANSURPRISE)) turn_off(*tp, CANSURPRISE);

	    /* If the player hit something that can summon, it will try to */
	    if (on(*tp, CANSUMMON) && rnd(40) < tp->t_stats.s_lvl) {
	        register char *helpname;
	        register int which, i;

		turn_off(*tp, CANSUMMON);
	        msg("the %s summons help", mname);
	        helpname = monsters[tp->t_index].m_typesum;
	        for (which=1; which<NUMMONST; which++) {
		     if (strcmp(helpname, monsters[which].m_name) == NULL)
		         break;
	        }
	        if (which >= NUMMONST)
		     debug("couldn't find summoned one");
	        for (i=0; i<monsters[tp->t_index].m_numsum; i++)
		     creat_mons(&player, which, FALSE);
	    }

	    /* Can the player confuse? */
	    if (on(player, CANHUH) && !thrown)
	    {
		msg("Your hands stop glowing red");
		msg("The %s appears confused.", mname);
		turn_on(*tp, ISHUH);
		turn_off(player, CANHUH);
	    }
	    /* Merchants just disappear if hit */
	    if (on(*tp, CANSELL)) {
		msg("The %s disappears with his wares in a flash.",mname);
		killed(item, FALSE, FALSE);
	    }

	    else if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE, TRUE);

	    /* If the monster is fairly intelligent and about to die, it
	     * may turn tail and run.
	     */
	    else if ((tp->t_stats.s_hpt < max(10, tp->maxstats.s_hpt/10)) &&
		     (rnd(25) < tp->t_stats.s_intel)) {
		turn_on(*tp, ISFLEE);

		/* If monster was suffocating, stop it */
		if (on(*tp, DIDSUFFOCATE)) {
		    turn_off(*tp, DIDSUFFOCATE);
		    extinguish(suffocate);
		}

		/* If monster held us, stop it */
		if (on(*tp, DIDHOLD) && (--hold_count == 0))
			turn_off(player, ISHELD);
		turn_off(*tp, DIDHOLD);
	    }
	}
	else {
	    if (thrown)
		bounce(weap, mname);
	    else
		miss(NULL, mname);
	}
    }
    count = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */

attack(mp, weapon, thrown)
register struct thing *mp;
register struct object *weapon;
bool thrown;
{
    register char *mname;
    register bool did_hit = FALSE;

    /* If the monster is in a wall, it cannot attack */
    if (on(*mp, ISINWALL)) return(FALSE);

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    player.t_quiet = 0;
    mp->t_quiet = 0;

    if (on(*mp, ISDISGUISE) && off(player, ISBLIND))
	turn_off(*mp, ISDISGUISE);
    mname = on(player, ISBLIND) ? "it" : monsters[mp->t_index].m_name;
    if (roll_em(mp, &player, weapon, thrown, wield_weap(weapon, mp)))
    {
	did_hit = TRUE;

	if (thrown) m_thunk(weapon, mname);
	else hit(mname, NULL);

	if (pstats.s_hpt <= 0)
	    death(mp->t_index);	/* Bye bye life ... */
	/*
	 * suprising monsters appear after they shoot at you 
	 */
	if (thrown) {
	    if (on(*mp, CANSURPRISE)) 
		turn_off(*mp, CANSURPRISE);
	}
	if (!thrown) {
	    /*
	     * If a vampire hits, it may take half your hit points
	     */
	    if (on(*mp, CANSUCK) && !save(VS_MAGIC)) {
		if (pstats.s_hpt == 1) death(mp->t_index);
		else {
		  pstats.s_hpt /= 2;
		  msg("You feel your life force being drawn from you.");
		}
	    }

	    /*
	     * Stinking monsters make player weaker (to hit)
	     */
	    if (on(*mp, CANSTINK)) {
		turn_off(*mp, CANSTINK);
		if (!save(VS_POISON)) {
		    msg("The stench of the %s sickens you.", mname);
		    if (on(player, HASSTINK)) lengthen(unstink, STINKTIME);
		    else {
			turn_on(player, HASSTINK);
			fuse(unstink, 0, STINKTIME, AFTER);
		    }
		}
	    }

	    /*
	     * Chilling monster reduces strength each time
	     */
	    if (on(*mp, CANCHILL)) {
		msg("You cringe at the %s's chilling touch.", mname);
		if (!ISWEARING(R_SUSABILITY)) {
		    chg_str(-1);
		    if (lost_str++ == 0)
			fuse(res_strength, 0, CHILLTIME, AFTER);
		    else lengthen(res_strength, CHILLTIME);
		}
	    }

	    /*
	     * itching monsters reduce dexterity (temporarily)
	     */
	    if (on(*mp, CANITCH) && !save(VS_POISON)) {
		msg("The claws of the %s scratch you", mname);
		if(ISWEARING(R_SUSABILITY)) {
		    msg("The scratch has no effect");
		}
		else {
		    msg("You feel a burning itch");
		    turn_on(player, HASITCH);
		    add_dexterity(TRUE);
		    lost_dext++;
		    fuse(un_itch, 0, roll(4,6), AFTER);
		}
	    }


	    /*
	     * If a hugging monster hits, it may SQUEEEEEEEZE
	     */
	    if (on(*mp, CANHUG)) {
		if (roll(1,20) >= 18 || roll(1,20) >= 18) {
		    msg("The %s squeezes you against itself.", mname);
		    if ((pstats.s_hpt -= roll(2,8)) <= 0)
			death(mp->t_index);
		}
	    }

	    /*
	     * If a disease-carrying monster hits, there is a chance the
	     * player will catch the disease
	     */
	    if (on(*mp, CANDISEASE) &&
		(rnd(pstats.s_const) < mp->t_stats.s_lvl) &&
		off(player, HASDISEASE)) {
		if (ISWEARING(R_HEALTH)) msg("The wound heals quickly.");
		else {
		    turn_on(player, HASDISEASE);
		    fuse(cure_disease, 0, roll(4,4) * SICKTIME, AFTER);
		    msg(terse ? "You have been diseased."
			: "You have contracted a disease!");
		}
	    }

	    /*
	     * If a rust monster hits, you lose armor
	     */
	    if (on(*mp, CANRUST) && cur_armor != NULL &&
		cur_armor->o_which != LEATHER &&
		cur_armor->o_which != PADDED_ARMOR &&
		!(cur_armor->o_flags & ISPROT) &&
		cur_armor->o_ac < pstats.s_arm+1) {
		msg(terse ? "Your armor weakens"
		    : "Your armor appears to be weaker now. Oh my!");
		cur_armor->o_ac++;
	    }

	    /* If a surprising monster hit you, you can see it now */
	    if (on(*mp, CANSURPRISE)) turn_off(*mp, CANSURPRISE);

	    /*
	     * If an infesting monster hits you, you get a parasite or rot
	     */
	    if (on(*mp, CANINFEST) && rnd(pstats.s_const) < mp->t_stats.s_lvl) {
		if (ISWEARING(R_HEALTH)) msg("The wound quickly heals.");
		else {
		    turn_off(*mp, CANINFEST);
		    msg(terse ? "You have been infested."
			: "You have contracted a parasitic infestation!");
		    infest_dam++;
		    turn_on(player, HASINFEST);
		}
	    }

	    /*
	     * Ants have poisonous bites
	     */
	    if (on(*mp, CANPOISON) && !save(VS_POISON)) {
		if (ISWEARING(R_SUSABILITY))
		    msg(terse ? "Sting has no effect"
			      : "A sting momentarily weakens you");
		else {
		    chg_str(-1);
		    msg(terse ? "A sting has weakened you" :
		    "You feel a sting in your arm and now feel weaker");
		}
	    }

	    /*
	     * Cause fear by touching
	     */
	    if (on(*mp, TOUCHFEAR)) {
		turn_off(*mp, TOUCHFEAR);
		if (!save(VS_WAND) &&
		    !(on(player, ISFLEE) && (player.t_dest == &mp->t_pos))) {
			turn_on(player, ISFLEE);
			player.t_dest = &mp->t_pos;
			msg("The %s's touch terrifies you.", mname);
		}
	    }

	    /*
	     * Suffocating our hero
	     */
	    if (on(*mp, CANSUFFOCATE) && (rnd(100) < 15) &&
		(find_slot(suffocate) == NULL)) {
		turn_on(*mp, DIDSUFFOCATE);
		msg("The %s is beginning to suffocate you.", mname);
		fuse(suffocate, 0, roll(4,2), AFTER);
	    }

	    /*
	     * Turning to stone
	     */
	    if (on(*mp, TOUCHSTONE)) {
		turn_off(*mp, TOUCHSTONE);
		if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		else {
		    if (!save(VS_PETRIFICATION) && rnd(100) < 3) {
		        msg("Your body begins to solidify.");
		        msg("You are turned to stone !!! --More--");
		        wait_for(' ');
			death(D_PETRIFY);
		    }
		    else {
			msg("The %s's touch stiffens your limbs.", mname);
			no_command = STONETIME;
		    }
		}
	    }

	    /*
	     * Wraiths might drain energy levels
	     */
	    if ((on(*mp, CANDRAIN) || on(*mp, DOUBLEDRAIN)) && rnd(100) < 15) {
		lower_level(mp->t_index);
		if (on(*mp, DOUBLEDRAIN)) lower_level(mp->t_index);
		turn_on(*mp, DIDDRAIN);  
	    }

	    /*
	     * Violet fungi stops the poor guy from moving
	     */
	    if (on(*mp, CANHOLD) && off(*mp, DIDHOLD)) {
		turn_on(player, ISHELD);
		turn_on(*mp, DIDHOLD);
		hold_count++;
	    }

	    /*
	     * Sucker will suck blood and run
	     */
	    if (on(*mp, CANDRAW)) {
		turn_off(*mp, CANDRAW);
		turn_on(*mp, ISFLEE);
		msg("The %s sates itself with your blood.", mname);
		if ((pstats.s_hpt -= 12) <= 0) death(mp->t_index);
	    }

	    /*
	     * Bad smell will force a reduction in strength
	     */
	    if (on(*mp, CANSMELL)) {
		turn_off(*mp, CANSMELL);
		if (save(VS_MAGIC) || ISWEARING(R_SUSABILITY))
		    msg("You smell an unpleasant odor.");
		else {
		    short odor_str = -(rnd(6)+1);

		    msg("You are overcome by a foul odor.");
		    if (lost_str == 0) {
			chg_str(odor_str);
			fuse(res_strength, 0, SMELLTIME, AFTER);
		    }
		    else lengthen(res_strength, SMELLTIME);
		    lost_str -= odor_str;
		}
	    }

	    /*
	     * Paralyzation
	     */
	    if (on(*mp, CANPARALYZE)) {
		turn_off(*mp, CANPARALYZE);
		if (!save(VS_PARALYZATION) && no_command == 0) {
		    if (on(player, CANINWALL))
			msg("The %s's touch has no effect.", mname);
		    else {
			msg("The %s's touch paralyzes you.", mname);
			no_command = FREEZETIME;
		    }
		}
	    }

	    /*
	     * Rotting
	     */
	    if (on(*mp, CANROT)) {
		turn_off(*mp, CANROT);
		turn_on(*mp, DOROT);
	    }

	    if (on(*mp, STEALGOLD)) {
		/*
		 * Leperachaun steals some gold
		 */
		register long lastpurse;
		register struct linked_list *item;
		register struct object *obj;

		lastpurse = purse;
		purse -= GOLDCALC;
		if (!save(VS_MAGIC))
		    purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		if (purse < 0)
		    purse = 0;
		if (purse != lastpurse) {
		    msg("Your purse feels lighter");

		    /* Give the gold to the thief */
		    for (item=mp->t_pack; item != NULL; item=next(item)) {
			obj = (struct object *) ldata(item);
			if (obj->o_type == GOLD) {
			    obj->o_count += lastpurse - purse;
			    break;
			}
		    }

		    /* Did we do it? */
		    if (item == NULL) {	/* Then make some */
			item = new_item(sizeof *obj);
			obj = (struct object *) ldata(item);
			obj->o_type = GOLD;
			obj->o_count = lastpurse - purse;
			obj->o_hplus = obj->o_dplus = 0;
			obj->o_damage = obj->o_hurldmg = "0d0";
			obj->o_ac = 11;
			obj->o_group = 0;
			obj->o_flags = 0;
			obj->o_mark[0] = '\0';
			obj->o_pos = mp->t_pos;

			attach(mp->t_pack, item);
		    }
		}

		turn_on(*mp, ISFLEE);
		turn_on(*mp, ISINVIS);
	    }

	    if (on(*mp, STEALMAGIC)) {
		register struct linked_list *list, *steal;
		register struct object *obj;
		register int nobj;

		/*
		 * Nymph's steal a magic item, look through the pack
		 * and pick out one we like.
		 */
		steal = NULL;
		for (nobj = 0, list = pack; list != NULL; list = next(list))
		{
		    obj = (struct object *) ldata(list);
		    if (obj != cur_armor && obj != cur_weapon &&
			obj != cur_ring[LEFT_1] && obj != cur_ring[LEFT_2] &&
			obj != cur_ring[LEFT_3] && obj != cur_ring[LEFT_4] &&
			obj != cur_ring[RIGHT_1] && obj != cur_ring[RIGHT_2] &&
			obj != cur_ring[RIGHT_3] && obj != cur_ring[RIGHT_4] &&
			!(obj->o_flags & ISPROT) &&
			is_magic(obj) && rnd(++nobj) == 0)
			    steal = list;
		}
		if (steal != NULL)
		{
		    register struct object *obj;

		    obj = (struct object *) ldata(steal);
		    remove_mon(&mp->t_pos, find_mons(mp->t_pos.y, mp->t_pos.x));
		    monst_dead = TRUE;
		    if (obj->o_count > 1 && obj->o_group == 0)
		    {
			register int oc;

			oc = --(obj->o_count);
			obj->o_count = 1;
			msg("She stole %s!", inv_name(obj, TRUE, -1));
			obj->o_count = oc;
		    }
		    else
		    {
			msg("She stole %s!", inv_name(obj, TRUE, -1));
			if (obj->o_type == AMULET)
			    amulet = FALSE;
			detach(pack, steal);
			discard(steal);
		        inpack--;
		    }
		    updpack(FALSE);
		}
	    }
	}
    }
    else {
	/* If the thing was trying to surprise, no good */
	if (on(*mp, CANSURPRISE)) turn_off(*mp, CANSURPRISE);

	if (on(*mp, DOROT)) {
	    msg(terse ? "You feel weaker."
		      : "Your skin crawls and you feel weaker.");

	    pstats.s_hpt -= 2;
	    if (pstats.s_hpt <= 0)
		death(mp->t_index);	/* Bye bye life ... */
	}
	else if (thrown) m_bounce(weapon, mname);
	else miss(mname, NULL);
    }
    if (fight_flush)
	flushout();
    count = 0;
    status(FALSE);
    return(did_hit);
}

/*
 * swing:
 *	returns true if the swing hits
 */

swing(class, at_lvl, op_arm, wplus)
short class;
int at_lvl, op_arm, wplus;
{
    register int res = rnd(20)+1;
    register int need;

    need = att_mat[class].base -
	   att_mat[class].factor *
	   ((min(at_lvl, att_mat[class].max_lvl) -
	    att_mat[class].offset)/att_mat[class].range) +
	   (10 - op_arm);
    if (need > 20 && need <= 25) need = 20;

    return (res+wplus >= need);
}

/*
 * check_level:
 *	Check to see if the guy has gone up a level.
 */

check_level()
{
    register int i, j, add = 0;
    register unsigned long exp, oexp;
    int nsides;

    i = 0;
    oexp = exp = e_levels[player.t_ctype];
    while (exp <= pstats.s_exp) {
	i++;
	exp *= 2L;
	if (exp < oexp)
		break;
	oexp = exp;
    }
    if (++i > pstats.s_lvl) {
	switch (player.t_ctype) {
	    when C_FIGHTER:	nsides = 10;
	    when C_MAGICIAN:	nsides = 4;
	    when C_CLERIC:	nsides = 8;
	    when C_THIEF:	nsides = 6;
	}

	/* Take care of multi-level jumps */
	for (j=0; j < (i-pstats.s_lvl); j++)
	    add += max(1, roll(1,nsides) + const_bonus());
	max_stats.s_hpt += add;
	if ((pstats.s_hpt += add) > max_stats.s_hpt)
	    pstats.s_hpt = max_stats.s_hpt;
	msg("Welcome, %s, to level %d",
	    cnames[player.t_ctype][min(i-1, 10)], i);
	pray_time = 0;	/* A new round of prayers */
	spell_power = 0; /* A new round of spells */
    }
    pstats.s_lvl = i;
}

/*
 * roll_em:
 *	Roll several attacks
 */

roll_em(att_er, def_er, weap, hurl, cur_weapon)
struct thing *att_er, *def_er;
struct object *weap;
bool hurl;
struct object *cur_weapon;
{
    register struct stats *att, *def;
    register char *cp;
    register int ndice, nsides, nplus, def_arm;
    register bool did_hit = FALSE;
    register int prop_hplus, prop_dplus;
    char *oldcp;
    char *strchr();	/* strchr was index */

    /* Get statistics */
    att = &att_er->t_stats;
    def = &def_er->t_stats;

    prop_hplus = prop_dplus = 0;
    if (weap == NULL)
	cp = att->s_dmg;
    else if (hurl)
	if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch)
	{
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	}
	else
	    cp = (weap->o_flags&ISMISL ? weap->o_damage : weap->o_hurldmg);
    else
    {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == STICK && weap->o_which == WS_HIT
	    && weap->o_charges == 0)
		{
		    weap->o_damage = "0d0";
		    weap->o_hplus = weap->o_dplus = 0;
		}
    }
    for (;;)
    {
	int damage;
	int hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	/* Is attacker weak? */
	if (on(*att_er, HASSTINK)) hplus -= 2;

	if (weap == cur_weapon)	/* Is it the player? */
	{
	    hplus += hitweight();	/* adjust for encumberence */
	    dplus += hung_dam();	/* adjust damage for hungry player */
	    dplus += ring_value(R_ADDDAM);
	}
	ndice = atoi(cp);
	if ((cp = strchr(cp, 'd')) == NULL)	/* strchr was index */
	    break;
	nsides = atoi(++cp);
	oldcp = cp;
	if ((cp = strchr(cp, '+')) != NULL) nplus = atoi(++cp);
	else {
	    nplus = 0;
	    cp = oldcp;
	}

	if (def == &pstats)
	{
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac;
	    else
		def_arm = def->s_arm;
	    def_arm -= ring_value(R_PROTECT);
	}
	else
	    def_arm = def->s_arm;
	if (swing(att_er->t_ctype, att->s_lvl, def_arm-dext_prot(def->s_dext),
		  hplus+str_plus(att->s_str)+dext_plus(att->s_dext)))
	{
	    register int proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1)
		debug("Damage for %dd%d came out %d.", ndice, nsides, proll);
	    damage = dplus + proll + nplus + add_dam(att->s_str);

	    /* Check for half damage monsters */
	    if (on(*def_er, HALFDAMAGE)) damage /= 2;

	    /* Check for no-damage and division */
	    if (on(*def_er, BLOWDIVIDE)) {
		damage = 0;
		creat_mons(def_er, def_er->t_index, FALSE);
	    }

	    def->s_hpt -= max(0, damage);	/* Do the damage */
	    did_hit = TRUE;
	}
	if ((cp = strchr(cp, '/')) == NULL)	/* strchr was index */
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */

char *
prname(who, upper)
register char *who;
bool upper;
{
    static char tbuf[LINELEN];

    *tbuf = '\0';
    if (who == 0)
	strcpy(tbuf, "you"); 
    else if (on(player, ISBLIND))
	strcpy(tbuf, "it");
    else
    {
	strcpy(tbuf, "the ");
	strcat(tbuf, who);
    }
    if (upper)
	*tbuf = toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */

hit(er, ee)
register char *er, *ee;
{
    register char *s;

    addmsg(prname(er, TRUE));
    if (terse)
	s = " hit.";
    else
	switch (rnd(4))
	{
	    when 0: s = " scored an excellent hit on ";
	    when 1: s = " hit ";
	    when 2: s = (er == 0 ? " have injured " : " has injured ");
	    when 3: s = (er == 0 ? " swing and hit " : " swings and hits ");
	}
    addmsg(s);
    if (!terse)
	addmsg(prname(ee, FALSE));
    endmsg();
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */

miss(er, ee)
register char *er, *ee;
{
    register char *s;

    addmsg(prname(er, TRUE));
    switch (terse ? 0 : rnd(4))
    {
	when 0: s = (er == 0 ? " miss" : " misses");
	when 1: s = (er == 0 ? " swing and miss" : " swings and misses");
	when 2: s = (er == 0 ? " barely miss" : " barely misses");
	when 3: s = (er == 0 ? " don't hit" : " doesn't hit");
    }
    addmsg(s);
    if (!terse)
	addmsg(" %s", prname(ee, FALSE));
    endmsg();
}

/*
 * save_throw:
 *	See if a creature save against something
 */
save_throw(which, tp)
int which;
struct thing *tp;
{
    register int need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}
/*
 * save:
 *	See if he saves against various nasty things
 */

save(which)
int which;
{
    return save_throw(which, &player);
}

/*
 * dext_plus:
 *	compute to-hit bonus for dexterity
 */

dext_plus(dexterity)
register int dexterity;
{
    return ((dexterity-10)/3);
}


/*
 * dext_prot:
 *	compute armor class bonus for dexterity
 */

dext_prot(dexterity)
register int dexterity;
{
    return ((dexterity-9)/2);
}
/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */

str_plus(str)
register short str;
{
    return((str-10)/3);
}

/*
 * add_dam:
 *	compute additional damage done for exceptionally high or low strength
 */

add_dam(str)
register short str;
{
    return((str-9)/2);
}

/*
 * hung_dam:
 *	Calculate damage depending on players hungry state
 */
hung_dam()
{
	reg int howmuch;

	switch(hungry_state) {
		case F_OK:
		case F_HUNGRY:	howmuch = 0;
		when F_WEAK:	howmuch = -1;
		when F_FAINT:	howmuch = -2;
	}
	return howmuch;
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */

raise_level()
{
    unsigned long exp;

    if (pstats.s_exp < e_levels[player.t_ctype]/2)
	pstats.s_exp = e_levels[player.t_ctype];
    else {
	exp = pstats.s_exp;
	pstats.s_exp *= 2;
	if (exp > pstats.s_exp)
		pstats.s_exp = exp;
    }
    check_level();
}

/*
 * thunk:
 *	A missile hits a monster
 */

thunk(weap, mname)
register struct object *weap;
register char *mname;
{
    if (weap->o_type == WEAPON)
	msg("The %s hits %s", weaps[weap->o_which].w_name,prname(mname, FALSE));
    else
	msg("You hit %s.", prname(mname, FALSE));
}

/*
 * mthunk:
 *	 A missile from a monster hits the player
 */

m_thunk(weap, mname)
register struct object *weap;
register char *mname;
{
    if (weap->o_type == WEAPON)
	msg("%s's %s hits you.", prname(mname, TRUE), weaps[weap->o_which].w_name);
    else
	msg("%s hits you.", prname(mname, TRUE));
}

/*
 * bounce:
 *	A missile misses a monster
 */

bounce(weap, mname)
register struct object *weap;
register char *mname;
{
    if (weap->o_type == WEAPON)
	msg("The %s misses %s",weaps[weap->o_which].w_name,prname(mname,FALSE));
    else
	msg("You missed %s.", prname(mname, FALSE));
}

/*
 * m_bounce:
	  A missle from a monster misses the player
 */

m_bounce(weap, mname)
register struct object *weap;
register char *mname;
{
    if (weap->o_type == WEAPON)
	msg("%s's %s misses you.", prname(mname, TRUE), weaps[weap->o_which].w_name);
    else
	msg("%s misses you.", prname(mname, TRUE));
}

/*
 * remove_mon:
 *	Remove a monster from the screen
 */
remove_mon(mp, item)
register coord *mp;
register struct linked_list *item;
{
    mvwaddch(mw, mp->y, mp->x, ' ');
    mvwaddch(cw, mp->y, mp->x, ((struct thing *) ldata(item))->t_oldch);
    free_list(((struct thing *) ldata(item))->t_pack);
    detach(mlist, item);
    discard(item);
    monst_dead = TRUE;
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */

is_magic(obj)
register struct object *obj;
{
    switch (obj->o_type)
    {
	case ARMOR:
	    return obj->o_ac != armors[obj->o_which].a_class;
	when WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when POTION:
	case SCROLL:
	case STICK:
	case RING:
	case AMULET:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *	Called to put a monster to death
 */

killed(item, pr, points)
register struct linked_list *item;
bool pr, points;
{
    register struct thing *tp;
    register struct linked_list *pitem, *nexti;
    unsigned long exp;
    coord pos;

    tp = (struct thing *) ldata(item);
    if (pr)
    {
	addmsg(terse ? "Defeated " : "You have defeated ");
	if (on(player, ISBLIND))
	    msg("it.");
	else
	{
	    if (!terse)
		addmsg("the ");
	    msg("%s.", monsters[tp->t_index].m_name);
	}
    }

    /* Take care of any residual effects of the monster */
    check_residue(tp);

    if (points) {
	exp = pstats.s_exp;
	pstats.s_exp += tp->t_stats.s_exp;
	if (pstats.s_exp < exp)
		pstats.s_exp = exp;

	/*
	 * Do adjustments if he went up a level
	 */
	check_level();
    }

    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    tp->t_pack = NULL;
    pos = tp->t_pos;

    /*
     * Get rid of the monster.
     */
    remove_mon(&tp->t_pos, item);
    while (pitem != NULL)
    {
	register struct object *obj;

	nexti = next(pitem);
	obj = (struct object *) ldata(pitem);
	obj->o_pos = pos;

	if (points) fall(pitem, FALSE);
	else discard(pitem);

	pitem = nexti;
    }
}


/* Returns a pointer to the weapon the monster is wielding corresponding to
 * the given thrown weapon
 */

struct object *
wield_weap(weapon, mp)
struct object *weapon;
struct thing *mp;
{
    int look_for;
    register struct linked_list *pitem;

    if (weapon == NULL)
	return(NULL);
    switch (weapon->o_which) {
	case BOLT:	/* Find the crossbow */
	    look_for = CROSSBOW;
	    break;
	case ARROW:	/* Find the bow */
	    look_for = BOW;
	    break;
	case ROCK:	/* find the sling */
	    look_for = SLING;
	    break;
	default:
	    return(NULL);
    }

    for (pitem=mp->t_pack; pitem; pitem=next(pitem))
	if (((struct object *) ldata(pitem))->o_which == look_for)
	    return((struct object *) ldata(pitem));

    return(NULL);
}
