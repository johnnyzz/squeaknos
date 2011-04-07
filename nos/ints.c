/*	$Id: ints.c,v 1.19 2001/08/15 05:30:40 gera Exp $	*/

#include "ints.h"
#include "sq.h"

volatile unsigned long timer=0;

t_IRQSemaphores IRQSemaphores;

declareNativeISR(clock);
declareMasterSemaphoreISR(1);
declareMasterSemaphoreISR(2);
declareMasterSemaphoreISR(3);
declareMasterSemaphoreISR(4);
declareMasterSemaphoreISR(5);
declareMasterSemaphoreISR(6);
declareMasterSemaphoreISR(7);
declareSlaveSemaphoreISR(8);
declareSlaveSemaphoreISR(9);
declareSlaveSemaphoreISR(10);
declareSlaveSemaphoreISR(11);
declareSlaveSemaphoreISR(12);
declareSlaveSemaphoreISR(13);
declareOneArgumentISR(pageFault);
declareSlaveSemaphoreISR(15);

void initInts() {
	static uint32 IDT[0x100*2]={0};
	int i;

	clts();     // Clear Task Switch flag, just in case the bootloader left it set (OFW does)
	fninit();	// Initialize FPU, don't check for pending exceptions
	cli();      // Stop interrupts
	for (i=0;i<sizeof IRQSemaphores/sizeof IRQSemaphores[0];i++)
		IRQSemaphores[i]=0;

	for (i=0x0;i<0x100;i++)
		setIDT(IDT,i,voidISR);
	
	// master PIC
	setIDT(IDT,0x0,clock_interrupt);
	setIDT(IDT,0x1,irq_1_handler);
	setIDT(IDT,0x2,irq_2_handler);
	setIDT(IDT,0x3,irq_3_handler);
	setIDT(IDT,0x4,irq_4_handler);
	setIDT(IDT,0x5,irq_5_handler);
	setIDT(IDT,0x6,irq_6_handler);
	setIDT(IDT,0x7,irq_7_handler);
	
	// Slave PIC
	setIDT(IDT,0x8,irq_8_handler);
	setIDT(IDT,0x9,irq_9_handler);
	setIDT(IDT,0xa,irq_10_handler);
	setIDT(IDT,0xb,irq_11_handler);
	setIDT(IDT,0xc,irq_12_handler);
	setIDT(IDT,0xd,irq_13_handler);
	setIDT(IDT,0xe,pageFault_interrupt);
	setIDT(IDT,0xf,irq_15_handler);

	// Init PIC and make IRQs go from 0 to 0x10
	
	outb(0x11, 0x20);	// 8086/88 mode, Cascade, Edge triggered
	outb(0x11, 0xa0);	// same for seconds PIC
	outb(0x00, 0x21);	// Offset for 1st PIC is INT 0
	outb(0x08, 0xa1);	// Offset for 2nd PIC is INT 8
	outb(0x04, 0x21);	// Slave is connected to IRQ2
	outb(0x02, 0xa1);	// Slave is connected to IRQ2
	outb(0x01, 0x21);	// 8086 Mode
	outb(0x01, 0xa1);	// 8086 Mode

	// set timer frequency

	outb(0x34, 0x43);	// timer 0, mode binary, write 16 bits count
	outb(TIMER_DIVISOR & 0xff, 0x40);
	outb((TIMER_DIVISOR >> 8) & 0xff, 0x40);
	outb(~(IRQ_TIMER),0x21);
	lidt((uint32)IDT,sizeof(IDT));
	sti();      // Resume interrupts*/
}

inline getCS() { 
   register unsigned short CS; 
   asm("mov %%cs, %0" : "=r" (CS)); 
   return CS; 
}

void setIDT(uint32 *IDT,unsigned int intNum,void *ISR) {
	IDT[2*intNum]=(getCS() << 16) | ((uint32)ISR & 0x0000ffff);
#if 0
	IDT[2*intNum]=0x00600000 | ((uint32)ISR & 0x0000ffff);
#endif
	IDT[2*intNum+1]=((uint32)ISR & 0xffff0000) | 0x00008E00;
}

