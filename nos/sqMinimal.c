#include "sq.h"

/*** Stub Definitions ***/
#define STUBBED_OUT { success(false); }
#define DO_NOTHING { }

/*** Enumerations ***/
enum { appleID = 1, fileID };
enum { quitItem = 1 };

/*** Variables -- image and path names ***/
#define IMAGE_NAME_SIZE 300
char imageName[IMAGE_NAME_SIZE + 1];  /* full path to image */

#define SHORTIMAGE_NAME_SIZE 100
char shortImageName[] = "SqueakNOS.image";  /* just the image file name */

#define VMPATH_SIZE 300
char vmPath[] = "";  /* full path to interpreter's directory */

/*** VM Home Directory Path ***/

int vmPathSize(void) {
	/* return the length of the path string for the directory containing the VM. */
	return sizeof(vmPath)-1;
}

int vmPathGetLength(int sqVMPathIndex, int length) {
	/* copy the path string for the directory containing the VM into the given Squeak string. */
	char *stVMPath = (char *) sqVMPathIndex;
	int count, i;

	count = sizeof(vmPath)-1;
	count = (length < count) ? length : count;

	/* copy the file name into the Squeak string */
	for (i = 0; i < count; i++) {
		stVMPath[i] = vmPath[i];
	}
	return count;
}

/*** Image File Name ***/

int imageNameSize(void) {
	/* return the length of the Squeak image name. */
	return sizeof(shortImageName)-1;
}

int imageNameGetLength(int sqImageNameIndex, int length) {
	char *stVMPath = (char *) sqImageNameIndex;
	int count, i;

	count = sizeof(shortImageName)-1;
	count = (length < count) ? length : count;

	/* copy the file name into the Squeak string */
	for (i = 0; i < count; i++) {
		stVMPath[i] = shortImageName[i];
	}
	return count;
}

int imageNamePutLength(int sqImageNameIndex, int length) STUBBED_OUT

/*** Clipboard Support ***/

int clipboardReadIntoAt(int count, int byteArrayIndex, int startIndex) {
	/* return number of bytes read from clipboard; stubbed out. */
	return 0;
}

int clipboardSize(void) {
	/* return the number of bytes of data the clipboard; stubbed out. */
	return 0;
}

int clipboardWriteFromAt(int count, int byteArrayIndex, int startIndex) {
	/* write count bytes to the clipboard; stubbed out. */
	return 0;
}

/*** System Attributes ***/

char * GetAttributeString(int id) {
	/* This is a hook for getting various status strings back from
	   the OS. In particular, it allows Squeak to be passed arguments
	   such as the name of a file to be processed. Command line options
	   are reported this way as well, on platforms that support them.
	*/

	// id #0 should return the full name of VM; for now it just returns its path
	if (id == 0) return vmPath;
	// id #1 should return imageName, but returns empty string in this release to
	// ease the transition (1.3x images otherwise try to read image as a document)
	if (id == 1) return shortImageName;  /* will be imageName */
	if (id == 2) return "";

	/* the following attributes describe the underlying platform: */
	if (id == 1001) return "SqueakNOS";
	if (id == 1002) return "v1";
	if (id == 1003) return "Intel x86";

	/* attribute undefined by this platform */
	success(false);
	return "";
}

int attributeSize(int id) {
	/* return the length of the given attribute string. */
	return strlen(GetAttributeString(id));
}

int getAttributeIntoLength(int id, int byteArrayIndex, int length) {
	/* copy the attribute with the given id into a Squeak string. */
	char *srcPtr, *dstPtr, *end;
	int charsToMove;

	srcPtr = GetAttributeString(id);
	charsToMove = strlen(srcPtr);
	if (charsToMove > length) {
		charsToMove = length;
	}

	dstPtr = (char *) byteArrayIndex;
	end = srcPtr + charsToMove;
	while (srcPtr < end) {
		*dstPtr++ = *srcPtr++;
	}
	return charsToMove;
}

#if 0
/*** Directory Stubs ***/

int dir_Create(char *pathString, int pathStringLength)						STUBBED_OUT
int dir_Delimitor(void)														{ return ':'; }
int dir_Lookup(char *pathString, int pathStringLength, int index,
  char *name, int *nameLength, int *creationDate, int *modificationDate,
  int *isDirectory, int *sizeIfFile)										STUBBED_OUT
dir_SetMacFileTypeAndCreator(char *filename, int filenameSize,
  char *fType, char *fCreator)												DO_NOTHING
int dir_Delete(char *pathString, int pathStringLength)						STUBBED_OUT

#endif

/*** Profiling Stubs ***/

int clearProfile(void)														STUBBED_OUT
int dumpProfile(void)														STUBBED_OUT
int startProfiling(void)													STUBBED_OUT
int stopProfiling(void)														STUBBED_OUT

/*** External Primitive Support (No-ops) ***/

void *ioLoadModule(char *pluginName) { return 0; }
void *ioFindExternalFunctionIn(char *lookupName, void *moduleHandle) { return 0; }
int ioFreeModule(void *moduleHandle) { return 0; }


