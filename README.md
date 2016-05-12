# uub-linux
PetaLinux distribution for UUB

## Steps to build

Replace $(PATH_TO_PETALINUX) and $(PATH_TO_XILINX) with your paths to the petalinux and Xilinx installs, respectively.

1: Clone the repository (git clone https://github.com/barawn/uub-linux.git uub-linux)
2: Source the PetaLinux settings (source $(PATH_TO_PETALINUX)/settings.sh)
3: Source the Vivado settings (source $(PATH_TO_XILINX)/Vivado/2015.2/settings64.sh)
3: Change into the repository (cd uub-linux)
4: Build (petalinux-build).

Note that it needs to be settings64.sh, as Vivado doesn't support 32-bit Linux.

## Steps to create image

Once petalinux-build completes, it creates a bunch of files in the images/ directory. One thing it does NOT
create is the FSBL, because our configuration tells it NOT to build the FSBL. Because we need to use a custom FSBL,
thanks to the QSPI reset errata.

The FSBL is located in prebuilt_images/zynq_fsbl.elf. Copy that to the images directory:

```
$ cp prebuilt_images/zynq_fsbl.elf images/linux/
```

Now create the "pre-built" images. These are the images that PetaLinux uses to boot Linux **over JTAG**. In order to do this,
it needs to use a different FSBL, so we'll grab that one too.

```
$ petalinux-build --prebuilt
$ cp prebuilt_images/zynq_fsbl_JTAG.elf pre-built/linux/zynq_fsbl.elf
```

## Steps to first boot of U-Boot

The first boot needs to be done over JTAG, because nothing is in flash yet. We want to use XMD and petalinux-boot to do this. petalinux-boot does not seem to be able to force , however, so to be safe, we need to do (replace CABLE_CONNECTION_HERE with your
cable settings (e.g. -cable type xilinx_platformusb, or -cable url tcp:IP_ADDRESS:3121)

```
$ xmd
****** Xilinx Microprocessor Debugger (XMD) Engine
****** XMD v2015.2 (64-bit)
  **** SW Build 1266856 on Fri Jun 26 16:35:25 MDT 2015
    ** Copyright 1986-2015 Xilinx, Inc. All Rights Reserved.

                                                                                
XMD% 
XMD% connect arm hw -cable CABLE_CONNECTION_HERE
(... connection info comes here ...)
XMD% rst
System reset successfully

0
XMD% exit
```

Now, we can use petalinux-boot. Open up a serial connection to the UUB (9600 bps now!) to get ready. Then do

```
$ petalinux-boot --jtag --prebuilt 2 
```

If you're using a remote hardware server (like me) and you're not directly connected to the cable, you'll need to add --hw_server-url tcp:IP_ADDRESS_OF_HOST:3121 as well.

**NOTE** : if this is the FIRST TIME booting this on a machine, this next step will take a LONG TIME. As in, about an hour. It will look like it hangs at "attaching mtd1 to ubi0". Just wait! Be patient.

```
Xilinx First Stage Boot Loader
Release 2015.2  Jan  7 2016-11:21:38
Devcfg driver initialized
Silicon Version 3.1
Boot mode is JTAG
RESET_SUCCESS is high.
Model: uub-linux
DRAM:  ECC disabled 512 MiB
SF: Detected N25Q1024 with page size 256 Bytes, erase size 64 KiB, total 256 MiB
UBI: attaching mtd1 to ubi0
UBI: scanning is finished
UBI: attached mtd1 (name "mtd=1", size 24 MiB) to ubi0
UBI: PEB size: 65536 bytes (64 KiB), LEB size: 65408 bytes
UBI: min./max. I/O unit sizes: 1/256, sub-page size 1
UBI: VID header offset: 64 (aligned 64), data offset: 128
UBI: good PEBs: 384, bad PEBs: 0, corrupted PEBs: 0
UBI: user volume: 3, internal volumes: 1, max. volumes count: 128
UBI: max/mean erase counter: 2/0, WL threshold: 4096, image sequence number: 85381688
UBI: available PEBs: 0, total reserved PEBs: 384, PEBs reserved for bad PEB handling: 0
In:    serial
Out:   serial
Err:   serial
Net:   Gem.e000b000
Error: Gem.e000b000 address not set.

Hit any key to stop autoboot:  0
U-Boot-PetaLinux>
```

Now do
```
U-Boot-PetaLinux> saveenv
Saving Environment to UBI...
(... bunch of UBI details here...)
Writing to UBI... done
```

and **do it again**:

```
U-Boot-PetaLinux> saveenv
Saving Environment to UBI...
(... bunch of UBI details here...)
Writing to redundant UBI... done
```

There are 2 copies of the U-Boot environment in flash, so this updates both of them.

## Steps to first boot of Linux

Now U-Boot has prepared the UBI environment. So we now need to boot Linux to get access to everything!

So again do

```
$ xmd
****** Xilinx Microprocessor Debugger (XMD) Engine
****** XMD v2015.2 (64-bit)
  **** SW Build 1266856 on Fri Jun 26 16:35:25 MDT 2015
    ** Copyright 1986-2015 Xilinx, Inc. All Rights Reserved.

                                                                                
XMD% 
XMD% connect arm hw -cable CABLE_CONNECTION_HERE
(... connection info comes here ...)
XMD% rst
System reset successfully

0
XMD% exit
```

and now do

```
$ petalinux-boot --jtag --prebuilt 3
```

The 'prebuilt 3' launches Linux. This will take MUCH LONGER to load! 5-10 minutes maybe.

## Loading FSBL, U-Boot, and Linux into SPI flash

You'll need some kind of access onto the UUB - Ethernet, or maybe USB. I've always used Ethernet. You can initialize Ethernet via "udhcpc" to get a DHCP address. Then you need to create the BOOT.BIN image.

```
$ petalinux-package --boot --fsbl=images/zynq_fsbl.elf --u-boot
```

Now get that BOOT.BIN file onto the UUB somehow (wget, tftp, USB). There is an FTP server running on the UUB, too. That's the easiest way. (ftp, login, and put BOOT.BIN. BOOT.BIN will get put into /var/ftp). Next, do 

```
root@uub-linux:/var/ftp# flash_erase /dev/mtd1 0 0
Erasing 64 Kibyte @ 1f0000 -- 100 % complete
root@uub-linux:/var/ftp# flashcp BOOT.BIN /dev/mtd1
root@uub-linux:/var/ftp#
```

At this point the UUB should be able to boot U-Boot. It still cannot boot Linux yet.
