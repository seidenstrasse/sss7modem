AVRMCU ?= atmega2560
F_CPU ?= 16000000
ISPPORT ?= /dev/ttyUSB0

VERSION = 0.1

HEADERS = ardusss7.h ../sss7core/sss7.h
SRC_C = ../sss7core/sss7.c
SRC_CPP = main.cpp ardusss7.cpp
TARGET = sss7
OBJDIR = bin
ARDUINO_DIR = ./arduino-1.6.13
ARDUINO_BOARD = mega

CC = avr-gcc
CPP = avr-g++
AR = avr-ar
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

SRC_C_TMP = $(subst ../,,$(SRC_C))
OBJ = $(SRC_C_TMP:%.c=$(OBJDIR)/$(AVRMCU)/%.o)
OBJ += $(SRC_CPP:%.cpp=$(OBJDIR)/$(AVRMCU)/%.o)

ARDU_HEADERS = Arduino.h binary.h Client.h HardwareSerial.h IPAddress.h \
	new.h Printable.h Print.h Server.h \
	Stream.h Udp.h USBAPI.h USBCore.h USBDesc.h WCharacter.h \
	wiring_private.h WString.h

ARDU_C_SRC = WInterrupts.c wiring_digital.c wiring_analog.c wiring_pulse.c \
	wiring.c wiring_shift.c

ARDU_CPP_SRC = CDC.cpp new.cpp HardwareSerial0.cpp PluggableUSB.cpp \
	HardwareSerial1.cpp Print.cpp HardwareSerial2.cpp Stream.cpp \
	HardwareSerial3.cpp Tone.cpp HardwareSerial.cpp USBCore.cpp \
	IPAddress.cpp WMath.cpp main.cpp WString.cpp \

ARDU_FULL_DIR = $(ARDUINO_DIR)/hardware/arduino/avr/cores/arduino
ARDU_VARIANT_DIR = $(ARDUINO_DIR)/hardware/arduino/avr/variants/$(ARDUINO_BOARD)
ARDU_FULL_HEADERS = $(ARDU_HEADERS:%.h=$(ARDU_FULL_DIR)/%.h)
ARDU_FULL_HEADERS += $(ARDU_VARIANT_DIR)/pins_arduino.h
ARDU_OBJS = $(ARDU_C_SRC:%.c=$(OBJDIR)/$(AVRMCU)/arduino/%.o)
ARDU_OBJS += $(ARDU_CPP_SRC:%.cpp=$(OBJDIR)/$(AVRMCU)/arduino/%.o)

CFLAGS = -I $(ARDU_FULL_DIR) -I $(ARDU_VARIANT_DIR) -I ../sss7core/ -Os -Wall -Wstrict-prototypes
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fshort-enums -fpack-struct -funsigned-char -funsigned-bitfields
CFLAGS += -mmcu=$(AVRMCU) -DF_CPU=$(F_CPU)UL -DVERSION=$(VERSION)

CPPFLAGS = -I $(ARDU_FULL_DIR) -I $(ARDU_VARIANT_DIR) -I ../sss7core/ -Os
CPPFLAGS += -ffunction-sections -fdata-sections
CPPFLAGS += -fshort-enums -fpack-struct -funsigned-char -funsigned-bitfields
CPPFLAGS += -mmcu=$(AVRMCU) -DF_CPU=$(F_CPU)UL -DVERSION=$(VERSION)

LDFLAGS = -mmcu=$(AVRMCU) -Wl,--gc-sections

all: start $(OBJDIR)/$(AVRMCU)/$(TARGET).hex size
	@echo ":: Done !"

start:
	@echo "SSS7 AVR port $(VERSION)"
	@echo "========================"
	@echo ":: Building for $(AVRMCU)"
	@echo ":: MCU operating frequency is $(F_CPU)Hz"


$(OBJDIR)/$(AVRMCU)/%.o : %.c $(HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/$(AVRMCU)/%.o : %.cpp $(HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/$(AVRMCU)/sss7core/%.o : ../sss7core/%.c $(HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/$(AVRMCU)/arduino/%.o : $(ARDU_FULL_DIR)/%.c $(ARDU_FULL_HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/$(AVRMCU)/arduino/%.o : $(ARDU_FULL_DIR)/%.cpp $(ARDU_FULL_HEADERS) Makefile
	@mkdir -p $$(dirname $@)
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(OBJDIR)/$(AVRMCU)/arduino/libarduino.a: $(ARDU_OBJS)
	${AR} crs $@ $+

$(OBJDIR)/$(AVRMCU)/$(TARGET).elf : $(OBJ) $(OBJDIR)/$(AVRMCU)/arduino/libarduino.a
	$(CC) $(LDFLAGS) $+ -o $@

$(OBJDIR)/$(AVRMCU)/$(TARGET).hex : $(OBJDIR)/$(AVRMCU)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

size : $(OBJDIR)/$(AVRMCU)/$(TARGET).elf
	@echo
	@$(SIZE) --mcu=$(AVRMCU) -C $(OBJDIR)/$(AVRMCU)/$(TARGET).elf
	@echo

clean :
	@rm -rf $(OBJDIR)

flash : all
	avrdude -c stk500v2 \
		-p $(AVRMCU) -P $(ISPPORT) -D \
        -U flash:w:$(OBJDIR)/$(AVRMCU)/$(TARGET).hex

test : flash
	screen $(ISPPORT) 38400
