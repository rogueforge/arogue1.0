/*
 * Rogue definitions and variable declarations
 *
 */

#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#ifdef BSD
#define	strchr	index
#define	lrand48	rand
#define	srand48	srand
#define	TCFLSH	TIOCFLUSH
#endif

/*
 * Maximum number of different things
 */
#define	MAXROOMS	9
#define	MAXTHINGS	9
#define	MAXOBJ		9
#define	MAXPACK		23
#define	MAXTREAS	15	/* number monsters/treasure in treasure room */
#define	MAXTRAPS	15
#define	MAXTRPTRY	8	/* attempts/level allowed for setting traps */
#define	MAXDOORS	4	/* Maximum doors to a room */
#define	MAXPRAYERS	12	/* Maximum number of prayers for cleric */
#define	MAXSPELLS	21	/* Maximum number of spells (for magician) */
#define	NUMMONST	106	/* Current number of monsters */
#define NUMUNIQUE	16	/* number of UNIQUE creatures */
#define	NLEVMONS	3	/* Number of new monsters per level */
#define	MAXFOODS	1
#define	MAXAMULET	1
#define AMLEVEL		35	/* first level at which amulet might appear */
#define MAXPURCH	4	/* max purchases per trading post visit */
#define LINELEN		80	/* characters in a buffer */

/* Movement penalties */
#define BACKPENALTY 3
#define SHOTPENALTY 2		/* In line of sight of missile */
#define DOORPENALTY 1		/* Moving out of current room */

/*
 * stuff to do with encumberance
 */
#define NORMENCB	1500	/* normal encumberance */
#define F_OK		 0	/* have plenty of food in stomach */
#define F_HUNGRY	 1	/* player is hungry */
#define F_WEAK		 2	/* weak from lack of food */
#define F_FAINT		 3	/* fainting from lack of food */

/*
 * return values for get functions
 */
#define	NORM	0	/* normal exit */
#define	QUIT	1	/* quit option setting */
#define	MINUS	2	/* back up one option */

/* 
 * The character types
 */
#define	C_FIGHTER	0
#define	C_MAGICIAN	1
#define	C_CLERIC	2
#define	C_THIEF		3
#define	C_MONSTER	4

/*
 * values for games end
 */
#define SCOREIT -1
#define KILLED 	 0
#define CHICKEN  1
#define WINNER   2

/*
 * definitions for function step_ok:
 *	MONSTOK indicates it is OK to step on a monster -- it
 *	is only OK when stepping diagonally AROUND a monster
 */
#define MONSTOK 1
#define NOMONST 2

/*
 * used for ring stuff
 */
#define LEFT_1	 0
#define LEFT_2	 1
#define LEFT_3	 2
#define LEFT_4	 3
#define RIGHT_1	 4
#define RIGHT_2	 5
#define RIGHT_3	 6
#define RIGHT_4	 7

/*
 * All the fun defines
 */
#define next(ptr) (*ptr).l_next
#define prev(ptr) (*ptr).l_prev
#define ldata(ptr) (*ptr).l_data
#define inroom(rp, cp) (\
    (cp)->x <= (rp)->r_pos.x + ((rp)->r_max.x - 1) && (rp)->r_pos.x <= (cp)->x \
 && (cp)->y <= (rp)->r_pos.y + ((rp)->r_max.y - 1) && (rp)->r_pos.y <= (cp)->y)
