
-include vm.conf.default

#use '-include' instead of 'include' so it doesn't fail if the file doesn't exist
-include vm.conf 


ifndef SRCDIR
include src32/plugins.ext
include src32/plugins.int

INTERNAL_LIBS = $(addsuffix .lib, $(INTERNAL_PLUGINS))
#INTERNAL_LIBS = $(filter-out SurfacePlugin.lib, $(INTERNAL_LIBS2))
EXTERNAL_LIBS = $(addsuffix .dll, $(EXTERNAL_PLUGINS))
endif

VM = SqueakNOS.obj


ifndef SQIMAGE
SQIMAGE = SqueakNOS
endif

all: $(VM)

kernel: all
	make -C boot SqueakNOS.kernel

try: all
	cp $(SQIMAGE).image $(ISODIR)/SqueakNOS.image
	cp $(SQIMAGE).changes $(ISODIR)/SqueakNOS.changes
	make -C boot SqueakNOS.try

iso: all
	cp -r $(SRCDIR)/boot/grub $(ISODIR)/boot/
	cp $(SQIMAGE).image $(ISODIR)/SqueakNOS.image
	cp $(SQIMAGE).changes $(ISODIR)/SqueakNOS.changes
	make -C boot SqueakNOS.iso SqueakNOS.cd.vmx

# kernel is not totally needed (all would be enough) but helps to detect errors early
distro: kernel
	mkdir -p $(DISTRODIR)/platforms/squeaknos
	mkdir -p $(DISTRODIR)/boot
	cp -r $(SRCDIR)/boot/grub $(DISTRODIR)/boot/
	rsync -a  $(SRCDIR)/boot $(SRCDIR)/common $(SRCDIR)/nos $(SRCDIR)/plugins $(SRCDIR)/scripts $(SRCDIR)/src32 $(DISTRODIR)/platforms/squeaknos
	rsync -a --exclude=.svn $(SRCDIR)/../Cross $(DISTRODIR)/platforms
	rsync -a $(SQIMAGE).image $(DISTRODIR)/SqueakNOS.image
	rsync -a $(SQIMAGE).changes $(DISTRODIR)/SqueakNOS.changes
	make -C boot ISODIR='../$(DISTRODIR)' SqueakNOS_distro.iso SqueakNOS_distro.cd.vmx
	cd $(BLDDIR) && tar cjf SqueakNOS-`date +%d-%b-%Y`.tar.bz2 SqueakNOS_distro.iso SqueakNOS_distro.cd.vmx
	cd $(BLDDIR) && zip -9 SqueakNOS-`date +%d-%b-%Y`.zip SqueakNOS_distro.iso SqueakNOS_distro.cd.vmx

AR = ar
CP = cp
RM = rm
LD = ld

CFLAGS = -fno-stack-protector -g  -O4 -fno-inline -fomit-frame-pointer -funroll-loops -fschedule-insns2
LDFLAGS = -static -nostdlib -lm -Xlinker -r
CXXFLAGS= $(CFLAGS) -felide-constructors
DEFS  = -DNO_STD_FILE_SUPPORT -DDEBUG -DLSB_FIRST -DX86 -D_FORTIFY_SOURCE=0 $(XDEFS)
XDEFS = -DSQUEAK_BUILTIN_PLUGIN

ifndef SRCDIR
SRCDIR=.
PLUGIN=.
endif

BLDDIR= $(SRCDIR)/release
ISODIR= $(BLDDIR)/iso
DISTRODIR= $(BLDDIR)/distro

ifndef OBJDIR
OBJDIR= $(BLDDIR)
endif

