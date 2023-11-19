/* definitions for HA-8-3 */

#define	VPNEV		0	/* no external video */
#define	VP16K		0x80	/* 16k ram chips */
#define	VPDDP		0	/* blank display */
#define	VPEDP		0x40	/* enable display */
#define	VPDI		0	/* disable interrupts */
#define	VPPM		0	/* pattern mode (Graphics I) */
#define	VPTX		0x10	/* text mode (GFR) */
#define	VPG2		0x200	/* enable Graphics II mode */
#define	VPS1		0x02	/* size 1 (16x16 bit) sprites */
#define	VPM1		0x01	/* sprite magnification (2x) */


#define	c_transp		0
#define	c_black			1
#define	c_midgrn		2
#define	c_ltgrn			3
#define	c_dkblue		4
#define	c_ltblue		5
#define	c_dkred			6
#define	c_cyan			7
#define	c_midred		8
#define	c_ltred			9
#define	c_dkyell		10
#define	c_ltyell		11
#define	c_dkgrn			12
#define	c_magenta		13
#define	c_gray			14
#define	c_white			15

/* PSG */
int tonefreq();
int toneperiod();
int rdtoneperiod();
int noisefreq();
int noiseperiod();
int rdnoiseperiod();
int psgoptions();
int rdenable();
int envenable();
int chanampl();
int rdchanampl();
int ecycltime();
int ecyclperiod();
int rdenvperiod();
int envshape();
int rdenvcntrl();
int wrtpsgport();
int rdpsgport();
	
/* A/D */
int rdadchan();
	
/* VDP */
int vramallocate();
int initmcmode();
int initg2mode();
int mcmove();
int move();
int mcdraw();
int draw();
int eraser();
int colorset();
int areafill();
int mcfill();
int areaclear();
int rdpixel();
int vdpstatus();
int vdpoptions();
int pattnametable();
int colgentable();
int pattgentable();
int sprnametable();
int sprpatrntable();
int bordercolor();
int blockwrite();
int blockread();
int wrtvramdirect();
int rdvramdirect();
int compmode();
	
/* Sprites */
int defsprite();
int positsprite();
int wrtearlybit();
int chgsprcolor();
int crpattern();
int asgsprpattern();
int crsprite();
