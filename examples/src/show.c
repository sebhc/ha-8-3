/* show - plays a "slide show" of images, displaying
** them on the HA-8-3 graphics card for the H8.  This
** program uses the HA83.REL library of software routines
** which I have recreated using the library originally
** provided with the Lucidata Pascal compiler. It also
** uses PIO.REL, which provides port I/O functions.
**
** Each picture consists of two files: a '.TIP' file,
** which contains the pixel information, and a '.TIC'
** file, which contains the color values for each 8-bit
** line of pixels.
**
** Image files are pre-processed by the HarmlessLion
** Convert9918 program on a Windows PC:
**	http://harmlesslion.com/cgi-bin/onesoft.cgi?2
** 
** Many thanks to Terry Smedley for developing the
** initial version of this program in Assembly language.
**
** Usage:
**	SHOW <file1> .. <filen> <switches>
**
** file names can include or exclude the file type
** (e.g. FLOWER, FLOWER.TIP, or FLOWER.TIC are all valid).
** any number of files may be specified and wild cards are also
** valid, e.g. SHOW SY2:*.TIP
**
** Valid switches include:
**		-h	print help
**		-l	loop continuously through all pictures
**		-r	reset (clear) the screen on completion
**		-w<ddd> wait <ddd> seconds between images
**		-b<ccc> specify color index 'ccc' for background
**
** The program has been successfully run at up to 4Mhz on
** a Z80 processor system.
**
** link command:
**
** 	l80 show,ha83,pio,command,printf,stdlib/s,clibrary/s,show/n/m/e
**
** Glenn Roberts
** 30 March 2023
**
*/
#include "ha83.h"
#include "printf.h"

#define	TRUE	1
#define	FALSE	0

/* global switch settings */
int loop, waittime, reset, bcolor;

/* VRAM MAP
**
**		Mode: Graphics II
**
**		0..6143(6K) Pattern Generator Table (PGTAB)
**		6144..6911 (768 bytes) Pattern Name Table (PNTAB)
**		7168..7295 (128 bytes) Sprite Attribute Table (SNTAB)
**		8192..14335 (6K) Pattern Color Table (CGTAB)
**		14336..16383 (2K) Sprite Generator Table (SPTAB)
**
*/
#define	PGTAB	0
#define	PNTAB	6144
#define SNTAB	7168
#define CGTAB	8192
#define	SPTAB	14336

/* pointers to dynamically-allocated arrays for
** pattern data and color data
*/
char *pdata;
char *cdata;

char *colorvalues[16] = {
	"Transparent",
	"Black",
	"Mid Green",
	"Light Green",
	"Dark Blue",
	"Light Blue",
	"Dark Red",
	"Cyan",
	"Mid Red",
	"Light Red",
	"Dark Yellow",
	"Light Yellow",
	"Dark Green",
	"Magenta",
	"Gray",
	"White"
};

/* HDOS TICCNT location */
#define TICCNT	0x201B
unsigned *Ticptr = TICCNT;
unsigned timeout;

/* wait - wait a specified number of seconds */
wait(seconds)
unsigned seconds;
{
	mswait(seconds*1000);
}

/* mswait - wait a specified number of milliseconds */
mswait(milliseconds)
unsigned milliseconds;
{
	/* 2 ms per tick */
	timeout = *Ticptr + (milliseconds >> 1);
	while (*Ticptr != timeout)
		/* do nothing, just wait... */;
}

/* clrvram - clear all VRAM */
clrvram()
{
	int iaddr;
	
	for (iaddr=0; iaddr<0x4000; iaddr++)
		wrtvramdirect(iaddr, 0);
}
		
/* cfgscreen - configure screen */
cfgscreen()
{
	int i;
	static char pnt[768];

	/* Initialize for Graphics II mode. FG and BG
	** colors don't matter - we'll build our own
	** color table later
	*/
	initg2mode(bcolor, 0, 0, 0, 0);
	
	/* disable all sprites */
	wrtvramdirect(SNTAB, 0xD0);
	
	/* set Pattern Name Table to just be 0..255 for
	** each of the three display blocks.
	*/
	for (i=0; i<768; i++)
		pnt[i] = i%256;
	blockwrite(pnt, PNTAB, 768);
}

