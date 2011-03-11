/*	$Id: ints.h,v 1.11 2001/04/04 03:29:55 gera Exp $	*/

#ifndef __INTS_H
#define __INTS_H

#include "sqNosCommonStructures.h"

typedef unsigned int uint32;
typedef int t_IRQSemaphores[16];

extern t_IRQSemaphores IRQSemaphores;

void voidISR();

void initInts();
void setIDT(uint32 *IDT,unsigned int intNum,void *ISR);

unsigned long isInsideRootTable(unsigned long virtualAddressFailure);

#define REAL_TIMER_FREQUENCY	2000
#define TIMER_DIVISOR	((int)(4772727/4/REAL_TIMER_FREQUENCY+1))
#define TIMER_FREQUENCY	(4772727.0/4/TIMER_DIVISOR)

#define IRQ_NONE	0x00
#define IRQ_TIMER	0x01
#define IRQ_KEYBOARD	0x02
#define IRQ_CASCADE	0x04
#define IRQ_3		0x08
#define IRQ_4		0x10
#define IRQ_5		0x20
#define IRQ_6		0x40
#define IRQ_7		0x80
#define IRQ_PS2		0x10

#define SAVE_ALL \
	"pushl %es\n\t" \
	"pushl %ds\n\t" \
	"pushl %eax\n\t" \
	"pushl %ebp\n\t" \
	"pushl %edi\n\t" \
	"pushl %esi\n\t" \
	"pushl %edx\n\t" \
	"pushl %ecx\n\t" \
	"pushl %ebx\n\t" \

#define RESTORE_ALL	\
	"popl %ebx\n\t"	\
	"popl %ecx\n\t"	\
	"popl %edx\n\t"	\
	"popl %esi\n\t"	\
	"popl %edi\n\t"	\
	"popl %ebp\n\t"	\
	"popl %eax\n\t"	\
	"popl %ds\n\t"	\
	"popl %es\n\t"	\

#define __ALIGN_STR	".align 16"

#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif

// #define asmlinkage CPP_ASMLINKAGE __attribute__((syscall_linkage))
#define asmlinkage CPP_ASMLINKAGE __attribute__((regparm(0)))

#define declareOneArgumentISR(name)		\
	void name##_interrupt();	\
	void name##ISR();	\
	asm( 			\
	".text\n" 	\
	 __ALIGN_STR "\n" 	\
	#name"_interrupt:\n\t"	\
	"push %eax\n\t"		\
	"movl 4(%esp), %eax\n\t"	\
	SAVE_ALL		\
	"push %eax\n\t"		\
	"call "#name"ISR \n\t"	\
	"addl $4,%esp\n\t" 	\
	RESTORE_ALL		\
	"pop %eax\n\t"		\
	"addl $4,%esp\n\t" 	\
	"iret\n\t"		\
	)


#define declareNativeISR(name)		\
	void name##_interrupt();	\
	void name##ISR();	\
	asm( 			\
	".text\n" 	\
	 __ALIGN_STR "\n" 	\
	#name"_interrupt:\n\t"	\
	SAVE_ALL		\
	"call "#name"ISR \n\t"	\
	RESTORE_ALL		\
	"iret\n\t"		\
	)

#define declareMasterSemaphoreISR(number)		\
	void irq_##number##_handler();	\
	asmlinkage void ISR_##number() {	\
		if (0!=IRQSemaphores[number])	\
		  signalSemaphoreWithIndex(IRQSemaphores[number]);	\
		else outb(0x20, 0x20);	\
	}				\
	declareAsmISR(number)

#define declareSlaveSemaphoreISR(number)		\
	void irq_##number##_handler();	\
	asmlinkage void ISR_##number() {	\
		if (0!=IRQSemaphores[number])	\
		  signalSemaphoreWithIndex(IRQSemaphores[number]);	\
		else {			\
		  outb(0x20, 0xA0);	\
		  outb(0x20, 0x20);	\
		}			\
	}				\
	declareAsmISR(number)

#define declareAsmISR(number)		\
	asm(				\
	".text\n"			\
	__ALIGN_STR"\n" 		\
	"irq_"#number"_handler:\n"	\
	SAVE_ALL			\
	"call ISR_"#number" \n\t"	\
	RESTORE_ALL			\
	"iret\n\t"			\
	)

#define cli()               asm("cli")
#define sti()        	    asm("sti")
#define clts()              asm("clts")
#define fninit()            asm("fninit")

#define outb(byte, port)	\
	if (port < 0x100) asm("outb %1, %0" :: "i" ((unsigned char)port), "a" ((unsigned char)byte));	\
	else asm("outb %0, %1" :: "d" ((unsigned char)port), "a" ((unsigned char)byte));

void inline lidt(uint32 offset, unsigned short size);

#endif /* __INTS_H */
