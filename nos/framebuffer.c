
#include "framebuffer.h"
#include <sq.h>

//#undef DEBUG

extern Computer computer;

void mark(int col)
{
#ifdef DEBUG
	static count = 1;
	short *video = 0xfd000000;
	int i;

	for (i=10*count; i<1200*10-10*count; i++)
		video[i+count*1200*10] = col;

	count++;
	count++;
#endif
}

void fill_rectangle(int width, int height, int x, int y, unsigned int color)
{
	int dest             = computer.videoInfo.address;
	int bytesPerScanLine = computer.videoInfo.bytesPerScanLine; // the amount of bytes occupied by a line of the screen's framebuffer
	int depth            = computer.videoInfo.depth;

	char* components = (char*)&color;
	register int firstWord = bytesPerScanLine * y + bytesPerPixels(x, depth);
	register int bytesPerRow = bytesPerPixels(width, depth);
	
	register int line;
	int i;
	
	for (line = y - height; line < y; line++, firstWord -= bytesPerScanLine)
	{
		char *pos;
		for (i = 0, pos = dest+firstWord; i < bytesPerRow; i++, pos++)
		{
			*pos = components[i % 4];
		}
	}
}


void bitblt_32bit_to_fb(char *bitmap, int width, int height, int x, int y)
{
	int dest             = computer.videoInfo.address;
	int bytesPerScanLine = computer.videoInfo.bytesPerScanLine; // the amount of bytes occupied by a line of the screen's framebuffer
	int depth            = computer.videoInfo.depth;
		
	if (depth != 32)
		while(1); // we should have a fallback or something
		
	register int firstWord = bytesPerScanLine * y + bytesPerPixels(x, depth);
	register int bytesPerRow = bytesPerPixels(width, depth);
	
	register int line;
	int i;
	
	for (line = y - height; line < y; line++, firstWord -= bytesPerScanLine)
	{
		char *pos;
		for (i = 0, pos = dest+firstWord; i < bytesPerRow; i++, pos++)
		{
			*pos = *(bitmap++);
		}
	}
}