/* showhelp - print help information */
showhelp()
{
	printf("Usage:\n");
	printf("  SHOW <f1> ... <fn> <switches>\n");
	printf("\n");
	printf("Where <f1> is .TIC or .TIP file name\n");
	printf("Switches:\n");
	printf("\t-l    : loop continuously through all images\n");
	printf("\t-r    : reset VDP upon exit\n");
	printf("\t-wddd : wait 'ddd' seconds after each image\n");
	printf("\t-bccc : set border color to 'ccc'\n");
	printf("\t-h    : print this message\n");
}

/* showimage - fname contains the name of a file containing
** picture information. the suffix of the file, if any, is
** deleted and two file names are created: one with suffix
** 'TIP' (containing the pattern information) and one with
** suffix 'TIC' (containing the color table).
**
** The pattern and color files are opened, read into RAM
** and then transferred to the appropriate locations in VRAM
*/
showimage(fname)
char *fname;
{
	int c;
	int fp, fc;
	
	char patfile[20];
	char colfile[20];

	/* strip off any extension */
	if((c=index(fname, ".")) != -1)
		fname[c] = 0;
	
	strcpy(patfile, fname);
	strcat(patfile, ".TIP");
	
	strcpy(colfile, fname);
	strcat(colfile, ".TIC");
	
	if ((fp=fopen(patfile, "rb")) != 0) {
		if ((fc=fopen(colfile, "rb")) != 0) {
			/* both files opened successfully. read
			** files and store in appropriate VRAM
			** locations
			*/
			printf("Displaying: %s\n", fname);
			read(fp, pdata, 6144);
			blockwrite(pdata, PGTAB, 6144);
			
			read(fc, cdata, 6144);
			blockwrite(cdata, CGTAB, 6144);
			
			fclose(fc);
			fclose(fp);
		}
		else {
			printf("Error opening color file %s\n", colfile);
			fclose(fp);
		}
	}
	else {
		printf("Error opening pattern file %s\n", patfile);
	}
}

/* process switches */
int dosw(argc, argv)
int argc;
char *argv[];
{
	int i;
	char *s;

	/* set default switch values */
	loop = FALSE;
	waittime = 5;
	reset = FALSE;
	bcolor = c_black;

	/* process right to left */
	for (i=argc; i>0; i--) {
		s = argv[i];
		if (*s++ == '-') {
			/* have a switch! */
			switch (*s) {
			/* W = wait time between images (sec.) */
			case 'W':
				++s;
				waittime = atoi(s);
			   break;
			/* B = screen border color */
			case 'B':
				++s;
				bcolor = atoi(s);
			   break;
			/* L = loop continuously */
			case 'L':
				loop = TRUE;
				break;
			/* R = reset screen on exit */
			case 'R':
				reset = TRUE;
				break;
			/* H = print help message */
			case 'H':
				showhelp();
				break;
			default:
		    printf("Invalid switch %c\n", *s);
				break;
			}
		}
	}
}

main(argc, argv)
int argc;
char *argv[];
{
	int iarg;
	
	if (argc < 2)
		showhelp();
	else {
		/* expand any file name wild cards on command line */
		command(&argc, &argv);
	
		/* process any switches */
		dosw(argc, argv);
		
		/* print intro and show settings */
		printf("SHOW - slide show for the HA-8-3, Ver. 1\n");
		printf("G. Roberts 31 March 2023\n");
		printf("%s\n", loop ? "Looping" : "Single Pass");
		printf("Delay between slides: %d seconds\n", waittime);
		printf("Border color: %s\n", colorvalues[bcolor]);
		printf("%seset screen on exit\n", reset ? "R" : "No R");

		clrvram();
		cfgscreen();
	
		if ((pdata = alloc(6144)) == 0)
			printf("Insufficient RAM for Pattern array\n");
		else if ((cdata = alloc(6144)) == 0)
			printf("Insufficient RAM for Color array\n");
		else {
			/* all set, now loop over files to display */
			do {
				for (iarg=1; iarg<argc; iarg++) {
					/* skip over switches */
					if (isalpha(argv[iarg][0])) {
						showimage(argv[iarg]);
					wait(waittime);
					}
				}
			} while (loop);
		}
	
		if (reset) {
			/* write anything to port 66Q to reset HA-8-3 */
			outp(066, 0);
		}
	}
}
