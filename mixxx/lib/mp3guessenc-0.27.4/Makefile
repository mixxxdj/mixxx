#
# Makefile
#
# If you wish, you can edit to suit your system.
#

# choose build type
BUILD = release
#BUILD = debug

# choose the target OS - `other' is good for both *nix, win32 systems and haiku
# 64 bit as well.
# NOTE: in haiku 32 bit with gcc2 some options need to be avoided since the compiler
# is quite old (gcc-2.95.3), with gcc4/5 the selected options won't raise any
# issue. If uncertain, select `haiku32_gcc2'
# UPDATE: the most recent nightly builds of haiku 32 bit adopted a different
# compiler naming scheme after suppressing the `setgcc' utility. Now gcc2 is
# called `gcc' and gcc4/5 is called `gcc-x86' and this Makefile is smart enough
# to set the right compiler name for you.
TARGET_OS = other
#TARGET_OS = haiku32_gcc2
#TARGET_OS = haiku32_gcc4

# here you can edit the installation path (only one will be used)
# On *nix systems you may want to have mp3guessenc installed under /usr/local
# In Haiku it is useful to put executables under /boot/home/config/non-packaged
PREFIX = /usr/local
#PREFIX = /boot/home/config/non-packaged

# common preferences
CC = gcc
#CC = tcc
#CC = clang
CFLAGS = -Wall
LDFLAGS =
LIBS =

# no need to change anything below here
#--------------------------------------

# is make running into windows?
ifdef SYSTEMROOT
  BIN_EXT = .exe
else
  BIN_EXT =
endif

ifeq ($(TARGET_OS),haiku32_gcc4)
  ifeq ($(CC),gcc)
    CC = gcc-x86
  endif
endif

ifneq ($(TARGET_OS),haiku32_gcc2)
  CFLAGS += -Wextra -fsingle-precision-constant
endif

ifeq ($(BUILD),release)
  CFLAGS += -O2
  LDFLAGS += -s
else
  CFLAGS += -g
  ifneq ($(TARGET_OS),haiku32_gcc2)
    CFLAGS += -fno-stack-protector
  endif
endif

SRC0 = mp3guessenc.c bit_utils.c tags.c decode.c
OBJ0 = mp3guessenc.o bit_utils.o tags.o decode.o

all: $(OBJ0) mp3guessenc
make: all

mp3guessenc: $(OBJ0)
	gcc $(LDFLAGS) -o mp3guessenc$(BIN_EXT) $(OBJ0) $(LIBS)

mp3guessenc.o: mp3guessenc.c mp3guessenc.h bit_utils.h tags.h decode.h scrambled.h mp3g_io_config.h
bit_utils.o: bit_utils.c bit_utils.h mp3g_io_config.h
tags.o: tags.c tags.h mp3guessenc.h mp3g_io_config.h
decode.o: decode.c

install:
	mkdir -p $(PREFIX)/bin
	cp -v -f mp3guessenc$(BIN_EXT) $(PREFIX)/bin/

clean:
	rm -f $(OBJ0) mp3guessenc$(BIN_EXT)

distclean: clean

uninstall:
	rm $(PREFIX)/bin/mp3guessenc$(BIN_EXT)

