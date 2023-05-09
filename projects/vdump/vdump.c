/* vdump - dump HA-8-3 VRAM in hex
**
**	G. Roberts 29 mar 2023
*/

#include "printf.h"
#include "ha83.h"

#define	BLOCKSIZE	256

char buff[BLOCKSIZE];

/* hexdump - dump a buffer to stdout in hexadecimal format */
hexdump(b,n)
char *b;
int n;
{
  int i;

  /* print header */  
  printf("\n\n\t0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");
  printf("  0123456789ABCDEF\n");
  printf("\t\b-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --");
  printf("  ----------------\n");
  for (i=0; i<n; i+=16) {
    printf("%04x",i);
    printf(" :");
    hexline(b+i,16);
    printf("  ");
    asciiline(b+i,16);
    printf("\n");
  }
}

/* hexline - print n bytes in hexadecimal */
hexline(b,n)
char *b;
int n;
{
  int i;
  
  for (i=0; i<n; i++)
    printf(" %02x",b[i]&0xFF);
}

/* asciiline - print n characters in ascii */
/* (subsitite '.' for non printable characters) */
asciiline(b,n)
char *b;
int n;
{
  int i;
  
  for (i=0; i<n; i++) {
    if (isprint(b[i]))
      putchar(b[i]);
    else
      putchar('.');
  }
}

int isprint(c)
char c;
{
  return ((c>0x1F) && (c<0x7F));
}

main (argc, argv)
int argc;
char *argv[];
{
  int i, vramaddr;
  
	if (argc < 2)
		printf("Usage: vdump <vramaddr>\n");
	else {
		vramaddr = atoi(argv[1]);
		do {
			printf("VRAM Address: %04x:\n\n", vramaddr);
			blockread(buff, vramaddr, BLOCKSIZE);
			hexdump(buff, BLOCKSIZE);
			vramaddr += BLOCKSIZE;
			/* pause and wait for keyboard */
			printf("\nPress any key to continue...\n");
			getchar();
		} while (1);
	}
}
