#include "curses.h"
#include "rogue.h"

main()
{
	struct magic_item *p;
	struct init_armor *ap;
	struct init_weps *wp;
	int vcompar();
	int x;

	qsort(s_magic, MAXSCROLLS, sizeof(struct magic_item), vcompar);
	printf("\n                         S C R O L L S :\n\n");
	printf("%-21s       Buy             Sell        chance\n", "");
	printf("%-21s [val   v+  v-]   [val   v+  v-]   %%+ %%-\n\n", "name");
	for (p = s_magic; p < &s_magic[MAXSCROLLS]; p++)
		printf("%-21s  %3d %4d %3d     %3d %4d %3d    %2d %2d\n",
			p->mi_name,
			p->mi_worth * 3,
			p->mi_bless ? p->mi_worth*3/2 * 3 : 0,
			p->mi_curse ? p->mi_worth/2 * 3 : 0,
			p->mi_worth,
			p->mi_bless ? p->mi_worth*3/2 : 0,
			p->mi_curse ? p->mi_worth/2 : 0,
			p->mi_bless,
			p->mi_curse);
	qsort(p_magic, MAXPOTIONS, sizeof(struct magic_item), vcompar);
	printf("\n\n                        P O T I O N S :\n\n");
	printf("%-21s       Buy             Sell        chance\n", "");
	printf("%-21s [val   v+  v-]   [val   v+  v-]   %%+ %%-\n\n", "name");
	for (p = p_magic; p < &p_magic[MAXPOTIONS]; p++)
		printf("%-21s  %3d %4d %3d     %3d %4d %3d    %2d %2d\n",
			p->mi_name,
			p->mi_worth * 3,
			p->mi_bless ? p->mi_worth*3/2 * 3 : 0,
			p->mi_curse ? p->mi_worth/2 * 3 : 0,
			p->mi_worth,
			p->mi_bless ? p->mi_worth*3/2 : 0,
			p->mi_curse ? p->mi_worth/2 : 0,
			p->mi_bless,
			p->mi_curse);
	printf("\n\n                        R I N G S :\n\n");
	printf("%-21s     Trader/               Trading Post             chance\n", "");
	printf("%-21s                              (buy)\n", "");
	printf("%-21s [val   +1  +2]    [-2  -1  val  +1  +2  +2+  +3+]  %%+  %%-\n\n", "name");
	for (p = r_magic; p < &r_magic[MAXRINGS]; p++)
		printf("%-21s  %3d %4d %3d     %3d %3d %4d %3d %3d %4d %4d   %2d %3d\n",
			p->mi_name,
			qmworth(p - r_magic, p->mi_worth, 0),
			qmworth(p - r_magic, p->mi_worth, 1),
			qmworth(p - r_magic, p->mi_worth, 2),
			tworth(p - r_magic, p->mi_worth, -2, -1) * 3,
			tworth(p - r_magic, p->mi_worth, -1, -1) * 3,
			tworth(p - r_magic, p->mi_worth, 0, 0) * 3,
			tworth(p - r_magic, p->mi_worth, 1, 0) * 3,
			tworth(p - r_magic, p->mi_worth, 2, 0) * 3,
			tworth(p - r_magic, p->mi_worth, 2, 1) * 3,
			tworth(p - r_magic, p->mi_worth, 3, 1) * 3,
			p->mi_bless,
			p->mi_curse);
	printf("------------\n");
	printf("NOTE: -1 delusion, aggrivate, teleport, and burden only mean cursed\n");
	printf("\n\n");   /* end of page */
	printf("\n                        R I N G S ( C O N ' T ) :\n\n");
	printf("%-21s         Trading Post\n", "");
	printf("%-21s            (sell)\n", "");
	printf("%-21s -2-  -2 -1-  -1 val  +1  +2 +2+ +3+\n", "name");
	for (p = r_magic; p < &r_magic[MAXRINGS]; p++)
		printf("%-21s  %2d %3d %3d %3d %3d %3d %3d %3d %3d\n",
			p->mi_name,
			tworth(p - r_magic, p->mi_worth, -2, -1),
			tworth(p - r_magic, p->mi_worth, -2, 0),
			tworth(p - r_magic, p->mi_worth, -1, -1),
			tworth(p - r_magic, p->mi_worth, -1, 0),
			tworth(p - r_magic, p->mi_worth, 0, 0),
			tworth(p - r_magic, p->mi_worth, 1, 0),
			tworth(p - r_magic, p->mi_worth, 2, 0),
			tworth(p - r_magic, p->mi_worth, 2, 1),
			tworth(p - r_magic, p->mi_worth, 3, 1));
	printf("\n\n                        A R M O R :\n\n");
	printf("                  (Trading Post = value * 3 or 75 if cursed )\n");
	printf("%-21s               Trader\n", "");
	printf("%-21s        [val * (plus*10 + 1)]\n", "");
	printf("%-21s ac wt   val   +1    +2    +3    +4    +5 chance\n\n", "type");
	for (ap = armors; ap < &armors[MAXARMORS]; ap++)
		printf("%-21s %1d %3d   %3d %4d %5d %5d %5d %5d   %2d\n",
			ap->a_name,
			ap->a_class,
			ap->a_wght,
			ap->a_worth,
			ap->a_worth * 11,
			ap->a_worth * 21,
			ap->a_worth * 31,
			ap->a_worth * 41,
			ap->a_worth * 51,
			ap==armors ? ap->a_prob : ap->a_prob - (ap-1)->a_prob);
	printf("\n%-21s             Weights\n", "");
	printf("%-21s   -3  -2  -1  wt  +1  +2  +3\n\n", "type");
	for (ap = armors; ap < &armors[MAXARMORS]; ap++)
		printf("%-21s  %3d %3d %3d %3d %3d %3d %3d\n",
			ap->a_name,
			(int)((ap->a_wght*10 + ap->a_wght*3)/10 * 1.2),
			(int)((ap->a_wght*10 + ap->a_wght*2)/10 * 1.2),
			(int)((ap->a_wght*10 + ap->a_wght*1)/10 * 1.2),
			(ap->a_wght*10 + ap->a_wght*0)/10,
			(ap->a_wght*10 - ap->a_wght*1)/10,
			(ap->a_wght*10 - ap->a_wght*2)/10,
			(ap->a_wght*10 - ap->a_wght*3)/10);
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n");		/* end of page */
	printf("\n                        W E A P O N S :\n\n");
	printf("%-21s                    Trader          Trading Post\n", "");
	printf("%-21s                   (* count)        (cursed = 75)\n", "");
	printf("%-21s                            (sell = val * (2 + 4*h + 4*d))\n", "");
	printf("%-21s  dam  wt  [val +1   +2   +3   +4] [val   +2   +4]\n\n", "type");
	for (wp = weaps; wp < &weaps[MAXWEAPONS]; wp++)
		printf("%-21s %4s %3d   %2d %3d %4d %4d %4d   %3d %4d %4d\n",
			wp->w_name,
			wp->w_dam,
			wp->w_wght,
			wp->w_worth,
			wp->w_worth * 11,
			wp->w_worth * 21,
			wp->w_worth * 31,
			wp->w_worth * 41,
			max(wp->w_worth * 2 * 3, 75),
			max(wp->w_worth * (2 + 4 + 4) * 3, 75),
			max(wp->w_worth * (2 + 8 + 8) * 3, 75));
	printf("\n\n                        S T I C K S :\n\n");
	printf("%-21s                      Trader\n", "");
	printf("%-21s val  +5  +6  +7  +8  +9 +10 +11 +12 +13 +14 +15\n\n", "name");
	for (p = ws_magic; p < &ws_magic[MAXSTICKS]; p++) {
		printf("%-21s %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
			p->mi_name,
			p->mi_worth,
			p->mi_worth + 20*5,
			p->mi_worth + 20*6,
			p->mi_worth + 20*7,
			p->mi_worth + 20*8,
			p->mi_worth + 20*9,
			p->mi_worth + 20*10,
			p->mi_worth + 20*11,
			p->mi_worth + 20*12,
			p->mi_worth + 20*13,
			p->mi_worth + 20*14,
			p->mi_worth + 20*15);
	}
	printf("----------\n");
	printf("note - for trader: cost = worth + 20*charges, bless: *3/2, curse: /2\n");
	printf("       to buy: cost * 3\n");
}

