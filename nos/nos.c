#include "sq.h"
#include "sqPlatformSpecific.h"
#include "sqLibc.h"
#include "ints.h"
#include "multiboot.h"
#include "framebuffer.h"
#include "ofw.h"

Computer computer;
MemoryFile block;

unsigned long tabs = 0; // used for outputting to console

void *OFW_callout;
void *os_exports[][3] = { {NULL, NULL, NULL} };

void initializeComputer(unsigned long magic, multiboot_info_t *mbi);
void parseVideoInfo   (DisplayInfo *videoInfo, char *videoConfigLine);
void parseVideoInfoOFW(DisplayInfo *videoInfo);

static
char* parseString(char string[], int *variable, char separator_token);

void* getImageFromModules (multiboot_info_t *mbi);

void set_std_console_debugging(int debugging);
#include "../shared/splashscreen.c"

static unsigned int length = 0;

void _main (unsigned long magic, multiboot_info_t *mbi)
{
	//enable_paging();
	initializeComputer(magic, mbi);
	
	//fill_rectangle(100, 50, 700, 300, 0x00ff0000);
	//fill_rectangle(100, 50, 700, 350, 0x0000ff00);
	//fill_rectangle(100, 50, 700, 400, 0x000000ff);
	
	memcpy (computer.videoInfo.address, splashscreen_image.pixel_data, splashscreen_image.width * splashscreen_image.height * 4);
	//bitblt_32bit_to_fb(splashscreen_image.pixel_data, splashscreen_image.width, splashscreen_image.height, 0, 0);
	//printf("aaaa\nbbbb\ncccc\ndddd\neeee\nffff\ngggg");

	if (computer.image)
	{
		printf_pocho("Found image at 0x%x\n", computer.image);
		initInts();
		sqMain(computer.image, length);
	}
	mark(0x00ff);
}

void initializeComputer(unsigned long magic, multiboot_info_t *mbi)
{
	computer.snapshotStartAddress = 0;
	computer.snapshotEndAddress = 0;

	computer.image = NULL;
	
	// set the memory map that grubs passes (grub gets it by asking the bios) to
	// a variable we can query from the image by using a primitive	
	computer.mbi = mbi;
	computer.inPageFault = 0;
	computer.inGC = 0;
		/* Are mods_* valid?  */
	if (magic == MULTIBOOT_BOOTLOADER_MAGIC)
	{
		if (mbi->flags && MULTIBOOT_INFO_CMDLINE)
		{
			parseVideoInfo(&computer.videoInfo, (char*)mbi->cmdline);
		}

		initialize_std_console();

		if (mbi->flags && MULTIBOOT_INFO_MODS)
		{
			computer.image = getImageFromModules(mbi);
		}
	}
	else
	{
		OFW_callout = (void*)magic;
		parseVideoInfoOFW(&computer.videoInfo);
		computer.image = (void*)0x800000;
	}

}

void parseVideoInfoOFW(DisplayInfo *videoInfo)
{
	videoInfo->width = 1200;
	videoInfo->height = 900;
	videoInfo->depth = 16;
	videoInfo->address = 0xfd000000;
	videoInfo->bytesPerScanLine = 2400;
}


/**
 * videoConfigLine must be something like video=1024x768x32@0xf0000000,4096
 * where 0xf0000000 is the base address and 4096 is the number of bytes per scanline
 * if the number of byte per scanline is not present, it's going to be guessed (X/8)
**/
void parseVideoInfo(DisplayInfo *videoInfo, char *videoConfigLine)
{
	/* videoConfigLine is {width}x{height}x{depth}@{address},{bytesPerScanLine}\0 */

	char sep_tokens[] = { /*width*/ 'x', /*height*/'x', /*depth*/ '@', /*address*/ ',', /*bytesPerScanLine*/ '\0'};
	
	videoConfigLine = strstr(videoConfigLine, "video=") + 6;

	videoConfigLine = parseString(videoConfigLine, &videoInfo->width,   sep_tokens[0]);
	videoConfigLine = parseString(videoConfigLine, &videoInfo->height,  sep_tokens[1]);
	videoConfigLine = parseString(videoConfigLine, &videoInfo->depth,   sep_tokens[2]);
	videoConfigLine = parseString(videoConfigLine, &videoInfo->address, sep_tokens[3]);
	
	if (videoConfigLine)
		parseString(videoConfigLine, &videoInfo->bytesPerScanLine, sep_tokens[4]);
	else
		videoInfo->bytesPerScanLine = bytesPerLine(videoInfo->width, videoInfo->depth);
}

static char* parseString(char string[], int *variable, char separator_token)
{
	if (!string)
		return 0;
		
	char *nextChar = string;
	
	while (*nextChar && *nextChar != separator_token)
	{
		nextChar++;
	}

	char original = *nextChar;
	*nextChar = 0;
	*variable = _atoi(string);
	
	// if we got into the end of the string return 0, else return next position
	return original == 0 ? 0 : nextChar + 1;
}

void* getImageFromModules (multiboot_info_t *mbi)
{
	module_t *mod = (module_t *) mbi->mods_addr;

	void *image = 0;

	if (mbi->mods_count >= 1)
	{
		image = (void*)mod->mod_start;
		length = mod->mod_end - mod->mod_start;
	}
	
	printf_pocho ("mods_count = %d, mods_addr = 0x%x\n", (int) mbi->mods_count, (int) mbi->mods_addr);
	
	int i;
	for (i = 0; i < mbi->mods_count; i++, mod++)
	{
		printf_pocho("mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
				(unsigned) mod->mod_start,
				(unsigned) mod->mod_end,
				(char *)   mod->string);
				
		if (strcmp((char *)mod->string, "/SqueakNOS.config") == 0)
		{
			char *conf_str = (char *)mod->mod_start;
			*((char*)mod->mod_end) = 0;
			printf_pocho("Se encontro el modulo de config\n");
			printf_pocho("Dice %s\n", conf_str);
			set_std_console_debugging(conf_str[0] - '0');
			printf_pocho("Fin el modulo de config\n");
		}
	}
	
	return image;
}

sqInt setMicroSecondsandOffset(sqLong * microSeconds, int * utcOffset) {
	flag("toRemove");
	return -1;
}

void printMemoryState(){
	extern unsigned long endOfMemory,memory,memoryLimit,freeBlock,youngStart;
	printf_pocho("Memory: %d \n", memory);
	printf_pocho("Young start: %d \n", youngStart);
	printf_pocho("Free block: %d \n", freeBlock);
	printf_pocho("End of memory: %d \n", endOfMemory);
	printf_pocho("Memory limit: %d \n", memoryLimit);
}	
