/*
 * machine dependicies
 *
 */
/*
 * where scorefile should live
 */
#define SCOREFILE	"/obelix/mnt/scottha/etc/scoreboard"

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play
 */

/*	#define	MAXUSERS	15	/* max number of users for this game */
/*	#undef	MAXLOAD		40	/* 10 * max 15 minute load average */

/*	#if MAXUSERS|MAXLOAD */
/*	#define	CHECKTIME	15	/* number of minutes between load checks */
				/* if not defined checks are only on startup */
/* #endif

	#ifdef MAXLOAD
*/
/*	#define	LOADAV			/* defined if rogue should provide loadav() */

/*
	#ifdef LOADAV
*/
/*	#define	NAMELIST	"/unix"	/* where the system namelist lives */
/*
	#endif
	#endif

	#ifdef MAXUSERS
*/
/*	#define	UCOUNT			/* defined if rogue should provide ucount() */

/*
	#ifdef UCOUNT
*/
/*	#define UTMP	"/etc/utmp"	/* where utmp file lives */
/*
	#endif
	#endif
*/
/*
 * define the current author of the program for "special handling"
 */
#define AUTHOR 10