VMDIR1 = $(SRCDIR)/src32/vm
VMDIR2 = $(SRCDIR)/../Cross/vm
VMDIR3 = $(SRCDIR)/nos
VMSRC = $(filter-out interp.c,$(notdir $(wildcard $(VMDIR2)/*.c) $(wildcard $(VMDIR3)/*.c) $(wildcard $(VMDIR1)/*.c)))
VMOBJ:=	$(VMSRC:.c=.o) gnu-interp.o
INCS =-I$(VMDIR1) -I$(VMDIR2) -I$(VMDIR3) -I$(SRCDIR)/../Cross/plugins/$(PLUGIN)

# wildcards are hacks so that ffi and Alien plugins, which require extra compiling files, compile
ALIENPLUGINEXTRASRC = $(wildcard $(SRCDIR)/../Cross/plugins/$(PLUGIN)/Alien*.c) $(wildcard $(SRCDIR)/../Cross/plugins/$(PLUGIN)/ia32abicc*.c)
FFIPLUGINEXTRASRC = $(wildcard $(SRCDIR)/plugins/$(PLUGIN)/x86-sysv*.c) $(wildcard $(SRCDIR)/../Cross/plugins/$(PLUGIN)/sqManualSurface*.c)
SURFACEPLUGINEXTRASRC = $(wildcard $(SRCDIR)/../Cross/plugins/$(PLUGIN)/SurfacePlugin*.c)
PLUGINEXTRASRC = $(FFIPLUGINEXTRASRC) $(ALIENPLUGINEXTRASRC) $(SURFACEPLUGINEXTRASRC)
LIBSRC = $(wildcard *.c) $(PLUGINEXTRASRC)
LIBSRCS = $(wildcard $(SRCDIR)/plugins/$(PLUGIN)/x86-sysv*.S)
LIBOBJ = $(LIBSRC:.c=.o) $(LIBSRCS:.S=.o)


# Here we put which internal object files we need from libc. As each
# version of libc changes, you may need to change this. The way to find
# which ones you need is this:
# ar t nos/bin/libc.a | grep strcmp # if we want to find all .o related
# to strcmp. We got all this:
STRCMPOBJS = strcmp.o strncmp.o strcmp-ssse3.o strcmp-sse4.o strncmp-c.o strncmp-ssse3.o strncmp-sse4.o
MEMCPYOBJS = memcpy.o
# memcpy-ssse3.o memcpy-ssse3-rep.o
MEMCMPOBJS = memcmp.o
# memcmp-ssse3.o memcmp-sse4.o
LONGJMPOBJS = setjmp.o sigjmp.o longjmp.o __longjmp.o
# longjmp_chk.o ____longjmp_chk.o 
#sigprocmask.o
DLSUPPORTOBJS = dl-support.o enbl-secure.o getenv.o access.o

#LIBCOBJS = strchr.o strcpy.o strlen.o memchr.o memcpy.o longjmp.o bsd-_setjmp.o __longjmp.o jmp-unwind.o sigprocmask.o
LIBCOBJS = strchr.o strcpy.o strlen.o memchr.o $(MEMCPYOBJS) $(LONGJMPOBJS) bsd-_setjmp.o jmp-unwind.o 

LIBGCCOBJS = _divdi3.o _moddi3.o

ALLOBJ=		$(VMOBJ) libc.o libgcc.o


# Where to look for files?
VPATH=		$(VMDIR1) $(VMDIR2) $(VMDIR3) $(BLDDIR)

.SUFFIXES:
.SUFFIXES:	.ccg .cc .c .o .s .S .i .rc .res .cg .hg .ccg
.INTERMEDIATE:  gnu-interp.c

$(VM): .ensureRelease $(ALLOBJ) $(INTERNAL_LIBS) 
		$(CC) -o $(BLDDIR)/$@ $(addprefix $(BLDDIR)/,$(ALLOBJ)) $(addprefix $(BLDDIR)/,$(INTERNAL_LIBS)) $(LDFLAGS) 

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

libgcc.o: 
	$(AR) x `gcc -print-libgcc-file-name` $(LIBGCCOBJS)
	ld -r -o $(BLDDIR)/$@ $(LIBGCCOBJS)
	$(RM) $(LIBGCCOBJS)



gnu-interp.c: interp.c
	./gnuify $< > $@
	cp gnu-interp.c gnu-interp.c.bak


.ensureRelease:
	mkdir -p $(BLDDIR)
	mkdir -p $(ISODIR)
	mkdir -p $(ISODIR)/boot

### housekeeping
clean:
#	make -C boot clean
	-rm -rf $(BLDDIR)/iso $(BLDDIR)/distro
	-rm -f $(BLDDIR)/* *.iso *.vmx

cleannos:
	rm  $(BLDDIR)/SqueakNOSPlugin.lib
