/* psgio.h - Programmable Sound Generator definitions */
#define PSGADR	0272		/* PSG I/O address */
/*
**	Bit masks for enable register
*/
#define	PS_PBI	0x80		/* port B input */
#define	PS_PBO	0x00		/* port B output */
#define	PS_PAI	0x40		/* port A input */
#define	PS_PAO	0x00		/* port A output */
#define	PS_ENC	0x20		/* enable noise on channel C */
#define	PS_DNC	0x00		/* disable noise on channel C */
#define	PS_ENB	0x10		/* enable noise on channel B */
#define	PS_DNB	0x00		/* disable noise on channel B */
#define	PS_ENA	0x08		/* enable noise on channel A */
#define	PS_DNA	0x00		/* disable noise on channel A */
#define	PS_ETC	0x04		/* enable tone on channel C */
#define	PS_DTC	0x00		/* disable tone on channel C */
#define	PS_ETB	0x01		/* enable tone on channel B */
#define	PS_DTB	0x00		/* disable tone on channel B */
#define	PS_ETA	0x01		/* enable tone on channel A */
#define	PS_DTA	0x00		/* disable tone on channel A */
/*
**	Bit masks for amplitude control registers
*/
#define	PS_FLA	0x00		/* fixed level amplitude */
#define	PS_VLA	0x10		/* variable level amplitude */
/*
**	Bit masks for envelope control register
*/
#define	PS_CNT	0x08		/* continue */
#define	PS_ATT	0x04		/* attack */
#define	PS_ALT	0x02		/* alternate */
#define	PS_HLD	0x01		/* hold */
/*
**	PSG register definitions
*/
#define	PS_ATF	0		/* channel A tone fine */
#define	PS_ATC	1		/* channel A tone course */
#define	PS_BTF	2		/* channel B tone fine */
#define	PS_BTC	3		/* channel B tone course */
#define	PS_CTF	4		/* channel C tone fine */
#define	PS_CTC	5		/* channel C tone course */
#define	PS_NPR	6		/* noise period register */
#define	PS_ENR	7		/* enable register */
#define	PS_AAR	8		/* channel A amplitude register */
#define	PS_BAR	9		/* channel B amplitude register */
#define	PS_CAR	10		/* channel C amplitude register */
#define	PS_EPF	11		/* envelope period fine */
#define	PS_EPC	12		/* envelope period course */
#define	PS_ECR	13		/* envelope control register */
#define	PS_PAR	14		/* parallel port A */
#define	PS_PBR	15		/* parallel port B */