int
vcompar(m1, m2)
struct magic_item *m1, *m2;
{
	return(m1->mi_worth - m2->mi_worth);
}

qmworth(ring, value, plus)
int ring, value, plus;
{
	switch (ring) {
	case R_ADDSTR:
	case R_ADDDAM:
	case R_PROTECT:
	case R_ADDHIT:
	case R_ADDINTEL:
	case R_ADDWISDOM:
		return(value + plus * 50);
	case R_DIGEST:
		value += plus * 50;
		return(plus > 0 ? value : 0);
	default:
		return(plus == 0 ? value : 0);
	}
}

tworth(ring, worth, plus, bless_curse)
int ring, worth, plus, bless_curse;
{
	int v;

	switch(ring) {
	case R_ADDSTR:
	case R_ADDWISDOM:
	case R_ADDINTEL:
	case R_PROTECT:
	case R_ADDHIT:
	case R_ADDDAM:
		break;
	case R_DIGEST:
		if (plus == -2 || plus == 0 || plus == 3)
			return(0);
		if (plus == 2 && bless_curse == 0)
			return(0);
		break;
	case R_DELUSION:
		if (plus != -1 && plus != 0)
			return(0);
		plus = 11;
		break;
	case R_AGGR:
	case R_TELEPORT:
	case R_HEAVY:
		if (plus != -1)
			return(0);
		plus = 11;
		break;
	default:
		if (plus != 0)
			return(0);
		plus = 11;
		break;
	}
	v = worth + plus * 40;
	if (bless_curse == -1)
		v /= 2;
	else if (bless_curse == 1)
		v = v * 3 / 2;
	if (v < 25)
		v = 25;
	return(v);
}
