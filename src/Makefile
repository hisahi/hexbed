
# main makefile

CC ?= gcc
CXX ?= g++
LD := $(CXX)
RM=rm -f

# release flags:
ifdef RELEASE
CFLAGS=-O3 -DNDEBUG
CXXFLAGS=-O3 -DNDEBUG
LDFLAGS=-O3 -flto
else
# debug flags:
CFLAGS=-g3 -O0 -Wall -Wextra -Werror -Wno-unused-parameter
CXXFLAGS=-g3 -O0 -Wall -Wextra -Werror -Wno-unused-parameter
LDFLAGS=
endif

# the rest
CFLAGS := $(CFLAGS) -MMD -MP
CXXFLAGS := -std=c++20 $(CXXFLAGS) -MMD -MP \
            `wx-config --cflags` \
            `pkg-config --cflags icu-uc` -DHAS_ICU=1
LDFLAGS := $(LDFLAGS) `wx-config --libs std,aui` \
           `pkg-config --libs icu-uc`
LDLIBS=-lm
OBJS=
SUBDIRS=common file app ui/dialogs \
		plugins plugins/export plugins/import plugins/inspector \
		ui/settings ui/tools ui

TARGET := ../hexbed
IROOT := .

CXXFLAGS := -I$(IROOT) $(CXXFLAGS)

include $(addsuffix /Makefile.inc, $(SUBDIRS))

DEPS := $(OBJS:.o=.d)

default: all

.PHONY: all clean
all: $(TARGET)
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS)

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)
