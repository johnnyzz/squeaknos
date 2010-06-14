
-include vm.conf.default

#use '-include' instead of 'include' so it doesn't fail if the file doesn't exist
-include vm.conf 


ifndef SRCDIR
include src32/plugins.ext
include src32/plugins.int

INTERNAL_LIBS = $(addsuffix .lib, $(INTERNAL_PLUGINS))
EXTERNAL_LIBS = $(addsuffix .dll, $(EXTERNAL_PLUGINS))
endif

VM       = SqueakNOS.obj
VMOUTDIR = $(BLDDIR)

ifndef SQIMAGE
SQIMAGE = SqueakNOS
endif

all: $(VM)

try: all
	cp $(SQIMAGE).image $(ISODIR)/SqueakNOS.image
	cp $(SQIMAGE).changes $(ISODIR)/SqueakNOS.changes
	cp $(VMOUTDIR)/$(VM) $(ISODIR)/boot/SqueakNOS.o
	make -C boot SqueakNOS.try

iso: all
	cp -r $(SRCDIR)/boot/grub $(ISODIR)/boot/
	cp $(SQIMAGE).image $(ISODIR)/SqueakNOS.image
	cp $(SQIMAGE).changes $(ISODIR)/SqueakNOS.changes
	cp $(VMOUTDIR)/$(VM) $(BLDDIR)/SqueakNOS.o
	make -C boot SqueakNOS.iso SqueakNOS.cd.vmx


DISTROEXCLUDES = .git $(BLDDIR) release isorelease bla mount otros testdata backup info sm bootdisk.raw
EXCLUDESTRING = $(addprefix --exclude platforms/squeaknos/, $(DISTROEXCLUDES))

distro:
	make -C boot clean
	echo $(EXCLUDESTRING)
	cd ../../ && tar --exclude package-cache $(EXCLUDESTRING) --exclude *.img --exclude *.image --exclude *.changes --exclude .svn --exclude $(SQIMAGE).* -cvf sources.tar platforms/squeaknos platforms/Cross
	mkdir -p $(ISODIR)
	cd $(ISODIR) && tar xvf ../../../../sources.tar
	make iso
	rm ../../sources.tar
	cd $(BLDDIR) && tar cjvf SqueakNOS-`date +%d-%b-%Y`.tar.bz2 SqueakNOS.iso SqueakNOS.cd.vmx
	cd $(BLDDIR) && zip -9 SqueakNOS-`date +%d-%b-%Y`.zip SqueakNOS.iso SqueakNOS.cd.vmx

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
ISODIR= $(BLDDIR)/iso

ifndef OBJDIR
OBJDIR= $(BLDDIR)
endif

VMDIR1 = $(SRCDIR)/src32/vm
VMDIR2 = $(SRCDIR)/../Cross/vm
VMDIR3 = $(SRCDIR)/nos
VMSRC = $(filter-out interp.c,$(notdir $(wildcard $(VMDIR2)/*.c) $(wildcard $(VMDIR3)/*.c) $(wildcard $(VMDIR1)/*.c)))
VMOBJ:=	$(VMSRC:.c=.o) gnu-interp.o
INCS =-I$(VMDIR1) -I$(VMDIR2) -I$(VMDIR3) -I$(SRCDIR)/../Cross/plugins/$(PLUGIN)

# second and third wildcards are hacks so that ffi plugin compiles
LIBSRC = $(wildcard *.c) $(wildcard $(SRCDIR)/plugins/$(PLUGIN)/x86-sysv*.c) $(wildcard $(SRCDIR)/../Cross/plugins/$(PLUGIN)/sqManualSurface*.c)
LIBSRCS = $(wildcard $(SRCDIR)/plugins/$(PLUGIN)/x86-sysv*.S)
LIBOBJ = $(LIBSRC:.c=.o) $(LIBSRCS:.S=.o)

LIBCOBJS = strchr.o strstr.o strcpy.o strncmp.o strcmp.o strlen.o memchr.o memcpy.o memcmp.o longjmp.o bsd-_setjmp.o __longjmp.o jmp-unwind.o sigprocmask.o

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
	mkdir -p $(ISODIR)
	mkdir -p $(ISODIR)/boot

### housekeeping
clean:
#	make -C boot clean
	-rm -rf $(BLDDIR)/iso
	-rm -f $(BLDDIR)/* *.iso *.vmx

cleannos:
	rm  $(BLDDIR)/SqueakNOSPlugin.lib
