#include "curses.h"
#include "rogue.h"

char *index();
char *hitpts();

main()
{
	struct monster *p;
	int cnt;

	for (p = monsters + 1; p <= &monsters[NUMMONST]; p++) {
		if ((p - monsters - 1)%66 == 0)
			printf("\nintel  lvls ex hit + a+ mxd hpts   a name\n\n");
		printf("%-5s %2d-%-2d %2d %3d %1d %2d %3d %-6s %c %s",
			p->m_intel,
			lvl(p-monsters, -1),
			lvl(p-monsters, 1),
			p->m_stats.s_lvl,
			7 - p->m_stats.s_lvl/2,
			(p->m_stats.s_str - 10)/3,
			10 - p->m_stats.s_arm,
			calc_max(p->m_stats.s_dmg) + (p->m_stats.s_str-9)/2,
			hitpts(p->m_stats.s_hpt),
			p->m_appear,
			p->m_name);
		if (isflag(p, CANSUMMON))
			printf(" (%d %ss)",
				p->m_numsum,
				p->m_typesum);
		cnt = 0;
		if (isflag(p, CANBRANDOM))
			cnt++;
		if (isflag(p, CANBACID))
			cnt++;
		if (isflag(p, CANBFIRE))
			cnt++;
		if (isflag(p, CANBBOLT))
			cnt++;
		if (isflag(p, CANBGAS))
			cnt++;
		if (isflag(p, CANBICE))
			cnt++;
		if (isflag(p, CANBPGAS))
			cnt++;
		if (isflag(p, CANBSGAS))
				cnt++;
		if (isflag(p, CANBSLGAS))
			cnt++;
		if (isflag(p, CANBFGAS))
			cnt++;
		if (cnt > 0)
			printf("  (%dX)", cnt);
		printf("\n");
		if ((p - monsters - 1)%66 == 65)
			printf("\nintel  lvls ex hit + a+ mxd hpts   a name\n\n");
	}
	if ((p - monsters - 1)%66 != 65)
		printf("\nintel  lvls ex hit + a+ mxd hpts   a name\n");
}

lvl(monster, which)
int monster, which;
{
	if (which == -1)
		return((monster + NLEVMONS - 1)/NLEVMONS);
	else
		return((monster + NLEVMONS - 1)/NLEVMONS + 3);
}

char *
hitpts(h)
char *h;
{
	int mn, mx;
	static char buf[10];

	mx = calc_max(h);
	mn = calc_min(h);
	if (mx == mn)
		sprintf(buf, "%d", mx);
	else
		sprintf(buf, "%d-%d", mn, mx);
	return(buf);
}

calc_max(s)
char *s;
{
	char *sl, *d, *p;
	int val = 0;
	int x;

	do {
		sl = index(s, '/');
		if (sl)
			*sl = '\0';
		d = index(s, 'd');
		if (d)
			*d = '\0';
		x = atoi(s);
		if (d) {
			*d++ = 'd';
			x *= atoi(d);
			s = d;
		}
		val += x;
		p = index(s, '+');
		if (p)
			val += atoi(++p);
		if (sl)
			*sl = '/';
		s = sl + 1;
	} while (sl);
	return(val);
}

calc_min(s)
char *s;
{
	char *p;
	int x;

	x = atoi(s);
	p = index(s, '+');
	if (p)
		x += atoi(++p);
	return(x);
}

isflag(p, flag)
struct monster *p;
int flag;
{
	int i;

	for (i = 0; i < 10; i++)
		if (p->m_flags[i] == flag)
			return(1);
	return(0);
}
