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
	 
void sqImageFileClose(sqImageFile f) {
	extern Computer computer;
	extern MemoryFile block;
	extern usqInt memory;
	//printf_pocho("entre al imageFileClose");
	computer.snapshotEndAddress = block.offset;
	makeReadOnly(memory, memory + computer.snapshotEndAddress);
	saveSpecialPages();	
	return 1;
}

MemoryFile* sqImageCopyMemoryBlock(){
		extern Computer computer;
		extern MemoryFile block; 
		extern usqInt memory;
		extern usqInt memoryLimit;
		usqInt memorySize;
		memorySize = memoryLimit - memory + 1;
		//printf_pocho("Snapshot end : %u\n",computer.snapshotEndAddress);
		block.start = computer.snapshotEndAddress - memorySize;
		computer.snapshotStartAddress = block.start;
		//printf_pocho("Snapshot start : %u\n",block.start);
		block.length = memorySize;
		block.offset = 0;
		return &block;
}

sqInt sqMemoryFileWrite(char *ptr, sqInt sz, sqInt count, sqImageFile f) {
	extern MemoryFile block;
	//printf_pocho("Posicion a escribir: %u\n, cantidad a Escribir: %u\n",block.start+block.offset, count);
	if (count < 2) memcpy(block.start+block.offset,ptr,count*sz);
	block.offset += count*sz;
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

sqMain(void *image, unsigned int image_length) {
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

	readImageFromFileHeapSize(f,
		image_length == 0 ? initialHeapSize : image_length + 1024*1024);

	mark(0x0e70);
	interpret();
}
