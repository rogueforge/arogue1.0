#include "curses.h"
#include "rogue.h"

main()
{
	int i;
	unsigned int exp;
	struct spells *sp;
	struct magic_item *tab;
	char *s;
	char c;
	char *findundead();

	printf("Fighter:\n");
	printf("lvl hit         exp  %%m %%b %%w %%p %%s\n\n");
	exp = 0;
	for (i = 1; i < 28; i++) {
		if (i == 2)
			exp = 113;
		else if (i > 2)
			exp *= 2;
		printf(" %2d %3d %11u  %2d %2d %2d %2d %2d\n",
			i, 10 - 2*min(i, 17)/2, exp,
			max(((18 - i/2) + 1)*5, 0),
			max(((17 - i/2) + 1)*5, 0),
			max(((16 - i/2) + 1)*5, 0),
			max(((15 - i/2) + 1)*5, 0),
			max(((14 - i/2) + 1)*5, 0));
	}
	printf("\n\n\nMagician:\n");
	printf("lvl hit         exp  %%m %%b %%w %%p %%s        cost name\n\n");
	exp = 0;
	for (sp = magic_spells, i = 1; i < 27; sp++, i++) {
		if (i == 2)
			exp = 135;
		else if (i > 2)
			exp *= 2;
		printf(" %2d %3d %11u  %2d %2d %2d %2d %2d         ",
			i, 9 - 2*min(i, 21)/5, exp,
			max(((18 - i/2) + 1)*5, 0),
			max(((17 - i/2) + 1)*5, 0),
			max(((16 - i/2) + 1)*5, 0),
			max(((15 - i/2) + 1)*5, 0),
			max(((14 - i/2) + 1)*5, 0));
		if (sp < &magic_spells[MAXSPELLS]) {
			switch (sp->s_type) {
			case TYP_POTION:
				tab = p_magic;
				break;
			case TYP_SCROLL:
				tab = s_magic;
				break;
			case TYP_STICK:
				tab = ws_magic;
				break;
			}
			printf("%2d  %s", sp->s_cost, tab[sp->s_which].mi_name);
		}
		printf("\n");
	}
	printf("\n\nCleric:\n");
	printf("lvl hit         exp        cost name                 a undead\n\n");
	exp = 0;
	for (sp = cleric_spells, i = 1; i < 28; sp++, i++) {
		if (i == 2)
			exp = 87;
		else if (i > 2)
			exp *= 2;
		printf(" %2d %3d %11u         ", i, 10 - 2*min(i, 19)/3, exp);
		if (sp < &cleric_spells[MAXPRAYERS]) {
			switch(sp->s_type) {
			case TYP_POTION:
				tab = p_magic;
				break;
			case TYP_SCROLL:
				tab = s_magic;
				break;
			case TYP_STICK:
				tab = ws_magic;
				break;
			}
			printf("%2d  %-17s", sp->s_cost, tab[sp->s_which].mi_name);
		} else
			printf("    %17s", "");
		if ((s = findundead(&c)) != (char*)0)
			printf("    %c %s", c, s);
		printf("\n");
	}
	printf("------------\n");
	printf("affect: level > 3 * mlevel  => dead\n");
	printf("        ((100 * (2*mlevel - level)) / level)%%  => turned\n");
	printf("\n\nThief:\n");
	printf("lvl hit         exp  %%m %%b %%w %%p %%s\n\n");
	exp = 0;
	for (i = 1; i < 28; i++) {
		if (i == 2)
			exp = 72;
		else if (i > 2)
			exp *= 2;
		printf(" %2d %3d %11u  %2d %2d %2d %2d %2d\n",
			i, 10 - 2*min(i, 21)/4, exp,
			max(((18 - i/2) + 1)*5, 0),
			max(((17 - i/2) + 1)*5, 0),
			max(((16 - i/2) + 1)*5, 0),
			max(((15 - i/2) + 1)*5, 0),
			max(((14 - i/2) + 1)*5, 0));
	}
	printf("\n\n");
	printf("steal: (thief_bonus + 2*dexterity + 5*level - 5*(mlevel-3))%%\n");
	printf("\ntrap:  rnd(50) < dexterity + level/2 + thief_bonus\n");
	printf("       max traps per level = 15\n");
	printf("       max tries to set traps per level = 8\n");
	printf("\n%%m: confuse, suck, smell, breath hits\n");
	printf("%%b: breath half dam, breath has effects\n");
	printf("%%w: fear, blind\n");
	printf("%%p: stone (+ rnd(100) < 3)\n");
	printf("%%s: snore, stink, itch, poison, paralyze, dart trap is poison,\n");
	printf("    potion of healing\n");
}

char *
findundead(c)
char *c;
{
	static int n = 0;
	int i;
	struct monster *p;

	++n;
	for (p = &monsters[n]; p < &monsters[NUMMONST]; n++, p++)
		for (i = 0; i < 10; i++)
			if (p->m_flags[i] == ISUNDEAD) {
				*c = p->m_appear;
				return(p->m_name);
			}
	return((char*)0);
}
