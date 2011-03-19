#include "sq.h"
#include "sqPlatformSpecific.h"

/**
 * 
 * This file contains the libc functions that we have to reimplement
 * for squeaknos platform because they are platform dependent.
 *
**/

/**
 *  These two are globals declared as extern in some place of libc headers. We have to define them
 *  to avoid undefined reference linker errors.
 */
int errno;
int *__errno_location = &errno;

__thread int __libc_errno;

int printf_pocho (const char *format, ...);
int printf_pochoTab (unsigned long tab, const char *format, ...);

void exit(int _)
{
	mark(0x00f8);
	printf_pocho("exit()\n");
	ioExit();
}

#define HEAP_SIZE       1024*1024*10

void *malloc(unsigned int size)
{
	static char heap[HEAP_SIZE];
	static char *heap_end = &heap[HEAP_SIZE];
	static char *heap_new = heap;

	unsigned long long total;
	total = size;

	while ((int)heap_new % 4 != 0) // align to 4 bytes
	{
		heap_new++;
	}	

	if (heap_new + total < heap_end)
	{
		char *answer = heap_new;
		
		heap_new += total;
		return answer;
	}
	printf_pocho("malloc got out of space. You asked for (%d) bytes.\n", size);
	ioExit();
}

void *realloc(void * ptr, unsigned int size){
	printf_pocho("Someone called unimplemented realloc. Exiting.");
	ioExit();	
}


// this asumes that malloc will get each block in a contiguous always forward way
void* valloc(size_t size)
{
	unsigned int result = (unsigned int)malloc(4); // get one byte to see where we are placed.
	
	// now look where is the next aligned position (could be exactly result
	// or something near it).
	unsigned int pagesize      = getpagesize();
	unsigned int first_aligned = (result + pagesize - 1) & ~(pagesize - 1);
	unsigned int wasted        = first_aligned - result; // calc how many bytes are wasted due to alignment

	// malloc the needed amount
	if (malloc(pagesize + wasted - 4) != (void*)result+4)
	{
		//this should never happen. If it happens it's a big mistake.
		printf_pocho("ERROR in valloc: malloc not allocating contiguous positions\n");
		ioExit();
	}

	return (void*)first_aligned;
}


void *calloc(unsigned int count, unsigned int size)
{
	return malloc(count*size);
}

void free(void *p)
{
	printf_pocho("free(%x)\n",p);
	ioExit();
}

// Console especific code

/* Some screen stuff.  */
/* The number of columns.  */
#define COLUMNS                 80
/* The number of lines.  */
#define LINES                   24
/* The attribute of an character.  */
#define ATTRIBUTE               7
/* The video memory address.  */
#define VIDEO                   0xB8000

/* Variables.  */
/* Save the X position.  */
static int xpos;
/* Save the Y position.  */
static int ypos;
/* Point to the video memory.  */
static volatile unsigned char *video;

/* Clear the screen and initialize VIDEO, XPOS and YPOS.  */
static void cls (void)
{
	int i;

	video = (unsigned char *) VIDEO;

	for (i = 0; i < COLUMNS * LINES * 2; i++)
		*(video + i) = 0;

	xpos = 0;
	ypos = 0;
}

/* Scroll one line up */
static void scroll (void)
{
	int i;

	video = (unsigned char *) VIDEO;

	for (i = 0; i < COLUMNS * (LINES+1) * 2; i++)
		*(video + i) = *(video + i + COLUMNS * 2);
	for (i = 0; i < COLUMNS * 2; i++)
		*(video + i + (LINES+1)*COLUMNS*2) = 0;
}

/* Convert the integer D to a string and save the string in BUF. If
   BASE is equal to 'd', interpret that D is decimal, and if BASE is
   equal to 'x', interpret that D is hexadecimal.  */
static void
itoa (char *buf, int base, int d)
{
	char *p = buf;
	char *p1, *p2;
	unsigned long ud = d;
	int divisor = 10;

	/* If %d is specified and D is minus, put `-' in the head.  */
	if (base == 'd' && d < 0)
	{
		*p++ = '-';
		buf++;
		ud = -d;
	}
	else if (base == 'x')
		divisor = 16;

	/* Divide UD by DIVISOR until UD == 0.  */
	do
	{
		int remainder = ud % divisor;

		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
	}
	while (ud /= divisor);

	/* Terminate BUF.  */
	*p = 0;

	/* Reverse BUF.  */
	p1 = buf;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}
}


/**
 * Parses the C string str interpreting its content as an integral number, which is returned as an int value.
**/
int _atoi(char *str) {
  unsigned int answer=0;
  int base = 10;

  if (*str == '0' && str[1] == 'x') {
          base = 0x10;
          str += 2;
  }

  while (*str) {
    answer *= base;
    if (*str > '9') 
      answer += (*str | 0x20) - 'a' + 0xa;
    else 
      answer += *str - '0';
    str++;
  }
  return answer;
}

void write_serial(unsigned char a);