void clockISR() {
	timer++;
	// *(long*)(0xfd000000+timer)=0;
	outb(0x20,0x20);
}

typedef struct VmStatus {
	usqInt methodStatus,messageSelectorStatus,newMethodStatus;
	sqInt receiverClassStatus,methodClassStatus,lkupClassStatus,newNativeMethodStatus,argumentCountStatus, successFlagStatus, primitiveIndexStatus;
	void *primitiveFunctionPointerStatus;
} VmStatus;

void saveStatus(VmStatus *status){
	extern usqInt method,messageSelector,newMethod;
	extern sqInt receiver, receiverClass,methodClass,lkupClass,newNativeMethod,argumentCount,successFlag, primitiveIndex;
	extern void *primitiveFunctionPointer;
	status->messageSelectorStatus = messageSelector;
	status->newMethodStatus = newMethod;
	status->methodClassStatus = methodClass;
	status->lkupClassStatus = lkupClass;
	status->receiverClassStatus = receiverClass;
	status->newNativeMethodStatus = newNativeMethod;
	status->argumentCountStatus = argumentCount;
	status->successFlagStatus = successFlag;
	status->primitiveIndexStatus = primitiveIndex;
	status->primitiveFunctionPointerStatus = primitiveFunctionPointer;
	printf_pocho("Sali con: \n");
	printStringOf(messageSelector);
	printf_pocho("\n");
	printNameOfClasscount(lkupClass,5);
	printf_pocho("\n");
	printNameOfClasscount(receiverClass,5);
	printf_pocho("\n");
}

void releaseStatus(VmStatus *status){
	extern usqInt method,messageSelector,newMethod;
	extern sqInt receiver, receiverClass,methodClass,lkupClass,newNativeMethod,argumentCount,successFlag, primitiveIndex;
	extern void *primitiveFunctionPointer;
	printf_pocho("Volvi con: \n");
	printStringOf(messageSelector);
	printf_pocho("\n");
	printNameOfClasscount(lkupClass,5);
	printf_pocho("\n");
	printNameOfClasscount(receiverClass,5);
	printf_pocho("\n");
	messageSelector = status->messageSelectorStatus;
	newMethod = status->newMethodStatus;
	methodClass = status->methodClassStatus;
	lkupClass = status->lkupClassStatus;
	receiverClass = status->receiverClassStatus;
	newNativeMethod = status->newNativeMethodStatus;
	argumentCount = status->argumentCountStatus;
	successFlag = status->successFlagStatus;
	primitiveIndex = status->primitiveIndexStatus;
	primitiveFunctionPointer = status->primitiveFunctionPointerStatus;
	printf_pocho("Retorno a: \n");
	printStringOf(messageSelector);
	printf_pocho("\n");
	printNameOfClasscount(lkupClass,5);
	printf_pocho("\n");
	printNameOfClasscount(receiverClass,5);
	printf_pocho("\n");
}

