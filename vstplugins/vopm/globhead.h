/*
 *  globhead.h
 *  VOPM_test
 *
 */

#ifdef __GNUC__
	#ifndef __int64
		#define __int64 long long
	#endif
	#ifndef FALSE
		#define FALSE false
	#endif
	#ifndef TRUE
		#define TRUE true
	#endif
#endif

extern int OpmRate;
extern int Samprate;
extern unsigned int irnd(void);
 
 

