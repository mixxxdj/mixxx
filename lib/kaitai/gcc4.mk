# Makefile for ancient gcc4 (tested on Ubuntu 7.10 with 4.1.2-16ubuntu2)
# Compiles Kaitai Struct C++ runtime and tests

BUILD_DIR := build

SOURCES := \
	kaitai/kaitaistream.cpp \
	tests/unittest.cpp

OBJS := \
	$(BUILD_DIR)/kaitaistream.o \
	$(BUILD_DIR)/unittest.o

CXXFLAGS := -std=c++98 -Wall -Wextra -pedantic

# `-pedantic` enables `-Wlong-long`, which for some reason turns into
# `error: ISO C++ does not support 'long long'`, even though we don't use `-Werror`. Since
# we're using `long long` intentionally, we suppress this error using `-Wno-long-long`
# (see https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Warning-Options.html#index-Wlong_002dlong-305).
CXXFLAGS += -Wno-long-long

# NOTE: the meaning of `<n>` values in `-Wstrict-aliasing=<n>` is different in GCC 4.1.2
# than in recent versions of GCC. According to
# https://gcc.gnu.org/onlinedocs/gcc-4.1.2/gcc/Warning-Options.html#index-Wstrict_002daliasing-246,
# GCC 4.1.2 only has `-Wstrict-aliasing` and `-Wstrict-aliasing=2`, with
# `-Wstrict-aliasing=2` being the option that catches more cases but has more false
# positives. Here is an example of a case detected by `-Wstrict-aliasing=2` but not by
# `-Wstrict-aliasing`: https://godbolt.org/z/qTahqTYTf
#
# In GCC 13.3, however, the most aggressive option (with the most false positives) is
# `-Wstrict-aliasing=1`. That's why we use `-Wstrict-aliasing=2` here (because this
# Makefile is exclusively for GCC 4), but `-Wstrict-aliasing=1` in `Common.cmake` (which
# is used by modern compilers).
CXXFLAGS += -fstrict-aliasing -Wstrict-aliasing=2

DEFINES := -DKS_STR_ENCODING_ICONV -DKS_ZLIB -DGTEST_NANO

LDFLAGS :=
LDLIBS := -lz

INCLUDES := -I.

.PHONY: all
all: $(BUILD_DIR) $(BUILD_DIR)/unittest

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/unittest: $(OBJS)
	$(CXX) $(CXXFLAGS) -o build/unittest $(OBJS) $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/kaitaistream.o: kaitai/kaitaistream.cpp kaitai/kaitaistream.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@

$(BUILD_DIR)/unittest.o: tests/unittest.cpp tests/gtest-nano.h kaitai/kaitaistream.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(DEFINES) -c $< -o $@
