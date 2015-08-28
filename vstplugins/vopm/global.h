
#ifndef __GLOBAL__
#define __GLOBAL__


#include	<stdio.h>
//#include	<stdlib.h>
#include	<math.h>
//#include	<windows.h>
//#include	<windowsx.h>
//#include	<conio.h>
//#include	<ctype.h>

#define	N_CH	8

#define	PRECISION_BITS	(10)
#define	PRECISION	(1<<PRECISION_BITS)
#define	SIZEALPHATBL_BITS	(10)
#define	SIZEALPHATBL	(1<<SIZEALPHATBL_BITS)

#define	SIZESINTBL_BITS	(10)
#define	SIZESINTBL	(1<<SIZESINTBL_BITS)
#define	MAXSINVAL	(1<<(SIZESINTBL_BITS+2))

#define	PI	3.1415926535897932
#define	MAXELVAL_BITS	(30)
#define	MAXELVAL	(1<<MAXELVAL_BITS)
#define	MAXARTIME_BITS	(20)
#define	SIZEARTBL	(100)
#define	MAXARTIME	(SIZEARTBL*(1<<MAXARTIME_BITS))

extern int	STEPTBL[11*12*64];
extern int	STEPTBL3[11*12*64];
#define	ALPHAZERO	(SIZEALPHATBL*3)

extern unsigned short	ALPHATBL[ALPHAZERO+SIZEALPHATBL+1];
extern short	SINTBL[SIZESINTBL];

extern int STEPTBL_O2[12*64];
extern int	D1LTBL[16];
extern int	DT1TBL[128+4];
extern int	DT1TBL_org[128+4];

typedef struct
{
    int	val_and;
    int	add;
}XR_ELE;

extern XR_ELE XRTBL[64+32];

extern int DT2TBL[4];
extern unsigned short	NOISEALPHATBL[ALPHAZERO+SIZEALPHATBL+1];

#endif

