all: BootLoader Kernel32 Kernel64 Disk.img Application

BootLoader:
	@echo [*] Build Boot Loader

	make -C 00.BootLoader
	
	@echo [*] Build Boot Loader Complete

Kernel32:
	@echo [*] Build 32bit Kernel

	make -C 01.Kernel32
	
	@echo [*] Build 32bit Kernel Complete

Kernel64:
	@echo [*] Build 64bit Kernel

	make -C 02.Kernel64

	@echo [*] Build 64bit Complete

Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin 02.Kernel64/Kernel64.bin
	@echo [*] Build Disk Image

	04.Utility/00.ImageMaker/ImageMaker $^
	cat Disk.img Package.img > DiskWithPackage.img
	
	@echo [*] Build Disk Image Complete

Application:
	@echo [*] Application Build Start

	make -C 03.Application

	@echo [*] Build Application Complete

clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	make -C 02.Kernel64 clean
	make -C 03.Application clean
	rm -f Disk.img
	rm -f DiskWithPackage.img