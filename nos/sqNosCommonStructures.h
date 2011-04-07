#ifndef __SQNOS_COMMON_STRUCTURES_H__
#define __SQNOS_COMMON_STRUCTURES_H__

#include "multiboot.h"
#include "sqMemoryAccess.h"

// Video Mode Information
typedef struct DisplayInfo {
	int width, height, depth, address, bytesPerScanLine;
} DisplayInfo;

typedef struct ReadOnlyPage {
	unsigned long virtualAddress;
	unsigned long physicalAddress;
	unsigned char contents[4096];
} ReadOnlyPage;

typedef struct SnapshotInfo {
	unsigned long pagesSaved;
	unsigned long pagesToSave;
	ReadOnlyPage *pages;
} SnapshotInfo;

typedef struct Computer {
	DisplayInfo videoInfo;
	multiboot_info_t *mbi;
	void *image;
	unsigned long snapshotStartAddress, snapshotEndAddress;
	void (*pageFaultHandler)(unsigned long);
	SnapshotInfo snapshot;
	unsigned long inPageFault;
	unsigned long pageFaultAddress;
	unsigned long totalPageFaults;
	unsigned long inGC;
} Computer;

#endif  /* __SQNOS_COMMON_STRUCTURES_H__ */