#include "sq.h"
#include "sqPlatformSpecific.h"
#include "multiboot.h"

#include "ofw.h"

#undef DEBUG


void initInts();

int errno;
int *__errno_location = &errno;
multiboot_info_t *__mbi = 0;


int printf_pocho (const char *format, ...);
static void cls (void);
void parseVideoInfo(char *);

void parseVideoInfoOFW();
void *OFW_callout;

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))

void mark(int col)
{
#ifdef DEBUG
	static count = 1;
	short *video = 0xfd000000;
	int i;

	for (i=10*count; i<1200*10-10*count;i++)
		video[i+count*1200*10] = col;

	count++;
	count++;
#endif
}


void _main (unsigned long magic, multiboot_info_t *mbi)
{
	int i;
	void *image = NULL;
	
	// set the memory map that grubs passes (grub gets it by asking the bios) to
	// a variable we can query from the image by using a primitive
	__mbi = mbi;

	enable_paging();
	
	/* Are mods_* valid?  */
	//printf_pocho("Paging has been enabled\n");
	if (magic == MULTIBOOT_BOOTLOADER_MAGIC)
	{
		if (CHECK_FLAG (mbi->flags, 2))
		{
			char *video = (char*)mbi->cmdline;
			int i, len = strlen(video);
			// cmdline must be something like video=1024x768x32@0xf0000000,4096
			// where 0xf0000000 is the base address and 4096 is the number of bytes per scanline
			// if the number of byte per scanline is not present, it's going to be guessed (X/8)

			for (i = 6; i<len; i++)
			{
				if (video[i] == '=')
					if (!memcmp(&video[i-6], " video", 6))
					{
						video = &video[i+1];
						parseVideoInfo(video);
						break;
					}
			}
		}

		if (CHECK_FLAG (mbi->flags, 3))
		{
			module_t *mod;
			int i;

			printf_pocho ("mods_count = %d, mods_addr = 0x%x\n",
			              (int) mbi->mods_count, (int) mbi->mods_addr);
			for (i = 0, mod = (module_t *) mbi->mods_addr;
			        i < mbi->mods_count;
			        i++, mod++)
				printf_pocho (" mod_start = 0x%x, mod_end = 0x%x, string = %s\n",
				              (unsigned) mod->mod_start,
				              (unsigned) mod->mod_end,
				              (char *) mod->string);

			if (mbi->mods_count == 1)
			{
				module_t *mod = (module_t*)mbi->mods_addr;
				image = (void*)mod->mod_start;
			}
		}
	}
	else
	{
		OFW_callout = (void*)magic;
		parseVideoInfoOFW();
		image = (void*)0x800000;
	}

	initialize_std_console();

	if (image)
	{
		printf_pocho("Found image at 0x%x\n", image);
		initInts();
		sqMain(image);
	}
	mark(0x00ff);
}