#define winat(y, x) (mvwinch(mw, y, x)==' '?mvwinch(stdscr, y, x):winch(mw))
#define debug if (wizard) msg
#define RN (((seed = seed*11109+13849) & 0x7fff) >> 1)
#define unc(cp) (cp).y, (cp).x
#define cmov(xy) move((xy).y, (xy).x)
#define DISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)	(struct object *)((*what).l_data)
#define THINGPTR(what)	(struct thing *)((*what).l_data)
#define when break;case
#define otherwise break;default
#define until(expr) while(!(expr))
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define draw(window) wrefresh(window)
#define hero player.t_pos
#define pstats player.t_stats
#define max_stats player.maxstats
#define pack player.t_pack
#define attach(a, b) _attach(&a, b)
#define detach(a, b) _detach(&a, b)
#define free_list(a) _free_list(&a)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#define on(thing, flag) \
    (((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & flag) != 0)
#define off(thing, flag) \
    (((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & flag) == 0)
#define turn_on(thing, flag) \
    ((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag & ~FLAGMASK))
#define turn_off(thing, flag) \
    ((thing).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~flag)
#define ALLOC(x) malloc((unsigned int) x)
#define FREE(x) free((char *) x)
#define	EQSTR(a, b, c)	(strncmp(a, b, c) == 0)
#undef CTRL
#define CTRL(ch) (ch & 037)
#define GOLDCALC (rnd(50 + 10 * level) + 2)
#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)
#define ISWEARING(r)	(ISRING(LEFT_1, r) || ISRING(LEFT_2, r) ||\
			 ISRING(LEFT_3, r) || ISRING(LEFT_4, r) ||\
			 ISRING(RIGHT_1, r) || ISRING(RIGHT_2, r) ||\
			 ISRING(RIGHT_3, r) || ISRING(RIGHT_4, r))
#define newgrp() ++group
#define o_charges o_ac
#define ISMULT(type) (type == FOOD)
#define isrock(ch) ((ch == WALL) || (ch == '-') || (ch == '|'))
#define is_stealth(tp) \
    (rnd(25) < (tp)->t_stats.s_dext || (tp == &player && ISWEARING(R_STEALTH)))
#define flushout() ioctl(_tty_ch,TCFLSH,0)
#define mi_wght mi_worth

/*
 * Ways to die
 */
#define	D_PETRIFY	-1
#define	D_ARROW		-2
#define	D_DART		-3
#define	D_POISON	-4
#define	D_BOLT		-5
#define	D_SUFFOCATION	-6
#define	D_POTION	-7
#define	D_INFESTATION	-8
#define D_DROWN		-9

/*
 * Things that appear on the screens
 */
#define	WALL		' '
#define	PASSAGE		'#'
#define	DOOR		'+'
#define	FLOOR		'.'
#define VPLAYER 	'@'
#define IPLAYER 	'_'
#define	POST		'^'
#define	TRAPDOOR	'>'
#define	ARROWTRAP	'{'
#define	SLEEPTRAP	'$'
#define	BEARTRAP	'}'
#define	TELTRAP		'~'
#define	DARTTRAP	'`'
#define POOL		'"'
#define MAZETRAP	'\\'
#define	SECRETDOOR	'&'
#define	STAIRS		'%'
#define	GOLD		'*'
#define	POTION		'!'
#define	SCROLL		'?'
#define	MAGIC		'$'
#define	BMAGIC		'>'	/*	Blessed	magic	*/
#define	CMAGIC		'<'	/*	Cursed	magic	*/
#define	FOOD		':'
#define	WEAPON		')'
#define	ARMOR		']'
#define	AMULET		','
#define	RING		'='
#define	STICK		'/'
#define	CALLABLE	-1
#define	MARKABLE	-2

/*
 * Various constants
 */
#define	PASSWD		"mTPPyEFPuSOpk"
#define	BEARTIME	3
#define	SLEEPTIME	4
#define	FREEZETIME	6
#define	HEALTIME	30
#define	HOLDTIME	2
#define	CHILLTIME	(roll(2, 4))
#define	SMELLTIME	20
#define	STONETIME	8
#define	SICKTIME	10
#define	STPOS		0
#define	WANDERTIME	70
#define	BEFORE		1
#define	AFTER		2
#define	HUHDURATION	20
#define	SEEDURATION	850
#define	CLRDURATION	15
#define GONETIME	200
#define	PHASEDURATION	300
#define	HUNGERTIME	1300
#define	MORETIME	150
#define	STINKTIME	6
#define	STOMACHSIZE	2000
#define	ESCAPE		27
#define	BOLT_LENGTH	10
#define	MARKLEN		20

/*
 * Save against things
 */
#define	VS_POISON		00
#define	VS_PARALYZATION		00
#define	VS_DEATH		00
#define	VS_PETRIFICATION	01
#define	VS_WAND			02
#define	VS_BREATH		03
#define	VS_MAGIC		04

/*
 * attributes for treasures in dungeon
 */
#define ISCURSED     	       01
#define ISKNOW      	       02
#define ISPOST		       04	/* object is in a trading post */
#define	ISMETAL     	      010
#define ISPROT		      020	/* object is protected */
#define ISBLESSED     	      040
#define ISMISL      	   020000
#define ISMANY     	   040000
/*
 * Various flag bits
 */
#define ISDARK	    	       01
#define ISGONE	    	       02
#define	ISTREAS     	       04
#define ISFOUND     	      010
#define ISTHIEFSET	      020
/*
 * 1st set of creature flags (this might include player)
 */
#define ISBLIND     	       01
#define	ISINWALL     	       02
#define ISRUN	    	       04
#define	ISFLEE	    	      010
#define ISINVIS     	      020
#define ISMEAN      	      040
#define ISGREED     	     0100
#define CANSHOOT     	     0200
#define ISHELD      	     0400
#define ISHUH       	    01000
#define ISREGEN     	    02000
#define CANHUH      	    04000
#define CANSEE      	   010000
#define HASFIRE	    	   020000
#define ISSLOW	    	   040000
#define ISHASTE     	  0100000
#define ISCLEAR     	  0200000
#define CANINWALL   	  0400000
#define ISDISGUISE 	 01000000
#define CANBLINK   	 02000000
#define CANSNORE   	 04000000
#define HALFDAMAGE	010000000
#define	CANSUCK		020000000
#define	CANRUST		040000000
#define	CANPOISON      0100000000
#define	CANDRAIN       0200000000
#define ISUNIQUE       0400000000
#define	STEALGOLD     01000000000
#define	STEALMAGIC    02000000000
#define	CANDISEASE    04000000000

/* 
 * Second set of flags 
 */
#define HASDISEASE   010000000001
#define CANSUFFOCATE 010000000002
#define DIDSUFFOCATE 010000000004
#define BOLTDIVIDE   010000000010
#define BLOWDIVIDE   010000000020
#define NOCOLD	     010000000040
#define	TOUCHFEAR    010000000100
#define BMAGICHIT    010000000200
#define NOFIRE	     010000000400
#define NOBOLT	     010000001000
#define CARRYGOLD    010000002000
#define CANITCH      010000004000
#define HASITCH	     010000010000
#define DIDDRAIN     010000020000
#define WASTURNED    010000040000
#define CANSELL	     010000100000
#define CANBLIND     010000200000

/* 
 * Third set of flags 
 */
#define MAGICHIT     020000000001
#define CANINFEST    020000000002
#define HASINFEST    020000000004
#define NOMOVE	     020000000010
#define CANSHRIEK    020000000020
#define CANDRAW	     020000000040
#define CANSMELL     020000000100
#define CANPARALYZE  020000000200
#define CANROT	     020000000400
#define ISSCAVENGE   020000001000
#define DOROT	     020000002000
#define CANSTINK     020000004000
#define HASSTINK     020000010000
#define ISSHADOW     020000020000
#define CANCHILL     020000040000
#define	CANHUG	     020000100000
#define CANSURPRISE  020000200000
#define CANFRIGHTEN  020000400000
#define CANSUMMON    020001000000
#define TOUCHSTONE   020002000000
#define LOOKSTONE    020004000000
#define CANHOLD      020010000000
#define DIDHOLD      020020000000
#define DOUBLEDRAIN  020040000000
#define ISUNDEAD     020100000000

/* 
 * Fourth set of flags 
 */
#define CANBRANDOM   030000000001	/* Types of breath */
#define CANBACID     030000000002
#define CANBFIRE     030000000004
#define CANBBOLT     030000000010
#define CANBGAS      030000000020
#define CANBICE      030000000040
#define CANBPGAS     030000000100	/* Paralyze gas */
#define CANBSGAS     030000000200	/* Sleeping gas */
#define CANBSLGAS    030000000400	/* Slow gas */
#define CANBFGAS     030000001000	/* Fear gas */
#define CANBREATHE   030000001777	/* Can it breathe at all? */

/* Masks for choosing the right flag */
#define FLAGMASK     030000000000
#define FLAGINDEX    000000000003
#define FLAGSHIFT    30

/* 
 * Mask for cancelling special abilities 
 * The flags listed here will be the ones left on after the
 * cancellation takes place
 */
#define CANC0MASK (	ISBLIND		| ISINWALL	| ISRUN		| \
			ISFLEE		| ISMEAN	| ISGREED	| \
			CANSHOOT	| ISHELD	| ISHUH		| \
			ISSLOW		| ISHASTE	| ISCLEAR	| \
			ISUNIQUE	| CARRYGOLD )
