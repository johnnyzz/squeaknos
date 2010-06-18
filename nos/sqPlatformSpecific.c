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
		memcpy(ptr, f->start+f->offset,count*sz);
		f->offset += count*sz;
	}
	return count*sz;
}

static MemoryFile block;
MemoryFile* sqImageCopyMemoryBlock(){
		extern Computer computer;
		block.start = computer.snapshotStartAddress;
		block.length = computer.snapshotEndAddress - block.start + 1;
		block.offset = 0;
		return &block;
}

sqInt sqMemoryFileWrite(char *ptr, sqInt sz, sqInt count, sqImageFile f) {
	memcpy(f->start+f->offset,ptr,count*sz);
	f->offset += count*sz;
	return count;
}

/*** Memory ***/
// WARNING: This func has a hack, if you look at how is it called the first
// time (in sqMain below), desiredHeapSize is image file pointer casted to
// an int, and minHeapSize is -headerSize, made negative to know that it's
// being called by sqMain.

void * sqAllocateMemory(int minHeapSize, int desiredHeapSize) {
	static void *heap = 0;
	if (minHeapSize < 0) {
		// minHeapSize is -headerSize, desiredHeapSize is image
		// ==> heap = image - -headerSize = image+headerSize
		heap = (void*)(desiredHeapSize-minHeapSize);
		return 0;
	}
	/* allocate memory for Squeak object heap. */
	if (minHeapSize > 1024) return heap;
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
	f->start=image;
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
