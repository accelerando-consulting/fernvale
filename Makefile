PARTITION_SCHEME ?= min_spiffs
BAUD ?= 460800
BOARD ?= espressif:esp32:esp32
LIBDIRS ?= $(PWD)/libraries,$(HOME)/Documents/Arduino/libraries
STACX_DIR = stacx

#PROGRAMMER = esptool
ARCHIVE=n
PROGRAM ?= $(shell basename $(PWD))
MAIN = $(PROGRAM).ino
SRCS = $(MAIN) \
	stacx/*.h \
	config.h \
	app_*.h

DISTHOST ?= iotlab
DISTDIR ?= iotlab/www/iotlab/firmware/fernvale

#SCREEN = T10
SCREEN = T14
#SCREEN = CYB_2432S028

ifeq ($(SCREEN),T10)
TFTPINS=-DTFT_SCLK=5 \
        -DTFT_MOSI=23 \
        -DTFT_CS=16 \
        -DTFT_DC=17 \
        -DTFT_RST=9

TFTFONTS=-DLOAD_GLCD=1 \
        -DLOAD_FONT2=1 \
        -DLOAD_FONT4=1 \
        -DLOAD_FONT6=1 \
        -DLOAD_FONT7=1 \
        -DLOAD_FONT8=1 \
        -DLOAD_GFXFF=1 \
        -DSMOOTH_FONT=1

CPPFLAGS=-DST7735_DRIVER=1 \
        -DTFT_WIDTH=128 \
        -DTFT_HEIGHT=128 \
        -DTFT_INVERSION_OFF=1 \
        $(TFTPINS) \
        $(TFTFONTS) \
        -DTFT_RGB_ORDER=TFT_BGR \
        -DSPI_FREQUENCY=40000000 \
        -DSPI_READ_FREQUENCY=6000000 \
        -DUSER_SETUP_LOADED=1 \
	-DSCREEN_$(SCREEN)=1
endif

ifeq ($(SCREEN),T14)
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
        -DLVGL_BUFFER_ROWS=4 \
        -DTFT_INVERSION_OFF=1 \
        $(TFTPINS) \
        $(TFTFONTS) \
        -DTFT_RGB_ORDER=TFT_BGR \
        -DSPI_FREQUENCY=40000000 \
        -DSPI_READ_FREQUENCY=6000000 \
        -DUSER_SETUP_LOADED=1 \
	-DSCREEN_$(SCREEN)=1 \
	-DLV_CONF_INCLUDE_SIMPLE=1 -I$(PWD)

#	-DLV_CONF_PATH=$(PWD)/libraries/lv_conf.h
#	-DLV_CONF_INCLUDE_SIMPLE=1 -I$(PWD)
endif

ifeq ($(SCREEN),CYB_2432S028)
TFTPINS=-DTFT_SCLK=14 \
        -DTFT_MOSI=13 \
        -DTFT_MISO=-1 \
        -DTFT_CS=15 \
        -DTFT_DC=2 \
        -DTFT_RST=12 \
        -DTFT_BL=21 \
        -DTFT_BACKLIGHT_ON=HIGH \
        -DTOUCH_CS=33 

TFTFONTS=-DLOAD_GLCD=1 \
        -DLOAD_FONT2=1 \
        -DLOAD_FONT4=1 \
        -DLOAD_FONT6=1 \
        -DLOAD_FONT7=1 \
        -DLOAD_FONT8=1 \
        -DLOAD_GFXFF=1 \
        -DSMOOTH_FONT=1

CPPFLAGS=-DILI9341_2_DRIVER=1 \
        -DTFT_WIDTH=240 \
        -DTFT_HEIGHT=320 \
        -DTFT_INVERSION_OFF=1 \
        $(TFTPINS) \
        $(TFTFONTS) \
        -DSPI_FREQUENCY=65000000 \
        -DSPI_READ_FREQUENCY=20000000 \
        -DSPI_TOUCH_FREQUENCY=2500000 \
        -DUSER_SETUP_LOADED=1 \
	-DSCREEN_$(SCREEN)=1 \
	-DLV_CONF_PATH=$(PWD)/libraries/lv_conf.h \
	-DLV_CONF_INCLUDE_SIMPLE=1 -I$(PWD)
endif


# LIBS are the libraries you can install through the arduino library manager
# Format is LIBNAME[@VERSION]
LIBS =  \
	"Adafruit INA219" \
	ArduinoJson@6.14.0 \
	Bounce2 \
	lvgl@8.3.11 \
	TFT_eSPI \
	Time \
	NtpClientLib \
	"SparkFun SCD4x Arduino Library"

# EXTRALIBS are the libraries that are not in the arduino library manager, but installed via git
# Format is LIBNAME@REPOURL
EXTRALIBS = \
	AsyncTCP@https://github.com/me-no-dev/AsyncTCP \
	ESPAsyncTCP@https://github.com/me-no-dev/ESPAsyncTCP \
	async-mqtt-client@https://github.com/marvinroger/async-mqtt-client \
	Shell@https://github.com/unixbigot/Shell \
	SimpleMap@https://github.com/spacehuhn/SimpleMap \
	WIFIMANAGER-ESP32@https://github.com/unixbigot/WIFIMANAGER-ESP32

include $(STACX_DIR)/cli.mk