#define CANC1MASK (	HASDISEASE	| DIDSUFFOCATE	| CARRYGOLD 	| \
			HASITCH		| CANSELL )
#define CANC2MASK (	HASINFEST	| NOMOVE	| ISSCAVENGE	| \
			DOROT		| HASSTINK	| DIDHOLD	| \
			ISUNDEAD )
#define CANC3MASK (	CANBREATHE )

/* types of things */
#define TYP_POTION	0
#define TYP_SCROLL	1
#define TYP_FOOD	2
#define TYP_WEAPON	3
#define TYP_ARMOR	4
#define TYP_RING	5
#define TYP_STICK	6
#define TYP_AMULET	7
#define	NUMTHINGS	8
/*
 * Potion types
 */
#define	P_CLEAR		0
#define	P_ABIL		1
#define	P_SEEINVIS	2
#define	P_HEALING	3
#define	P_MFIND		4
#define	P_TFIND		5
#define	P_RAISE		6
#define	P_HASTE		7
#define	P_RESTORE	8
#define	P_PHASE		9
#define P_INVIS		10
#define	MAXPOTIONS	11
/*
 * Scroll types
 */
#define	S_CONFUSE	0
#define	S_MAP		1
#define	S_LIGHT		2
#define	S_HOLD		3
#define	S_SLEEP		4
#define	S_ALLENCH	5
#define	S_IDENT		6
#define	S_SCARE		7
#define	S_GFIND		8
#define	S_TELEP		9
#define	S_CREATE	10
#define	S_REMOVE	11
#define	S_PETRIFY	12
#define	S_GENOCIDE	13
#define	S_CURING	14
#define S_MAKEIT	15
#define S_PROTECT	16
#define	MAXSCROLLS	17