void pageFaultISR(unsigned long errorCode) {
	extern Computer computer;
	extern t_IRQSemaphores IRQSemaphores;
	extern unsigned long tabs;
	unsigned long virtualAddressFailure;
	computer.inPageFault++;
	computer.totalPageFaults++;
	asm volatile("movl %%cr2, %0" : "=a" (virtualAddressFailure));
	printf_pochoTab(tabs, "PageFaultISR: Entre\n");
	tabs+=1;
	printf_pochoTab(tabs,"PageFaultISR: Esta en la rootTable: %d\n",isInsideRootTable(virtualAddressFailure));
	computer.pageFaultAddress = virtualAddressFailure;
	sti();
	if ((errorCode & 1) == 1){
		// Protection page fault
		if ((computer.inPageFault > 1) || (computer.inGC)){
			printf_pochoTab(tabs,"PageFaultISR: Paginas salvadas a mano: %d de %d \n",computer.snapshot.pagesSaved,computer.snapshot.pagesToSave);
			printf_pochoTab(tabs,"PageFaultISR: Entre al pageFaultNativo en:%d\n",virtualAddressFailure);
			saveSnapshotPage(virtualAddressFailure);
		} else {
			VmStatus status;
			signalSemaphoreWithIndex(IRQSemaphores[15]);
			printf_pochoTab(tabs,"PageFaultISR: Entre al pageFaultCallback en:%d\n",virtualAddressFailure);
			saveStatus(&status);
			computer.pageFaultHandler(virtualAddressFailure);
			releaseStatus(&status);
		}
	} else {
		// page not present
		signalSemaphoreWithIndex(IRQSemaphores[15]);
		printf_pochoTab(tabs,"PageFaultISR: Inside a not present page fault");
		computer.pageFaultHandler(virtualAddressFailure);
	}
	tabs-=1;
	computer.inPageFault--;
	printf_pochoTab(tabs,"PageFaultISR: Sali\n");
}

unsigned long isInsideRootTable(unsigned long virtualAddressFailure){
	unsigned long i,oop;
	extern unsigned long rootTableCount,extraRootCount,tabs;
	extern unsigned long* rootTable;
	extern unsigned long* extraRoots;
	for (i = 1; i <= rootTableCount; i += 1) {
		oop = rootTable[i];
		if ((virtualAddressFailure >= oop) && (virtualAddressFailure <= oop + 100)){ printf_pochoTab(tabs,"IsInsideRootTable: RootTable: %d",oop);return 1;}
	}
	
	for (i = 1; i <= extraRootCount; i += 1) {
		oop = ((unsigned long *)(extraRoots[i]))[0];
		if (!((oop & 1))) {
			if ((virtualAddressFailure >= oop) && (virtualAddressFailure <= oop + 100)){printf_pochoTab(tabs,"IsInsideRootTable: ExtraRootTable: %d",oop);return 1;}
		}
	}
	return 0;
}


/* voidISR */
asm( \
        "\n" __ALIGN_STR"\n" \
        "voidISR:\n\t" \
	"incl (0xb800c)\n\t"	\
	"push	%eax\n\t"	\
	"movb $32, %al\n\t"	\
	"outb %al, $32\n\t"	\
	"pop	%eax\n\t"	\
        "iret\n\t"
);

void inline lidt(uint32 offset, unsigned short size) {

        static struct {
                unsigned short limit;
                unsigned long addr __attribute__((packed));
        } idt_descriptor;

        idt_descriptor.limit = size;
        idt_descriptor.addr = offset;

        __asm__ __volatile__ ("lidt %0" :: "m" (idt_descriptor));
}

void connectToAPM() {
	//connect to an APM interface
	asm("mov 0x53,%ah"); //this is an APM command
    asm("mov 3,%al"); // connecting to real mode
    asm("xor %bx,%bx"); // and device 0 (0 = APM BIOS)
    asm("int $0x15");     // call the BIOS function through interrupt 15h
	//asm("jc APM_error") // if the carry flag is set there was an error
}	

void enablePowerManagement() {
	asm("mov 0x53,%ah"); //this is an APM command
	asm("mov 0x8,%al");  // Change the state of power management...
	asm("mov 1, %bx"); // ...on all devices to...
	asm("mov 1, %cx"); //...power management on.
	asm("int $0x15"); // call the BIOS function through interrupt 15h
}

void shutdownComputer() {
	
	connectToAPM();
	enablePowerManagement();
	asm("mov 0x53, %ah"); // this is an APM command
	asm("mov 0x7, %al"); // Set the power state...
	asm("mov 1, %bx"); //...on all devices to...
	asm("mov 0x3, %cx"); // power mode: shutdown
	asm("int $0x15"); // call the BIOS function throu
}