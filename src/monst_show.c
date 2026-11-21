#include "curses.h"
#include "rogue.h"
#include <ctype.h>
#include <stdio.h>

#define MONST	"monsters"

monst_show()
{
	register struct monster *m;
	FILE *fp;
	int i;
	char *getflag();

	if ((fp = fopen(MONST, "w")) == NULL) {
		msg("Couldn't create %s", MONST);
		return;
	}
	for (m = &monsters[1]; m < &monsters[NUMMONST]; m++) {
		fprintf(fp, "[%c]%-35s %7s %2d/%-5d %3d %2d %5d %5s\n",
			m->m_appear,
			m->m_name,
			m->m_stats.s_hpt,
			m->m_stats.s_lvl,
			m->m_stats.s_exp,
			m->m_stats.s_arm,
			m->m_stats.s_str,
			m->m_add_exp,
			m->m_intel);
		fprintf(fp, "\t%s %s\n",
			m->m_stats.s_dmg,
			m->m_wander ? "wanders" : "sleeps");
		if (m->m_numsum) fprintf(fp, "\tcan summon %d %s%s\n",
			m->m_numsum,
			m->m_typesum,
			m->m_numsum > 1 ? "s" : "");
		for (i = 0; i < 10 && m->m_flags[i] != 0; i++)
			fprintf(fp, "\t%s\n", getflag(m->m_flags[i]));
		fprintf(fp, "\n");
	}
}

monst_displ(m)
struct monster *m;
{
	char *getflag();
	int i;

	wclear(hw);
	wprintw(hw, "[%c] %-35s %7s %3s/%-4s %3s %2s %5s %5s\n\r",
		'c', "name", "hpt", "lvl", "exp", "ac", "str", "+exp", "intel");
	wprintw(hw, "[%c] %-35s %7s %2d/%-5d %3d %2d %5d %5s\n\r",
		m->m_appear,
		m->m_name,
		m->m_stats.s_hpt,
		m->m_stats.s_lvl,
		m->m_stats.s_exp,
		m->m_stats.s_arm,
		m->m_stats.s_str,
		m->m_add_exp,
		m->m_intel);
	wprintw(hw, "\t%s %s\n\r",
		m->m_stats.s_dmg,
		m->m_wander ? "wanders" : "sleeps");
	if (m->m_numsum) wprintw(hw, "\tcan summon %d %s%s\n\r",
		m->m_numsum,
		m->m_typesum,
		m->m_numsum > 1 ? "s" : "");
	for (i = 0; i < 10 && m->m_flags[i] != 0; i++) {
		if (m->m_flags[i] == CANSUMMON)
			continue;
		wprintw(hw, "\t%s\n\r", getflag(m->m_flags[i]));
	}
	dbotline(hw, spacemsg);
	restscr(cw);
}

