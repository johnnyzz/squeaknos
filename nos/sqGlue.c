#include <sq.h>
#include "ints.h"
#include "framebuffer.h"

extern Computer computer;

// Mouse & Keyboard

sqInt ioGetButtonState(void)				{ return 0;  }
sqInt ioPeekKeystroke(void)				{ return -1; }
sqInt ioGetKeystroke(void)				{ return -1; }
sqInt ioGetNextEvent(sqInputEvent *evt)			{ return 0; }
sqInt ioMousePoint(void)				{ return 320 << 16 | 240; }

// Display & Sound

sqInt ioBeep(void)					{ return 0; }
sqInt ioForceDisplayUpdate(void)			{ return 1; }
sqInt ioHasDisplayDepth(sqInt depth)			{
  return abs(depth) == ioScreenDepth();
}

sqInt ioProcessEvents(void)				{ return 0; }
sqInt ioRelinquishProcessorForMicroseconds(sqInt mSecs)	{
    // I believe adding a hlt here is nicer for the microprocessor
    // I also believe it'll save battery life laptops, for example
    // but on VMWare makes time go slower, which actually is a good sign
    // Ok, got the problem. See documentation for ProcessorScheduler>>relinquishProcessorForMicroseconds:
    // This function may take less time than asked with mSecs,
    // but can't take longer than it. Our timer is at 18.2 Hz, giving 55ms
    // and this function is called from the ProcessorScheduler>>idleProcess
    // with 1ms (and also with 10ms from #sweepHandleProcess). Hence, if
    // we enable the "hlt", we'll stop for 55ms minimum (unless another IRQ
    // comes in).
    // To be able to enable this we need to increase the frequency to >1KHz
    // (a max divider of 1193 for the timer IRQ)
    // Should we try to use APIC instead?

	asm("hlt");
	return 0;
}


sqInt ioScreenDepth(void) {
  return computer.videoInfo.depth;
} 

sqInt ioScreenSize(void) {
  return computer.videoInfo.width << 16 | computer.videoInfo.height;
}


sqInt ioSetCursorWithMask(sqInt cursorBitsIndex, sqInt cursorMaskIndex, sqInt offsetX, sqInt offsetY) { return 1; }
sqInt ioSetCursor(sqInt cursorBitsIndex, sqInt offsetX, sqInt offsetY) { return 0; }
sqInt ioSetCursorARGB(sqInt cursorBitsIndex, sqInt extentX, sqInt extentY, sqInt offsetX, sqInt offsetY) { return 0; }
sqInt ioSetDisplayMode(sqInt width, sqInt height, sqInt depth, sqInt fullscreenFlag) { return 1; }
sqInt ioSetFullScreen(sqInt fullScreen)			{ return 1; }
sqInt ioFormPrint(sqInt bitsAddr, sqInt width, sqInt height, sqInt depth, double hScale, double vScale, sqInt landscapeFlag) { return false; }

static inline unsigned short repack(unsigned short pixel) {
  return ((pixel & 0x7fe0) << 1)
       | ((pixel & 0x001f) << 0);
}

inline unsigned long __swap32(unsigned int pixel) {
	unsigned int answer;
	asm("bswap %1" : "=r" (answer): "r" (pixel));
	return answer;
}

// #define swap32(from, to)	asm("bswap %1\nmov %1, %0" : "=o" (to): "r" (from));

inline unsigned long swap32(unsigned int pixel) {
	return (pixel >> 24) | 
	       (pixel << 24) |
	       ((pixel & 0xff0000) >> 8) |
	       ((pixel & 0xff00) << 8);
}


sqInt ioShowDisplay(sqInt fromImageData, sqInt width, sqInt height, sqInt depth,
                    sqInt affectedL, sqInt affectedR, sqInt affectedT, sqInt affectedB)
{
  int lastWord;
  register int firstWord;
  register int line, countPerLine;

  int toImageData = computer.videoInfo.address;
  int scanLine= computer.videoInfo.bytesPerScanLine;

  // fromImage = pointerForOop(fromImageData);
  firstWord= scanLine*affectedT + bytesPerLineRD(affectedL, depth);
  lastWord= scanLine*affectedT + bytesPerLine(affectedR, depth);
  countPerLine = lastWord - firstWord;

  switch (computer.videoInfo.depth) {
    case 16:
      for (line= affectedT; line < affectedB; line++, firstWord += scanLine) {
        short *from = fromImageData+firstWord;
        short *to = toImageData+firstWord;
        for (;to < toImageData+firstWord+countPerLine;to+=2,from+=2) {
          to[0] = repack(from[1]);
          to[1] = repack(from[0]);
        }
      }
      break;
    case 8:
      for (line= affectedT; line < affectedB; line++, firstWord += scanLine) {
        unsigned int *from = fromImageData+firstWord;
        unsigned int *to = toImageData+firstWord;
        for (;to < toImageData+firstWord+countPerLine;to+=1,from+=1) 
		  // to[0] = from[0];
          to[0] = swap32(from[0]);
      }
      break;
    default:
      for (line= affectedT; line < affectedB; line++, firstWord += scanLine)
        memcpy(toImageData+firstWord, fromImageData+firstWord, countPerLine);
      break;
  }
}


// Time functions
sqInt ioMicroMSecs(void) {
  /* return the highest available resolution of the millisecond clock */
  return ioMSecs();	/* this already to the nearest millisecond */
}

sqInt ioMSecs(void) {
	extern unsigned long timer;
	sqInt answer;
	mark(0x0ff0); // light green
	mark(0x0ff0); // light green
	answer = timer*1000.0/TIMER_FREQUENCY;
	mark(0x0ff0); // light green
	mark(0x0ff0); // light green
	mark(0x0ff0); // light green
	return answer;
}

sqInt ioSeconds(void) {
  /* Squeak epoch is Jan 1, 1901.  Unix epoch is Jan 1, 1970: 17 leap years
     and 52 non-leap years later than Squeak. */
//  return unixTime + ((52*365UL + 17*366UL) * 24*60*60UL);
	sqInt answer;
	mark(0xf0f0); // red
	mark(0xf0f0); // red
	mark(0xf0f0); // red
  answer = ioMSecs() / 1000;
	mark(0xf0f0); // red
	mark(0xf0f0); // red

  return answer;
}

// Others

sqInt ioDisablePowerManager(sqInt disableIfNonZero)	{ return true; }

void set_std_console_debugging(int debugging);
void enter_debug_mode();

sqInt ioExit(void)					{ 
	set_std_console_debugging(1);
	printf_pocho("ioExit()\n\n");
	printCallStack();
	mark(0x001F);
	mark(0x001F);
	enter_debug_mode();
}
sqInt ioSetInputSemaphore(sqInt semaIndex)		{ return true; }