/* Put the character C on the screen.  */
int putchar (int c)
{
	if (1)
	{
		std_console_put_char(c);
		write_serial(c);
		return;
	}
	
	if (c == '\n' || c == '\r')
	{
newline:
		for (;xpos<COLUMNS;xpos++)
		{
			*(video + (xpos + ypos * COLUMNS) * 2) = 0;
			*(video + (xpos + ypos * COLUMNS) * 2 + 1) = 0;
		}
		xpos = 0;
		ypos++;
		if (ypos > LINES+1)
		{
			scroll();
			ypos--;
		}
		return;
	}

	*(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
	*(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

	xpos++;
	if (xpos >= COLUMNS)
		goto newline;
}

/* Format a string and print it on the screen, just like the libc
   function printf_pocho.  */
int
bprintf (const char *format, char **arg)
{
	int c;
	char buf[20];

	arg++;

	while ((c = *format++) != 0)
	{
		if (c != '%')
			putchar (c);
		else
		{
			char *p;

do_next:
			c = *format++;
			switch (c)
			{
			case 'l':
				goto do_next;
			case 'd':
			case 'u':
			case 'x':
				itoa (buf, c, *((int *) arg++));
				p = buf;
				goto string;
				break;

			case 's':
				p = *arg++;
				if (! p)
					p = "(null)";

string:
				while (*p)
					putchar (*p++);
				break;

			default:
				putchar (*((int *) arg++));
				break;
			}
		}
	}
}

int __printf_chk (int __flag, __const char *__restrict __format, ...)
{
	char **arg = (char **) &__format;

	printf_pocho(__format, arg);
}

int printf_pocho (const char *format, ...)
{
	char **arg = (char **) &format;

	bprintf(format, arg);
}

int printf_fixed_size(const char *string, const long size)
{
	int i = 0;
	for (i = 0; i < size; i++)
	{
		putchar (string[i]);
	}
}

int printf_pochoTab (unsigned long tab, const char *format, ...)
{
	extern unsigned long tabs;
	//printf_pocho("entre a printfPochoTab %d", tab);
	if ((tab < 0) || (tab>20)) tabs = 0;
	unsigned long i;
	for(i = 0; i < tabs; i++){
		printf_pocho("\t");
	}
	char **arg = (char **) &format;
	bprintf(format, arg);
}

int printf (const char *format, ...)
{
	char **arg = (char **) &format;

	bprintf(format, arg);
}

int puts(const char *msg)
{
	printf_pocho("%s", msg);
}

FILE *stderr = (FILE *)-2;

int fprintf(FILE *file, const char *format, ...)
{
	char **arg = (char **) &format;

	if (file == stderr) bprintf(format, arg);
	else printf_pocho("fprintf called with file %x.\n", file);
}

int fputs(const char *msg, FILE *file)
{
	if (file == stderr) printf_pocho("%s", msg);
	else printf_pocho("fputs called with file %x.\n", file);
}

size_t fwrite(const void *buf, size_t size, size_t nmemb, FILE *file)
{
	char *msg = (char*)buf;
	int total = size*nmemb;
	if (file == stderr)
		while (total--) putchar(*msg++);
	else printf_pocho("fwrite called with file %x.\n", file);
}

int getchar(void)
{
	return 0;
}

time_t time(time_t *t)
{
	time_t answer;

	answer = (time_t)ioSeconds();
	if (t) *t = answer;
	return answer;
}

void
__assert_fail(const char *assertion,
              const char *file,
              unsigned int line,
              const char *function)
{
	printf_pocho("ASSERTION (%s) FAILED in \"%s:%d\" in function \"%s\".\n",
	             assertion, file, line, function);
	exit(0);
}


int getpagesize(void)
{
	return 4096;
}



void *memset(void *s, int c, size_t n)
{
	unsigned char cc = (unsigned char) c;
	for (; n > 0; n--)
		*(unsigned char*)s++ = cc;
}

int memcmp (const void *s1, const void *s2, size_t len)
{
	const unsigned char *sp1, *sp2;

	sp1 = s1;
	sp2 = s2;

	while (len != 0 && *sp1 == *sp2)
		sp1++, sp2++, len--;

	if (len == 0)
		return 0;

	return *sp1 - *sp2;
}


int strcmp (const char *s1, const char *s2)
{
	while (*s1 != 0 && *s1 == *s2)
		s1++, s2++;

	if (*s1 == 0 || *s2 == 0)
		return (unsigned char) *s1 - (unsigned char) *s2;

	return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, register size_t n)
{
	register unsigned char u1, u2;

	while (n-- > 0)
	{
		u1 = (unsigned char) *s1++;
		u2 = (unsigned char) *s2++;
		
		if (u1 != u2)
			return u1 - u2;
		if (u1 == '\0')
			return 0;
	}
	
	return 0;
}


char *
strstr(const char *s1, const char *s2)
{
  const char *p, *q;

	for (; *s1; s1++)
	{
		p = s1, q = s2;
		while (*q && *p)
		{
			if (*q != *p)
				break;
				
			p++, q++;
		}
		
		if (*q == 0)
			return (char *)s1;
	}
	
	return 0;
}

/// System call. Nobody should use system calls, but we need it to avoid
/// libc compile errors
int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
	return 0; // everythink OK (?)
}

int __sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  return sigprocmask (how, set, oset);
}

int mprotect(void *addr, size_t len, int prot)
{
	return 0; // FIXME: STUB
}

void perror(const char *s)
{
	// FIXME: Should print also errno and a corresponding message
	if (s)
		printf_pocho(s);
}