hstat()
{
	coord pos;
	int ch;
	struct linked_list *item;
	struct thing *tp;
	char buf[LINELEN];
	struct stats *c, *m;
	char *nm;
	struct monster *mp;
	int i;
	int hpwidth;
	struct linked_list *list;
	struct object *obj;
	int au;
	int know;
	int sav;
	char *getflag();

	pos = player.t_pos;
	for (;;) {
		pos.y += delta.y;
		pos.x += delta.x;
		if (!shoot_ok(ch = winat(pos.y, pos.x)) || ch == DOOR)
			break;
	}
	if (!isalpha(ch) || (item = find_mons(pos.y, pos.x)) == NULL) {
		msg("Cannot find any monster there");
		return;
	}
	know = getbless();
	tp = (struct thing *) ldata(item);
	mp = &monsters[tp->t_index];
	nm = mp->m_name;
	wclear(hw);
	wprintw(hw,"you see the view as a%s %s\n\r", vowelstr(nm), nm);
	if (tp->t_no_move)
		wprintw("It cannot move for %d moves\n\r", tp->t_no_move);
	for (i = 0; i < 10 && mp->m_flags[i] != 0; i++)
		wprintw(hw, "\t%s\n\r", getflag(mp->m_flags[i]));
	wprintw(hw, "inventory:\n\r");
	au = 0;
	for (i = 0, list = tp->t_pack; list; list = next(list), i++) {
		obj = (struct object *)ldata(list);
		if (obj->o_type == GOLD)
			au = obj->o_count;
		else {
			sav = (obj->o_flags & ISKNOW);
			if (know >= 0)
				obj->o_flags |= ISKNOW;
			wprintw(hw, "\t%d) %s\n\r", i, inv_name(obj, FALSE, know >= 0));
			if (know == 0 && !sav)
				obj->o_flags &= ~ISKNOW;
		}
	}
	c = &tp->t_stats;
	m = &tp->maxstats;
	sprintf(buf, "Int:%d(%d)  Str:%d(%d)  Wis:%d(%d)  Dxt:%d(%d)  Const:%d(%d)  Carry:%d(%d)",
		c->s_intel, m->s_intel,
		c->s_str, m->s_str,
		c->s_wisdom, m->s_wisdom,
		c->s_dext, m->s_dext,
		c->s_const, m->s_const,
		c->s_pack/10, c->s_carry/10);
	mvwaddstr(hw, LINES - 3, 0, buf);
	wclrtoeol(hw);
	for (hpwidth = 0, i = m->s_hpt; i; hpwidth++)
		i /= 10;
	sprintf(buf, "Lvl:%d  Au:%d  Hp:%*d(%*d)  Ac:%d  Exp:%d/%ld  %s",
		level, au, hpwidth, c->s_hpt, hpwidth, m->s_hpt, c->s_arm,
		c->s_lvl, c->s_exp, nm);
	mvwaddstr(hw, LINES - 2, 0, buf);
	wclrtoeol(hw);
	dbotline(hw, spacemsg);
	restscr(cw);
}

char *flaglist[4][FLAGSHIFT] = {
	{ "is blind",		"is in wall",		"is running",
	  "runs away from you", "is invisible",		"is mean",
	  "is greedy",		"can shoot at you",	"is held",
	  "is confused",	"recovers rapidly",	"can confuse you",
	  "can see invisible",	"has fire (lights room)", "is slow",
	  "is fast",		"is clear",	      "can chase through walls",
	  "is disguised",
	  "can blink",		"can put you to sleep",	"receives half damage",
	  "can suck blood",	"can rust",		"can poison you",
	  "can lower exp level","only one/treasure room","steals gold",
	  "steals magic",	"can give you a disease",		      },
	{ "has disease",	"can suffocate you",	"did suffocate",
	  "divides when hit by a lightning bolt",
				"divides when touched",	"not affected by ice",
	  "frightens you when touched",
				"need +2,+2 weapon to hit it",
							"not affected by fire",
	  "not affected by lightning",
				"carries gold",		"causes itching",
	  "did drain",		"was turned",		"can sell",
	  "can sell",		"can blind you"				      },
	{ "need +1,+1 weapon to hit it",
				"can give you a parasite", "has infestged",
	  "it doesn't move",	"can shriek",		"can draw blood",
	  "can smell",		"can paralyze you",	"can give you rot",
	  "is a scavenger",	"do rot",		"can stink",
	  "has stunk",		"is invisible most of the time",
							"can give you a chill",
	  "can hug you",	"can surprise you",	"can frighten you",
	  "can summon",		"can turn you into stone when touched",
				"can turn you into stone when you see it",
	  "can hold you",	"did hold",	     "can lower exp level by 2",
	  "is undead",							      },
	{ "can breath at random","can breath acid",	"can breath fire",
	  "can breath lighting","can breath gas",	"can breath ice",
	  "can breath paralyzing gas",
	  "can breath sleeping gas",
	  "can breath slow gas",
	  "can breath fear gas" }
};

char *
getflag(x)
long x;
{
	int index = (x>>FLAGSHIFT) & FLAGINDEX;
	int ctr;

	for (ctr = 0; ctr < FLAGSHIFT; ctr++) {
		if (x & 01)
			return(flaglist[index][ctr]);
		x >>= 1;
	}
	return("What the???");
}
