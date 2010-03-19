ifndef SRCDIR
include src32/plugins.ext
include src32/plugins.int

INTERNAL_LIBS = $(addsuffix .lib, $(INTERNAL_PLUGINS))
EXTERNAL_LIBS = $(addsuffix .dll, $(EXTERNAL_PLUGINS))
endif

VM       = SqueakNOS.obj
VMOUTDIR = $(BLDDIR)

#SQIMAGE  = Squeak-ESUG
#SQIMAGE  = Squeak3.8-6665full
#SQIMAGE = SqueakNOS
SQIMAGE = PharoNOS

all: $(VM)

try: all
	cp $(SQIMAGE).image boot/iso.template/SqueakNOS.image
	cp $(SQIMAGE).changes boot/iso.template/SqueakNOS.changes
	cp $(VMOUTDIR)/$(VM) boot/SqueakNOS.o
	make -C boot SqueakNOS.try

iso: all
	cp $(SQIMAGE).image boot/iso.template/SqueakNOS.image
	cp $(SQIMAGE).changes boot/iso.template/SqueakNOS.changes
	cp $(VMOUTDIR)/$(VM) boot/SqueakNOS.o
	make -C boot SqueakNOS.iso SqueakNOS.cd.vmx
	mv boot/SqueakNOS.iso $(BLDDIR)
	mv boot/SqueakNOS.cd.vmx $(BLDDIR)/SqueakNOS.vmx

distro:
	make clean
	make -C boot clean
	rm -rf boot/iso.template/SqueakNOS.*
	rm -rf boot/iso.template/platforms
	# cd ../../../ && tar --exclude package-cache --exclude $(BLDDIR)/* --exclude sm --exclude $(SQIMAGE).* --exclude backup -cvf sources.tar trunk/platforms/SqueakNOS trunk/platforms/Cross
	#cd ../../../ && tar --exclude package-cache --exclude $(BLDDIR)/* --exclude info --exclude sm --exclude $(SQIMAGE).* --exclude backup -cvf sources.tar trunk/platforms/SqueakNOS trunk/platforms/Cross
	#cd boot/iso.template && tar xvf ../../../../../sources.tar
	cd ../../ && tar --exclude package-cache --exclude $(BLDDIR)/* --exclude info --exclude sm --exclude $(SQIMAGE).* --exclude backup -cvf sources.tar platforms/SqueakNOS platforms/Cross
	cd boot/iso.template && tar xvf ../../../../sources.tar
	rm ../../sources.tar
	make iso
	cd $(BLDDIR) && tar cjvf SqueakNOS-`date +%d-%b-%Y`.tar.bz2 SqueakNOS.iso SqueakNOS.vmx
	cd $(BLDDIR) && zip -9 SqueakNOS-`date +%d-%b-%Y`.zip SqueakNOS.iso SqueakNOS.vmx

AR = ar
CP = cp
RM = rm
LD = ld

CFLAGS = -fno-stack-protector -g  -O4 -fno-inline -fomit-frame-pointer -funroll-loops -fschedule-insns2
LDFLAGS = -static -nostdlib -lm -Xlinker -r
CXXFLAGS= $(CFLAGS) -felide-constructors
DEFS  = -DNO_STD_FILE_SUPPORT -DDEBUG -DLSB_FIRST -DX86 $(XDEFS)
XDEFS = -DSQUEAK_BUILTIN_PLUGIN

ifndef SRCDIR
SRCDIR=.
PLUGIN=.
endif

BLDDIR= $(SRCDIR)/release

ifndef OBJDIR
OBJDIR= $(BLDDIR)
endif

VMDIR1 = $(SRCDIR)/src32/vm
VMDIR2 = $(SRCDIR)/../Cross/vm
VMDIR3 = $(SRCDIR)/nos
VMSRC = $(filter-out interp.c,$(notdir $(wildcard $(VMDIR2)/*.c) $(wildcard $(VMDIR3)/*.c) $(wildcard $(VMDIR1)/*.c)))
VMOBJ:=	$(VMSRC:.c=.o) gnu-interp.o
INCS =-I$(VMDIR1) -I$(VMDIR2) -I$(VMDIR3) -I$(SRCDIR)/../Cross/plugins/$(PLUGIN)

LIBSRC = $(wildcard *.c) $(wildcard $(SRCDIR)/plugins/$(PLUGIN)/x86-sysv*.c)
LIBSRCS = $(wildcard $(SRCDIR)/plugins/$(PLUGIN)/x86-sysv*.S)
LIBOBJ = $(LIBSRC:.c=.o) $(LIBSRCS:.S=.o)

LIBCOBJS = strcpy.o strncmp.o strcmp.o strlen.o memcpy.o memcmp.o longjmp.o bsd-_setjmp.o __longjmp.o jmp-unwind.o sigprocmask.o

ALLOBJ=		$(VMOBJ) libc.o


# Where to look for files?
VPATH=		$(VMDIR1) $(VMDIR2) $(VMDIR3) $(VMOUTDIR)

.SUFFIXES:
.SUFFIXES:	.ccg .cc .c .o .s .S .i .rc .res .cg .hg .ccg
.INTERMEDIATE:  gnu-interp.c

$(VM): .ensureRelease $(ALLOBJ) $(INTERNAL_LIBS) 
		$(CC) -o $(VMOUTDIR)/$@ $(addprefix $(VMOUTDIR)/,$(ALLOBJ)) $(addprefix $(VMOUTDIR)/,$(INTERNAL_LIBS)) $(LDFLAGS) 

# Building plugins

makelib: $(LIBOBJ) 
	$(AR) rc $(LIB) $(LIBOBJ)
	$(RM) $(LIBOBJ)

%.lib:
	@$(MAKE) -C src32/vm/intplugins/$* -f ../../../../Makefile SRCDIR=../../../.. LIB=$*.lib OBJDIR=. XDEFS=-DSQUEAK_BUILTIN_PLUGIN PLUGIN=$* makelib
	$(CP) src32/vm/intplugins/$*/$*.lib $(BLDDIR)/$*.lib
	$(RM) src32/vm/intplugins/$*/$*.lib

