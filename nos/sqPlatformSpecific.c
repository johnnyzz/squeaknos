#include "sq.h"

/*** Image File Read/Write ***/

int sqFileInit(void)	{ return true; }

sqInt sqGetFilenameFromString(char *aCharBuffer, char *aFilenameString, sqInt filenameLength, sqInt aBoolean)
{
  memcpy(aCharBuffer, aFilenameString, filenameLength);
  aCharBuffer[filenameLength]= 0;
  return 0;
}

sqInt sqImageFileRead(char *ptr, sqInt sz, sqInt count, sqImageFile f) {

	if (count*sz < 1024) {
		memcpy(ptr, f->file+f->offset,count*sz);
		f->offset += count*sz;
	}
	return count*sz;
}

sqInt sqImageFileWrite(char *ptr, sqInt sz, sqInt count, sqImageFile f) {
	memcpy(f->file+f->offset,ptr,count*sz);
	f->offset += count*sz;
	return count*sz;
}

/*** Memory ***/

void * sqAllocateMemory(int minHeapSize, int desiredHeapSize) {
	static void *image = 0;
	if (minHeapSize < 0) {
		// minHeapSize is -headerSize, desiredHeapSize is image
		// ==> image = image - -headerSize = image+headerSize
		image = (void*)(desiredHeapSize-minHeapSize);
		return 0;
	}
	/* allocate memory for Squeak object heap. */
	if (minHeapSize > 1024) return image;
	return 0;
}

sqInt sqGrowMemoryBy(sqInt oldLimit, sqInt delta)	{
       // XXX: We should check available memory here.
	return oldLimit+delta; 
}

sqInt sqShrinkMemoryBy(sqInt oldLimit, sqInt delta)	{ return oldLimit; }
sqInt sqMemoryExtraBytesLeft(sqInt includingSwap)	{ return 0; }

sqMain(void *image) {
	static SQFile imgFile;
	sqInt swapBytes;
	sqInt headerSize;

	// startUpTime = timer;

	/* read the image file and allocate memory for Squeak heap */
	sqImageFile f=&imgFile;
	f->file=image;
	sqImageFileSeek(f, 0);
	swapBytes = checkImageVersionFromstartingAt(f, 0);
	headerSize = getLongFromFileswap(f, swapBytes);
	sqImageFileSeek(f, 0);

	// Hack so we don't need to change interp.c
	// otherwise we would need to comment out the call to sqAllocateMemory
	mark(0x0e70); // green
	sqAllocateMemory(-headerSize, (int)image);

	mark(0x0e70); // green

	readImageFromFileHeapSize(f, initialHeapSize);

	mark(0x0e70);
	interpret();
}
