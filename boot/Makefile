TARGET = cd
# TARGET = floppy

FLOPPY = img.big.template
FLOPPY_VMX = vmx.floppy.template

CD = iso.template
CD_VMX = vmx.cd.template

img=GARCHA.img
mnt=MONTO.mnt

all: 

clean:
	rm -f *.img *.vmx *.vmsd *.vmem *.vmss *.o *.k *.log *.iso nvram

%.o: %.s
	as -o $@ $<

%.o: %.c
	gcc -o $@ -c $< -Wall -nostdlib

%.k: %.o loader.o kernel.ld
	ld -o $@ -T kernel.ld loader.o $< 

%.k: %.elf
	cp $< $@

%.img: %.k $(FLOPPY)
	cp $(FLOPPY) $(img)
	mkdir $(mnt)
	sudo mount -o loop $(img) $(mnt)
	sudo cp $< $(mnt)/boot/kernel
	sudo umount $(mnt)
	rmdir $(mnt)
	mv $(img) $@

%.iso: %.k $(CD) 
	cp $< $(CD)/SqueakNOS.kernel
	mkisofs -J -hide-rr-moved -joliet-long -l -r -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o $@ $(CD)

.PRECIOUS: %.img
	
%.floppy.vmx: %.img $(FLOPPY_VMX)
	cp $(FLOPPY_VMX) $@
	chmod +x $@
	echo 'floppy0.fileName = "$*.img"' >> $@

%.cd.vmx: %.iso $(CD_VMX)
	cp $(CD_VMX) $@
	chmod +x $@
	echo 'ide0:0.fileName = "$*.iso"' >> $@

%.try: %.$(TARGET).vmx
	vmplayer $<
#	vmware-server-console -m -x -l "`pwd`/$<"
#	make clean
