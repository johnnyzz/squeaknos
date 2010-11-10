TARGET = cd
# TARGET = floppy

FLOPPY = img.big.template
FLOPPY_VMX = vmx.floppy.template

CD_VMX = vmx.cd.template

img=GARCHA.img
mnt=MONTO.mnt

ifndef SRCDIR
SRCDIR=..
endif

BLDDIR= $(SRCDIR)/release
ISODIR= $(BLDDIR)/iso

all: 

clean:
	rm -rf $(ISODIR)
	rm -f $(ISODIR) *.img *.vmx *.vmsd *.vmem *.vmss *.o *.k *.log *.iso nvram

%.o: %.s
	as -o $@ $<

%.o: %.c
	gcc -o $(BLDDIR)/$@ -c $< -Wall -nostdlib

%.kernel: $(BLDDIR)/SqueakNOS.obj loader.o kernel.ld
	ld -o $(BLDDIR)/SqueakNOS.kernel -T kernel.ld loader.o $< 

%.kernel: %.elf
	cp $(BLDDIR)/$< $(BLDDIR)/$@

%.img: %.kernel $(FLOPPY)
	cp $(FLOPPY) $(img)
	mkdir $(mnt)
	sudo mount -o loop $(img) $(mnt)
	sudo cp $< $(mnt)/boot/kernel
	sudo umount $(mnt)
	rmdir $(mnt)
	mv $(img) $@

%.iso: %.kernel
	cp $(BLDDIR)/SqueakNOS.kernel $(ISODIR)/SqueakNOS.kernel
	cp $(SRCDIR)/SqueakNOS.config $(ISODIR)/SqueakNOS.config
	mkisofs -J -hide-rr-moved -joliet-long -l -r -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $(BLDDIR)/$@ $(ISODIR)


.PRECIOUS: %.img
	
%.floppy.vmx: %.img $(FLOPPY_VMX)
	cp $(FLOPPY_VMX) $@
	chmod +x $@
	echo 'floppy0.fileName = "$*.img"' >> $@

%.cd.vmx: %.iso $(CD_VMX)
	cp $(CD_VMX) $(BLDDIR)/$@
	chmod +x $(BLDDIR)/$@
	echo 'ide0:0.fileName = "$*.iso"' >> $(BLDDIR)/$@

%.try: %.$(TARGET).vmx
	vmplayer $(BLDDIR)/$<
#	vmware-server-console -m -x -l "`pwd`/$<"
#	make clean