# Rules for automated builds

.S.o:
	$(CC) -o $(OBJDIR)/$@ $(CFLAGS) $(INCS) $(DEFS) -c $<

.c.o: 
	$(CC) -o $(OBJDIR)/$@ $(CFLAGS) $(INCS) $(DEFS) -c $<

.cc.o:
	$(CXX) -o $(OBJDIR)/$@ $(CXXFLAGS) $(INCS) $(DEFS) -c $<

.c.s:
	$(CC) -S -o $@ -fverbose-asm -Wa,ah $(CFLAGS) $(INCS) $(DEFS) -c $<

.cc.s:
	$(CXX) -S -o $@ -fverbose-asm -Wa,ah $(CXXFLAGS) $(INCS) $(DEFS) -c $<

.c.i:
	$(CC) -E -o $@ $(CFLAGS) $(INCS) $(DEFS) -c $<

.cg.c:
	$(CCG) -n -o $@ $<

.hg.h:
	$(CCG) -n -o $@ $<

.ccg.cc:
	$(CCG) -n -o $@ $<

# Extra specific dependencies

libc.o: /usr/lib/libc.a
	cd $(OBJDIR)
	$(AR) x $< $(LIBCOBJS)
	ld -r -o $(BLDDIR)/$@ $(LIBCOBJS)
	$(RM) $(LIBCOBJS)

gnu-interp.c: interp.c
	./gnuify $< > $@

.ensureRelease:
	mkdir -p $(BLDDIR)

### housekeeping
clean:
	make -C boot clean
	-rm -f $(BLDDIR)/* *.iso *.vmx
