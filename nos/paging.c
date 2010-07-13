
/**
 *  This file handles the basic stuff related to memory paging
**/

// we add 1024 extra so we can get aligned to 1024 blocks

// returns a pointer to an array of 1024 page directory entries
long* page_directory()
{
	static long pd[1024+1024];
	return (long*)((long)(pd+1023) & 0xfffff000);
}

// returns a pointer to an array of 1024 page tables, of 1024 entries each.
// 1024 pages per table * 1024 tables = 1 M entries
// 4 KB per page * 1 M pages = 4 GB
long* page_tables_start()
{
	static long pt[1024*1024+1024];
	return (long*)((long)(pt+1023) & 0xfffff000);
}

void generate_empty_page_directory()
{
	//set each entry to not present
	int i = 0;
	long *next_page_table = page_tables_start();
	for(i = 0; i < 1024; i++, next_page_table += 1024)
	{
		//attribute: supervisor level, read/write, present.
		page_directory()[i] = (long)next_page_table | 3;
	}
	
	next_page_table = page_tables_start();
	//~ next_page_table += 1024 * 10;
	//~ 
	//~ for(i = 0x10; i < 1020; i++, next_page_table += 1024)
	//~ {
		//~ //attribute: supervisor level, read/write, present.
		//~ page_directory()[i] = (long)next_page_table;
	//~ }
}


void generate_empty_page_tables()
{
	// holds the physical address where we want to start mapping these pages to.
	// in this case, we want to map these pages to the very beginning of memory.
	unsigned int address = 0; 
	unsigned int i;
	 
	//we will fill all 1024*1024 entries, mapping 4 gigabytes
	for(i = 0; i < 1024*1024; i++)
	{
		page_tables_start()[i] = address | 3; // attributes: supervisor level, read/write, present.
		address = address + 4096; //advance the address to the next page boundary
	}
}


void enable_paging_in_hardware()
{
	unsigned int cr0;
	long *pd = page_directory();
	
	asm volatile("xchg %%bx, %%bx" ::: "ebx");
	
	//moves pd (which is a pointer) into the cr3 register.
	asm volatile("mov %0, %%cr3":: "b"(pd));
	
	//reads cr0, switches the "paging enable" bit, and writes it back.
	
	asm volatile("mov %%cr0, %0": "=b"(cr0));
	cr0 |= 0x80000000;
	asm volatile("mov %0, %%cr0":: "b"(cr0));
}


void enable_paging()
{
	generate_empty_page_tables();
	generate_empty_page_directory();
	enable_paging_in_hardware();
	
	asm volatile("xchg %%bx, %%bx" ::: "ebx"); // This is only qemu debugging stuff...
}
