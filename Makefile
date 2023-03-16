PARTITION_SCHEME ?= min_spiffs
BOARD ?= espressif:esp32:esp32

PROGRAMMER = esptool

PROGRAM ?= $(shell basename $(PWD))
MAIN = $(PROGRAM).ino
SRCS = $(MAIN) \
	stacx/*.h \
	config.h \
	app_*.h

TFTPINS=-DTFT_SCLK=18 \
        -DTFT_MOSI=23 \
        -DTFT_MISO=19 \
        -DTFT_CS=5 \
        -DTFT_DC=16 \
        -DTFT_RST=17

TFTFONTS=-DLOAD_GLCD=1 \
        -DLOAD_FONT2=1 \
        -DLOAD_FONT4=1 \
        -DLOAD_FONT6=1 \
        -DLOAD_FONT7=1 \
        -DLOAD_FONT8=1 \
        -DLOAD_GFXFF=1 \
        -DSMOOTH_FONT=1

CPPFLAGS=-DST7789_DRIVER=1 \
        -DTFT_WIDTH=240 \
        -DTFT_HEIGHT=320 \
        -DTFT_INVERSION_OFF=1 \
        $(TFTPINS) \
        $(TFTFONTS) \
        -DTFT_RGB_ORDER=TFT_BGR \
        -DSPI_FREQUENCY=40000000 \
        -DSPI_READ_FREQUENCY=6000000 \
        -DUSER_SETUP_LOADED=1 

#	-DLV_CONF_PATH=$(PWD)/libraries/lv_conf.h


#	-DLV_CONF_INCLUDE_SIMPLE=1 -I$(PWD)

# LIBS are the libraries you can install through the arduino library manager
# Format is LIBNAME[@VERSION]
LIBS =  \
	ArduinoJson@6.14.0 \
	Bounce2 \
	TFT_eSPI \
	Time \
	NtpClientLib 

# EXTRALIBS are the libraries that are not in the arduino library manager, but installed via git
# Format is LIBNAME@REPOURL
EXTRALIBS = \
	AsyncTCP@https://github.com/me-no-dev/AsyncTCP \
	ESPAsyncTCP@https://github.com/me-no-dev/ESPAsyncTCP \
	async-mqtt-client@https://github.com/marvinroger/async-mqtt-client \
	Shell@https://github.com/unixbigot/Shell \
	SimpleMap@https://github.com/spacehuhn/SimpleMap \
	WIFIMANAGER-ESP32@https://github.com/unixbigot/WIFIMANAGER-ESP32

include stacx/cli.mk