/*
 * Weapon types
 */
#define MACE		0		/* mace */
#define SWORD		1		/* long sword */
#define BOW		2		/* short bow */
#define ARROW		3		/* arrow */
#define DAGGER		4		/* dagger */
#define ROCK		5		/* rocks */
#define TWOSWORD	6		/* two-handed sword */
#define SLING		7		/* sling */
#define DART		8		/* darts */
#define CROSSBOW	9		/* crossbow */
#define BOLT		10		/* crossbow bolt */
#define SPEAR		11		/* spear */
#define TRIDENT		12		/* trident */
#define SPETUM		13		/* spetum */
#define BARDICHE	14 		/* bardiche */
#define PIKE		15		/* pike */
#define BASWORD		16		/* bastard sword */
#define HALBERD		17		/* halberd */
#define BATTLEAXE	18		/* battle axe */
#define MAXWEAPONS	19		/* types of weapons */
#define NONE		100		/* no weapon */

/*
 * Armor types
 */
#define	LEATHER		0
#define	RING_MAIL	1
#define	STUDDED_LEATHER	2
#define	SCALE_MAIL	3
#define	PADDED_ARMOR	4
#define	CHAIN_MAIL	5
#define	SPLINT_MAIL	6
#define	BANDED_MAIL	7
#define	PLATE_MAIL	8
#define	PLATE_ARMOR	9
#define	MAXARMORS	10

/*
 * Ring types
 */
#define	R_PROTECT	0
#define	R_ADDSTR	1
#define	R_SUSABILITY	2
#define	R_SEARCH	3
#define	R_SEEINVIS	4
#define	R_ALERT		5
#define	R_AGGR		6
#define	R_ADDHIT	7
#define	R_ADDDAM	8
#define	R_REGEN		9
#define	R_DIGEST	10
#define	R_TELEPORT	11
#define	R_STEALTH	12
#define	R_ADDINTEL	13
#define	R_ADDWISDOM	14
#define	R_HEALTH	15
#define R_HEAVY		16
#define R_LIGHT		17
#define R_DELUSION	18
#define	MAXRINGS	19

/*
 * Rod/Wand/Staff types
 */

