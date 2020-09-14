#include "types.h"
#include "port.h"

// global cursor is stored at 0x80100
static uint16_t *g_Cursor = (uint16_t *)0x80100;
static char g_Trans[16] = { 0 };
static int g_pTrans = 0;

WORD 
get_cursor(void)
{
	return *g_Cursor;
}

static void 
set_cursor(WORD pos)
{
	/* pos = LINE * 80 + COL (0 <= pos <= 1999)
	 * (80 = 1 * 80 + 0 => Line 1 & Col 0) 
	 */
	*g_Cursor = pos;

	outb(0xf, 0x3d4);
	outb((BYTE)pos, 0x3d5);
	outb(0xe, 0x3d4);
	outb((BYTE)(pos >> 8), 0x3d5);
}

void 
clear_screen(void)
{
	uint8_t *p = (uint8_t *)0xb8000;
	
	for (int i = 0; i < 25 * 80; ++i)
	{
		*p++ = ' ';
		*p++ = 0x0f;
	}

	set_cursor(0);
}

void 
print_char(char ch)
{
	uint8_t *pos = (uint8_t *)(0xb8000 + *g_Cursor * 2);
	BYTE attr = 0x0f;

	*pos++ = ch;
	*pos = attr;

	// Update cursor
	*g_Cursor += 1;
	set_cursor(*g_Cursor);
}

void 
scroll_screen(void)
{
	uint16_t *p = (uint16_t *)0xb8000;

	for (int i = 160; i <= 3998; i += 2)
	{
		*(p + i - 160) = *(p + i);
	}
}

void 
trans(unsigned int n, int base)
{
	unsigned int remainder;
	unsigned int quotient;

	g_pTrans = 0;
	quotient = n;
	while (quotient)
	{
		remainder = quotient % base;
		if (remainder > 9)    g_Trans[g_pTrans++] = (char)(remainder + 55);
		else    g_Trans[g_pTrans++] = (char)(remainder + 48);
		quotient /= base;
	}
}

void 
print_num(unsigned int num, int base)
{
	trans(num, base);
	if (base == 16)
	{
		print_char('0');
		print_char('x');
	}
	for (g_pTrans--; g_pTrans >= 0; g_pTrans--)    print_char(g_Trans[g_pTrans]);
	print_char('\n');
	g_pTrans = 0;
}
