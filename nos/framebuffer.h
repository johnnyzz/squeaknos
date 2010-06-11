
#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#define bytesPerLine(width, depth)	((((width) + 31) >> 5 << 2) * (depth))
#define bytesPerLineRD(width, depth)	((((width) >> 5) << 2) * (depth))

#define bytesPerPixels(width, depth)	(width * (depth>>3))


#endif /// _FRAMEBUFFER_H_