#define	WS_LIGHT	0
#define	WS_HIT		1
#define	WS_ELECT	2
#define	WS_FIRE		3
#define	WS_COLD		4
#define	WS_POLYMORPH	5
#define	WS_MISSILE	6
#define	WS_SLOW_M	7
#define	WS_DRAIN	8
#define	WS_CHARGE	9
#define	WS_TELMON	10
#define	WS_CANCEL	11
#define WS_CONFMON	12
#define WS_ANNIH	13
#define WS_ANTIM	14
#define WS_PARALYZE	15
#define WS_MDEG		16
#define	MAXSTICKS	17

/*
 * Now we define the structures and types
 */

/*
 * level types
 */
typedef enum {
	NORMLEV,	/* normal level */
	POSTLEV,	/* trading post level */
	MAZELEV		/* maze level */
} LEVTYPE;

/*
 * Help list
 */

struct h_list {
    char h_ch;
    char *h_desc;
};

/*
 * Coordinate data type
 */
typedef struct {
    int x;
    int y;
} coord;

/*
 * Linked list data type
 */
struct linked_list {
    struct linked_list *l_next;
    struct linked_list *l_prev;
    char *l_data;			/* Various structure pointers */
};

/*
 * Stuff about magic items
 */

struct magic_item {
    char *mi_name;
    int mi_prob;
    int mi_worth;
    int mi_curse;
    int mi_bless;
};

/*
 * Room structure
 */
struct room {
    coord r_pos;			/* Upper left corner */
    coord r_max;			/* Size of room */
    long r_flags;			/* Info about the room */
    short r_fires;			/* Number of fires in room */
    int r_nexits;			/* Number of exits */
    coord r_exit[MAXDOORS];		/* Where the exits are */
};

/*
 * Array of all traps on this level
 */

struct trap {
    char tr_type;			/* What kind of trap */
    char tr_show;			/* Where disguised trap looks like */
    coord tr_pos;			/* Where trap is */
    long tr_flags;			/* Info about trap (i.e. ISFOUND) */
};

/*
 * Structure describing a fighting being
 */
