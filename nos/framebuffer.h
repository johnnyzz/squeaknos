
#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#define bytesPerLine(width, depth)	((((width) + 31) >> 5 << 2) * (depth))
#define bytesPerLineRD(width, depth)	((((width) >> 5) << 2) * (depth))

#define bytesPerPixels(width, depth)	(width * (depth>>3))

void fill_rectangle(int width, int height, int x, int y, unsigned int color);
void bitblt_32bit_to_fb(char *bitmap, int width, int height, int x, int y);

#endif /// _FRAMEBUFFER_H_
