#!/bin/bash

ORIG=../archos-kernel/
NEW=./

if [[ "$1" == "c" ]]
then
DIFF="colordiff -Npur"
PAGER="less -R"
else
DIFF="diff -Npur"
PAGER="cat"
fi

{
	for file in \
	 arch/arm/mach-omap2/archos-audio.c                \
	 arch/arm/mach-omap2/archos-clock.c                \
	 arch/arm/mach-omap2/archos-lcd-boe-wsvga10.c      \
	 arch/arm/mach-omap2/archos-lcd-lg-fwvga43.c       \
	 arch/arm/mach-omap2/archos-touchscreen.c          \
	 arch/arm/mach-omap2/archos-usb-musb.c             \
	 arch/arm/mach-omap2/archos-usb-ohci.c             \
	 arch/arm/mach-omap2/archos-usb-touchscreen.c      \
	 arch/arm/mach-omap2/archos-wifi.c                 \
	 arch/arm/mach-omap2/board-archos-a10it.c          \
	 arch/arm/mach-omap2/board-archos-a43.c            \
	 drivers/video/omap2/displays/panel-boe-ht101wsb.c \
	 drivers/video/omap2/displays/panel-lg-4573.c      \
	 sound/soc/omap/archos.c                           
	do
		$DIFF $ORIG/$file $NEW/$file
	done


	for file in \
	 arch/arm/mach-omap2/include/mach/archos-audio.h   \
	 arch/arm/mach-omap2/include/mach/archos-gpio.h    \
	 arch/arm/mach-omap2/include/mach/board-archos.h   
	do
		$DIFF $ORIG/${file/mach-omap2/plat-omap} $NEW/$file
	done
} | $PAGER