struct stats {
    short s_str;			/* Strength */
    short s_intel;			/* Intelligence */
    short s_wisdom;			/* Wisdom */
    short s_dext;			/* Dexterity */
    short s_const;			/* Constitution */
    short s_charisma;			/* Charisma */
    unsigned long s_exp;		/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    int s_hpt;				/* Hit points */
    int s_pack;				/* current weight of his pack */
    int s_carry;			/* max weight he can carry */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure describing a fighting being (monster at initialization)
 */
struct mstats {
    short s_str;			/* Strength */
    long s_exp;				/* Experience */
    int s_lvl;				/* Level of mastery */
    int s_arm;				/* Armor class */
    char *s_hpt;			/* Hit points */
    char *s_dmg;			/* String describing damage done */
};

/*
 * Structure for monsters and player
 */
struct thing {
    bool t_turn;			/* If slowed, is it a turn to move */
    bool t_wasshot;			/* Was character shot last round? */
    char t_type;			/* What it is */
    char t_disguise;			/* What mimic looks like */
    char t_oldch;			/* Character that was where it was */
    short t_ctype;			/* Character type */
    short t_index;			/* Index into monster table */
    short t_no_move;			/* How long the thing can't move */
    short t_quiet;			/* used in healing */
    short t_doorgoal;			/* What door are we heading to? */
    coord t_pos;			/* Position */
    coord t_oldpos;			/* Last position */
    coord *t_dest;			/* Where it is running to */
    unsigned long t_flags[4];		/* State word */
    struct linked_list *t_pack;		/* What the thing is carrying */
    struct stats t_stats;		/* Physical description */
    struct stats maxstats;		/* maximum(or initial) stats */
};

/*
 * Array containing information on all the various types of monsters
 */
struct monster {
    char *m_name;			/* What to call the monster */
    short m_carry;			/* Probability of carrying something */
    bool m_normal;			/* Does monster exist? */
    bool m_wander;			/* Does monster wander? */
    char m_appear;			/* What does monster look like? */
    char *m_intel;			/* Intelligence range */
    long m_flags[10];			/* Things about the monster */
    char *m_typesum;			/* type of creature can he summon */
    short m_numsum;			/* how many creatures can he summon */
    short m_add_exp;			/* Added experience per hit point */
    struct mstats m_stats;		/* Initial stats */
};

/*
 * Structure for a thing that the rogue can carry
 */

struct object {
    int o_type;				/* What kind of object it is */
    coord o_pos;			/* Where it lives on the screen */
    char *o_text;			/* What it says if you read it */
    char o_launch;			/* What you need to launch it */
    char *o_damage;			/* Damage if used like sword */
    char *o_hurldmg;			/* Damage if thrown */
    int o_count;			/* Count for plural objects */
    int o_which;			/* Which object of a type it is */
    int o_hplus;			/* Plusses to hit */
    int o_dplus;			/* Plusses to damage */
    int o_ac;				/* Armor class */
    long o_flags;			/* Information about objects */
    int o_group;			/* Group number for this object */
    int o_weight;			/* weight of this object */
    char o_mark[MARKLEN];		/* Mark the specific object */
};
/*
 * weapon structure
 */
struct init_weps {
    char *w_name;		/* name of weapon */
    char *w_dam;		/* hit damage */
    char *w_hrl;		/* hurl damage */
    char w_launch;		/* need to launch it */
    int  w_flags;		/* flags */
    int  w_wght;		/* weight of weapon */
    int  w_worth;		/* worth of this weapon */
};

/*
 * armor structure 
 */
struct init_armor {
	char *a_name;		/* name of armor */
	int  a_prob;		/* chance of getting armor */
	int  a_class;		/* normal armor class */
	int  a_worth;		/* worth of armor */
	int  a_wght;		/* weight of armor */
};

struct matrix {
    int base;			/* Base to-hit value (AC 10) */
    int max_lvl;		/* Maximum level for changing value */
    int factor;			/* Amount base changes each time */
    int offset;			/* What to offset level */
    int range;			/* Range of levels for each offset */
};

struct spells {
    short s_which;		/* which scroll or potion */
    short s_cost;		/* cost of casting spell */
    short s_type;		/* scroll or potion */
    bool  s_blessed;		/* is the spell blessed? */
};

struct linked_list *find_mons(), *find_obj(), *get_item(), *new_item();
struct linked_list *new_thing(), *wake_monster(), *get_hurl(), *spec_item();

struct object *wield_weap();

char *malloc(), *getenv(), *tr_name(), *new();
char *vowelstr(), *inv_name(), *strcpy(), *strcat();
char *ctime(), *num(), *ring_num(), *blesscurse(), *typ_name();

struct room *roomin();

coord *rndmove(), *can_shoot();

int msg(char *fmt, ...), addmsg(char *fmt, ...);

short randmonster(), id_monst();

int	auto_save(), quit(), endit(), nohaste(), res_strength(), doctor(), 
	runners(), swander(), tstp(), unconfuse(), unsee(), unclrhead(), 
	unphase(), noslow(), rollwand(), stomach(), sight(), unstink(), 
	suffocate(), cure_disease(), un_itch(), appear(), wghtchk();

bool	blue_light(), can_blink(), creat_mons(), shoot_bolt(), add_pack(),
	straight_shot(), maze_view();

void	byebye();

#ifdef CHECKTIME
int checkout();
#endif


struct trap *trap_at();

extern struct h_list helpstr[];
/*
 * Now all the global variables
 */

extern struct trap traps[];
extern struct room rooms[];		/* One for each room -- A level */
extern struct room *oldrp;		/* Roomin(&oldpos) */
extern struct linked_list *mlist;	/* List of monsters on the level */
extern struct thing player;		/* The rogue */
extern struct monster monsters[];	/* The initial monster states */
extern struct linked_list *lvl_obj;	/* List of objects on this level */
extern struct object *cur_weapon;	/* Which weapon he is weilding */
extern struct object *cur_armor;	/* What a well dresssed rogue wears */
extern struct object *cur_ring[];	/* Which rings are being worn */
extern struct magic_item things[];	/* Chances for each type of item */
extern struct magic_item s_magic[];	/* Names and chances for scrolls */
extern struct magic_item p_magic[];	/* Names and chances for potions */
extern struct magic_item r_magic[];	/* Names and chances for rings */
extern struct magic_item ws_magic[];	/* Names and chances for sticks */
extern struct spells magic_spells[];	/* spells for magic users */
extern struct spells cleric_spells[];	/* spells for magic users */
extern char *cnames[][11];		/* Character level names */
extern char curpurch[];			/* name of item ready to buy */
extern char PLAYER;			/* what the player looks like */
extern int char_type;			/* what type of character is player */
extern int foodlev;			/* how fast he eats food */
extern int level;			/* What level rogue is on */
extern int trader;			/* number of purchases */
extern int curprice;			/* price of an item */
extern int purse;			/* How much gold the rogue has */
extern int mpos;			/* Where cursor is on top line */
extern int ntraps;			/* Number of traps on this level */
extern int no_move;			/* Number of turns held in place */
extern int no_command;			/* Number of turns asleep */
extern int inpack;			/* Number of things in pack */
extern int total;			/* Total dynamic memory bytes */
extern int lastscore;			/* Score before this turn */
extern int no_food;			/* Number of levels without food */
extern int seed;			/* Random number seed */
extern int count;			/* Number of times to repeat command */
extern int dnum;			/* Dungeon number */
extern int max_level;			/* Deepest player has gone */
extern int food_left;			/* Amount of food in hero's stomach */
extern int group;			/* Current group number */
extern int hungry_state;		/* How hungry is he */
extern int infest_dam;			/* Damage from parasites */
extern int lost_str;			/* Amount of strength lost */
extern int lost_dext;			/* amount of dexterity lost */
extern int hold_count;			/* Number of monsters holding player */
extern int trap_tries;			/* Number of attempts to set traps */
extern int pray_time;			/* Number of prayer points/exp level */
extern int spell_power;			/* Spell power left at this level */
extern char take;			/* Thing the rogue is taking */
extern char prbuf[];			/* Buffer for sprintfs */
extern char outbuf[];			/* Output buffer for stdout */
extern char runch;			/* Direction player is running */
extern char *s_names[];			/* Names of the scrolls */
extern char *p_colors[];		/* Colors of the potions */
extern char *r_stones[];		/* Stone settings of the rings */
extern struct init_weps weaps[];	/* weapons and attributes */
extern struct init_armor armors[];	/* armors and attributes */
extern char *ws_made[];			/* What sticks are made of */
extern char *release;			/* Release number of rogue */
extern char whoami[];			/* Name of player */
extern char fruit[];			/* Favorite fruit */
extern char huh[];			/* The last message printed */
extern char *s_guess[];			/* Players guess at what scroll is */
extern char *p_guess[];			/* Players guess at what potion is */
extern char *r_guess[];			/* Players guess at what ring is */
extern char *ws_guess[];		/* Players guess at what wand is */
extern char *ws_type[];			/* Is it a wand or a staff */
extern char file_name[];		/* Save file name */
extern char score_file[];		/* Score file name */
extern char home[];			/* User's home directory */
extern WINDOW *cw;			/* Window that the player sees */
extern WINDOW *hw;			/* Used for the help command */
extern WINDOW *mw;			/* Used to store mosnters */
extern bool pool_teleport;		/* just teleported from a pool */
extern bool inwhgt;			/* true if from wghtchk() */
extern bool running;			/* True if player is running */
extern bool playing;			/* True until he quits */
extern bool wizard;			/* True if allows wizard commands */
extern bool after;			/* True if we want after daemons */
extern bool notify;			/* True if player wants to know */
extern bool fight_flush;		/* True if toilet input */
extern bool terse;			/* True if we should be short */
extern bool door_stop;			/* Stop running when we pass a door */
extern bool jump;			/* Show running as series of jumps */
extern bool slow_invent;		/* Inventory one line at a time */
extern bool firstmove;			/* First move after setting door_stop */
extern bool waswizard;			/* Was a wizard sometime */
extern bool askme;			/* Ask about unidentified things */
extern bool s_know[];			/* Does he know what a scroll does */
extern bool p_know[];			/* Does he know what a potion does */
extern bool r_know[];			/* Does he know what a ring does */
extern bool ws_know[];			/* Does he know what a stick does */
extern bool amulet;			/* He found the amulet */
extern bool in_shell;			/* True if executing a shell */
extern bool monst_dead;			/* Indicates if monster got killed */
extern coord oldpos;			/* Position before last look() call */
extern coord delta;			/* Change indicated to get_dir() */
extern char *spacemsg;
extern char *morestr;
extern char *retstr;
extern LEVTYPE levtype;
